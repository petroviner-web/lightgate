-- Schema for SQLite (can be moved to PostgreSQL with minor changes).
-- The backend runs this file at startup; every statement is idempotent.

CREATE TABLE IF NOT EXISTS athletes (
    id          INTEGER PRIMARY KEY,
    full_name   TEXT    NOT NULL,
    club        TEXT
);

CREATE TABLE IF NOT EXISTS events (
    id          INTEGER PRIMARY KEY,
    name        TEXT    NOT NULL,
    distance_m  INTEGER NOT NULL CHECK (distance_m > 0),
    held_on     TEXT    NOT NULL DEFAULT (date('now'))
);

CREATE TABLE IF NOT EXISTS runs (
    id           INTEGER PRIMARY KEY,
    event_id     INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,
    athlete_id   INTEGER REFERENCES athletes (id) ON DELETE SET NULL,
    time_us      INTEGER NOT NULL CHECK (time_us > 0),
    recorded_at  TEXT    NOT NULL
);

-- Speeds up the leaderboard query (filter by event, sort by time).
CREATE INDEX IF NOT EXISTS ix_runs_event_time ON runs (event_id, time_us);

-- Default event so runs from the device have somewhere to go.
INSERT OR IGNORE INTO events (id, name, distance_m) VALUES (1, 'Trening - 60 m', 60);
