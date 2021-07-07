#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sqlite3
import pathlib
import pandas
from basf2 import B2DEBUG


class SQLiteDB():
    """
    Parameters:
      database_path (pathlib.Path): The path to the database file we want to create/connect to.

    Keyword Arguments:
      schema (dict): Database table schema for the DB of the form:
                     {"tablename": ["columnname1 text primary key",
                                    "columnname2 int"]
                     }
      read_only (bool): Should the connection be treated as a read-only connection (no update/insert calls)
      timeout (float): What timeout value should the connection have. How long to wait for other changes to commit.
      isolation_level (str): How should the connection behave when making transactions?
        Choices are [None, "DEFERRED", "IMMEDIATE", "EXCLUSIVE"] where None is autocommit behaviour.
    """

    def __init__(self, database_path, schema=None, read_only=False, timeout=5.0, isolation_level=None):
        self.database_path = database_path
        self.schema = schema
        self.conn = None
        self.read_only = read_only
        self.timeout = timeout
        self.isolation_level = isolation_level
        try:
            if not self.database_path.exists() and not self.read_only:
                if not self.schema:
                    raise ValueError("The requested database did not exist, "
                                     "but you didn't provide a schema to create the tables.")
                else:
                    self.open()
                    self.create_schema()
            elif not self.database_path.exists() and read_only:
                raise ValueError("The requested database did not exist, "
                                 "but you specified that this was a read_only connection.")
            else:
                self.open()
        except AttributeError as err:
            if not isinstance(self.database_path, pathlib.Path):
                raise TypeError("You did not use a pathlib.Path object as the database_path.")
            else:
                raise err

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.close()

    def close(self):
        if self.conn:
            if not self.read_only:
                B2DEBUG(29, f"Committing changes and closing Connection for database {self.database_path}.")
                self.conn.commit()
            self.conn.close()

    def open(self):
        B2DEBUG(29, f"Opening Connection for database {self.database_path}. Readonly = {self.read_only}")
        connection_uri = self.get_uri()
        self.conn = sqlite3.connect(connection_uri, uri=True, timeout=self.timeout, isolation_level=self.isolation_level)

    def commit(self):
        self.conn.commit()

    def query(self, sql, parameters=tuple()):
        cursor = self.conn.cursor()
        B2DEBUG(29, f"execute({sql}, {parameters}).")
        return cursor.execute(sql, parameters)

    def create_schema(self):
        for table_name, fields in self.schema.items():
            columns = ",".join(fields)
            sql = f"CREATE TABLE {table_name} ({columns})"
            self.query(sql)
        self.conn.commit()

    def get_uri(self):
        uri = f"file:{self.database_path.as_posix()}"
        if self.read_only:
            uri += "?mode=ro"
        return uri


class CAFDB(SQLiteDB):
    """
    Parameters:
      database_path (pathlib.Path): The path to the database file we want to create/connect to.

    Keyword Arguments:
      read_only (bool): Should the connection be treated as a read-only connection (no update/insert calls)
      timeout (float): What timeout value should the connection have. How long to wait for other changes to commit.
    """

    default_schema = {"calibrations": ["name text primary key",
                                       "state text",
                                       "checkpoint text",
                                       "iteration int"]}

    def __init__(self, database_path, read_only=False, timeout=30.0, isolation_level=None):
        super().__init__(database_path, self.default_schema, read_only, timeout, isolation_level)

    def insert_calibration(self, calibration_name, state="init", checkpoint="init", iteration=0):
        self.query("INSERT INTO calibrations VALUES (?,?,?,?)", (calibration_name, state, checkpoint, iteration))

    def update_calibration_value(self, calibration_name, column_name, new_value, attempts=3):
        attempt = 1
        finished = False
        while not finished:
            try:
                self.query("UPDATE calibrations SET {}=? WHERE name=?".format(column_name), (new_value, calibration_name))
                finished = True
            except sqlite3.OperationalError as e:
                if attempt < attempts:
                    attempt += 1
                else:
                    raise e

    def get_calibration_value(self, calibration_name, column_name):
        return self.query("SELECT {} FROM calibrations WHERE name=?".format(column_name), (calibration_name,)).fetchone()[0]

    def output_calibration_table(self):
        data = {"name": [], "state": [], "checkpoint": [], "iteration": []}
        for row in self.query("SELECT * FROM calibrations"):
            data["name"].append(row[0])
            data["state"].append(row[1])
            data["checkpoint"].append(row[2])
            data["iteration"].append(row[3])

        table = pandas.DataFrame(data)
        table_string = table.to_string()

        line_len = len(table_string.split("\n")[1])
        title = " Calibrations Table ".center(line_len, " ")
        border = line_len * "="
        header = "\n".join((border, title, border))
        return "\n".join((header, table_string, border))
