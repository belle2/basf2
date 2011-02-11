#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Class to deal with all required parameters for gbasf.

import DIRAC
from DIRAC.Core.Base import Script
import os

# initial implementation uses CLI and steering file
# potentially we can also get some parameters from the user environment in the future

# class name is CLIParams to be consistent with DIRAC


class CLIParams:

    EvtPerMin = 45
    steering_file = None
    project = 'Ungrouped'
    priority = 0
    query = None
    swver = 'release-00-01-00'
    sysconfig = 'Belle-v2r1'
    datatype = None
    experiments = None
    inputsandboxfiles = None
    maxevents = None

    def setSteeringFile(self, arg):
        self.steering_file = arg
        return DIRAC.S_OK()

    def setProject(self, arg):
    # FIXME: check for printable characters
        self.project = arg
        return DIRAC.S_OK()

    def setEvtPerMin(self, arg):
        if arg > 0:
            self.EvtPerMin = int(arg)
            return DIRAC.S_OK()
        else:
            return DIRAC.S_ERROR('EvtPerMin is <=0')

    def setJobPriority(self, arg):
    # FIXME: check query is within range
        self.priorty = arg
        return DIRAC.S_OK()

    def setQuery(self, arg):
    # FIXME: check query is valid against schema
        self.query = arg
        return DIRAC.S_OK()

    def setDataType(self, arg):
        if arg == 'data' or arg == 'MC':
            self.datatype = arg
            return DIRAC.S_OK()
        else:
            return DIRAC.S_ERROR('Data Type can only be data or MC')

    def setExperiments(self, arg):
    # FIXME check experiment meets range and list of valid experiments
        print 'Experiments=' + arg
        self.experiments = arg
        return DIRAC.S_OK()

    def setSwVer(self, arg):
    # FIXME: check swver is valid against list
        self.swver = arg
        return DIRAC.S_OK()

    def setSysConfig(self, arg):
    # FIXME: check swver is valid against list
        self.sysconfig = arg
        return DIRAC.S_OK()

    def setInputFiles(self, arg):
    # FIXME: check all files exist
        self.inputsandboxfiles = arg.rstrip().split(',')
        for file in self.inputsandboxfiles:
            if os.path.exists(file) is False:
                return DIRAC.S_ERROR(file + ' does not exist.')
        return DIRAC.S_OK()

    def setMaxEvents(self, arg):
      # FIXME : check for integer
        if arg > 0:
            self.maxevents = arg.strip()
        else:
            return DIRAC.S_ERROR

    def getSteeringFile(self):
        return self.steering_file

    def getProject(self):
        return self.project

    def getEvtPerMin(self):
        return self.EvtPerMin

    def getJobPriority(self):
        return self.priority

    def getQuery(self):
        return self.query

    def getDataType(self):
        return self.datatype

    def getExperiments(self):
        return self.experiments

    def getSwVer(self):
        return self.swver

    def getSysConfig(self):
        return self.sysconfig

    def getInputFiles(self):
        return self.inputsandboxfiles

    def getMaxEvents(self):
        return self.maxevents

  # registers alll of the possible commandline options with the DIRAC Script handler
  # This is also used to generate the --help option

    def registerCLISwitches(self):
        Script.registerSwitch('s:', 'steering=', 'basf2 steering file',
                              self.setSteeringFile)
        Script.registerSwitch('p:', 'project=', 'Name for project',
                              self.setProject)
        Script.registerSwitch('c:', 'evtpermin=',
                              'estimated EvtPerMin (in seconds)',
                              self.setEvtPerMin)
        Script.registerSwitch('w:', 'priority=',
                              '(optional) Job priority: 0 is default',
                              self.setJobPriority)
        Script.registerSwitch('m:', 'query=', 'Metadata Query', self.setQuery)
        Script.registerSwitch('t:', 'type=', "Type of Data ('data' or 'MC')",
                              self.setDataType)
        Script.registerSwitch('e:', 'experiments=',
                              'Experiments (comma separated list)',
                              self.setExperiments)
        Script.registerSwitch('l:', 'swver=', 'Software Version',
                              self.setSwVer)
        Script.registerSwitch('b:', 'sysconfig=',
                              '(Advanced) DIRAC System Configuration Version',
                              self.setSysConfig)
        Script.registerSwitch('f:', 'inputsandboxfiles=',
                              '(optional) Files required for the job (comma separated list, max 10MB)'
                              , self.setInputFiles)
        Script.registerSwitch('x:', 'maxevents=',
                              '(optional) Maximum number of events to use',
                              self.setMaxEvents)
        Script.addDefaultOptionValue('LogLevel', 'debug')

  # loop through the steering file to determine any options set there

    def registerSteeringOptions(self):
        if self.steering_file is None:
            return DIRAC.S_ERROR("No steering file defined - can't get options."
                                 )
        else:
      # hackhackhack option-function mapping :)
            options = {
                'project': 'setProject',
                'evtpermin': 'setEvtPerMin',
                'priority': 'setJobPriority',
                'query': 'setQuery',
                'type': 'setDataType',
                'experiments': 'setExperiments',
                'swver': 'setSwVer',
                'inputsandboxfiles': 'setInputFiles',
                'maxevents': 'setMaxEvents',
                }
      # read the options
            f = open(self.steering_file)
            for line in f:
                for option in options.keys():
                    if option in line[0:len(option)]:
                        setFunction = getattr(self, options[option])
                        # DEBUG print line
                        setFunction(line.split('=', 1)[1].strip().replace('"',
                                    '').replace("'", ''))


