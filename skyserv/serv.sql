-- CREATE DATABASE karia2_resource;
DROP TABLE IF EXISTS meta_links;
CREATE TABLE meta_links (
       uid BIGSERIAL,
       url VARCHAR(3000) NOT NULL,
       file_name VARCHAR(128) NOT NULL,
       file_name_md5  VARCHAR(32) NOT NULL,
       file_content_md5 VARCHAR(32),
       file_content_digest_md5 VARCHAR(32),
       ed2k_hash VARCHAR(32),
       use_level INTEGER NOT NULL,
       valid_flag INTEGER NOT NULL,
       supply_user_name VARCHAR(60),
       ctime TIMESTAMP NOT NULL,
       mtime TIMESTAMP,
       gid BIGINT,
       PRIMARY  KEY (uid),
       UNIQUE (url)
);

DROP TABLE IF EXISTS group_links;
CREATE TABLE group_links (
       gid BIGSERIAL,
       link_count INTEGER NOT NULL,
       PRIMARY KEY (gid)
);

DROP TABLE IF EXISTS mirror_hosts;
CREATE TABLE mirror_hosts (
       hid SERIAL,
       host_name VARCHAR(128) NOT NULL,
       mid INTEGER,
       use_level INTEGER NOT NULL,
       valid_flag INTEGER NOT NULL,
       PRIMARY KEY (hid),
       UNIQUE (host_name)
);
