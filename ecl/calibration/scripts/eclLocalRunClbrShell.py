#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
import os
import re
import cmd
import json
import subprocess
import sqlite3
import curses
import time
from itertools import chain
from ROOT import TFile, TGraph, TCanvas


class EclLRCalibrator:
    def __init__(self):
        self.__optPath = self.__getOptionsPath__()
        self.__curexp = None
        self.__localDBDir = None
        self.__globalDB = None
        self.__dir = None
        self.__currun = None
        self.__eclMapDB = None
        self.__eclClbrLogDB = None
        self.__checked = True
        self.__saved = True
        self.__userName = ''
        self.__userId = None
        self.__loginTime = None
        self.__tmpLastRunFigs = '.ecl_local_run_shift_last_check.root'
        self.loadOptions()
        if not os.path.exists(self.__eclClbrLogDB):
            self.__createClbrLogDB__()

        self.setUser()
        self.writeOptsToLogDB()

    def __getTime__(self):
        return int(1.e+03 * time.time())

    def isNewExp(self):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        SELECT count(*)
        FROM run
        WHERE
        expnum = %(expnum)d
        ''' % {'expnum': self.__curexp})
        nruns = cursor.fetchone()[0]
        conn.close()
        return nruns == 0

    def __printCurExpNumIsNotSet__(self):
        print('Current experiment number is not set.')

    def __updateRunQuality__(self, runnum, quality):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        UPDATE run
        SET quality = %(quality)d
        WHERE
        expnum = %(expnum)d AND
        runnum = %(runnum)d
        ''' % {
            'expnum': self.__curexp,
            'runnum': runnum,
            'quality': quality})
        conn.commit()
        conn.close()

    def commentLocalRun(self, expnum, runnum, msg):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        SELECT id
        FROM run
        WHERE
        expnum = %(expnum)d AND
        runnum = %(runnum)d
        ''' % {
            'expnum': expnum,
            'runnum': runnum})
        runid = cursor.fetchone()[0]
        curTime = self.__getTime__()
        cursor.execute('''
        INSERT INTO comment
        (runid, userid, time, comment)
        VALUES
        (%(runid)d, %(userid)d, %(time)d, "%(comment)s")
        ''' % {
            'runid': runid,
            'userid': self.__userId,
            'time': curTime,
            'comment': msg})
        conn.commit()
        conn.close()

    def writeOptsToLogDB(self):
        opts = self.__loadJSONOptions__()
        opts['time'] = self.__getTime__()
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        INSERT INTO options
        (time, localdbdir, globaldb, eclmapdb, logdb, dir)
        VALUES
        (%(time)d, "%(localdbdir)s",
        "%(globaldb)s", "%(eclmapdb)s",
        "%(logdb)s", "%(dir)s")
        ''' % opts)
        conn.commit()
        conn.close()

    def setUser(self):
        userName = os.environ.get('USER')
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        SELECT count(*)
        FROM user
        WHERE
        username = "%(username)s"
        ''' % {
            'username': self.__userName})
        if cursor.fetchone()[0] == 0:
            cursor.execute('''
            INSERT INTO user
            (username)
            VALUES
            ("%(username)s")
            ''' % {
                'username': self.__userName})

        cursor.execute('''
        SELECT id
        FROM user
        WHERE
        username = "%(username)s"
        ''' % {'username': self.__userName})
        self.__userId = cursor.fetchone()[0]
        conn.commit()
        conn.close()
        eventTime = self.logEvent(1)

    def isSaved(self):
        return self.__saved

    def getCurRunNum(self):
        return self.__currun

    def isChecked(self):
        return self.__checked

    def __createClbrLogDB__(self):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        CREATE TABLE experiment
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        expnum INTEGER UNIQUE NOT NULL)
        ''')
        cursor.execute('''
        CREATE TABLE run
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        runnum INTEGER NOT NULL,
        expnum INTEGER NOT NULL,
        endtime INTEGER NOT NULL,
        quality INTEGER NOT NULL,
        UNIQUE(runnum, expnum)
        FOREIGN KEY(expnum) REFERENCES experiment(expnum))
        ''')
        cursor.execute('''
        CREATE TABLE comment
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        userid INTEGER NOT NULL,
        runid INTEGER NOT NULL,
        time INTEGER NOT NULL,
        comment TEXT NOT NULL,
        FOREIGN KEY(runid) REFERENCES run(id))
        ''')
        cursor.execute('''
        CREATE TABLE refrun
        (runid INTEGER UNIQUE NOT NULL PRIMARY KEY,
        lowrun INTEGER NOT NULL,
        highrun INTEGER NOT NULL,
        FOREIGN KEY(runid) REFERENCES run(id))
        ''')
        cursor.execute('''
        CREATE TABLE user
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        username TEXT UNIQUE NOT NULL)
        ''')
        cursor.execute('''
        CREATE TABLE event_type
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        type TEXT NOT NULL)
        ''')
        cursor.execute('''
        INSERT INTO event_type
        (id, type)
        VALUES
        (1, "login"),
        (2, "exit"),
        (3, "new_exp"),
        (4, "set_ref_mark"),
        (5, "calibrate"),
        (6, "draw_abs"),
        (7, "draw_norm"),
        (8, "check"),
        (9, "check_fwd"),
        (10, "check_barrel"),
        (11, "save"),
        (12, "comment");
        ''')
        cursor.execute('''
        CREATE TABLE event
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        typeid INTEGER NOT NULL,
        userid INTEGER NOT NULL,
        arguments TEXT,
        success INTEGER,
        time INTEGER UNIQUE NOT NULL,
        FOREIGN KEY(typeid) REFERENCES event_type(id),
        FOREIGN KEY(userid) REFERENCES user(id))
        ''')
        cursor.execute('''
        CREATE TABLE log
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        evid INTEGER NOT NULL,
        msg TEXT NOT NULL,
        FOREIGN KEY(evid) REFERENCES event(id))
        ''')
        cursor.execute('''
        CREATE TABLE options
        (id INTEGER UNIQUE NOT NULL PRIMARY KEY,
        time INTEGER NOT NULL,
        localdbdir TEXT NOT NULL,
        globaldb TEXT NOT NULL,
        eclmapdb TEXT NOT NULL,
        logdb TEXT NOT NULL,
        dir TEXT NOT NULL)
        ''')
        conn.commit()
        conn.close()

    def __clbrDBAdd__(self, runnum):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        SELECT count(*)
        FROM
        experiment
        WHERE
        expnum = %(expnum)d
        ''' % {
            'expnum': self.__curexp})
        endtime = self.__getTime__()
        cursor.execute('''
        INSERT INTO run
        (runnum,
        expnum,
        endtime,
        quality)
        VALUES
        (%(runnum)d,
        %(expnum)d,
        %(endtime)d,
        %(quality)d)
        ''' % {
            'runnum': runnum,
            'expnum': self.__curexp,
            'endtime': endtime,
            'quality': 0})
        conn.commit()
        conn.close()

    def __getEclMapDBPath__(self):
        dirName = os.path.dirname(sys.argv[0])
        return os.path.join(dirName, 'ecl_map.db')

    def __getLocalDBPath__(self, expnum):
        return os.path.join(
            self.__localDBDir,
            'localdb-%(expnum)04d' % {'expnum': expnum},
            'database.txt')

    def __getOptionsPath__(self):
        dirName = os.path.dirname(sys.argv[0])
        path = os.path.join(
            dirName,
            'options_eclLocalRunClbrShell.json')
        return path

    def __keyPressPause__(self):
        stdscr = curses.initscr()
        curses.noecho()
        c = 0
        stdscr.addstr(0, 0, 'Press [S] to continue:',
                      curses.A_REVERSE)
        while chr(c) != 'S':
            c = stdscr.getch()

        curses.endwin()

    def __loadJSONOptions__(self):
        with open(self.__optPath) as fl:
            data = json.load(fl)

        return data

    def __makeSingleRunFigs__(self, expnum, runnum, withref):
        dbname = self.__getLocalDBPath__(expnum)
        cmd = '''eclLocalRunCalibAnalyzer --dbname %(dbname)s \
        --exp %(exp)d --run %(run)d --path %(path)s''' % {
            'dbname': dbname,
            'exp': expnum,
            'run': runnum,
            'path': self.__tmpLastRunFigs}
        if withref:
            cmd += ' --withref'

        subprocess.call(cmd, shell=True)

    def __loadSingleRunFigs__(self, withref):
        fl = TFile.Open(self.__tmpLastRunFigs, 'read')
        raw_graphs = dict()
        graphs = dict()
        raw_graphs['time_count'] = fl.time_count_c
        raw_graphs['ampl_count'] = fl.ampl_count_c
        if withref:
            raw_graphs['time_mean'] = fl.norm_time_mean_c
            raw_graphs['ampl_mean'] = fl.norm_ampl_mean_c
        else:
            raw_graphs['time_mean'] = fl.time_mean_c
            raw_graphs['ampl_mean'] = fl.ampl_mean_c

        for key in raw_graphs:
            graphs[key] = TGraph(raw_graphs[key].GetN())
            graphs[key].SetName(key)
            graphs[key].SetMarkerStyle(7)

        dname = os.path.dirname(sys.argv[0])
        conn = sqlite3.connect(self.__eclMapDB)
        cursor = conn.cursor()
        cursor.execute('''
        SELECT cell_id, electronic_id
        FROM ecl_map
        ''')
        for row in cursor:
            cell_id = row[0]
            electronic_id = row[1]
            for key in raw_graphs:
                graphs[key].SetPoint(
                    cell_id - 1,
                    electronic_id,
                    raw_graphs[key].GetY()[cell_id - 1])

        conn.close()
        graphs['time_count'].SetTitle('time: count')
        graphs['ampl_count'].SetTitle('amplitude: count')
        if withref:
            graphs['time_mean'].SetTitle('time: mean - mean_{ref}')
            graphs['ampl_mean'].SetTitle('amplitude: mean / mean_{ref}')
        else:
            graphs['time_mean'].SetTitle('time: mean')
            graphs['ampl_mean'].SetTitle('amplitude: mean')

        return graphs

    def isRunInDB(self, expnum, runnum):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('''
        SELECT count(*)
        FROM run
        WHERE
        runnum = %(runnum)d AND
        expnum = %(expnum)d
        ''' % {
            'runnum': runnum,
            'expnum': expnum})
        counts = cursor.fetchone()[0]
        conn.close()
        return (counts != 0)

    def __printNoSuchRun__(self, expnum, runnum):
        print('''The run %(runnum)05d of the experiment \
%(expnum)04d is not in the database!''' % {
            'runnum': runnum,
            'expnum': expnum})

    def drawSingleRunFigs(self, eventTime, expnum, runnum, withref=True):
        self.logEventLog(eventTime, 'Begining draw.')
        if not self.isRunInDB(expnum, runnum):
            self.__printNoSuchRun__(expnum, runnum)
            self.logEventSuccess(eventTime, 0)
            self.logEventLog(eventTime, 'The run %(run)05d is not in the DB.' % {
                'run': runnum})
            return

        self.__makeSingleRunFigs__(expnum, runnum, withref)
        graphs = self.__loadSingleRunFigs__(withref)
        cnv1 = TCanvas('cnv1', '', 900, 700)
        cnv1.Draw()
        cnv1.Divide(2, 2, 0.01, 0.01)
        cnv1.cd(1)
        graphs['time_count'].Draw('ap')
        cnv1.cd(2)
        graphs['ampl_count'].Draw('ap')
        cnv1.cd(3)
        graphs['time_mean'].Draw('ap')
        cnv1.cd(4)
        graphs['ampl_mean'].Draw('ap')
        cnv1.Update()
        self.logEventLog(eventTime, 'Waiting for key press.')
        self.__keyPressPause__()
        self.logEventSuccess(eventTime, 1)
        self.logEventLog(eventTime, 'Succesfully finished.')

    def getCurExpNum(self):
        return self.__curexp

    def getIoVInfo(self, expnum, runnum):
        cmd = 'eclLocalRunCalibControl'
        localDB = self.__getLocalDBPath__(expnum)
        out = str(subprocess.check_output(
            [cmd,
             '--show',
             '--calibampl',
             '--dbname', localDB,
             '--exp', str(expnum),
             '--run', str(runnum)]))
        n = len(out)
        out = out[2:n - 1]
        out = out.replace('\\n', ' ')
        template = '''==================== Calibration run:  exp = (\d+) run = (\d+) -------------------- \
Interval of Validity: exp low = (\d+) exp high = (\d+) run low = (\d+) run high = ([+-]?\d+) ===================='''
        res = re.match(template, out)
        return {
            'exp': int(res.groups()[0]),
            'run': int(res.groups()[1]),
            'exp_low': int(res.groups()[2]),
            'exp_high': int(res.groups()[3]),
            'run_low': int(res.groups()[4]),
            'run_high': int(res.groups()[5])}

    def loadCurExpNum(self):
        if not os.path.exists(self.__eclClbrLogDB):
            return

        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        SELECT count(*)
        FROM experiment
        ''')
        nexps = cursor.fetchone()[0]
        if nexps == 0:
            return

        cursor.execute('''
        SELECT max(id)
        FROM experiment
        ''')
        maxExpId = cursor.fetchone()[0]
        cursor.execute('''
        SELECT expnum
        FROM experiment
        WHERE
        id == %(maxid)d
        ''' % {
            'maxid': maxExpId})
        self.__curexp = cursor.fetchone()[0]
        conn.close()

    def loadOptions(self):
        data = self.__loadJSONOptions__()
        self.__localDBDir = data['localdbdir']
        self.__globalDB = data['globaldb']
        self.__eclMapDB = data['eclmapdb']
        self.__eclClbrLogDB = data['logdb']
        self.__dir = data['dir']
        self.loadCurExpNum()

    def __updateCurExpNum__(self, expnum):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        INSERT INTO experiment
        (expnum)
        VALUES
        (%(expnum)d)
        ''' % {'expnum': expnum})
        conn.commit()
        conn.close()

    def __canBeNewRun__(self, runnum):
        comp_data = self.getIoVInfo(self.__curexp,
                                    int(1.e+09))
        if (comp_data['run'] >= runnum):
            return False

        return True

    def checkExpExistance(self, expnum):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('''
        SELECT count(*)
        FROM experiment
        WHERE
        expnum = %(expnum)d
        ''' % {'expnum': expnum})
        isExists = cursor.fetchone()[0]
        conn.close()
        return isExists > 0

    def logEvent(self, typeid):
        eventTime = self.__getTime__()
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        INSERT INTO event
        (typeid, userid, time)
        VALUES
        (%(typeid)d, %(userid)d, %(time)d)
        ''' % {
            'typeid': typeid,
            'userid': self.__userId,
            'time': eventTime})
        conn.commit()
        conn.close()
        return eventTime

    def logEventArgs(self, eventTime, args):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        UPDATE event
        SET arguments = "%(args)s"
        WHERE time = %(time)d
        ''' % {
            'time': eventTime,
            'args': args.replace("'", '__').replace('"', '__')})
        conn.commit()
        conn.close()

    def logEventSuccess(self, eventTime, success):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        UPDATE event
        SET success = "%(success)s"
        WHERE time = %(time)d
        ''' % {
            'time': eventTime,
            'success': success})
        conn.commit()
        conn.close()

    def logEventLog(self, eventTime, logmsg):
        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        SELECT id
        FROM event
        WHERE
        time = %(time)d
        ''' % {'time': eventTime})
        evid = cursor.fetchone()[0]
        cursor.execute('''
        INSERT INTO log
        (evid, msg)
        VALUES
        (%(evid)d, "%(msg)s")''' % {
            'evid': evid,
            'msg': logmsg.replace("'", '__').replace('"', '__')})
        conn.commit()
        conn.close()

    def setNewExpNum(self, eventTime, expnum):
        if self.checkExpExistance(expnum):
            self.logEventSuccess(eventTime, 0)
            self.logEventLog(eventTime,
                             'Database for %(exp)04d already exists.' % {
                                 'exp': expnum})
            print('''A database for experiment %(exp)04d \
already exists! \
The new experiment number has not been created. \
The current experiment nuber is %(exp)04d.''' % {
                'exp': expnum})
            return

        self.__curexp = expnum
        self.__updateCurExpNum__(expnum)
        dirName = os.path.dirname(self.__getLocalDBPath__(self.__curexp))
        self.logEventLog(eventTime, 'Make dir: %(dirname)s' % {
            'dirname': dirName})
        os.mkdir(dirName)
        print('''The new experiment number has been created (%(expnum)04d). \
The current experiment number has been set to %(expnum)04d.''' % {
            'expnum': self.__curexp})
        self.logEventSuccess(eventTime, 1)

    def makeRef(self, runnum, lowrun=-1, highrun=-1, evtime=None):
        if evtime is None:
            evtime = self.logEvent(4)

        if not self.isRunInDB(self.__curexp, runnum):
            self.__printNoSuchRun__(self.__curexp, runnum)
            self.logEventSuccess(evtime, 0)
            self.logEventLog(evtime, 'The run %(run)05d is not in DB.' % {
                'run': runnum})
            return

        if lowrun == -1:
            lowrun = runnum

        cmd = '''eclLocalRunCalibControl --dbname %(dbname)s --addref \
        --exp %(exp)d --run %(run)d --lowrun %(lowrun)d --highrun %(highrun)d''' % {
            'dbname': self.__getLocalDBPath__(self.__curexp),
            'exp': self.__curexp,
            'run': runnum,
            'lowrun': lowrun,
            'highrun': highrun}

        subprocess.call(cmd, shell=True)
        self.logEventSuccess(evtime, 1)

        conn = sqlite3.connect(self.__eclClbrLogDB)
        cursor = conn.cursor()
        cursor.execute('PRAGMA foreign_keys = ON')
        cursor.execute('''
        INSERT OR REPLACE INTO
        refrun
        (runid,
        lowrun,
        highrun)
        VALUES
        (
        (SELECT id
        FROM run
        WHERE
        expnum = %(expnum)d AND
        runnum = %(runnum)d),
        %(lowrun)d,
        %(highrun)d)
        ''' % {
            'expnum': self.__curexp,
            'runnum': runnum,
            'lowrun': lowrun,
            'highrun': highrun})
        conn.commit()
        conn.close()

    def __getListOfLocalRunFiles__(self):
        output = list()
        expDir = os.path.join(
            self.__dir,
            '%(expnum)04d' % {'expnum': self.__curexp})
        template = os.path.join(self.__dir,
                                '(\d+)/(\d+)/ecl.(\d+).(\d+).HLT1.f00000.sroot')
        for x in os.walk(expDir):
            for fname in x[2]:
                path = os.path.join(x[0], fname)
                res = re.match(template, path)
                if res:
                    runnum = str(int(res.groups()[1]))
                    output.append(runnum)

        return output

    def __getRunFilePath__(self, expnum, runnum):
        subpath = '%(expnum)04d/%(runnum)05d/ecl.%(expnum)04d.%(runnum)05d.HLT1.f00000.sroot' % {
            'expnum': expnum,
            'runnum': runnum}
        return os.path.join(self.__dir, subpath)

    def calib(self, eventTime, runnum):
        if self.__curexp is None:
            self.__printCurExpNumIsNotSet__()
            self.logEventSuccess(eventTime, 0)
            self.logEventLog(eventTime, 'The current experiment number is not set.')
            return

        isNewExp = self.isNewExp()
        out = True
        if not isNewExp:
            out = self.__canBeNewRun__(runnum)

        if not out:
            print('''The run %(run)05d already \
exists or is not the last one. \
The local run calibration for \
run %(run)05d is not completed.''' % {
                'run': runnum})
            self.logEventSuccess(eventTime, 0)
            self.logEventLog(eventTime, '''The run %(run)05d already exists or not \
the last one.''' % {
                'run': runnum})
            return

        path = self.__getRunFilePath__(self.__curexp, runnum)
        if not os.path.exists(path):
            self.logEventSuccess(eventTime, 0)
            msg = '''The file %(filename)s for local run %(runnum)05d \
of experiment %(expnum)04d does not exist.''' % {
                'filename': path,
                'expnum': self.__curexp,
                'runnum': runnum}
            self.logEventLog(eventTime, msg)
            print(msg)
            return

        cmd = 'ecl/calibration/scripts/eclLocalRunCalib.py --tree'
        cmd += ' --dbname %s' % (self.__getLocalDBPath__(self.__curexp),)
        cmd += ' --filename %s' % (path,)
        if not isNewExp:
            cmd += ' --changeprev'

        subprocess.call(cmd, shell=True)
        self.logEventSuccess(eventTime, 1)
        self.logEventLog(eventTime, 'Succesfully finished.')
        self.__clbrDBAdd__(runnum)
        if isNewExp:
            self.makeRef(runnum)

        self.__checked = False
        self.__saved = False
        self.__currun = runnum

    def check_negampl(self, runnum):
        cmd = 'eclLocalRunCalibNotify'
        out = str(subprocess.check_output(
            [cmd,
             '--negampl',
             '--dbname', self.__getLocalDBPath__(self.__curexp),
             '--exp', str(self.__curexp),
             '--run', str(runnum)]))
        lst = out.split('\\n')
        text = 'There are negative amplitudes!'
        clst = [text in lst for st in lst]

        if True in clst:
            print('''NEGATIVE AMPLITUDES HAVE BEEN FOUND!''')
        else:
            print('''NO NEGATIVE AMPLITUDES: OK''')

    def print_offsets(self, lst):
        conn = sqlite3.connect(self.__eclMapDB)
        cursor = conn.cursor()
        print('cid, eid, creit, offset')
        for entry in lst:
            cell_id = entry['cellid']
            cursor.execute('''
            SELECT
            electronic_id,
            creit
            FROM
            ecl_map
            WHERE
            cell_id = %(cell_id)d
            ''' % {'cell_id': cell_id})
            row = cursor.fetchone()
            electronic_id = row[0]
            creit = row[1]
            print('%(cell_id)d, %(electronic_id)d, %(creit)d, %(offset)f' % {
                'cell_id': cell_id,
                'electronic_id': electronic_id,
                'creit': creit,
                'offset': entry['offset']})

        conn.close()

    def __updateOffsetList__(self, lst, rng):
        tmplst = []
        for x in lst:
            if x['cellid'] in rng:
                tmplst.append(x)

        return tmplst

    def check_offsets(self, runnum, time_offset, ampl_offset, eclPart):
        cmd = 'eclLocalRunCalibNotify'
        out = str(subprocess.check_output(
            [cmd,
             '--offset',
             '--dbname', self.__getLocalDBPath__(self.__curexp),
             '--exp', str(self.__curexp),
             '--run', str(runnum),
             '--timeoffset', str(time_offset),
             '--amploffset', str(ampl_offset)]))
        lst = out.split('\\n')
        offset_template = 'cellid (\d+): offset = ([+-]?\d+\.\d+)'
        ampl_template = 'Cellids, where the AMPLITUDE offset is greater than (.*).'
        isAmpl = False
        time_line = lst[0][21:]
        ampl_line = None
        time_lst = []
        ampl_lst = []
        for line in lst:
            if re.match(ampl_template, line):
                isAmpl = True
                ampl_line = line[19:]
            else:
                res = re.match(offset_template, line)
                if res:
                    if isAmpl:
                        ampl_lst.append({
                            'cellid': int(res.groups()[0]),
                            'offset': float(res.groups()[1])})
                    else:
                        time_lst.append({
                            'cellid': int(res.groups()[0]),
                            'offset': float(res.groups()[1])})

        if eclPart == 1:  # FWD
            rng = chain(range(1, 1253), range(7777, 8737))
            time_lst = self.__updateOffsetList__(time_lst, rng)
            rng = chain(range(1, 1253), range(7777, 8737))
            ampl_lst = self.__updateOffsetList__(ampl_lst, rng)
            print('FWD:')
        elif eclPart == 2:  # Barrel
            rng = chain(range(1253, 7777))
            time_lst = self.__updateOffsetList__(time_lst, rng)
            rng = chain(range(1253, 7777))
            ampl_lst = self.__updateOffsetList__(ampl_lst, rng)
            print('Barrel:')

        if len(time_lst) > 0:
            print(time_line)
            self.print_offsets(time_lst)
        else:
            print('''TIME OFFSETS: OK''')

        if len(ampl_lst) > 0:
            print(ampl_line)
            self.print_offsets(ampl_lst)
        else:
            print('''AMPLITUDE OFFSETS: OK''')

    def check_quality(self, eclPart, eventTime, runnum, time_offset=2., ampl_offset=3.e-03):
        if self.__currun == runnum and eclPart == 0:
            self.__checked = True

        if self.isRunInDB(self.__curexp, runnum):
            self.logEventLog(eventTime, 'Begin checking negative amplitudes.')
            self.check_negampl(runnum)
            self.logEventLog(eventTime, 'Finished checking negative amplitudes.')
            self.logEventLog(eventTime, 'Begin checking offsets.')
            self.check_offsets(runnum, time_offset, ampl_offset, eclPart)
            self.logEventLog(eventTime, 'Finished checking offsets.')
            self.logEventSuccess(eventTime, 1)
        else:
            print('''The run %(runnum)05d of the experiment %(expnum)04d \
            is not in the database.''' % {
                'expnum': self.__curexp,
                'runnum': runnum})
            self.logEventSuccess(eventTime, 0)
            self.logEventLog(eventTime, 'The %(run)05d of the  experiment %(exp)04d is not in the DB' % {
                'exp': self.__curexp,
                'run': runnum})

    def copy(self, eventTime, runnum):
        if not self.isRunInDB(self.__curexp, runnum):
            print('''The run %(runnum)05d of the %(expnum)04d \
             is not in the database.''' % {
                'expnum': self.__curexp,
                'runnum': runnum})
            return

        cmd = '''eclCopySingleLocalRun --src-localdb --dest-localdb \
        --src-dbname %(srcdb)s --dest-dbname %(destdb)s \
        --exp %(exp)d --run %(run)d''' % {
            'srcdb': self.__getLocalDBPath__(self.__curexp),
            'destdb': self.__globalDB,
            'exp': self.__curexp,
            'run': runnum}
        subprocess.call(cmd, shell=True)
        self.__saved = True
        self.logEventLog(eventTime, '''Begin updating quality \
of the run %(runnum)05d (experiment %(expnum)04d).''' % {
            'expnum': self.__curexp,
            'runnum': runnum})
        self.__updateRunQuality__(runnum, 1)
        self.logEventLog(eventTime, '''Quality of the run %(runnum)05d \
(experiment %(expnum)04d) set to 1.''' % {
            'expnum': self.__curexp,
            'runnum': runnum})


class EclLRClbrShell(cmd.Cmd):
    def __init__(self):
        super(EclLRClbrShell, self).__init__()
        self.intro = '''Welcome to the ECL local run calibration shell. \
Type help or ? to list commands. Type shift_readme to read shift instructions.'''
        self.prompt = '[ecl-lr-calib]'
        self.calibrator = EclLRCalibrator()

    def __printWrogUsageMsg__(self):
        print('''Wrong ussage of the last command. \
        Use help command to find right ussage.''')

    def __printNoCurRunNumber__(self):
        print('''Run number is not selected.''')

    def __printCurRunNotChecked__(self):
        print('''The run has not been checked.''')

    def __checkNumberOfArgs__(self, arg, numargs):
        n_args = len(arg.split())
        if n_args in numargs:
            return True

        return False

    def __ask__(self, question):
        reply = str(input(question + ' (y/n): ')).lower().strip()
        if reply[0] == 'y':
            return True

        if reply[0] == 'n':
            return False
        else:
            self.__ask__('Please enter (y/n): ')

    def do_comment(self, arg):
        '''Comment local run.
        Usage:
        (1) comment'''
        eventTime = self.calibrator.logEvent(12)
        self.calibrator.logEventArgs(eventTime, arg)
        if len(arg) == 0:
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        expnum = self.calibrator.getCurExpNum()
        runnum = self.calibrator.getCurRunNum()
        if runnum is None:
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Current run number is not set.')
            print('Current run number is not set!')
            return

        if not self.calibrator.isRunInDB(expnum, runnum):
            self.__printNoSuchRun__(expnum, runnum)
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, '''The run %(run)05d of the eperiment \
            %(exp)04d is not in the DB.''' % {
                'exp': expnum,
                'run': runnum})
            return

        runnum = self.calibrator.getCurRunNum()
        self.calibrator.commentLocalRun(expnum, runnum,
                                        arg.replace("'", '__').replace('"', '__'))
        self.calibrator.logEventSuccess(eventTime, 0)

    def do_shift_readme(self, arg):
        '''Instruction for shifter.
        Usage:
        (1) shift_readme'''
        if not self.__checkNumberOfArgs__(arg, (0,)):
            self.__printWrogUsageMsg__()

        print('''(I) For each calibration run:
        (1) calibrate <run_number>
        (2) check
        (3) save (if quality of local run is good)
(II) If you need to set a new reference mark, \
use set_ref_mark command.
(III) If you are begining a new experiment, \
you need to use command new_exp <experiment number> \
before calibration.
(IV) You can write a comment to a corresponding local run \
using comment <text> command.''')

    def do_new_exp(self, arg):
        '''Set new experiment number.
        Usage:
        (1) set_new_exp <experiment number>'''
        eventTime = self.calibrator.logEvent(3)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (1, 2)):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        expnum = int(arg.split()[0])
        self.calibrator.setNewExpNum(eventTime, expnum)

    def do_set_ref_mark(self, arg):
        '''Set reference mark.
        Usage:
        (1) set_ref_mark
        (2) set_ref_mark <run number>
        (3) set_ref_mark <run number> <low run> <high run>'''
        eventTime = self.calibrator.logEvent(4)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (0, 1, 3)):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        lst = arg.split()
        runnum = self.calibrator.getCurRunNum()
        if len(lst) > 0:
            runnum = int(lst[0])

        if runnum is None:
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'No current run number.')
            self.__printNoCurRunNumber__()
            return

        if len(lst) == 1:
            self.calibrator.makeRef(runnum, evtime=eventTime)
        else:
            lowrun = int(lst[1])
            highrun = int(lst[2])
            self.calibrator.makeRef(runnum, lowrun, highrun, eventTime)

    def do_calibrate(self, arg):
        '''Run ECL local run calibration \
procedure for the selected run.
        Usage:
        (1) calibrate <run number>'''
        eventTime = self.calibrator.logEvent(5)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (1, )):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        runnum = int(arg.split()[0])
        if not self.calibrator.isChecked():
            self.__printCurRunNotChecked__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(
                eventTime, 'Trying to start new calibration without checking previous one.')
            print('''You cannot start new calibration \
without checking the current run.''')
            return

        if not self.calibrator.isSaved():
            self.calibrator.logEventLog(eventTime, 'Previous calibration is not saved.')
            yes = self.__ask__('''Are you sure you want to process new calibration run without \
 saving current run calibration results into the global database?''')
            if not yes:
                self.calibrator.logEventSuccess(eventTime, 0)
                self.calibrator.logEventLog(eventTime, 'NO')
                return

        isNewExp = self.calibrator.isNewExp()
        self.calibrator.logEventLog(eventTime, 'YES')
        self.calibrator.calib(eventTime, runnum)
        if isNewExp:
            self.do_draw_abs('')
        else:
            self.do_draw_norm('')

    def do_draw_abs(self, arg):
        '''Draw the results of \
the ECL local run calibration \
procedure for the selected run.
        Uasge:
        (1) draw_abs
        (2) draw_abs <run number>
        (3) draw_abs <experiment number> <runnumber>'''
        eventTime = self.calibrator.logEvent(6)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (0, 1, 2)):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        lst = arg.split()
        expnum = self.calibrator.getCurExpNum()
        runnum = self.calibrator.getCurRunNum()
        if len(lst) > 0:
            runnum = int(lst[0])

        if runnum is None:
            self.__printNoCurRunNumber__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Current run number is not set.')
            return

        if len(lst) == 2:
            expnum = int(lst[0])
            runnum = int(lst[1])

        self.calibrator.drawSingleRunFigs(eventTime, expnum, runnum, withref=False)

    def do_draw_norm(self, arg):
        '''Draw the results (normalized \
according to the reference run) of \
the ECL local run calibration \
procedure for the selected run.
        Usage:
        (1) draw_norm
        (2) draw_norm <run number>
        (3) draw_norm <experiment number> <run number>'''
        eventTime = self.calibrator.logEvent(7)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (0, 1, 2)):
            self.__printWrogUsageMsg__()
            return

        lst = arg.split()
        expnum = self.calibrator.getCurExpNum()
        runnum = self.calibrator.getCurRunNum()
        if len(lst) > 0:
            runnum = int(lst[0])

        if runnum is None:
            self.__printNoCurRunNumber__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Current run number is not set.')
            return

        if len(lst) == 2:
            expnum = int(lst[0])
            runnum = int(lst[1])

        self.calibrator.drawSingleRunFigs(eventTime, expnum, runnum, withref=True)

    def do_check(self, arg):
        '''Check quality of calibration runs.
        Usage:
        (1) check
        (2) check <run number>
        (3) check <run number> <max time offset> <max amplitude offset>'''
        self.__check__(0, 8, arg)

    def do_check_fwd(self, arg):
        '''Check quality of calibration runs (FWD part only).
        Usage:
        (1) check_fwd
        (2) check_fwd <run number>
        (3) check_fwd <run number> <max time offset> <max amplitude offset>'''
        self.__check__(1, 9, arg)

    def do_check_barrel(self, arg):
        '''Check quality of calibration runs (Barrel part only).
        Usage:
        (1) check_barrel
        (2) check_barrel <run number>
        (3) check_barrel <run number> <max time offset> <max amplitude offset>'''
        self.__check__(2, 10, arg)

    def __check__(self, eclPart, cmdid, arg):
        '''Check quality of calibration runs.
        Usage:
        (1) check
        (2) check <run number>
        (3) check <run number> <max time offset> <max amplitude offset>'''
        eventTime = self.calibrator.logEvent(cmdid)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (0, 1, 3)):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        lst = arg.split()
        runnum = self.calibrator.getCurRunNum()
        if len(lst) > 0:
            runnum = int(lst[0])

        if runnum is None:
            self.__printNoCurRunNumber__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Current run number is not set.')
            return

        if len(lst) <= 1:
            self.calibrator.check_quality(eclPart, eventTime, runnum)
        else:
            time_offset = float(lst[1])
            ampl_offset = float(lst[2])
            self.calibrator.check_quality(eclPart, eventTime, runnum, time_offset, ampl_offset)

    def do_save(self, arg):
        '''Save calibration results \
into global database (Use \
this command only if you sure, \
that the local run has a good quality).
        Usage:
        (1) save
        (2) save <run number>'''
        eventTime = self.calibrator.logEvent(11)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (0, 1)):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        lst = arg.split()
        runnum = self.calibrator.getCurRunNum()
        if len(lst) > 0:
            runnum = int(lst[0])

        if runnum is None:
            self.__printNoCurRunNumber__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Current run number is not set.')
            return

        if (runnum == self.calibrator.getCurRunNum() and
                not self.calibrator.isChecked()):
            self.__printCurRunNotChecked__()
            print('''You can not save calibration \
information without checking the run. \
To check current run use command "check".''')
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Trying to save without checking calibration qulaity.')
            return

        if (runnum != self.calibrator.getCurRunNum()):
            self.calibrator.logEventLog(eventTime, '''Ask for saving the run %(run)05d, \
which is not current run.''' % {'run': runnum})
            yes = self.__ask__('Are you sure that you want to save this run?')
            if not yes:
                self.calibrator.logEventSuccess(eventTime, 0)
                self.calibrator.logEventLog(eventTime, 'NO')
                return

            self.calibrator.logEventLog(eventTime, 'YES')
            self.calibrator.logEventLog(eventTime, 'Have you checked this run?')
            yes = self.__ask__('Have you checked this run?')
            if not yes:
                self.calibrator.logEventLog(eventTime, 'NO')
                print('''Check run before copying it! \
To check selected run use command "check <run number>".''')
                return

        self.calibrator.logEventLog(eventTime, 'YES')
        self.calibrator.copy(eventTime, runnum)
        self.calibrator.logEventSuccess(eventTime, 1)

    def emptyline(self):
        '''Called when an empty line is entered in response to the prompt. \
        If this method is not overridden, it repeats the last nonempty \
        command entered.'''
        if self.lastcmd:
            self.lastcmd = ""
            return self.onecmd('\n')

    def complete_calibrate(self, text, line, begidx, endidx):
        if self.calibrator.getCurExpNum() is None:
            return

        avRuns = self.calibrator.__getListOfLocalRunFiles__()
        return [i for i in avRuns if i.startswith(text)]

    def do_exit(self, arg):
        '''Exit calibration shell.
        Usage:
        (1) exit'''
        eventTime = self.calibrator.logEvent(2)
        self.calibrator.logEventArgs(eventTime, arg)
        if not self.__checkNumberOfArgs__(arg, (0,)):
            self.__printWrogUsageMsg__()
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, 'Wrong usage.')
            return

        if not self.calibrator.isChecked():
            self.__printCurRunNotChecked__()
            print('''You cannot exit the shell \
without checking current run. \
To check current run use command "check".''')
            self.calibrator.logEventSuccess(eventTime, 0)
            self.calibrator.logEventLog(eventTime, '''Cannot exist the shell without \
checking current run.''')
            return

        yes = True
        if not self.calibrator.isSaved():
            self.calibrator.logEventLog(eventTime, 'Exit without saving calibration results?')
            yes = self.__ask__('''Are you sure you want to \
exit calibration shell without saving current run \
calibration results into the global database?''')

        if yes:
            self.calibrator.logEventLog(eventTime, 'YES')
            self.calibrator.logEventSuccess(eventTime, 1)
            sys.exit(0)
        else:
            self.calibrator.logEventLog(eventTime, 'NO')
            self.calibrator.logEventSuccess(eventTime, 0)


def main():
    EclLRClbrShell().cmdloop()


if __name__ == '__main__':
    main()
