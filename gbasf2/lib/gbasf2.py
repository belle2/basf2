#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# gBasf2 - http://b2comp.kek.jp/~twiki/bin/view/Computing/GBasf2
# gBasf2 is the commandline client for submitting grid-based basf2 jobs.
# It uses the DIRAC Distributed Computing Framework to control the jobs.
#
# Tom Fifield (fifieldt@unimelb.edu.au) - 2010-11
#

import os
import tarfile
import DIRAC
from DIRAC.Core.Base import Script
from DIRAC.Core.Security.Misc import *
# used for commandline and steeringfile option parsing
from gbasf2util import CLIParams
from AmgaSearch import AmgaSearch
from DIRAC import gLogger
from util import CheckAndRemoveProjectIfForce, make_jdl, prepareProxy, make_tar


def main():
    ''' gBasf2 takes a number of options - either from the commandline or in a steering file (see
        gbasf2utils.py) and uses them to (currently)
        1. conduct a metadata query to match appropriate data to work with - a set of LFNs
        2. performs all necessary tasks to get the user a proxy
        3. construct a project based on the name provided and appropriate JDLs - presently just 1 per job
        4. submits the created jdls to the DIRAC Workload Management System
    '''

    exitCode = 0
    errorList = []
    lfns = []

  # setup options
    cliParams = CLIParams()
    if os.environ.has_key('BELLE2_RELEASE'):
        cliParams.setSwVer(os.environ['BELLE2_RELEASE'])
    cliParams.registerCLISwitches()
    Script.parseCommandLine(ignoreErrors=True)
    cliParams.registerSteeringOptions()

    gLogger.setLevel(cliParams.getLogLevel())

  # setup dirac - import here because it pwns everything
    from DIRAC.Interfaces.API import Dirac
    dirac = Dirac.Dirac()

  # FIXME - think about enableCS here.
  # completes all necessary steps to setup the proxy we need, or exits out
    proxyinfo = prepareProxy()

    cliParams.validOption()
    status = CheckAndRemoveProjectIfForce(proxyinfo['Value']['username'],
            cliParams.getProject())
  # added the next 2 lines because status[1] was nonetype for some reason
    if status is None:
        status = ('New', 0)

    if cliParams.getDataType == 'gen-mc':
        results = {}
    else:
  # perform the metadata query
        asearch = AmgaSearch()
        asearch.setDataType(cliParams.getDataType())
        if cliParams.getExperiments():
            asearch.setExperiments([int(s) for s in
                                   cliParams.getExperiments().split(',')])
        asearch.setQuery(cliParams.getQuery())
        asearch.setAttributes(['lfn', 'events'])
        asearch.setUserData(cliParams.getUserData())
        asearch.setUsername(proxyinfo['Value']['username'])
        results = asearch.executeAmgaQueryWithAttributes()

  # deal with empty queries
    if results == {}:
        results[0] = {}
        results[0]['lfn'] = 'None'
        results[0]['events'] = ''
        # print 'Query returned no results - do you want to run with no input?'
        # noinput = raw_input('Y/N')
        # if noinput == 'Y':
        #    results[0] = {}
        #    events = raw_input('How many events are you generating?')
        #    try:
        #        int(events)
        #    except ValueError:
        #        print 'Number of events needs to be an integer'
        #        DIRAC.exit(1)

        #    results[0]['lfn'] = 'None'
        #    results[0]['events'] = events
        # elif noinput == 'N':

        #    DIRAC.exit(1)
        # else:
        #    print 'Unhandled value. Exiting'
        #    DIRAC.exit(1)

  # create the input sandbox
    tar = make_tar(cliParams.getProject(), cliParams.getInputFiles())

  # keep track of the number of events submitted
    totalevents = 0
    numberOfJobs = status[1]
    numberofLfns = 1
    tmp = 0
  # for each of the lfns, make a job and submit it
    for result in results:
        tmp += 1
        lfns.append(results[result]['lfn'])
        if results[result]['events'] == '':
            results[result]['events'] = 0
        if tmp < numberofLfns:
            continue
        gLogger.info('The lfn used is %s.' % str(lfns))
        numberOfJobs += 1
        jdl = make_jdl(  # Events/sec into CPUSecs
                         # XXX
                         # results[result]['lfn'].replace('belle2', 'belle'),
            cliParams.makeSteeringFile(lfns, numberOfJobs),
            cliParams.getProject(),
            int(float(results[result]['events']) / (cliParams.getEvtPerMin()
                / 60.0)),
            cliParams.getJobPriority(),
            lfns,
            cliParams.getSysConfig(),
            cliParams.getSwVer(),
            tar,
            numberOfJobs,
            cliParams.getSite(),
            )
        subresult = dirac.submit(jdl)
        if subresult['OK']:
            print 'JobID = %s' % subresult['Value']
            # remove the JDL - we keep it on error
            os.remove(jdl)
            totalevents = totalevents + int(results[result]['events'])
            # Yes, that's right - this is done after submission, so MaxEvents isn't really max!
            if totalevents > cliParams.getMaxEvents():
                print 'Maximum number of events exceeded - skipping the other files'
                break
        else:
            errorList.append('[' + results[result]['lfn'] + '] '
                             + subresult['Message'])
            exitCode = 2
        lfns = []
        tmp = 0

  # print any errors encountered during submission
    for error in errorList:
        print 'ERROR %s' % error

  # clean temporary files
    if exitCode != 2 and tar is not None:
        os.remove(tar)
    else:
        print 'Something went wrong - leaving JDL and sandbox in place'

    print str(totalevents) + ' events to process!'
  # FIXME - retrieve this URL automatically from DIRAC
    print 'Now visit https://dirac.ifj.edu.pl/DIRAC/Belle-Development/belle/systems/projects/overview  and select Project ' \
        + cliParams.getProject()
    DIRAC.exit(exitCode)


if __name__ == '__main__':
    main()
