#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Author : Yanliang Han
# Date:    2011-09-29
# Email:   hanyl@ihep.ac.cn
#

"""
kill some submitted jobs or project
Usage:
 1) kill one job    
    ./gkill.py -j 8059
 2) kill several jobs
    ./gkill.py -j "8058,8059"
 3) kill a project
    ./gkill.py -p <project name> 
"""

import DIRAC
from DIRAC.Core.Base import Script
Script.registerSwitch('j:', 'JobID=', 'Job ID')
Script.registerSwitch('p:', 'Project=', 'Project Name')
Script.parseCommandLine(ignoreErrors=True)


def usage():
    print 'Usage: %s [Try -h,--help for more information]' % Script.scriptName
    DIRAC.exit(2)


args = Script.getPositionalArgs()
if args:
    usage()

# parse option
jobid = None
project = None
jobs = None
for switch in Script.getUnprocessedSwitches():
    if switch[0].lower() == 'j' or switch[0].lower() == 'jobid':
        jobid = switch[1]
    elif switch[0].lower() == 'p' or switch[0].lower() == 'project':
        project = switch[1]

# get the jobs list
if project is not None:
    from DIRAC.Core.DISET.RPCClient import RPCClient
    startDate = '2011-07-04'
    monitoring = RPCClient('WorkloadManagement/JobMonitoring', timeout=120)
    result = monitoring.getJobs({'JobGroup': project}, startDate)
    if result['OK']:
        jobs = ','.join(result['Value'])
        print 'There are %d jobs for the project %s.' % (len(result['Value']),
                project)
        if len(result['Value']) != 0:
            print 'They are %s.' % jobs
    else:
        print str(result['Value'])
        DIRAC.exit(2)
else:
    jobs = jobid

# kill jobs with DIRAC API
from DIRAC.Interfaces.API.Dirac import Dirac
dirac = Dirac()
exitCode = 0
errorList = []
for job in jobs.split(','):
    try:
        job = int(job)
    except Exception, x:
        errorList.append(('Expected integer for jobID', job))
        exitCode = 2
        continue
    result = dirac.kill(job)
    if result['OK']:
        print 'Killed job %s' % job
    else:
        errorList.append((job, result['Message']))
        exitCode = 2

for error in errorList:
    print 'ERROR %s: %s' % error

DIRAC.exit(exitCode)
