#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to use the logging
# object exposed by the framework
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# show default loglevel
print 'Default loglevel:', logging.log_level

# change loglevel to show also debug messages
logging.log_level = LogLevel.DEBUG
# or alternatively:
set_log_level(LogLevel.DEBUG)
print 'New loglevel:    ', logging.log_level

# change debug level
logging.debug_level = 123
# or alternatively:
set_debug_level(123)

# set specific LogConfig for pxd package:
# log level WARNING, debug level 0, abort on errors
pxd_logging = LogConfig(LogLevel.WARNING, 0)
pxd_logging.abort_level = LogLevel.ERROR
logging.set_package('pxd', pxd_logging)

# set specific log options for svd package:
# debug level 10, print level, message, and file name for info messages
logging.package('svd').debug_level = 10
logging.package('svd').set_info(LogLevel.INFO, LogInfo.LEVEL | LogInfo.MESSAGE
                                | LogInfo.FILE)

# show default loginfo for all levels
for (name, value) in LogLevel.names.items():
    if name != 'default':
        print 'LogInfo for level %8s: %3d' % (name, logging.get_info(value))

# reset logging connections (-> the default log connection to the console is deleted)
logging.reset()
# add logfile for output
logging.add_file('test.log')
# add colored console output
logging.add_console(True)

# It is now possible to send log messages from python to have consistend messages
# print some log messages
B2DEBUG(100, 'Debug Message')
B2INFO('Info Message')
B2WARNING('Warning Message')
B2ERROR('Error Message')
# next line will bail since the default abort-level is FATAL, so leaving it commented
# B2FATAL('Fatal Message')

# show number of log_messages per level
for (level, num) in logging.log_stats.items():
    print 'Messages for level %8s: %2d' % (level.name, num)

# configure logging for the EvtMetaGen module:
# loglevel FATAL and info for FATAL message is file name and line number
evtmetagen = register_module('EvtMetaGen')
evtmetagen.logging.log_level = LogLevel.FATAL
evtmetagen.logging.set_info(LogLevel.FATAL, LogInfo.FILE | LogInfo.LINE)
# or alternatively:
evtmetagen.set_log_level(LogLevel.FATAL)
evtmetagen.set_log_info(LogLevel.FATAL, LogInfo.FILE | LogInfo.LINE)
