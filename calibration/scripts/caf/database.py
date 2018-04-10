#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sqlite3
import pathlib
import pandas
from basf2 import B2DEBUG


class SQLiteDB():

    def __init__(self, database_path, schema=None):
        self.database_path = database_path
        self.schema = schema
        self.conn = None
        try:
            if not self.database_path.exists():
                if not self.schema:
                    raise ValueError("The requested database did not exist, "
                                     "but you didn't provide a schema to create the tables.")
                else:
                    self.open(self.database_path)
                    self.create_schema()
            else:
                self.open(self.database_path)
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
            B2DEBUG(29, "Committing changes and closing Connection for database {}".format(self.database_path))
            self.conn.commit()
            self.conn.close()

    def open(self, database_path):
        B2DEBUG(29, "Opening Connection for database {}".format(self.database_path))
        self.conn = sqlite3.connect(str(database_path))

    def commit(self):
        self.conn.commit()

    def query(self, sql, parameters=tuple()):
        cursor = self.conn.cursor()
        B2DEBUG(29, "execute({}, {})".format(sql, parameters))
        return cursor.execute(sql, parameters)

    def create_schema(self):
        for table_name, fields in self.schema.items():
            columns = ",".join(fields)
            sql = "CREATE TABLE {} ({})".format(table_name, columns)
            self.query(sql)
        self.conn.commit()


class CAFDB(SQLiteDB):
    default_schema = {"calibrations": ["name text primary key",
                                       "state text",
                                       "checkpoint text",
                                       "iteration int"]}

    def __init__(self, database_path):
        super().__init__(database_path, self.default_schema)

    def insert_calibration(self, calibration_name, state="init", checkpoint="init", iteration=0):
        self.query("INSERT INTO calibrations VALUES (?,?,?,?)", (calibration_name, state, checkpoint, iteration))

    def update_calibration_value(self, calibration_name, column_name, new_value):
        self.query("UPDATE calibrations SET {}=? WHERE name=?".format(column_name), (new_value, calibration_name))

    def get_calibration_value(self, calibration_name, column_name):
        return self.query("SELECT {} FROM calibrations WHERE name=?".format(column_name), (calibration_name,)).fetchone()[0]

    def output_calibration_table(self):
        table_string = pandas.read_sql_query("SELECT * FROM calibrations", self.conn).to_string()
        line_len = len(table_string.split("\n")[0])
        title = " Calibrations Table ".center(line_len, " ")
        border = line_len * "="
        header = "\n".join((border, title, border))
        return "\n".join((header, table_string, border))
