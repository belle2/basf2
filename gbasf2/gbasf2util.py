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

    CPUTime = 862400
    steering_file = None
    project = 'Ungrouped'
    priority = 0
    query = None
    swver = 'Belle-v1r0'
    datatype = None
    experiments = None
    inputfiles = None

    def setSteeringFile(self, arg):
        self.steering_file = arg
        return DIRAC.S_OK()

    def setProject(self, arg):
    # FIXME: check for printable characters
        self.project = arg
        return DIRAC.S_OK()

    def setCPUTime(self, arg):
        if arg > 0:
            self.CPUTime = arg
            return DIRAC.S_OK()
        else:
            return DIRAC.S_ERROR('CPUTime is <=0')

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

    def setInputFiles(self, arg):
    # FIXME: check all files exist
        self.inputfiles = arg.rstrip().split(',')
        for file in self.inputfiles:
            if os.path.exists(file) is False:
                return DIRAC.S_ERROR(file + ' does not exist.')
        return DIRAC.S_OK()

    def getSteeringFile(self):
        return self.steering_file

    def getProject(self):
        return self.project

    def getCPUTime(self):
        return self.CPUTime

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

    def getInputFiles(self):
        return self.inputfiles

  # registers alll of the possible commandline options with the DIRAC Script handler
  # This is also used to generate the --help option

    def registerCLISwitches(self):
        Script.registerSwitch('s:', 'steering_file=', 'basf2 steering file',
                              self.setSteeringFile)
        Script.registerSwitch('p:', 'project=', 'Name for project',
                              self.setProject)
        Script.registerSwitch('c:', 'CPUTime=',
                              'estimated CPUTime (in seconds)',
                              self.setCPUTime)
        Script.registerSwitch('w:', 'priority=', 'Job priority: 0 is default',
                              self.setJobPriority)
        Script.registerSwitch('m:', 'query=', 'Metadata Query', self.setQuery)
        Script.registerSwitch('t:', 'type=', "Type of Data ('data' or 'MC')",
                              self.setDataType)
        Script.registerSwitch('e:', 'experiments=',
                              'Experiments (comma separated list)',
                              self.setExperiments)
        Script.registerSwitch('l:', 'swver=', 'Software Version',
                              self.setSwVer)
        Script.registerSwitch('f:', 'inputfiles=',
                              'Files required for the job (comma separated list, max 10MB)'
                              , self.setInputFiles)
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
                'CPUTime': 'setCPUTime',
                'priority': 'setJobPriority',
                'query': 'setQuery',
                'type': 'setDataType',
                'experiments': 'setExperiments',
                'swver': 'setSwVer',
                'inputfiles': 'setInputFiles',
                }
      # read the options
            f = open(self.steering_file)
            for line in f:
                for option in options.keys():
                    if option in line[0:len(option)]:
                        setFunction = getattr(self, options[option])
                        print setFunction
                        print line
                        setFunction(line.split('=', 1)[1].strip().replace('"',
                                    '').replace("'", ''))


