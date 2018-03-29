#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sqlite3
import pathlib
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
