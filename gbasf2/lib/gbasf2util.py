#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Class to deal with all required parameters for gbasf.

import os
import mdclient
import AmgaClient
import DIRAC
from DIRAC import gLogger
from DIRAC.Core.Base import Script

# initial implementation uses CLI and steering file
# potentially we can also get some parameters from the user environment in the future

# class name is CLIParams to be consistent with DIRAC


class CLIParams:

    '''used to parse options from files or commandline'''

    EvtPerMin = 45
    steering_file = None
    project = 'Ungrouped'
    priority = 0
    query = None
    swver = 'build-2011-07-01'
    sysconfig = 'Belle-v2r2'
    datatype = None
    experiments = None
    inputsandboxfiles = []
    maxevents = None
    numberOfFiles = 1
    userdata = None
    site = None
    LogLevel = 'WARN'

    def __init__(self):  # hanyl added
        self.datatype = None
        self.experiments = None

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
        if arg == 'data' or arg == 'MC' or arg == 'user' or arg == 'gen-mc':
            self.datatype = arg
            return DIRAC.S_OK()
        else:
            return DIRAC.S_ERROR('Data Type can only be data or MC')

    def setExperiments(self, arg):
    # FIXME check experiment meets range and list of valid experiments
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

    def setNumberOfFiles(self, arg):
      # FIXME : check for integer
        if arg > 0:
            self.maxevents = arg.strip()
        else:
            return DIRAC.S_ERROR

    def setUserData(self, arg):
        '''set the project name for user data'''

        self.userdata = arg.strip()
        return DIRAC.S_OK

    def setSite(self, arg):
        '''set the site name to which you want to submit'''

        self.site = arg.strip()
        return DIRAC.S_OK

    def setLogLevel(self, arg):
        '''set the LogLevel'''

        self.LogLevel = arg.strip()
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

    def getNumberOfFiles(self):
        return self.numberOfFiles

    def getUserData(self):
        return self.userdata

    def getLogLevel(self):
        return self.LogLevel

    def getSite(self):
        return self.site

    def showHelp(self, dummy=False):
        """
      Printout help message including a Usage message if defined via setUsageMessage method
      """

        cmdlist = Script.localCfg.commandOptionList
        for iPos in range(len(cmdlist)):
            optionTuple = cmdlist[iPos]
            gLogger.notice('  -%s --%s : %s' % (optionTuple[0].ljust(3),
                           optionTuple[1].ljust(15), optionTuple[2]))
            iLastOpt = iPos
            if optionTuple[0] == 'h':
          # Last general opt is always help
                break
        if iLastOpt + 1 < len(cmdlist):
            gLogger.notice(' \nOptions:')
            for iPos in range(iLastOpt + 1, len(cmdlist)):
                optionTuple = cmdlist[iPos]
                gLogger.notice('  -%s --%s : %s' % (optionTuple[0].ljust(3),
                               optionTuple[1].ljust(15), optionTuple[2]))
        DIRAC.exit(0)

  # registers alll of the possible commandline options with the DIRAC Script handler
  # This is also used to generate the --help option

    def registerCLISwitches(self):
        Script.localCfg.commandOptionList = []  # hanyl clear Script's options
        Script.registerSwitch('h', 'help', 'show the usage', self.showHelp)
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
        Script.registerSwitch('n:', 'numberOfFiles=',
                              '(optional) Number of data files per job',
                              self.setNumberOfFiles)
        Script.registerSwitch('u:', 'userdata=',
                              '(optional) The project naem of user data',
                              self.setUserData)
        Script.registerSwitch('', 'site=',
                              '(optional) The site name to which you want to submit'
                              , self.setSite)
        Script.registerSwitch('', 'LogLevel=', 'Log Level', self.setLogLevel)

        # Script.addDefaultOptionValue('LogLevel', 'debug')

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
                'numberOfFiles': 'setNumberOfFiles',
                'userdata': 'setUserData',
                'LogLevel': 'setLogLevel',
                'site': 'setSite',
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

    def validOption(self):
        """Make sure the datatype and experiments be given.
           If not, the datatype will be set to data and the experiment to all
        """

        if self.getDataType() is None:  # check the datatype
            print 'No datatype is given. We will set datatype to data. Continue?'
            noinput = raw_input('Please type Y or N: ')
            if noinput.upper() == 'N':
                os.sys.exit(-1)
            elif noinput.upper() != 'Y':
                print 'You should give Y or N'
                os.sys.exit(-1)
            self.setDataType('data')
        elif self.getDataType() == 'data' or self.getDataType() == 'MC':
            if self.getExperiments() is None:  # check the experiments
                print 'No experiments is given. We will set experiments to all. Continue?'
                noinput = raw_input('Please type Y or N: ')
                if noinput.upper() == 'N':
                    os.sys.exit(-1)
                elif noinput.upper() != 'Y':
                    print 'You should give Y or N'
                    os.sys.exit(-1)
                tmp = []
                ac = AmgaClient.AmgaClient()
                for t in ac.getSubdirectories('/belle2/data', relative=True):
                    tmp.append(t[-2:])
                self.setExperiments(','.join(tmp))
        if self.getQuery() is None:  # check the query
            print 'No query is given. We will set query to true. Continue?'
            noinput = raw_input('Please type Y or N: ')
            if noinput.upper() == 'N':
                os.sys.exit(-1)
            elif noinput.upper() != 'Y':
                print 'You should give Y or N'
                os.sys.exit(-1)
            self.setQuery('2>1')

    def makeSteeringFile(self, lfns, number):
        """Make steering file for the given lfn by adding the number as a suffix of lfn
           return the new steering file.
        """

        files = []
        for lfn in lfns:
            files.append(os.path.basename(lfn))

        OldSteeringFile = self.getSteeringFile()
        if OldSteeringFile.endswith('.py'):
            NewSteeringFile = OldSteeringFile[:-3] + '-' + str(number) + '.py'
        else:
            NewSteeringFile = OldSteeringFile + '-' + str(number) + '.py'
        fold = open(OldSteeringFile)
        fnew = open(NewSteeringFile, 'w')

        for eachline in fold:
            if not eachline.strip().startswith('#'):
                if eachline.find('outputFileName') > 0:  # the output file
                    head = eachline.split(',')[0]
                    tail = eachline.split(',')[1]
                    tail = tail.split('.')[0] + '-' + str(number) + '.' \
                        + tail.split('.')[1]
                    eachline = head + ',' + tail
                if self.getDataType == 'gen-user':
                    if eachline.find('SimpleInput') > -1:  # close the original input
                        eachline = '#' + eachline
                    if eachline.find('from basf2 import *') > -1:  # the input file
                        if eachline.endswith('\r\n'):
                            eachline += '\r\n'
                            eachline += \
                                "sinput = fw.register_module('SimpleInput')\r\n"
                            if len(files) > 1:
                                eachline += "sinput.param('inputFileName'," \
                                    + str(files) + ')\r\n'
                            else:
                                eachline += "sinput.param('inputFileName','" \
                                    + files[0] + "')\r\n"
                        else:
                            eachline += '\n'
                            eachline += \
                                "sinput = fw.register_module('SimpleInput')\n"
                            if len(files) > 1:
                                eachline += "sinput.param('inputFileName'," \
                                    + str(files) + ')\n'
                            else:
                                eachline += "sinput.param('inputFileName','" \
                                    + files[0] + "')\n"
            fnew.write(eachline)
        for lfn in lfns:
            fnew.write('LFN="%s"\r\n' % lfn)
        fold.close()
        fnew.close()
        return NewSteeringFile


if __name__ == '__main__':
    clipara = CLIParams()
    clipara.setSteeringFile('READER.py')
    # clipara.setSteeringFile('steering-simulationexample-withgrid.py')
    # print clipara.getSteeringFile()
    clipara.makeSteeringFile('hanyl', 2)
