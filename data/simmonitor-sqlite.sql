
-- SET SCHEMA 'simmonitor';

-- -----------------------------------------------------
-- Schema simmonitor
-- -----------------------------------------------------

-- -----------------------------------------------------
-- Table `track_config`
-- -----------------------------------------------------
DROP TABLE IF EXISTS track_config ;

CREATE TABLE IF NOT EXISTS track_config (
  track_config_id INTEGER PRIMARY KEY,
  track_name VARCHAR(150) NULL,
  config_name VARCHAR(150) NULL,
  display_name VARCHAR(150) NULL,
  country VARCHAR(100) NULL,
  city VARCHAR(100) NULL,
  length INT NULL);

-- -----------------------------------------------------
-- Table `event`
-- -----------------------------------------------------
DROP TABLE IF EXISTS events;

CREATE TABLE IF NOT EXISTS events (
  event_id INTEGER PRIMARY KEY,
  server_name VARCHAR(250) NULL,
  track_config_id INT NOT NULL,
  event_name VARCHAR(250) NULL,
  team_event SMALLINT NULL DEFAULT 0,
  active SMALLINT NULL DEFAULT 0,
  livery_preview SMALLINT NULL DEFAULT 0,
  use_number SMALLINT NULL DEFAULT 0,
  practice_duration INT NOT NULL DEFAULT -1,
  quali_duration INT NOT NULL DEFAULT -1,
  race_duration INT NOT NULL DEFAULT -1,
  race_duration_type SMALLINT NOT NULL DEFAULT 0,
  race_wait_time INT NOT NULL DEFAULT 0,
  race_extra_laps INT NOT NULL DEFAULT 0,
  reverse_grid_positions INT NOT NULL DEFAULT 0);

-- -----------------------------------------------------
-- Table `session`
-- -----------------------------------------------------
DROP TABLE IF EXISTS sessions;

CREATE TABLE IF NOT EXISTS sessions (
  session_id INTEGER PRIMARY KEY,
  event_id INT NOT NULL,
  sim_id INT NOT NULL,
  driver_id INT NOT NULL,
  car_id INT NOT NULL,
  session_type SMALLINT NULL,
  event_type SMALLINT NOT NULL DEFAULT 0,
  track_time VARCHAR(5) NULL,
  session_name VARCHAR(50) NOT NULL DEFAULT 'Session',
  start_time TIMESTAMP(6) DEFAULT CURRENT_TIMESTAMP,
  duration_min INT NOT NULL,
  elapsed_ms INT NOT NULL DEFAULT 0,
  laps SMALLINT NOT NULL DEFAULT 0,
  weather VARCHAR(100) NOT NULL,
  air_temp FLOAT NULL DEFAULT NULL,
  road_temp FLOAT NULL,
  start_grip FLOAT NULL,
  end_grip FLOAT NULL,
  is_finished SMALLINT NULL,
  finished_at TIMESTAMP(6) NULL DEFAULT NULL,
  http_port INT NOT NULL DEFAULT 0);

-- -----------------------------------------------------
-- Table `sims`
-- -----------------------------------------------------
DROP TABLE IF EXISTS sims;

CREATE TABLE IF NOT EXISTS sims (
  sim_id INTEGER PRIMARY KEY,
  sim_name VARCHAR(150) NOT NULL DEFAULT 'Test');

-- -----------------------------------------------------
-- Table `schema_info`
-- -----------------------------------------------------
DROP TABLE IF EXISTS schemainfo;

CREATE TABLE IF NOT EXISTS schemainfo (
  version VARCHAR(10) NOT NULL,
  game VARCHAR(10) NOT NULL);

-- -----------------------------------------------------
-- Table `cars`
-- -----------------------------------------------------
DROP TABLE IF EXISTS cars;

CREATE TABLE IF NOT EXISTS cars (
  car_id INTEGER PRIMARY KEY,
  display_name VARCHAR(150) NULL DEFAULT NULL,
  car_name VARCHAR(150) NOT NULL,
  manufacturer VARCHAR(150) NOT NULL,
  car_class VARCHAR(30) NOT NULL DEFAULT 'N/A');

-- -----------------------------------------------------
-- Table `team`
-- -----------------------------------------------------
DROP TABLE IF EXISTS teams;

CREATE TABLE IF NOT EXISTS teams (
  team_id INTEGER PRIMARY KEY,
  event_id INT NOT NULL,
  team_name VARCHAR(100) NULL,
  team_no SMALLINT NULL,
  car_id INT NOT NULL,
  livery_name VARCHAR(150) NULL,
  active SMALLINT NOT NULL DEFAULT 1,
  created_at TIMESTAMP NOT NULL);

-- -----------------------------------------------------
-- Table `drivers`
-- -----------------------------------------------------
DROP TABLE IF EXISTS drivers;

CREATE TABLE IF NOT EXISTS drivers (
  driver_id INTEGER PRIMARY KEY,
  driver_name VARCHAR(100) NOT NULL,
  prev_name VARCHAR(100) NULL DEFAULT NULL,
  steam64_id BIGINT NOT NULL,
  country VARCHAR(20) NOT NULL DEFAULT 'N/A');


-- -----------------------------------------------------
-- Table `team_member`
-- -----------------------------------------------------
DROP TABLE IF EXISTS team_members;

CREATE TABLE IF NOT EXISTS team_members (
  team_member_id INTEGER PRIMARY KEY,
  team_id INT NOT NULL,
  driver_id INT NOT NULL,
  role VARCHAR(20) NULL,
  active SMALLINT NOT NULL DEFAULT 1,
  created_at TIMESTAMP NOT NULL);

-- -----------------------------------------------------
-- Table `laps`
-- -----------------------------------------------------
DROP TABLE IF EXISTS laps;

CREATE TABLE IF NOT EXISTS laps (
  lap_id INTEGER PRIMARY KEY,
  stint_id INT NOT NULL,
  sector_1 INT NULL,
  sector_2 INT NULL,
  sector_3 INT NULL,
  grip FLOAT NULL DEFAULT -1,
  tyre VARCHAR(50) NOT NULL,
  time INT NULL,
  cuts SMALLINT NULL,
  crashes SMALLINT NULL,
  max_speed SMALLINT NULL,
  avg_speed SMALLINT NULL,
  right_front_tyre_temp FLOAT NULL DEFAULT -1,
  right_rear_tyre_temp FLOAT NULL DEFAULT -1,
  right_front_tyre_wear FLOAT NULL DEFAULT -1,
  right_rear_tyre_wear FLOAT NULL DEFAULT -1,
  right_front_tyre_press FLOAT NULL DEFAULT -1,
  right_rear_tyre_press FLOAT NULL DEFAULT -1,
  right_front_brake_temp FLOAT NULL DEFAULT -1,
  right_rear_brake_temp FLOAT NULL DEFAULT -1,
  left_front_tyre_temp FLOAT NULL DEFAULT -1,
  left_rear_tyre_temp FLOAT NULL DEFAULT -1,
  left_front_tyre_wear FLOAT NULL DEFAULT -1,
  left_rear_tyre_wear FLOAT NULL DEFAULT -1,
  left_front_tyre_press FLOAT NULL DEFAULT -1,
  left_rear_tyre_press FLOAT NULL DEFAULT -1,
  left_front_brake_temp FLOAT NULL DEFAULT -1,
  left_rear_brake_temp FLOAT NULL DEFAULT -1,
  finished_at TIMESTAMP(6) NULL DEFAULT NULL);


-- -----------------------------------------------------
-- Table `stints`
-- -----------------------------------------------------
DROP TABLE IF EXISTS stints ;

CREATE TABLE IF NOT EXISTS stints (
  stint_id INTEGER PRIMARY KEY,
  driver_id INT NULL,
  team_member_id INT NULL DEFAULT NULL,
  session_id INT NULL,
  car_id INT NULL,
  game_car_id INT NOT NULL,
  laps INT NULL,
  valid_laps INT NULL,
  best_lap_id INT NULL DEFAULT NULL,
  is_finished SMALLINT NOT NULL DEFAULT 0,
  tyre VARCHAR(50) NOT NULL,
  started_at TIMESTAMP(6) DEFAULT CURRENT_TIMESTAMP,
  finished_at TIMESTAMP(6) NULL DEFAULT NULL);

-- -----------------------------------------------------
-- Table `session_feed`
-- -----------------------------------------------------
DROP TABLE IF EXISTS session_feed ;

CREATE TABLE IF NOT EXISTS session_feed (
  session_feed_id INTEGER PRIMARY KEY,
  session_id INT NOT NULL,
  type INT NOT NULL,
  detail VARCHAR(200) NULL,
  time TIMESTAMP(6) NOT NULL);

-- -----------------------------------------------------
-- Table `telemetry`
-- -----------------------------------------------------
DROP TABLE IF EXISTS telemetry ;

CREATE TABLE IF NOT EXISTS telemetry (
  telemetry_id INTEGER PRIMARY KEY,
  lap_id INT NOT NULL,
  points INT NULL,
  brake BLOB NULL,
  accel BLOB NULL,
  speed BLOB NULL,
  steer BLOB NULL,
  turbo BLOB NULL,
  gear BLOB NULL,
  rpms BLOB NULL,
  kers BLOB NULL,
  wind BLOB NULL);

-- -----------------------------------------------------
-- Insert default values
-- -----------------------------------------------------
-- schema info
INSERT INTO schemainfo VALUES ('1.0', 'simmonitor');
-- sims
INSERT INTO sims VALUES (1, 'Assetto Corsa');
INSERT INTO sims VALUES (2, 'RFactor2');
INSERT INTO sims VALUES (3, 'Automobilista2');
INSERT INTO sims VALUES (4, 'EuroTrucks2');
INSERT INTO sims VALUES (5, 'AmericanTrucks2');
INSERT INTO sims VALUES (6, 'Assetto Corsa Competizione');
INSERT INTO sims VALUES (7, 'Assetto Corsa Evo');
