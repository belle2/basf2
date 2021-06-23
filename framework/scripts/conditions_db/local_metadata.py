#!/usr/bin/env python3

"""
Module containing all functionality necessary to mange local metadata and payload
information.
"""

import sqlite3
from conditions_db import PayloadInformation


class LocalMetadataProvider:
    """
    Class to handle local sqlite dump of conditions database metadata

    This class can create and read sqlite dumps of the central database in a format
    compatible with the local metadata provider in basf2.
    """
    #: Application ID to be stored int the sqlite file
    APPLICATION_ID = 0xb2cdb
    #: Schema version, to be increased when the table definitions change so that
    # we can check for safe append operation.
    SCHEMA_VERSION = 1
    #: SQL script to create all necessary tables and views
    SCHEMA_SQL = """
        -- first drop all tables we care about
        DROP VIEW IF EXISTS iov_payloads;
        DROP INDEX IF EXISTS iov_index;
        DROP TABLE IF EXISTS iovs;
        DROP TABLE IF EXISTS globaltags;
        DROP VIEW IF EXISTS full_payloads;
        DROP TABLE IF EXISTS payloads;
        DROP TABLE IF EXISTS payloadNames;
        DROP TABLE IF EXISTS baseUrls;

        -- table to store the base urls for normalization: Mostly we have the
        -- same base url for all payloads but we only want to store the string
        -- once so give it an id
        CREATE TABLE baseUrls (
            baseUrlId INTEGER PRIMARY KEY,
            baseUrl text UNIQUE
        );

        -- table to store all distinct payload names only once
        CREATE TABLE payloadNames (
            payloadNameId INTEGER PRIMARY KEY,
            payloadName TEXT UNIQUE
        );

        -- Payload information for each payload.
        -- payloadId, revision, checksum, payloadUrl are taken directly without
        -- change from the central database.
        -- payloadName and baseUrl are put into local tables defined above,
        -- these ids are local to the file.
        -- payloadUrl is taken from the database except if it follows the usual
        -- `dbstore/{payloadName}/dbstore_{payloadName}_rev_{revision}.root` in
        -- which case it is set to NULL and created on the fly from the pattern.
        CREATE TABLE payloads (
            payloadId INTEGER PRIMARY KEY,
            payloadNameId INTEGER NOT NULL,
            revision INTEGER NOT NULL,
            checksum text NOT NULL,
            payloadUrl text DEFAULT NULL,
            baseUrlId INTEGER NOT NULL,
            CONSTRAINT name_rev UNIQUE (payloadNameId, revision),
            FOREIGN KEY (payloadNameId) REFERENCES payloadNames (payloadNameId)
                ON UPDATE CASCADE ON DELETE RESTRICT
            FOREIGN KEY (baseUrlId) REFERENCES baseUrls (baseUrlId)
                ON UPDATE CASCADE ON DELETE RESTRICT
        );

        -- Payload information with the local ids and the payloadUrl resolved
        -- Information in this view is identical to the central server
        CREATE VIEW full_payloads AS
            SELECT payloadId, payloadName, revision, checksum,
                ifnull(payloadUrl, 'dbstore/' || payloadName || '/dbstore_' ||
                       payloadName || '_rev_' || revision || '.root')
                as payloadUrl, baseUrl
            FROM payloads NATURAL JOIN payloadNames NATURAL JOIN baseUrls;

        -- table for all globaltags in this file. All values taken directly from
        -- the central server
        CREATE TABLE globaltags (
            globalTagId INTEGER PRIMARY KEY,
            globalTagName text UNIQUE,
            globalTagStatus text NOT NULL
        );

        -- table for all iovs in all globaltags in this file, all values taken
        -- directly from the central server
        CREATE TABLE iovs (
            globalTagId INTEGER NOT NULL,
            payloadId INTEGER NOT NULL,
            firstExp INTEGER NOT NULL,
            firstRun INTEGER NOT NULL,
            finalExp INTEGER NOT NULL,
            finalRun INTEGER NOT NULL,
            FOREIGN KEY (globalTagId) REFERENCES globaltags (globalTagId)
                ON UPDATE CASCADE ON DELETE CASCADE,
            FOREIGN KEY (payloadId) REFERENCES payloads (payloadId)
                ON UPDATE CASCADE ON DELETE RESTRICT
        );
        -- composite index on the iovs to exclude duplicates and allow
        -- performant lookup
        CREATE UNIQUE INDEX iov_index on iovs (
            globalTagId, firstExp, firstRun, finalExp, finalRun, payloadId
        );

        -- full view returning the full information on all iovs in all globaltags
        CREATE VIEW iov_payloads AS
            SELECT globalTagName, payloadId, payloadName, revision, checksum,
                firstExp, firstRun, finalExp, finalRun, payloadUrl, baseUrl
            FROM globaltags NATURAL JOIN iovs NATURAL JOIN full_payloads;
        """

    def __init__(self, filename, mode="read"):
        """
        Open an sqlite database and make sure that the schema exists in the
        correct version or create it if ``mode=overwrite``

        Arguments:
          filename (str): name of the database file
          readonly (str): how to open the file. Can be one of ``read`` to open
            the file readonly, ``append` to append new data to an existing file
            and ``overwrite`` to recreate all tables and overwrite the contents.
        """

        #: Cache name->id mappings from the database
        self._cache = {}
        #: sqlite Database connection
        self._database = None

        # connect to the database file ...
        if mode == "read":
            self._database = sqlite3.connect(f"file:{filename}?mode=ro", uri=True)
        elif mode in ["append", "overwrite"]:
            self._database = sqlite3.connect(filename)
        else:
            raise RuntimeError("invalid mode: please supply one of 'read', 'append', 'overwrite'")

        if mode == "overwrite":
            # drop and recreate all tables
            self._database.executescript(self.SCHEMA_SQL)
            # and set the application id/schema version correctly
            self._database.execute(f"PRAGMA application_id = {self.APPLICATION_ID}")
            self._database.execute(f"PRAGMA user_version = {self.SCHEMA_VERSION}")
            self._database.commit()
        else:
            # make sure the application id/schema version is the same
            application_id = self._database.execute("PRAGMA application_id").fetchone()[0]
            if application_id != self.APPLICATION_ID:
                raise RuntimeError("Not a b2conditionsdb database file")
            schema_version = self._database.execute("PRAGMA user_version").fetchone()[0]
            if schema_version != self.SCHEMA_VERSION:
                raise RuntimeError("Cannot use sqlite file: different schema version, please recreate")

    def get_payload_count(self):
        """Get the number of distinct payloads known to this file"""
        cursor = self._database.execute("SELECT count(*) from full_payloads")
        return cursor.fetchone()[0]

    def _resolve_id(self, name, entity):
        """
        Resolve the id for a named entity in the database file.

        Create new entities on demand and cache all known entities

        Parameters:
          name (str): name to lookup
          entity (str): type of the entity, currently ``baseUrl`` or ``payloadName``
        """
        # fill existing entries on first access
        if entity not in self._cache:
            self._cache[entity] = {row[0]: row[1] for row in self._database.execute(f"SELECT {entity}, {entity}Id from {entity}s")}
        # and then check if we have this entry, otherwise make a new one
        cache = self._cache[entity]
        if name not in cache:
            cache[name] = len(cache) + 1
            self._database.execute(f"INSERT INTO {entity}s ({entity}Id, {entity}) VALUES (?,?)", (cache[name], name))
        return cache[name]

    def add_globaltag(self, tag_id, name, state, iovs):
        """
        Add a globaltag to the database file. If the globaltag already exists in
        the file its contents will be replaced.

        Parameters: tag_id (str): id of the globaltag in the central database
          name (str): name of the globaltag in the central database state (str):
          state of the globaltag in the central database iovs
          (list(PayloadInformation)): all iovs valid for this globaltag
        """
        self._database.execute("INSERT OR REPLACE INTO globaltags VALUES (?,?,?)", (tag_id, name, state))
        # remove existing iovs ... we want to append globaltags but we want all globaltags to be correct
        self._database.execute("DELETE from iovs WHERE globalTagId=?", (tag_id,))

        all_payloads = {}
        all_iovs = []
        for p in iovs:
            if p.payload_id not in all_payloads:
                base_url = self._resolve_id(p.base_url, 'baseUrl')
                name = self._resolve_id(p.name, 'payloadName')
                url = None
                if p.payload_url.lstrip('/') != f"dbstore/{p.name}/dbstore_{p.name}_rev_{p.revision}.root":
                    url = p.payload_url
                all_payloads[p.payload_id] = (p.payload_id, name, p.revision, p.checksum, url, base_url)
            all_iovs.append((tag_id, p.payload_id) + p.iov)

        self._database.executemany("INSERT OR REPLACE INTO payloads VALUES (?,?,?,?,?,?)", all_payloads.values())
        self._database.executemany("INSERT INTO iovs VALUES (?,?,?,?,?,?)", all_iovs)
        # make sure everything is comitted
        self._database.commit()
        self._database.execute("VACUUM")

    def get_globaltags(self):
        """Return the list of globaltags stored in the file

        Returns:
          a list of (id, name, state) tuples for all globaltags
        """
        return [row for row in self._database.execute("SELECT globalTagId, globalTagName, globalTagStatus FROM globaltags "
                                                      "ORDER by globalTagName")]

    def get_payloads(self):
        """Get all payloads existing in this file

        Returns:
          a sorted list of `PayloadInformation` objects for all payloads defined in this file with the iov set to None
        """
        payloads = sorted([PayloadInformation(*row) for row in
                           self._database.execute("SELECT payloadId, payloadName, revision, checksum, "
                                                  "payloadUrl, baseUrl from full_payloads")])
        return payloads

    def get_all_iovs(self, globalTag, exp=None, run=None, message=None):
        """Get all iovs for a given globaltag

        Parameters:
          globalTag (str): name of the globaltag
          exp (int): experiment number to check (or None to return all iovs)
          run (int): run number to check (or None to return all iovs)
          message (str): ignored, just for compatibility with `ConditionsDB.get_all_iovs`

        Returns:
          a sorted list of `PayloadInformation` objects
        """
        params = {"globalTag": globalTag}
        query = """\
            SELECT
              payloadId, payloadName, revision, checksum, payloadUrl, baseUrl,
              firstExp, firstRun, finalExp, finalRun
            FROM iov_payloads
            WHERE globalTagName=:globalTag"""
        if exp is not None:
            params.update({"exp": exp, "run": run})
            query += """ AND\
                ((firstExp==:exp AND firstRun<=:run) OR firstExp<:exp) AND
                (finalExp<0 OR (finalRun<0 AND finalExp>=:exp) OR (finalExp>:exp) OR (finalExp==:exp AND finalRun>=:run))"""
        iovs = sorted([PayloadInformation(*row[:6], iov_id=None, iov=row[6:]) for row in
                       self._database.execute(query, params)])
        return iovs
