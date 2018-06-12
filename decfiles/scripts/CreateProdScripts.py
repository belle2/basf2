#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
#  CreateProdScripts.py :
#  Author   Phillip Urquijo, December 2014
#
#  create an mc production file from a decay file
#  warning: this script is under development

version = 'v1'

bkk_first = True
eventid_inbkk = []
sql_first = True
eventid_insql = []
list_of_obsoletes = []
exit_status = 0
list_of_wg = ['SL', 'EWP', 'TCPV', 'HAD', 'CHARM', 'ONIA', 'TAU']

import os
import re
import string
import logging
import sys
import time
import glob
import mmap
from optparse import OptionParser, OptionValueError


class GenericOptionFile(object):

    """
    Class to write in an option file
    """

    def __init__(self):
        """
        Constructor.
        Parameters:
             filename   name of the file
             f          alternative name of the file
        """
        #: name of the file
        self.filename = None
        #: alternative name of the file
        self.f = None

    def __del__(self):
        """
        Destructor.
        """
        self.Close()

    def Close(self):
        """
        Close file.
        """
        if self.f:
            self.f.close()

    def OptionFileName(self):
        """
        Set option file name.
        """
        return self.filename

    def SetFileName(self, filename):
        """
        Set file name.
        """
        self.filename = os.path.normpath(filename + self.suffix)

    def Open(self):
        """
        Open file.
        """
        if self.filename:
            self.f = open(self.filename, 'w')

    def Write(self, lines):
        """
        Write the lines in the file.
        """

        self.f.writelines([l + '\n' for l in lines])

    def WriteHeader(self, eventtype, descriptor):
        """
        Write header of .dec file.
        """
        lines = [
            '{0} file {1} generated: {2}'.format(self.comment, self.filename,
                                                 time.strftime('%a, %d %b %Y %H:%M:%S', time.localtime())),
            '{0}'.format(self.comment),
            '{0} Event Type: {1}'.format(self.comment, eventtype),
            '{0}'.format(self.comment),
            '{0} ASCII decay Descriptor: {1}'.format(self.comment, descriptor),
            '{0}'.format(self.comment),
        ]
        self.Write(lines)

    def AddExtraOptions(self, eventtype):
        """
        Adds extra options.
        """
        self.AddInclude(eventtype.ExtraOptions())

    def AddEventTypeNumber(self, eventtype):
        """
        Adds the Event Type Number.
        """
        self.AddOptionValue('GenerationEventType', eventtype.EventTypeNumber())

    def AddEvtGenUserDecayFile(self, eventtype):
        """
        Set the EvtGen .dec file.
        """
        self.AddOptionValue('EvtGenUserDecayFile',
                            '"$DECFILESROOT/dec/{0}.dec"'.format(eventtype.DecayName()))

    def AddDecayOptions(self, eventtype):
        """
        Specify options for .dec file.
        """
        [self.AddOptionValue('ToolSvc.{0}Decay.{1}'.format(
            eventtype.DecayEngine(), eventtype.DecayOptions().split()[2 * i]),
            eventtype.DecayOptions().split()[2 * i + 1])
         for i in range(len(eventtype.DecayOptions().split()) / 2)]


class TextOptionFile(GenericOptionFile):

    """
    Class to read generic option file in .txt format.

    Attributes:
         comment     comment string
         suffix      suffix string
         true_string true string
         list_begin  open list parenthesis
         list_end    close list parenthesis

    """
    #: comments string
    comment = '//'
    #: suffix string
    suffix = '.opts'
    #: true string
    true_string = 'true'
    #: open list parenthesis
    list_begin = '{'
    #: close list parenthesis
    list_end = '}'

    def AddOptionValue(self, option, value, substitute=False):
        """
        Set the value of option.
        """
        self.Write(['{0} = {1};'.format(option, value)])

    def AddInclude(self, filename):
        """
        Add include statements.
        """
        self.Write(['#include "$DECFILESROOT/prod/{0}.py"'.format(filename)])

    def IncreaseOptionValue(self, option, value):
        """
        Add option string.
        """
        self.Write(['{0} += {1};'.format(option, value)])


class PythonOptionFile(GenericOptionFile):

    """
    Class to read generic option file in .py format.

    Attributes:
         comment     comment string
         suffix      suffix string
         true_string true string
         list_begin  open list parenthesis
         list_end    close list parenthesis

    """
    #: comment string
    comment = '#'
    #: suffix string
    suffix = '.py'
    #: true string
    true_string = 'True'
    #: open list parenthesis
    list_begin = '['
    #: close list parenthesis
    list_end = ']'

    def __init__(self):
        """
        Constructor.

        Attributes:
             list_algorithm list of algorithms
             list_tool      list of tools
        """
        #: list of algorithms
        self.list_algorithm = []
        #: list of tools
        self.list_tool = []
        super(PythonOptionFile, self).__init__()

    def AddOptionValue(self, option, value, substitute=False):
        """
        Add option string.
        """
        value = value.replace('{', '[')
        value = value.replace('}', ']')
        if substitute:
            value = value.replace('true', 'True')
            value = value.replace('false', 'False')
        self.Write(['{0} = {1}'.format(option, value)])

    def IncreaseOptionValue(self, option, value):
        """
        Add option string.
        """
        option = self.ConfigureToolAndAlgo(option)
        self.Write(['{0} += {1}'.format(option, value)])


class EventType:

    """
    Class to hold event type information.

    Attributes:
         MandatoryKeywords list of mandatory keywords for file description
         OptionalKeywords  list of optional keywords for file description
    """
    #: list of mandatory keywords for file description
    MandatoryKeywords = [
        'EventType',
        'Descriptor',
        'NickName',
        'Cuts',
        'Documentation',
        'PhysicsWG',
        'Tested',
        'Responsible',
        'Email',
        'Date',
    ]
    #: list of optional keywords for file description
    OptionalKeywords = [
        'Sample',
        'ExtraOptions',
        'DecayOptions',
        'DecayEngine',
        'CutsOptions',
        'Configuration',
        'ParticleType',
        'Momentum',
        'MomentumRange',
        'Id',
        'Production',
        'FullEventCuts',
        'ParticleValue',
        'ParticleTable',
        'InsertPythonCode',
    ]

    def __init__(self, filename, remove, technology):
        """
        Constructor.

        Attributes:
             DecayFileName     name of decay file
             KeywordDictionary dictionary of keywords
             remove            "remove file" flag - force removing the option file and create a new one
             OptionFile        flag for existence of option file
             technology        specify the langauge of the script
        """

        #: name of decay file
        self.DecayFileName = os.path.normpath(filename)
        #: dictionary of keywords
        self.KeywordDictionary = {}
        #:  "remove file" flag
        self.remove = remove
        #: flag for existence of option file
        self.OptionFile = None
        #: specify the langauge of the script
        self.technology = technology

    def DecodeDecayFile(self):
        """
        Operates deconding of decay file.
        """
        fullstring = ''
        with open(self.DecayFileName, 'rb') as f:
            for line in f:
                # Keep only lines starting with '#'
                if line.startswith('# '):
                    line = line.replace('# ', '', 1)
                    fullstring += line
                elif line.startswith('#'):
                    line = line.lstrip('#')
                    fullstring += line

        pattern1 = r'(?m)(\w+): (.+)'
        pattern2 = r'(Documentation):((?s).*)EndDocumentation'
        pattern3 = r'(InsertPythonCode):((?s).*)EndInsertPythonCode'
        matchObj1 = re.findall(pattern1, fullstring)
        matchObj2 = re.findall(pattern2, fullstring)
        matchObj3 = re.findall(pattern3, fullstring)
        if matchObj1 or matchObj2 or matchObj3:
            for matchobj in matchObj1 + matchObj2 + matchObj3:
                self.KeywordDictionary[matchobj[0]] = matchobj[1]
        keystodelete = []
        # delete keys found if Python Code
        if matchObj3:
            matchObj = re.findall(pattern1,
                                  self.KeywordDictionary['InsertPythonCode'])
            if matchObj:
                for matchobj in matchObj:
                    keystodelete.append(matchobj[0])
        # remove leading and ending spaces, except for python code
        for (k, v) in self.KeywordDictionary.items():
            if k != 'InsertPythonCode':
                self.KeywordDictionary[k] = v.rstrip(' \n').lstrip(' \n')
                if self.KeywordDictionary[k] == '':
                    keystodelete.append(k)
        for k in keystodelete:
            del self.KeywordDictionary[k]

    def Validate(self, obsoletes):
        """
        Check for the presence of mandatory keywords.
        """
        missing_mandatory = set(self.MandatoryKeywords) \
            - set(self.KeywordDictionary.keys())
        if len(missing_mandatory) != 0:
            logging.error('%s.dec is missing mandatory keywords: %s',
                          self.DecayName(), [key for key in missing_mandatory])
            raise SyntaxWarning
        unknown_keywords = set(self.KeywordDictionary.keys()) \
            - (set(self.MandatoryKeywords) | set(self.OptionalKeywords))
        if len(unknown_keywords) != 0:
            logging.error('%s.dec contains unknown keywords: %s',
                          self.DecayName(), [key for key in unknown_keywords])
            raise SyntaxWarning

        # check if the nickname is correct
        if self.NickName() != self.DecayName():
            logging.error('In %s, the nickname %s is not equal to the name of the file %s.',
                          self.DecayFileName,
                          self.KeywordDictionary['NickName'], self.DecayName())
            raise UserWarning

        # check if the date format is correct
        try:
            thetime = time.strptime(self.Date(), '%Y%m%d')
        except ValueError:
            logging.error('In %s, the date format is not correct, it should be YYYYMMDD instead of %s.',
                          self.DecayFileName, self.Date())
            raise UserWarning

        # check physics wg name
        if self.PhysicsWG() not in list_of_wg:
            logging.error('In %s, the name of the WG is not correct: %s.',
                          self.DecayFileName, self.PhysicsWG())

        # check the event type does not start with 0
        if self.EventTypeNumber()[0] == '0':
            logging.error('The EventType is not correct in %s.',
                          self.DecayFileName)
            logging.error('It cannot start with 0.')
            raise UserWarning

        # check the event type has at least 8 digits
        if len(self.EventTypeNumber()) < 8:
            logging.error('The EventType is not correct in %s.',
                          self.DecayFileName)
            logging.error('It must have at least 8 digits.')
            raise UserWarning

        # check if the event is obsolete
        if self.EventTypeNumber() in obsoletes:
            logging.error('The EventType %s is in use in the obsolete list, please change it.', self.EventTypeNumber())
            raise UserWarning

        # check Tested is equal to Yes or No
        self.KeywordDictionary['Tested'] = self.KeywordDictionary['Tested'].lower()
        if self.KeywordDictionary['Tested'] != 'yes' and self.KeywordDictionary['Tested'] != 'no':
            logging.error('In %s, Tested should be equal to Yes or No', self.DecayName())
            raise UserWarning

        # check that the file has been tested (check can be disabled with --force option)
        if self.KeywordDictionary['Tested'] == 'no':
            logging.error('The decay file %s has not been tested',
                          self.DecayName())
            raise SyntaxWarning

    def EventTypeNumber(self):
        """
        Returns event type number.
        """
        return self.KeywordDictionary['EventType'].replace(' ', '')

    def G(self):
        """
        Returns general flag of event type.
        """
        return self.EventTypeNumber()[0]

    def S(self):
        """
        Returns selection flag of event type.
        """
        return self.EventTypeNumber()[1]

    def D(self):
        """
        Returns the decay flag of even type.
        """
        return self.EventTypeNumber()[2]

    def C(self):
        """
        Returns the charm flag of even type.
        """
        return self.EventTypeNumber()[3]

    def L(self):
        """
        Returns the lepton flag of even type.
        """
        return self.EventTypeNumber()[4]

    def T(self):
        """
        Returns the track flag of even type.
        """
        return self.EventTypeNumber()[5]

    def N(self):
        """
        Returns the neutral flag of even type.
        """
        return self.EventTypeNumber()[6]

    def K(self):
        """
        Returns the neutral Kaons flag of even type.
        """
        return self.EventTypeNumber()[7]

    def E(self):
        """
        Returns the extra flag of even type.
        """
        return self.EventTypeNumber()[8]

    def U(self):
        """
        Returns the user flag of even type.
        """
        return self.EventTypeNumber()[9]

    def IsSpecialSource(self):
        """
        Check whether it is a special source.
        """
        return self.EventTypeNumber()[0] == '6' and self.EventTypeNumber()[3] \
            == '5' or self.EventTypeNumber()[0] == '6' \
            and self.EventTypeNumber()[3] == '4'

    def SetOptionFileName(self, filename=None):
        """
        Set name of option file.
        """
        if 'Text' in self.technology:
            self.OptionFile = TextOptionFile()
        else:
            self.OptionFile = PythonOptionFile()

        if not filename:
            filename = '{0}/prod/{1}'.format(os.environ['DECFILESROOT'], self.EventTypeNumber())

        self.OptionFile.SetFileName(filename)
        if os.path.exists(self.OptionFile.OptionFileName()):
            if self.remove:
                os.remove(self.OptionFile.OptionFileName())
            else:
                logging.warning('The file %s already exists.',
                                self.OptionFile.OptionFileName())
                logging.warning('To overwrite it, you should remove it first or run with the --remove option.'
                                )
                raise UserWarning
        self.OptionFile.Open()

    def DecayDescriptor(self):
        """
        Returns decay descriptor.
        """
        return self.KeywordDictionary['Descriptor']

    def HasPythonCodeToInsert(self):
        """
        Check if there is python code to be inserted.
        """
        return 'InsertPythonCode' in self.KeywordDictionary

    def PythonCodeToInsert(self):
        """
        Returns the python code to be inserted.
        """
        return self.KeywordDictionary['InsertPythonCode']

    def HasExtraOptions(self):
        """
        Check if it has extra options.
        """
        return 'ExtraOptions' in self.KeywordDictionary

    def ExtraOptions(self):
        """
        Returns the extra options.
        """
        return self.KeywordDictionary['ExtraOptions']

    def HasDecayEngine(self):
        """
        Check if decay engine has been specified.
        """
        return 'DecayEngine' in self.KeywordDictionary

    def DecayEngine(self):
        """
        Returns the decay engine.
        """
        return self.KeywordDictionary['DecayEngine']

    def HasDecayOptions(self):
        """
        Check if decay options have been specified.
        """
        return 'DecayOptions' in self.KeywordDictionary

    def DecayOptions(self):
        """
        Returns the decay options.
        """
        return self.KeywordDictionary['DecayOptions']

    def HasParticleTable(self):
        """
        Check whether particle table already exists.
        """
        return 'ParticleTable' in self.KeywordDictionary

    def ParticleTable(self):
        """
        Return particle table.
        """
        return self.KeywordDictionary['ParticleTable']

    def HasParticleValue(self):
        """
        Check if particle value parameter has been specified.
        """
        return 'ParticleValue' in self.KeywordDictionary

    def ParticleValue(self):
        """
        Returns particle value.
        """
        return self.KeywordDictionary['ParticleValue']

    def HasConfiguration(self):
        """
        Check if configuration is present in keyword dictionary.
        """
        return 'Configuration' in self.KeywordDictionary

    def Configuration(self):
        """
        Returns configuration.
        """
        return self.KeywordDictionary['Configuration'].split()

    def HasParticleType(self):
        """
        Check if Particle Type is present in keyword dictionary.
        """
        return 'ParticleType' in self.KeywordDictionary

    def ParticleType(self):
        """
        Returns Particle Type.
        """
        return self.KeywordDictionary['ParticleType'].split()

    def HasMomentum(self):
        """
        Check if momentum is present in keyword dictionary.
        """
        return 'Momentum' in self.KeywordDictionary

    def Momentum(self):
        """
        Returns momentum.
        """
        return self.KeywordDictionary['Momentum'].split()

    def HasMomentumRange(self):
        """
        Check if momentum range is present in keyword dictionary.
        """
        return 'MomentumRange' in self.KeywordDictionary

    def MomentumRange(self):
        """
        Returns momentum range.
        """
        return self.KeywordDictionary['MomentumRange'].split()

    def NickName(self):
        """
        Return decay NickName.
        """
        return self.KeywordDictionary['NickName'].replace(' ', '')

    def Date(self):
        """
        Return date.
        """
        return self.KeywordDictionary['Date']

    def PhysicsWG(self):
        """
        Return mode WG.
        """
        return self.KeywordDictionary['PhysicsWG']

    def DecayName(self):
        """
        Return decay name string.
        """
        return os.path.splitext(os.path.split(self.DecayFileName)[1])[0]

    def Sample(self):
        """
        Check if overriden.
        """
        if 'Sample' in self.KeywordDictionary:
            sample = self.KeywordDictionary['Sample']
        elif int(self.EventTypeNumber()[0]) in (1, 2, 3, 7):
            if int(self.EventTypeNumber()[1]) in (0, 9):
                sample = 'Inclusive'
            elif int(self.EventTypeNumber()[0]) == 1 and int(self.EventTypeNumber()[1]) in (1, 2, 3, 6, 7):
                sample = 'Signal'
        else:
            sample = 'otherTreatment'
        return sample

    def Production(self):
        """
        Checks for production algorithm.
        """
        production = 'Pythia'
        if 'Production' in self.KeywordDictionary:
            production = self.KeywordDictionary['Production']
        return production

    def HeaderOptions(self):
        """
        Write the header of the options file.
        """

        self.OptionFile.WriteHeader(self.EventTypeNumber(),
                                    self.DecayDescriptor())


def writeBkkTable(evttypeid, descriptor, nickname):
    """
    Write the file to create the entry in the ORACLE database.
    """

    global bkk_first, eventid_inbkk
    TableName = '../doc/table_event.txt'.format(os.environ['DECFILESROOT'])
    logging.warning(TableName)
    if bkk_first:
        bkk_first = False
        if not os.path.exists(TableName):
            with open(TableName, 'wb') as f:
                line = 'EventTypeID | NickName | Description\n'
                f.write(line)
        else:
            # read the file
            with open(TableName, 'rb') as f:
                for line in f:
                    eventid_inbkk.append(line.split()[0])

    if evttypeid not in eventid_inbkk:
        eventid_inbkk.append(evttypeid)
        nick = nickname[:255]
        desc = descriptor[:255]
        with open(TableName, 'a+') as f:
            line = '{0} | {1} | {2}\n'.format(evttypeid, nick, desc)
            f.write(line)


def writeSQLTable(evttypeid, descriptor, nickname):
    """
    Write the file to create the entry in the  database.
    """

    global sql_first, eventid_insql
    TableName = '../doc/table_event.sql'.format(os.environ['DECFILESROOT'])

    if sql_first:
        sql_first = False
        if not os.path.exists(TableName):
            os.system('touch ' + TableName)
        else:
            # read the file
            with open(TableName, 'rb') as f:
                for line in f:
                    eventid_insql.append(line.split()[2].strip(','))

    if evttypeid not in eventid_insql:
        eventid_insql.append(evttypeid)
        nick = nickname[:255]
        desc = descriptor[:255]
        with open(TableName, 'a+') as f:
            line = 'EVTTYPEID = {0}, DESCRIPTION = "{1}", PRIMARY = "{2}"\n'.format(evttypeid, nick, desc)
            f.write(line)


def readObsoleteTypeTable():
    """
    Read the table of obsolete events.
    """

    filename = 'doc/table_obsolete.sql'.format(os.environ['DECFILESROOT'])
    global list_of_obsoletes
    try:
        with open(filename, 'rb') as f:
            for line in f:
                list_of_obsoletes.append(line.split()[2].strip(','))
                logging.info(' This will be ignored %s', line)
    except IOError:
        logging.warning('No files containing obsolete event types found')
    except IndexError:
        pass


def run_create(dkfile, remove, python, force):
    """
    Create an options file corresponding to a single decay file.
    """

    technology = 'Text'
    if python:
        technology = 'Python'

    eventtype = EventType(dkfile, remove, technology)
    eventtype.DecodeDecayFile()

    logging.info('Creation of production script file for Decay file %s.dec',
                 eventtype.DecayName())
    try:
        eventtype.Validate(list_of_obsoletes)
    except SyntaxWarning:
        if force:
            pass
        else:
            raise UserWarning

    # check if the options file already exist and do not overwrite it
    eventtype.SetOptionFileName()

    # get the first digit of the eventtype
    AB = eventtype.EventTypeNumber()[0:2]
    ABX = eventtype.EventTypeNumber()[0:2] + eventtype.E()
    ABU = eventtype.EventTypeNumber()[0:2] + eventtype.U()

    eventtype.HeaderOptions()

    # Optional lines for all event types ---------------------------------
    # Check if exists ExtraOptions keyword
    if eventtype.HasExtraOptions():
        eventtype.OptionFile.AddExtraOptions(eventtype)

    # Mandatory lines to write -------------------------------------------
    # Event type number
    eventtype.OptionFile.AddEventTypeNumber(eventtype)

    # Decay tool
    if not eventtype.HasDecayEngine():
        eventtype.OptionFile.AddEvtGenUserDecayFile(eventtype)
    else:
        eventtype.OptionFile.AddDecayEngine(eventtype)

    # Generation.SAMPLE.GENERATOR.InclusivePIDList
    # if Inclusive
    if 'Inclusive' in eventtype.Sample():
        if eventtype.G() == '1':
            pidlist = '521, -521, 511, -511, 531, -531, 541, -541, 5122, -5122, 5222, -5222, 5212, -5212, 5112, -5112, ' \
                '5312, -5312, 5322, -5322, 5332, -5332, 5132, -5132, 5232, -5232'
        elif int(eventtype.G()) in (2, 7):
            pidlist = '421, -421, 411, -411, 431, -431, 4122, -4122, 443, 4112, -4112, 4212, -4212, 4222, -4222, 4312, ' \
                '-4312, 4322, -4322, 4332, -4332, 4132, -4132, 4232, -4232, 100443, 441, 10441, 20443, 445, 4214, -4214, ' \
                '4224, -4224, 4314, -4314, 4324, -4324, 4334, -4334, 4412, -4412, 4414,-4414, 4422, -4422, 4424, -4424, 4432, ' \
                '-4432, 4434, -4434, 4444, -4444, 14122, -14122,  14124, -14124, 100441'
        eventtype.OptionFile.AddInclusivePIDList(eventtype, pidlist)
    else:
        # if Type Signal
        listing = {  # tau
                     # Sigma
                     # Lambda
                     # Ks
                     # Xi
                     # Omega
            '10': '521, -521, 511, -511, 531, -531, 541, -541, 5122, -5122, 5222, -5222, 5212, -5212, 5112, -5112, 5312, -5312, '
                  '5322, -5322, 5332, -5332, 5132, -5132, 5232, -5232',
            '11': '511,-511',
            '12': '521,-521',
            '13': '531,-531',
            '14': '541,-541',
            '15': '5122,-5122',
            '19': '521, -521, 511, -511, 531, -531, 541, -541, 5122, -5122, 5332, -5332, 5132, -5132, 5232, -5232',
            '20': '421, -421, 411, -411, 431, -431, 4122, -4122, 443, 4112, -4112, 4212, -4212, 4222, -4222, 4312, -4312, 4322, '
                  '-4322, 4332, -4332, 4132, -4132, 4232, -4232, 100443, 441, 10441, 20443, 445, 4214, -4214, 4224, -4224, 4314, '
                  '-4314, 4324, -4324, 4334, -4334, 4412, -4412, 4414,-4414, 4422, -4422, 4424, -4424, 4432, -4432, 4434, -4434, '
                  '4444, -4444, 14122, -14122,  14124, -14124, 100441',
            '21': '411,-411',
            '22': '421,-421',
            '23': '431,-431',
            '24': '443',
            '25': '4122,-4122',
            '31': '15,-15',
            '32': '3222,-3222',
            '33': '3122,-3122',
            '34': '310',
            '35': '3312,-3312',
            '36': '3334,-3334',
            '70': '421, -421, 411, -411, 431, -431, 4122, -4122, 443, 4112, -4112, 4212, -4212, 4222, -4222, 4312, -4312, 4322, '
                  '-4322, 4332, -4332, 4132, -4132, 4232, -4232, 100443, 441, 10441, 20443, 445, 4214, -4214, 4224, -4224, 4314, '
                  '-4314, 4324, -4324, 4334, -4334, 4412, -4412, 4414,-4414, 4422, -4422, 4424, -4424, 4432, -4432, 4434, -4434, '
                  '4444, -4444, 14122, -14122,  14124, -14124, 100441',
            '71': '411,-411',
            '72': '421,-421',
            '73': '431,-431',
            '74': '443',
            '75': '4122,-4122',
        }
        listingExcited = {
            '270': '413,-413',
            '271': '423,-423',
            '272': '433,-433',
            '273': '435,-435',
            '274': '425,-425',
            '275': '415,-415',
            '276': '10433,-10433',
            '277': '413,-413',
            '770': '413,-413',
            '771': '423,-423',
            '772': '433,-433',
            '280': '100443',
            '281': '9920443',
            '282': '10443',
            '283': '10441',
            '284': '20443',
            '285': '445',
            '286': '441',
            '287': '30443',
            '180': '553',
            '181': '100553',
            '182': '200553',
            '183': '300553',
            '184': '9000553',
            '185': '10551',
            '186': '20553',
            '187': '555',
            '160': '5112,-5112',
            '161': '5212,-5212',
            '162': '5222,-5222',
            '163': '5132,-5132',
            '164': '5232,-5232',
            '165': '5332,-5332',
            '170': '513,-513',
            '171': '523,-523',
            '172': '533,-533',
            '173': '10513,-10513',
            '174': '10523,-10523',
            '175': '10533,-10533',
            '176': '515,-515',
            '177': '525,-525',
            '178': '535,-535',
        }

    # Check if exists ParticleTable keyword
    if eventtype.HasParticleTable():
        eventtype.OptionFile.AddParticleTable(eventtype)

    # Check if exists ParticleValue keyword
    if eventtype.HasParticleValue():
        eventtype.OptionFile.AddParticleValue(eventtype)

    # insert python lines directly
    if technology == 'Python':
        if eventtype.HasPythonCodeToInsert():
            eventtype.OptionFile.Write([eventtype.PythonCodeToInsert()])

    writeBkkTable(eventtype.EventTypeNumber(), eventtype.DecayDescriptor(),
                  eventtype.NickName())
    writeSQLTable(eventtype.EventTypeNumber(), eventtype.DecayDescriptor(),
                  eventtype.NickName())


def run_loop(remove, python, force):
    """
    Loop in the FILES directory to generate the options file.
    """

    files = glob.glob(os.environ['DECFILESROOT'] + '/dec/*.dec')
    for f in files:
        try:
            run_create(f, remove, python, force)
        except UserWarning:
            pass


def CheckFile(option, opt_str, value, parser):
    """
    Check if file exists.
    """

    if not os.path.exists('{0}/dec/{1}.dec'.format(os.environ['DECFILESROOT'],
                                                   value)):
        raise OptionValueError('Decay file %s.dec ' % value + 'does not ' +
                               'exist in the $DECFILESROOT/dec directory')
    setattr(parser.values, option.dest, value)


# ---------------------------------------------------------------------------
# Color formatting
# ---------------------------------------------------------------------------

(
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
) = list(range(8))

# The background is set with 40 plus the number of the color, and the foreground with 30

# These are the sequences need to get colored ouput
RESET_SEQ = "\033[0m"
COLOR_SEQ = "\033[1;%d;40m"
BOLD_SEQ = "\033[1m"

COLORS = {
    'WARNING': YELLOW,
    'INFO': WHITE,
    'DEBUG': BLUE,
    'CRITICAL': YELLOW,
    'ERROR': RED,
}


class ColoredFormatter(logging.Formatter):

    """
    Define color convention for output messages.
    """

    def __init__(self, msg, use_color=True):
        """
        Constructor.

        Attributes:
             use_color use color output flag
        """
        logging.Formatter.__init__(self, msg)

        #: use color output flag
        self.use_color = use_color

    def format(self, record):
        """
        Set output format.
        """
        levelname = record.levelname
        color = COLOR_SEQ % (30 + COLORS[levelname])
        message = logging.Formatter.format(self, record)
        message = message.replace('$RESET', RESET_SEQ).replace('$BOLD', BOLD_SEQ).replace('$COLOR', color)
        return message + RESET_SEQ


# ---------------------------------------------------------------------------
# Main routine
# ---------------------------------------------------------------------------

def main():
    global exit_status
    #: logging.basicConfig(level=logging.DEBUG)
    mylog = logging.StreamHandler()
    logging.getLogger().setLevel(logging.DEBUG)
    mylog.setFormatter(ColoredFormatter('$COLOR$BOLD[%(levelname)-10s]$RESET$COLOR  %(message)s', True))
    logging.getLogger().addHandler(mylog)
    usage = 'usage: %prog [options]'
    parser = OptionParser(usage=usage, version=version)
    parser.add_option(
        '-q',
        '--quiet',
        dest='verbose',
        default=False,
        action='store_false',
        help='switch off info printout',
    )
    parser.add_option('--remove', dest='remove', default=False,
                      action='store_true',
                      help='force the delete of the option file before '
                      'creating a new one, by default existing option '
                      'files are not overwritten')
    parser.add_option(
        '-d',
        '--decay',
        type='string',
        dest='NickName',
        help='name of the nick name of the decay to create option '
        'for, if not specified, loop over all decay files in the '
        'dec directory',
        action='callback',
        callback=CheckFile,
    )
    parser.add_option('--text', dest='python', default=True,
                      action='store_false',
                      help='create text option files instead of python options'
                      )
    parser.add_option('--force', dest='force', default=False,
                      action='store_true',
                      help='force create of option file even when the decay file '
                      'syntax is not correct')

    # Check that the environment variable DECFILESROOT exist otherwise
    # set it to ../dec
    if 'DECFILESROOT' not in os.environ:
        logging.warning('')
        logging.warning('The variable DECFILESROOT is not defined.')
        logging.warning('Use ../ instead.')
        logging.warning('Run the setup script of the package to set the correct value.'
                        )
        logging.warning('')
        os.environ['DECFILESROOT'] = '../'
    else:
        logging.info('')
        logging.info('The DECFILESROOT environment variable is set to: %s',
                     os.environ['DECFILESROOT'])
        logging.info('')

    (options, args) = parser.parse_args()

    if not options.verbose:
        logging.getLogger().setLevel(logging.INFO)

    readObsoleteTypeTable()

    if options.NickName:
        try:
            run_create('{0}/dec/{1}.dec'.format(os.environ['DECFILESROOT'],
                                                options.NickName), options.remove, options.python,
                       options.force)
        except UserWarning:
            exit_status = 1
            pass
    else:
        run_loop(options.remove, options.python, options.force)


if __name__ == '__main__':
    main()
    sys.exit(exit_status)
