DROP INDEX IF EXISTS name_index;
DROP INDEX IF EXISTS meta_index;

DROP TABLE IF EXISTS names;
DROP TABLE IF EXISTS data;
DROP TABLE IF EXISTS meta;

CREATE TABLE names (
	nameID INTEGER PRIMARY KEY,
	name    TEXT
);

CREATE TABLE data (
	dataID INTEGER PRIMARY KEY,
	data    TEXT,
	compressed INTEGER DEFAULT 0
);

CREATE TABLE meta (
	nameID  INTEGER,
	dataID  INTEGER,
	firstExp INTEGER DEFAULT 0, 
	lastExp  INTEGER DEFAULT -1,
	firstRun INTEGER DEFAULT 0,
	lastRun  INTEGER DEFAULT -1
);

CREATE UNIQUE INDEX name_index ON names (name);
CREATE INDEX meta_index ON meta (nameID,firstExp,lastExp,firstRun,lastRun);

INSERT INTO names (name) VALUES ("Belle2.xml");
INSERT INTO names (name) VALUES ("include.xml");
INSERT INTO data  (data) VALUES ('<?xml version="1.0" encoding="UTF-8"?>
<detector xmlns:xi="http://www.w3.org/2001/XInclude">
	<foo unit="mm">125</foo>
	<xi:include href="include.xml"/>
	<xi:include href="include.xml"/>
</detector>');
INSERT INTO data  (data) VALUES ('<?xml version="1.0" encoding="UTF-8"?><bar unit="foo">0.5</bar>');
INSERT INTO meta (nameID,dataID) VALUES (1,1);
INSERT INTO meta (nameID,dataID) VALUES (2,2);


