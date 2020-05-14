BEGIN;
CREATE TABLE account
(
    id integer PRIMARY KEY GENERATED BY DEFAULT AS IDENTITY,
    username varchar(20) NOT NULL UNIQUE,
    password char(32) NOT NULL,
    access_level INTEGER NOT NULL DEFAULT 0,
    email varchar(30) NOT NULL UNIQUE,
    created TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE character
(
    id integer PRIMARY KEY GENERATED BY DEFAULT AS IDENTITY,
    account_id integer NOT NULL REFERENCES account (id) ON DELETE CASCADE,
    name varchar(30) NOT NULL UNIQUE,
    level smallint NOT NULL DEFAULT 1,
    gender smallint NOT NULL DEFAULT 0,
    job smallint NOT NULL DEFAULT 0,
    money bigint NOT NULL DEFAULT 0,
    storage_money bigint NOT NULL DEFAULT 0,
    is_premium boolean NOT NULL DEFAULT FALSE,
    delete_time integer NOT NULL DEFAULT 0,
    birthstone integer NOT NULL DEFAULT 0,
    face integer NOT NULL DEFAULT 1,
    hair integer NOT NULL DEFAULT 0,
    class_id integer NOT NULL DEFAULT 0,
    union_id INTEGER NOT NULL DEFAULT 0,
    rank INTEGER NOT NULL DEFAULT 0,
    fame INTEGER NOT NULL DEFAULT 0,
    CONSTRAINT character_level_positive CHECK (level > 0),
    CONSTRAINT character_money_positive CHECK (money >= 0),
    CONSTRAINT character_storage_money_positive CHECK (storage_money >= 0)
);

CREATE TABLE item
(
    id integer PRIMARY KEY GENERATED BY DEFAULT AS IDENTITY,
    game_data_id integer NOT NULL DEFAULT 0,
    grade integer NOT NULL DEFAULT 0,
    CONSTRAINT grade_positive CHECK (grade >= 0)
);

CREATE TABLE equipment
(
    id integer PRIMARY KEY GENERATED BY DEFAULT AS IDENTITY,
    character_id integer NOT NULL DEFAULT 0 REFERENCES character (id) ON DELETE CASCADE,
    item_id integer NOT NULL DEFAULT 0 REFERENCES item (id) ON DELETE SET DEFAULT,
    slot varchar(5) NOT NULL,
    UNIQUE (character_id, item_id, slot),
    CONSTRAINT equipment_slot_valid CHECK (slot in ('head', 'chest', 'arms', 'feet', 'face', 'back', 'lhand', 'rhand'))
);
COMMIT;