#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import DIRAC
from DIRAC.Core.DISET.RPCClient import RPCClient
from DIRAC.Core.Base import Script
Script.registerSwitch('', 'Status=', 'Primary status')
Script.registerSwitch('', 'MinorStatus=', 'Secondary status')
Script.registerSwitch('', 'ApplicationStatus=', 'Application status')
Script.registerSwitch('', 'Site=', 'Execution site')
Script.registerSwitch('', 'Owner=', 'Owner (DIRAC nickname)')
Script.registerSwitch('', 'Project=', 'Select jobs for specified job group')
Script.registerSwitch('', 'Date=',
                      'Date in YYYY-MM-DD format, if not specified default is today'
                      )
Script.parseCommandLine(ignoreErrors=True)

status = None
minorStatus = None
appStatus = None
site = None
owner = None
project = None
date = None


def usage():
    print 'Usage: %s [Try -h,--help for more information]' % Script.scriptName
    DIRAC.exit(2)


args = Script.getPositionalArgs()
if args:
    usage()

for switch in Script.getUnprocessedSwitches():
    if switch[0].lower() == 'status':
        status = switch[1]
    elif switch[0].lower() == 'minorstatus':
        minorStatus = switch[1]
    elif switch[0].lower() == 'applicationstatus':
        appStatus = switch[1]
    elif switch[0].lower() == 'site':
        site = switch[1]
    elif switch[0].lower() == 'owner':
        owner = switch[1]
    elif switch[0].lower() == 'project':
        project = switch[1]
    elif switch[0].lower() == 'date':
        date = switch[1]

selDate = date
if not date:
    selDate = '2011-07-03'
condition = {
    'Status': status,
    'MinorStatus': minorStatus,
    'ApplicationStatus': appStatus,
    'Owner': owner,
    'JobGroup': project,
    }
for key in condition.keys():
    if condition[key] is None:
        del condition[key]
monitoring = RPCClient('WorkloadManagement/JobMonitoring', timeout=120)

result = monitoring.getJobs(condition, selDate)

if len(result['Value']) == 0:
    print 'There is no Job selected\n '
else:
    print '%d jobs are selected.\n' % len(result['Value'])
    jobIDs = ' '.join(result['Value'])
    cmd = 'dirac-wms-job-status ' + jobIDs
    f = os.popen(cmd)
    result = f.read()
    f.close()
    print result
