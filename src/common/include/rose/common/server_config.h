#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include "rose/common/common_interface.h"
#include "rose/common/game_config.h"
#include "rose/common/util.h"

#include <string>

namespace Rose {
namespace Common {
struct DatabaseConfig {
    std::string connection_string;
    std::string ip;
    uint32_t port;
    std::string name;
    std::string username;
    std::string password;

    DatabaseConfig():
        connection_string("postgres://postgres:postgres@localhost/rose-next"),
        ip("127.0.0.1"),
        port(1433),
        name("SHO"),
        username("seven"),
        password("tpqmsgkcm") {}
};

struct LoginServerConfig {
    std::string ip;
    uint32_t port;
    uint32_t server_port;
    std::string password;
    LogLevel log_level;
    std::string log_path;

    LoginServerConfig():
        ip("127.0.0.1"),
        port(29000),
        server_port(19000),
        password("rose-next"),
        log_level(LogLevel::Info),
        log_path(get_exe_dir().append("/log/loginserver.log")) {}
};

struct WorldServerConfig {
    static const int ENGLISH = 1;

    int language;
    std::string ip;
    uint32_t port;
    uint32_t server_port;
    std::string world_name;
    std::string data_dir;
    LogLevel log_level;
    std::string log_path;

    WorldServerConfig():
        language(ENGLISH),
        ip("127.0.0.1"),
        port(29100),
        server_port(19001),
        world_name("1Rose Next"),
        data_dir(get_exe_dir().append("data/")),
        log_level(Rose::Common::LogLevel::Info),
        log_path(get_exe_dir().append("/log/worldserver.log")) {}
};

struct GameServerConfig {
    static const int ENGLISH = 1;

    int language;
    std::string ip;
    uint32_t port;
    std::string server_name;
    std::string data_dir;
    LogLevel log_level;
    std::string log_path;

    GameServerConfig():
        language(ENGLISH),
        ip("127.0.0.1"),
        port(29200),
        server_name("Channel 1"),
        data_dir(get_exe_dir().append("data/")),
        log_level(Rose::Common::LogLevel::Info),
        log_path(get_exe_dir().append("/log/gameserver.log")) {}
};

class ServerConfig {
public:
    DatabaseConfig database;
    LoginServerConfig loginserver;
    WorldServerConfig worldserver;
    GameServerConfig gameserver;
    GameConfig game;

public:
    ServerConfig(): toml(nullptr) {}

    ~ServerConfig() { toml_free(this->toml); }

    bool load(const std::string& path, const std::string& prefix) {
        this->toml = toml_load(path.c_str());

        if (!this->toml) {
            // TODO: Save a default toml
            return false;
        }

        this->get_str("database", "connection_string", this->database.connection_string);
        this->get_str("database", "ip", this->database.ip);
        this->get_uint32("database", "port", this->database.port);
        this->get_str("database", "name", this->database.name);
        this->get_str("database", "username", this->database.username);
        this->get_str("database", "password", this->database.password);

        this->get_str("loginserver", "ip", this->loginserver.ip);
        this->get_uint32("loginserver", "port", this->loginserver.port);
        this->get_uint32("loginserver", "server_port", this->loginserver.server_port);
        this->get_str("loginserver", "password", this->loginserver.password);
        this->get_loglevel("loginserver", "log_level", this->loginserver.log_level);
        this->get_str("loginserver", "log_path", this->loginserver.log_path);

        this->get_str("worldserver", "ip", this->worldserver.ip);
        this->get_uint32("worldserver", "port", this->worldserver.port);
        this->get_uint32("worldserver", "server_port", this->worldserver.server_port);
        this->get_str("worldserver", "world_name", this->worldserver.world_name);
        this->get_str("worldserver", "data_dir", this->worldserver.data_dir);
        this->get_loglevel("worldserver", "log_level", this->worldserver.log_level);
        this->get_str("worldserver", "log_path", this->worldserver.log_path);

        this->get_str("gameserver", "ip", this->gameserver.ip);
        this->get_uint32("gameserver", "port", this->gameserver.port);
        this->get_str("gameserver", "server_name", this->gameserver.server_name);
        this->get_str("gameserver", "data_dir", this->gameserver.data_dir);
        this->get_loglevel("gameserver", "log_level", this->gameserver.log_level);
        this->get_str("gameserver", "log_path", this->gameserver.log_path);

        this->get_uint32("game", "base_move_speed", this->game.base_move_speed);
        this->get_uint32("game", "base_attack_damage", this->game.base_attack_damage);
        this->get_uint32("game", "base_attack_speed", this->game.base_attack_speed);
        this->get_uint32("game", "base_hit", this->game.base_hit_rate);
        this->get_uint32("game", "base_crit", this->game.base_crit);

        return true;
    }

private:
    Toml* toml;

private:
    bool get_int32(const std::string& table, const std::string& key, int32_t& val) {
        int64_t v = 0;
        const bool res = toml_get_int(this->toml, table.c_str(), key.c_str(), &v);
        val = static_cast<int32_t>(v);
        return res;
    }

    bool get_int64(const std::string& table, const std::string& key, int64_t& val) {
        return toml_get_int(this->toml, table.c_str(), key.c_str(), &val);
    }

    bool get_uint32(const std::string& table, const std::string& key, uint32_t& val) {
        int64_t v = 0;
        const bool res = toml_get_int(this->toml, table.c_str(), key.c_str(), &v);
        val = static_cast<uint32_t>(v);
        return res;
    }

    bool get_uint64(const std::string& table, const std::string& key, uint64_t& val) {
        int64_t v = 0;
        const bool res = toml_get_int(this->toml, table.c_str(), key.c_str(), &v);
        val = static_cast<uint64_t>(v);
        return res;
    }

    bool get_str(const std::string& table, const std::string& key, std::string& val) {
        FfiString* s = ffi_string_new();
        bool res = toml_get_str(this->toml, table.c_str(), key.c_str(), s);
        if (!res) {
            return false;
        }
        val = std::string(s->chars, s->len);
        ffi_string_free(s);
        return true;
    }

    bool get_loglevel(const std::string& table, const std::string& key, LogLevel& level) {
        int64_t val = 0;
        bool res = get_int64(table, key, val);
        level = static_cast<LogLevel>(val);
        return true;
    }
};
} // namespace Common
} // namespace Rose

#endif // CONFIG_H
