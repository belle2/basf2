#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# $Id: mdstandalone.py,v 1.5 2007/03/02 16:32:32 koblitz Exp $
#
import os
import re
import errno
import time
import mdinterface
from mdinterface import CommandException, MDInterface
from mdparser import MDParser
from mdtable import MDTable
from diskutils import RLock

DEBUG = False


def visitLocksRemove(arg, directory, files):
    age = arg
    for name in files:
        if name != 'LOCK':
            continue
        filename = os.path.join(directory, name)
        mtime = os.stat(filename).st_mtime
        if age > 0 and mtime > age:
            continue
        os.remove(filename)


def visitLocksList(arg, directory, files):
    (age, root, lines) = arg
    for name in files:
        if name != 'LOCK':
            continue
        filename = os.path.join(directory, name)
        mtime = os.stat(filename).st_mtime
        if age > 0 and mtime > age:
            continue
        line = filename[len(root) - 1:]  # Remove path to root directory
        line = os.path.normpath(line)
        line = line[0:len(line) - 5]  # Remove /LOCK
        if len(line) == 0:
            line = '/'
        line = os.path.normpath(line)
        lines.append(line)


class MDStandalone(mdinterface.MDInterface):

    def __mkdir(self, newdir):
        """ works the way a good mkdir should :)
        - already exists, silently complete
        - regular file in the way, raise an exception
        - parent directory(ies) does not exist, make them as well
        """

        if os.path.isdir(newdir):
            pass
        elif os.path.isfile(newdir):
            raise mdinterface.CommandException(16, 'Directory exists')
        try:
            os.mkdir(newdir)
        except OSError, e:
            if e[0] == errno.EPERM or e[0] == errno.EACCES:
                raise mdinterface.CommandException(4,
                        'Could not create dir: Permission denied')
            else:
                raise mdinterface.CommandException(16, 'Directory existed')

    def __init__(self, root):
        self.root = os.path.normpath(root)
        if self.root[-1] != '/':
            self.root += '/'
        self.rows = []
        self.rowPtr = 0
        self.tables = {}
        self.currentDir = '/'
#        os.stat_float_times(True)
        self.loaded_tables = {}
        self.upload = {}

    def __isDir(self, dirname):
        name = self.__systemPath(dirname)
        return os.path.isdir(name)

    def __absolutePath(self, table):
        if DEBUG:
            print '__absolutePath for ', table
        if len(table) and table[0] == '/':
            if DEBUG:
                print '__absolutePath: returning', table
            return table
        table = os.path.normpath(self.currentDir + '/' + table)
        # Replace // -> /
        p = table.find('//')
        while p > -1:
            table = table[:p] + table[p + 1:]
            p = table.find('//')
        if DEBUG:
            print '__absolutePath: returning', table
        return table

    def __systemPath(self, path):
        path = os.path.normpath(self.root + path)
        if DEBUG:
            print '__systemPath returns: ', path
        return path

    def __loadTable(self, table, update=True):
        # Absolute path of table
        if DEBUG:
            print 'Loading table', table
        (mdtable, table) = self._getTable(table)
        if table not in self.tables:
            if not mdtable.lock():
                raise CommandException(9, 'Could not acquire table lock %s'
                                       % table)
            self.tables[table] = mdtable
            mdtable.load()
        return (mdtable, table)

    def __saveTable(self, table):
        if DEBUG:
            print 'Saving table', table
        mdtable = self.loaded_tables[table]
        mdtable.save()

    def _getTable(self, table):
        table = self.__absolutePath(table)
        dirname = self.__systemPath(table)
        if not os.path.isdir(dirname):
            raise CommandException(1, 'File or directory does not exist')
        if table not in self.loaded_tables:
            self.loaded_tables[table] = MDTable(dirname)
        return (self.loaded_tables[table], table)

    def releaseAllLocks(self):
        for table in self.tables:
            self.tables[table].unlock()

    def eot(self):
        return self.rowPtr >= len(self.rows)

    def addEntries(self, entries):
        self.tables = {}
        try:
            tablename = os.path.dirname(entries[0])
            (mdtable, tablename) = self.__loadTable(tablename, True)
            entryDict = {}
            for i in range(0, len(mdtable.entries)):
                entryDict[mdtable.entries[i][0]] = i
            emptyAttr = [''] * len(mdtable.attributes)
            for entry in entries:
                if entryDict.has_key(os.path.basename(entry)):
                    raise CommandException(15, 'Entry exists')
                row = [os.path.basename(entry)]
                row.extend(emptyAttr)
                mdtable.entries.append(row)
            self.__saveTable(tablename)
        finally:
            self.releaseAllLocks()

    def addAttr(
        self,
        file,
        name,
        t,
        ):

        self.tables = {}
        try:
            dirname = self.__absolutePath(file)
            if not self.__isDir(dirname):
                dirname = os.path.dirname(dirname)
            (mdtable, tablename) = self.__loadTable(dirname, True)
            if mdtable.attributeDict.has_key(name):
                raise CommandException(15, 'Attribute exists')
            mdtable.attributes.append([name, t])
            mdtable.update()
            e_len = len(mdtable.attributes)
            for i in range(0, len(mdtable.entries)):
                (mdtable.entries[i])[e_len:] = ['']
                mdtable.entries.mark(i)
            self.__saveTable(tablename)
        finally:
            self.releaseAllLocks()

    def listAttr(self, file):
        self.tables = {}
        try:
            dirname = self.__absolutePath(file)
            if not self.__isDir(dirname):
                dirname = os.path.dirname(dirname)
            (mdtable, dirname) = self.__loadTable(dirname)
            names = []
            types = []
            for i in range(0, len(mdtable.attributes)):
                pair = mdtable.attributes[i]
                names.append(pair[0])
                types.append(pair[1])
            return (names, types)
        finally:
            self.releaseAllLocks()

    def addEntry(
        self,
        file,
        keys,
        values,
        ):

        self.tables = {}
        try:
            (tablename, entry) = os.path.split(file)
            (mdtable, tablename) = self.__loadTable(tablename, True)
            entryDict = {}
            for i in range(0, len(mdtable.entries)):
                entryDict[mdtable.entries[i][0]] = i
            if entryDict.has_key(entry):
                raise CommandException(15, 'Entry exists')
            e = [''] * (len(mdtable.attributes) + 1)
            e[0] = entry
            for i in range(0, len(keys)):
                e[mdtable.attributeDict[keys[i]] + 1] = values[i]
            mdtable.entries.append(e)
            self.__saveTable(tablename)
        finally:
            self.releaseAllLocks()

    def getattr(self, file, attrs):
        self.tables = {}
        try:
            (tablename, entry) = os.path.split(file)
            (mdtable, tablename) = self.__loadTable(tablename)
            self.rows = []
            self.rowPtr = 0
            pattern = entry.replace('*', '.*')
            pattern = pattern.replace('?', '.')
            for i in range(0, len(mdtable.entries)):
                r = re.match(pattern, mdtable.entries[i][0])
                if r and r.group(0) == mdtable.entries[i][0]:
                    row = []
                    row.append(mdtable.entries[i][0])
                    for j in range(0, len(attrs)):
                        index = mdtable.attributeDict[attrs[j]]
                        row.append(mdtable.entries[i][index + 1])
                    self.rows.append(row)
            self.rowPtr = 0
        finally:
            self.releaseAllLocks()

    def getEntry(self):
        (entry, row) = (self.rows[self.rowPtr][0],
                        (self.rows[self.rowPtr])[1:])
        self.rowPtr = self.rowPtr + 1
        return (entry, row)

    def createDir(self, dirname):
        if DEBUG:
            print 'createDir ', dirname
        newdir = self.__systemPath(self.__absolutePath(dirname))
        self.__mkdir(newdir)

    def listEntries(self, dirname):
        if DEBUG:
            print 'listEntries ', dirname
        self.tables = {}
        try:
            dirname = self.__absolutePath(dirname)
            if self.__isDir(dirname):
                entry = '*'
            else:
                (dirname, entry) = os.path.split(dirname)

            # List directories, first
            dirs = os.listdir(self.__systemPath(dirname))
            for d in dirs:
                if self.__isDir(dirname + '/' + d):
                    row = []
                    row.append(dirname + '/' + d)
                    row.append('collection')
                    self.rows.append(row)

            # Now list entries
            (mdtable, dirname) = self.__loadTable(dirname)
            pattern = entry.replace('*', '.*')
            pattern = pattern.replace('?', '.')
            for i in range(0, len(mdtable.entries)):
                r = re.match(pattern, mdtable.entries[i][0])
                if r and r.group(0) == mdtable.entries[i][0]:
                    row = []
                    row.append(mdtable.entries[i][0])
                    row.append('entry')
                    self.rows.append(row)
        finally:
            self.releaseAllLocks()

    def removeAttr(self, file, name):
        if DEBUG:
            print 'removeAttr ', file, name
        self.tables = {}
        try:
            dirname = self.__absolutePath(file)
            if not self.__isDir(dirname):
                dirname = os.path.dirname(dirname)
            (mdtable, dirname) = self.__loadTable(dirname, True)
            if not mdtable.attributeDict.has_key(name):
                raise CommandException(10, 'No such key')
            del mdtable.attributes[mdtable.attributeDict[name]]
            self.__saveTable(dirname)
        finally:
            self.releaseAllLocks()

    def rm(self, path):
        if DEBUG:
            print 'rm ', path
        self.tables = {}
        try:
            (dirname, entry) = os.path.split(path)
            (mdtable, dirname) = self.__loadTable(dirname, True)
            pattern = entry.replace('*', '.*')
            pattern = pattern.replace('?', '.')
            for i in range(len(mdtable.entries) - 1, -1, -1):
                r = re.match(pattern, mdtable.entries[i][0])
                if r and r.group(0) == mdtable.entries[i][0]:
                    del mdtable.entries[i]
            self.__saveTable(dirname)
        finally:
            self.releaseAllLocks()

    def sequenceCreate(
        self,
        name,
        directory,
        increment=1,
        start=1,
        ):

        name = self.__systemPath(self.__absolutePath(directory) + '/SEQ'
                                 + name)
        lockfile = name + '.LOCK'
        lock = RLock(lockfile)
        if lock.acquire():
            try:
                if os.path.exists(name):
                    raise CommandException(15, 'Sequence exists')
                f = open(name, 'w')
                line = str(start) + ' ' + str(increment) + ' ' + str(start)
                f.write(line + '\n')
                f.close()
            finally:
                lock.release()
        else:
            raise CommandException(9, 'Cannot lock %s' % name)

    def sequenceNext(self, name):
        (dirname, seq) = os.path.split(name)
        name = self.__systemPath(self.__absolutePath(dirname) + '/SEQ' + seq)
        lockfile = name + '.LOCK'
        lock = RLock(lockfile)
        if lock.acquire():
            try:
                try:
                    f = open(name, 'r')
                except IOError, e:
                    raise CommandException(17, 'Not a sequence')
                line = f.readline()
                if not line:
                    raise CommandException(17, 'Not a sequence')
                start = 0
                increment = 0
                now = 0
                (start, increment, now) = line.split()
                newval = int(now) + int(increment)
                f.close()
                f = open(name, 'w')
                line = str(start) + ' ' + str(increment) + ' ' + str(newval)
                f.write(line + '\n')
                f.close()
                return now
            finally:
                lock.release()
        else:
            raise CommandException(9, 'Cannot lock %s' % name)

    def sequenceRemove(self, name):
        (dirname, seq) = os.path.split(name)
        name = self.__systemPath(self.__absolutePath(dirname) + '/SEQ' + seq)
        lockfile = name + '.LOCK'
        lock = RLock(lockfile)
        if lock.acquire():
            try:
                if not os.path.isfile(name):
                    raise CommandException(17, 'Not a sequence')
                os.remove(name)
            finally:
                lock.release()
        else:
            raise CommandException(9, 'Cannot lock %s' % name)

    def removeDir(self, dirname):
        if DEBUG:
            print 'removeDir ', dirname
        name = self.__systemPath(self.__absolutePath(dirname))
        try:
            os.rmdir(name)
        except OSError, e:
            if e[0] == errno.ENOENT:
                raise mdinterface.CommandException(1, 'Directory not found')
            else:
                raise mdinterface.CommandException(11, 'Directory not empty')

    def selectAttr(self, attrs, query):
        if DEBUG:
            print 'selectAttr ', attrs, query
        self.tables = {}
        try:
            self.rows = []
            self.rowPtr = 0

            tList = []
            aList = []
            for a in attrs:
                (table, attr) = a.split(':', 1)
                (mdtable, table) = self.__loadTable(table)
                tList.append(table)
                aList.append(attr)

            parser = MDParser(query, self.tables, self.currentDir,
                              self.__loadTable)

            # First, only load necessary tables
            parser.parseWhereClause()
            allTables = []
            iterations = 0
            for (k, v) in self.tables.iteritems():
                v.initRowPointer()
                allTables.append(v)
                if iterations == 0:
                    iterations = len(v.entries)
                else:
                    iterations = iterations * len(v.entries)

            for i in range(0, iterations):
                if parser.parseWhereClause():
                    row = []
                    for j in range(0, len(attrs)):
                        t = self.tables[tList[j]]
                        if aList[j] == 'FILE':
                            index = 0
                        else:
                            index = t.attributeDict[aList[j]] + 1
                        row.append(t.entries[t.currentRow][index])
                    self.rows.append(row)
                for j in range(0, len(allTables)):
                    if allTables[j].currentRow < 0:
                        continue
                    allTables[j].currentRow = allTables[j].currentRow + 1
                    if allTables[j].currentRow < len(allTables[j].entries):
                        break
                    allTables[j].currentRow = 0
            self.rowPtr = 0
        finally:
            self.releaseAllLocks()

    def getSelectAttrEntry(self):
        attributes = self.rows[self.rowPtr]
        self.rowPtr = self.rowPtr + 1
        return attributes

    def updateAttr(
        self,
        pattern,
        updateExpr,
        condition,
        eval=True,
        ):

        if DEBUG:
            print 'updateAttr ', pattern, updateExpr, condition, eval
        self.tables = {}
        try:
            dirname = self.__absolutePath(pattern)
            if self.__isDir(dirname):
                entry = '*'
            else:
                (dirname, entry) = os.path.split(dirname)
            (mdtable, tablename) = self.__loadTable(dirname, True)
            pattern = entry.replace('*', '.*')
            pattern = pattern.replace('?', '.')

            parser = MDParser(condition, self.tables, tablename,
                              self.__loadTable)

            # First, only load necessary tables
            parser.parseWhereClause()

            # Prepare update expressions
            expressions = []
            for e in updateExpr:
                (var, exp) = self.splitUpdateClause(e)
                index = mdtable.attributeDict[var] + 1
                p = MDParser(exp, self.tables, tablename, self.__loadTable)
                parser.parseWhereClause()
                expressions.append((index, exp, p))

            # Find all tables which have entries and list them in allTables,
            # and count the number of iterations we need to do
            allTables = []
            iterations = 0
            for (k, v) in self.tables.iteritems():
                v.initRowPointer()
                if len(v.entries) == 0:
                    continue
                allTables.append(v)
                if iterations == 0:
                    iterations = len(v.entries)
                else:
                    iterations = iterations * len(v.entries)

            # Iterate over all combinations of table entries, check whether
            # update clause is fulfilled and do the update
            for i in range(0, iterations):
                r = re.match(pattern, mdtable.entries[mdtable.currentRow][0])
                if r and r.group(0) == mdtable.entries[mdtable.currentRow][0]:
                    if parser.parseWhereClause():
                        for j in range(0, len(expressions)):
                            (index, exp, p) = expressions[j]
                            if eval:
                                value = p.parseStatement()
                            else:
                                value = exp
                            mdtable.entries[mdtable.currentRow][index] = value
                            mdtable.entries.mark(mdtable.currentRow)
                for j in range(0, len(allTables)):
                    allTables[j].currentRow = allTables[j].currentRow + 1
                    if allTables[j].currentRow < len(allTables[j].entries):
                        break
                    else:
                        allTables[j].currentRow = 0
            self.__saveTable(tablename)
        finally:
            self.releaseAllLocks()

    def update(
        self,
        pattern,
        updateExpr,
        condition,
        eval=True,
        ):

        self.updateAttr(self, pattern, updateExpr, condition, False)

    def setAttr(
        self,
        file,
        keys,
        values,
        ):

        self.tables = {}
        (tablename, entry) = os.path.split(file)
        (mdtable, tablename) = self.__loadTable(tablename, True)
        try:
            pattern = entry.replace('*', '.*')
            pattern = pattern.replace('?', '.')
            for i in range(0, len(mdtable.entries)):
                r = re.match(pattern, mdtable.entries[i][0])
                if r and r.group(0) == mdtable.entries[i][0]:
                    for j in range(0, len(keys)):
                        index = mdtable.attributeDict[keys[j]]
                        mdtable.entries[i][index + 1] = values[j]
                        mdtable.entries.mark(i)
                self.__saveTable(tablename)
        finally:
            self.releaseAllLocks()

    def pwd(self):
        return self.currentDir

    def cd(self, dir):
        dir = self.__absolutePath(dir)
        if not self.__isDir(dir):
            raise CommandException(1, 'Not a directory')
        self.currentDir = dir

    def upload(self, collection, attributes):
        self.tables = {}
        (mdtable, tablename) = self.__loadTable(collection, True)
        self.upload['collection'] = tablename
        self.upload['attributes'] = attributes

    def put(self, file, values):
        (tablename, entry) = os.path.split(file)
        if tablename:
            assert tablename == self.upload['collection']
        mdtable = self.tables[self.upload['collection']]
        attrs = self.upload['attributes']

        def set_values():
            for i in range(0, len(attrs)):
                e[mdtable.attributeDict[attrs[i]] + 1] = values[i]

        for n in range(len(mdtable.entries)):
            e = mdtable.entries[n]
            if e[0] == entry:
                set_values()
                mdtable.entries.mark(n)
                break
        else:
            # new entry
            e = [''] * (len(mdtable.attributes) + 1)
            e[0] = entry
            set_values()
            mdtable.entries.append(e)

    def abort(self):
        try:
            del self.loaded_tables[self.upload['collection']]
            self.upload = {}
        finally:
            self.releaseAllLocks()

    def commit(self):
        try:
            self.__saveTable(self.upload['collection'])
        finally:
            self.releaseAllLocks()

    # Removes all locks, even those of other processes recursively
    # starting at the root directory by deleting the LOCK files
    # If age is given it restricts the deletion operation on most
    # operating systems to locks older than age seconds

    def removeAllLocks(self, age=-1):
        if age > 0:
            age = time.time() - age
        os.path.walk(self.root, visitLocksRemove, age)

    # Returns a list witha all currently held locks (locks older than age if age>0)

    def listAllLocks(self, age=-1):
        if age > 0:
            age = time.time() - age

        lines = []
        os.path.walk(self.root, visitLocksList, (age, self.root, lines))
        return lines


