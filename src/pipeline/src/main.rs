use std::collections::HashMap;
use std::fmt;
use std::fs;
use std::fs::File;
use std::io;
use std::path::{Path, PathBuf};
use std::process;
use std::time;

use roselib::files::STB;
use roselib::io::RoseFile;

use bincode;
use clap::{App, Arg, ArgMatches, SubCommand};
use globset;
use globset::{Glob, GlobSetBuilder};
use serde::{Deserialize, Serialize};
use walkdir::WalkDir;

const BAKE_INPUT_DIR: &str = "INPUT_DIR";
const BAKE_OUTPUT_DIR: &str = "OUTOUT_DIR";
const BAKE_CONFIG_NAME: &str = "config_name";

#[derive(Debug)]
enum PipelineError {
    Message(String),
}

impl fmt::Display for PipelineError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PipelineError::Message(s) => write!(f, "{}", s),
        }
    }
}

impl From<io::Error> for PipelineError {
    fn from(e: io::Error) -> PipelineError {
        PipelineError::Message(format!("IO Error: {}", e))
    }
}

impl From<globset::Error> for PipelineError {
    fn from(e: globset::Error) -> PipelineError {
        PipelineError::Message(format!("Glob Error: {}", e))
    }
}

impl From<csv::Error> for PipelineError {
    fn from(e: csv::Error) -> PipelineError {
        PipelineError::Message(format!("CSV Error: {}", e))
    }
}

impl From<roselib::Error> for PipelineError {
    fn from(e: roselib::Error) -> PipelineError {
        PipelineError::Message(format!("RoseLib Error: {}", e))
    }
}

impl From<std::str::Utf8Error> for PipelineError {
    fn from(e: std::str::Utf8Error) -> PipelineError {
        PipelineError::Message(format!("String Utf8 Error: {}", e))
    }
}

#[derive(Serialize, Deserialize, Clone, Copy, Debug, PartialEq)]
struct BakeFileMetadata {
    size: u64,
    modified: time::SystemTime,
}

impl Default for BakeFileMetadata {
    fn default() -> BakeFileMetadata {
        BakeFileMetadata {
            size: 0,
            modified: time::SystemTime::now(),
        }
    }
}

impl From<&fs::Metadata> for BakeFileMetadata {
    fn from(m: &fs::Metadata) -> BakeFileMetadata {
        BakeFileMetadata {
            size: m.len(),
            modified: m.modified().unwrap_or(time::SystemTime::now()),
        }
    }
}

type BakeCache = HashMap<PathBuf, BakeFileMetadata>;

/// Read the bake file metadata cache from disk
fn get_bake_cache(cache_file_path: &Path) -> Result<BakeCache, PipelineError> {
    if !cache_file_path.exists() {
        return Ok(HashMap::new());
    }

    let cache_file = File::open(cache_file_path)?;
    match bincode::deserialize_from(cache_file) {
        Ok(cache) => Ok(cache),
        Err(_) => Ok(HashMap::new()),
    }
}

/// Save the bake file metadata cache to disk
fn save_bake_cache(cache_file_path: &Path, cache: &BakeCache) -> Result<(), PipelineError> {
    if !cache_file_path.exists() {
        fs::create_dir_all(cache_file_path.parent().unwrap())?;
    }

    let cache_file = File::create(cache_file_path)?;
    if let Err(e) = bincode::serialize_into(cache_file, cache) {
        return Err(PipelineError::Message(format!(
            "Failed to save cache file: {}",
            e
        )));
    }

    Ok(())
}

/// Checks if the path needs to be re-baked by comparing current file data
/// to the value stored in the cache. Updates the cache with the new metadata.
///
/// Returns true if:
/// - `path` does not exist
/// - `path` is not in the cache
/// - `metadata` does not match the metadata in `cache`
fn should_rebake(path: &Path, metadata: &BakeFileMetadata, cache: &mut BakeCache) -> bool {
    if !path.exists() {
        return false;
    }

    let metadata = *metadata;
    if let Some(cached_metadata) = cache.insert(path.to_path_buf(), metadata) {
        cached_metadata != metadata
    } else {
        true
    }
}

fn bake(matches: &ArgMatches) -> Result<(), PipelineError> {
    let config_name = matches.value_of(BAKE_CONFIG_NAME).unwrap();

    let input_dir = PathBuf::from(matches.value_of(BAKE_INPUT_DIR).unwrap());
    if !input_dir.exists() {
        return Err(PipelineError::Message(format!(
            "Invalid input dir: {}",
            input_dir.to_str().unwrap()
        )));
    }

    let config_path = input_dir.join(config_name);
    if !config_path.exists() {
        return Err(PipelineError::Message(format!(
            "Config file does not exists: {}",
            config_path.to_str().unwrap()
        )));
    }

    if !config_path.is_file() {
        return Err(PipelineError::Message(format!(
            "Config argument is not a file: {}",
            config_path.to_str().unwrap()
        )));
    }

    let pipeline_dir = input_dir.join(".pipeline");
    let cache_file_path = pipeline_dir.join("bake");

    let mut cache = match get_bake_cache(&cache_file_path) {
        Ok(c) => c,
        Err(e) => {
            return Err(PipelineError::Message(format!(
                "Bake cache file error ({}): {:?}",
                &pipeline_dir.to_str().unwrap(),
                e
            )))
        }
    };

    let raw_config = fs::read_to_string(&config_path)?;
    let lines = raw_config
        .split('\n')
        .map(|line| line.trim().replace("\r", ""))
        .filter(|line| !line.starts_with('#') && !line.is_empty());

    let output_dir = PathBuf::from(matches.value_of(BAKE_OUTPUT_DIR).unwrap());
    if !output_dir.exists() {
        fs::create_dir_all(&output_dir)?;
    }

    let walk_dir = match fs::metadata(&input_dir) {
        Ok(m) => should_rebake(&input_dir, &BakeFileMetadata::from(&m), &mut cache),
        Err(_) => true,
    };

    let mut file_list = Vec::new();
    let mut file_metadata: HashMap<PathBuf, BakeFileMetadata> = HashMap::new();

    if walk_dir {
        for entry in WalkDir::new(&input_dir).into_iter() {
            if let Ok(ent) = entry {
                let entry_path = ent.into_path();
                if !entry_path.is_file() {
                    continue;
                }

                match fs::metadata(&entry_path) {
                    Ok(m) => {
                        let relative_path =
                            entry_path.strip_prefix(&input_dir).unwrap().to_path_buf();
                        file_metadata.insert(relative_path.clone(), BakeFileMetadata::from(&m));
                        file_list.push(relative_path.clone());
                    }
                    Err(e) => {
                        println!(
                            "Error reading metadata for file {}: {}",
                            entry_path.display(),
                            e
                        );
                        continue;
                    }
                }
            }
        }
    } else {
        for p in cache.keys() {
            file_list.push(p.strip_prefix(&input_dir).unwrap().to_path_buf());
        }
    }

    let mut glob_builder = GlobSetBuilder::new();
    let mut commands: Vec<Vec<String>> = Vec::new();

    for (line_number, line) in lines.enumerate() {
        let line_number = line_number + 1;

        let args: Vec<String> = line.split_whitespace().map(|s| s.to_string()).collect();
        if args.len() < 2 {
            continue;
        }

        let file_glob = &args[1];
        match Glob::new(&file_glob) {
            Ok(g) => {
                glob_builder.add(g);
                commands.push(args);
            }
            Err(e) => {
                println!("Skipping line {}: Error: {}", line_number, e);
            }
        }
    }

    let globs = glob_builder.build()?;

    'file_loop: for filepath in file_list {
        let input_filepath = input_dir.join(&filepath);

        if !input_filepath.exists() {
            let _ = cache.remove(&input_filepath);
            continue;
        }

        let rebake = match fs::metadata(&input_filepath) {
            Ok(m) => should_rebake(&input_filepath, &BakeFileMetadata::from(&m), &mut cache),
            Err(e) => {
                println!(
                    "Error reading file metadata {}: {}",
                    input_filepath.display(),
                    e
                );
                false
            }
        };

        let command_indices = globs.matches(&filepath);
        for command_index in command_indices.iter() {
            let command_index = *command_index;
            let args = &commands[command_index];

            let command = args[0].to_lowercase();
            let output_filepath = match command.as_str() {
                "copy" => output_dir.join(&filepath),
                "stb" => output_dir.join(&filepath).with_extension("stb"),
                "dds" => output_dir.join(&filepath).with_extension("dds"),
                _ => PathBuf::new(),
            };

            if !rebake && output_filepath.exists() {
                continue 'file_loop;
            }

            let output_filedir = output_filepath.parent().unwrap();
            if let Err(e) = fs::create_dir_all(&output_filedir) {
                eprintln!(
                    "Error creating output dir {}: {}",
                    output_filedir.display(),
                    e
                );
                continue;
            }

            match command.as_str() {
                "copy" => {
                    println!("Copying file {}", input_filepath.display());
                    if let Err(e) = fs::copy(&input_filepath, &output_filepath) {
                        eprintln!("Error copy file {}: {}", filepath.display(), e);
                    }
                }
                "stb" => {
                    let convert = || -> Result<(), PipelineError> {
                        let mut stb = STB::new();
                        let mut reader = csv::Reader::from_path(&input_filepath)?;
                        for header in reader.headers()? {
                            stb.headers.push(header.to_string())
                        }
                        for record in reader.records() {
                            let mut row = Vec::new();
                            for field in record?.iter() {
                                row.push(field.to_string());
                            }
                            stb.data.push(row);
                        }
                        stb.write_to_path(&output_filepath)?;
                        Ok(())
                    };

                    println!("Converting to STB {}", input_filepath.display());
                    if let Err(e) = convert() {
                        eprintln!("Error converting stb {}: {}", &input_filepath.display(), e);
                    }
                }
                "dds" => {
                    let convert = || -> Result<(), PipelineError> {
                        let out_dir = match &output_filepath
                            .parent()
                            .unwrap_or(output_dir.as_path())
                            .to_str()
                        {
                            Some(d) => d.to_owned(),
                            None => {
                                return Err(PipelineError::Message(
                                    "Failed to get output dir as string".to_string(),
                                ))
                            }
                        };

                        let in_file = match &input_filepath.to_str() {
                            Some(d) => d.to_owned(),
                            None => {
                                return Err(PipelineError::Message(
                                    "Failed to get input file as string".to_string(),
                                ))
                            }
                        };

                        println!("Converting to DDS {}", input_filepath.display());
                        let res = process::Command::new("texconv.exe")
                            .args(&["-y", "-nologo", "-f", "DXT5", "-o", &out_dir, &in_file])
                            .output()?;

                        if !res.status.success() {
                            let mut error_string = String::from("texconv_failed");
                            for stdres in &[&res.stdout, &res.stderr] {
                                if !stdres.is_empty() {
                                    error_string.push('\n');
                                    error_string.push_str(&std::str::from_utf8(stdres)?);
                                }
                            }
                            return Err(PipelineError::Message(error_string));
                        }

                        // Rename to lowercase DDS extension because Texconv defaults to upper
                        fs::rename(
                            output_filepath.with_extension("DDS"),
                            output_filepath.with_extension("dds"),
                        )?;

                        Ok(())
                    };

                    if let Err(e) = convert() {
                        eprintln!(
                            "Error converting to dds {}: {}",
                            &input_filepath.display(),
                            e
                        );
                    }
                }
                _ => {} // Unrecognized command
            }
        }
    }

    let _ = save_bake_cache(cache_file_path.as_path(), &cache);

    Ok(())
}

fn main() -> Result<(), PipelineError> {
    let app = App::new("Rose Next Pipeline Tool").subcommand(
        SubCommand::with_name("bake")
            .about("Rose Next Asset Compiler")
            .arg(Arg::with_name(BAKE_INPUT_DIR).required(true))
            .arg(Arg::with_name(BAKE_OUTPUT_DIR).required(true))
            .arg(
                Arg::with_name(BAKE_CONFIG_NAME)
                    .short("c")
                    .default_value("bake.manifest")
                    .help("Name of the manifest file in the input directory")
                    .required(true)
                    .takes_value(true),
            ),
    );
    let mut help = Vec::new();
    app.write_help(&mut help).unwrap();

    let matches = &app.get_matches();

    match matches.subcommand() {
        ("bake", Some(sub_matches)) => bake(sub_matches),
        _ => {
            println!("{}", String::from_utf8(help).unwrap());
            Ok(())
        }
    }
}
