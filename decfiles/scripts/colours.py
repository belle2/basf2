#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# --------------------------------------
# Class holding colours for printing coloured text to terminal and printing functions, also handles the general log
# Written by Phillip Urquijo
# December 2014
# --------------------------------------

import settings
import os

# settings imported from the settings file
logfile = settings.logfile

# handle the log filename
if not settings.overwrite:
    if os.path.exists(logfile):
        for i in range(10000):
            if not os.path.exists(logfile + '.' + str(i)):
                logfile = logfile + '.' + str(i)
                break

# open the log
log = open(logfile, 'w')
failed = False


# --------------------------------------
# Class holding colours for printing coloured text to terminal
# --------------------------------------

class bcolours:
    """
    Class to define colour format of output message
    Parameters:
         HEADER   colour code for header
         OKBLUE   colour code for ok blue message
         OKGREEN  colour code for ok green message
         WARNING  colour code for warning message
         FAIL     colour code for fail message
         ENDC     colour code for end message
    """
    #: colour code for header
    HEADER = '\033[95m'
    #: colour code for ok blue message
    OKBLUE = '\033[94m'
    #: colour code for ok green message
    OKGREEN = '\033[92m'
    #: colour code for warning message
    WARNING = '\033[100;93m'
    #: colour code for fail message
    FAIL = '\033[100;91m'
    #: colour code for end message
    ENDC = '\033[0m'

    def disable(self):
        """
        Disable colour output format
        """
        #: colour code for header
        self.HEADER = ''
        #: colour code for ok blue message
        self.OKBLUE = ''
        #: colour code for ok green message
        self.OKGREEN = ''
        #: colour code for warning message
        self.WARNING = ''
        #: colour code for fail message
        self.FAIL = ''
        #: colour code for end message
        self.ENDC = ''

    def enable(self):
        """
        Enable colour output format
        """
        #: colour code for header
        self.HEADER = ''
        #: colour code for ok blue message
        self.OKBLUE = ''
        #: colour code for ok green message
        self.OKGREEN = ''
        #: colour code for warning message
        self.WARNING = ''
        #: colour code for fail message
        self.FAIL = ''
        #: colour code for end message
        self.ENDC = ''


if not settings.enable_colours:
    bcolours.disable()


# --------------------------------------
# Text Functions
# --------------------------------------

def query(string):
    """
    print a query, wait, at the end of the dots for either fail() or done()
    """

    string = str(string) + (60 - len(str(string))) * '.'
    print(string, end=' ')

    # try to log it
    try:
        log.write(string)
    except BaseException:
        pass

    failed = False


def openlog():
    """
    open the logfile manually again after it has been closed
    """

    globals()['log'] = open(logfile, 'a')
    query('Opening logfile <' + logfile + '>')
    done()


query('Opening logfile <' + logfile + '> and loading colours')


def mesg(string):
    """
    Simple announcing information debug function, in white
    """

    string = str(string)
    print(string)
    log.write(string + '\n')


def stop(string):
    """
    Most serious error announcing function, in red
    """

    string = str(string)
    print(bcolours.FAIL + string + bcolours.ENDC)
    # log it
    log.write(string + '\n')


def done():
    """
    Prints DONE in green letters - to be used in conjunction with query() above
    """

    print(bcolours.OKGREEN + 'DONE' + bcolours.ENDC)

    # try to log it
    try:
        log.write('DONE\n')
    except BaseException:
        pass


def fail(list):
    """
    Prints FAIL in red letters and follows with debug information if they were
    passed in the list[strings] as an argument, to be used in conjunction with
    query() above
    """

    print(bcolours.FAIL + 'FAIL' + bcolours.ENDC)
    log.write('FAIL\n')

    # print the debug info
    if list != []:
        print(bcolours.WARNING + 'Printing debug info:\n')

        # log it
        log.write('Debug info:\n')
        for foo in list:
            print(foo)
            log.write(str(foo) + '\n')
        print(bcolours.ENDC)


def warning(string):
    """
    Prints a yellow warning string that is passed as the argument, moderate problem
    """

    string = str(string)
    # print if runlevel is 1 or higher
    print(bcolours.WARNING + string + bcolours.ENDC)
    log.write(string + '\n')


def closeLog():
    """
    closes log after it's been opened manually
    """
    log.close()


done()
