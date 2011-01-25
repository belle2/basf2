#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# gBasf2 - http://b2comp.kek.jp/~twiki/bin/view/Computing/GBasf2
# gBasf2 is the commandline client for submitting grid-based basf2 jobs.
# It uses the DIRAC Distributed Computing Framework to control the jobs.
#
# Tom Fifield (fifieldt@unimelb.edu.au) - 2010-11
#

# we require Amga tools from Polish devs for metadata
from AmgaSearch import AmgaSearch
# handy file/path features
import os
# we're part of DIRAC, yay
import DIRAC
from DIRAC.Core.Base import Script
from DIRAC.Core.Security.Misc import *
from DIRAC.Core.Security import Properties
from DIRAC.FrameworkSystem.Client.ProxyGeneration import generateProxy
from DIRAC.FrameworkSystem.Client.ProxyManagerClient import ProxyManagerClient
# used for commandline and steeringfile option parsing
from gbasf2util import CLIParams
# used to make tar for input sandbox
import tarfile

# make_jdl takes the options defined by the user and and lfn and makes a basic
# JDL file. This is then written into a temporary file in the same directory.
# Returns the path of the JDL


def make_jdl(
    steering_file,
    project,
    CPUTime,
    priority,
    lfn,
    sysconfig,
    swver,
    tar,
    ):

    f = open(project + '-' + os.path.basename(lfn) + '.jdl', 'w')
    f.write('[\n')
    f.write('    Executable = "basf2helper.sh";\n')
    f.write('    Arguments = "' + steering_file + ' ' + swver + '";\n')
    f.write('    JobGroup = ' + project + ';\n')
    f.write('    JobName = ' + os.path.basename(lfn) + ';\n')
    f.write('    PilotType = "private";\n')
    f.write('    SystemConfig = ' + sysconfig + ';\n')
    f.write('    Requirements = Member("VO-belle-' + swver
            + '",other.GlueHostApplicationSoftwareRunTimeEnvironment);\n')
    f.write('    InputSandbox = \n')
    f.write('    {\n')
    f.write('      "' + steering_file + '",\n')
    f.write('      "basf2helper.sh",\n')
    f.write('      "gbasf2util.py",\n')
    f.write('      "gbasf2output.py",\n')
    f.write('      "AmgaClient.py",\n')
    f.write('      "mdinterface.py",\n')
    f.write('      "mdstandalone.py",\n')
    f.write('      "mdclient.py",\n')
    f.write('      "mdparser.py",\n')
    if tar is not None:
        f.write('      "' + tar + '",\n')
    f.write('      "LFN:' + lfn + '"\n')
    f.write('''    };
\
      OutputSandbox =
\
        {
\
            "*.his",
\
            "*.root",
\
            "*.log",
\
            "std.err",
\
            "std.out"
\
        };
\
      StdError = "std.err";
\
      StdOutput = "std.out";
\
      OutputData = "std.out";
\
      MaxCPUTime = '''
            + str(CPUTime) + ';\n\
      Priority = ' + str(priority) + ' ;\n]'
            )
    f.close()
    return project + '-' + os.path.basename(lfn) + '.jdl'


# basic function to make a tar of input files, written to pwd
# FIXME - upload to SandboxSE, return an URL and reuse URL to speed up submission


def make_tar(project, files):
    if files is not None:
        tar = tarfile.open(project + '-inputsandbox.tar.bz2', 'w:bz2')
        for file in files:
            tar.add(file)
        return project + '-inputsandbox.tar.bz2'
    else:
        return None


# gBasf2 takes a number of options - either from the commandline or in a steering file (see
# gbasf2utils.py) and uses them to (currently)
# 1. conduct a metadata query to match appropriate data to work with - a set of LFNs
# 2. performs all necessary tasks to get the user a proxy
# 3. construct a project based on the name provided and appropriate JDLs - presently just 1 per job
# 4. submits the created jdls to the DIRAC Workload Management System


def main():
    exitCode = 0
    errorList = []
    lfns = []

  # setup options
    cliParams = CLIParams()
    if os.environ.has_key('BELLE2_RELEASE'):
        cliParams.setSwVer(os.environ['BELLE2_RELEASE'])
    cliParams.registerCLISwitches()
    Script.disableCS()
    Script.parseCommandLine(ignoreErrors=True)
    cliParams.registerSteeringOptions()

  # setup dirac - import here because it pwns everything
    from DIRAC.Interfaces.API import Dirac
    dirac = Dirac.Dirac()

  # FIXME - think about enableCS here.

  # check for proxy prescence and if not present, make it
  # FIXME - upload proxy for lifetime of certificate
  # FIXME - warn on certificate validity
    proxyinfo = getProxyInfo()
    if not proxyinfo['OK'] or 'username' not in proxyinfo['Value'].keys():
        print 'No proxy found - trying to generate one'
        proxyparams = DIRAC.FrameworkSystem.Client.ProxyGeneration.CLIParams()
        proxyparams.diracGroup = 'belle'
        proxyparams.proxyLifeTime = 604800
        proxyparams.checkClock = True
        proxyparams.debug = True
        retVal = generateProxy(proxyparams)
        if not retVal['OK']:
            print retVal['Message']
            DIRAC.exit(1)
        else:
            proxyinfo = getProxyInfo()
            if not proxyinfo['OK']:
                print 'Error: %s' % proxyinfo['Message']
                DIRAC.exit(1)
    else:
      # We need to enable the configuration service ourselves if we're not generating a proxy
        Script.enableCS()

    proxyProps = proxyinfo['Value']
    userName = proxyProps['username']
    if Properties.PROXY_MANAGEMENT not in proxyProps['groupProperties'] \
        and userName != CS.getUsernameForDN(proxyProps['issuer'])['Value']:
        print "You're trying to manage a proxy that is not yours, without permission!"
        DIRAC.exit(1)

    DNresult = CS.getDNForUsername(userName)
    if not DNresult['OK']:
        print 'Oops %s' % DNresult['Message']
        DIRAC.exit(1)

    dnList = DNresult['Value']
    if not dnList:
        print 'User %s has no DN defined!' % userName
        DIRAC.exit(1)

    # Next, check there's a proxy that's been uploaded
    pmc = ProxyManagerClient()
    PMCresult = pmc.userHasProxy(dnList[0], 'belle')
    if not PMCresult['OK']:
        print 'Could not retrieve the proxy list: %s' % PMCresult['Value']
        DIRAC.exit(1)
    else:
        lifetime = pmc.getUploadedProxyLifeTime(dnList[0], 'belle')
        if not lifetime['OK'] or lifetime['Value'] < 604799:
            uploadResult = pmc.uploadProxy()
            if not uploadResult['OK']:
                print "Couldn't find a valid uploaded proxy, and couldn't upload one %s " \
                    % uploadResult['Value']

    # Finally, try to add the VOMS attributes
    voms = VOMS()
    VOMSresult = voms.setVOMSAttributes(proxyProps['chain'],
            CS.getVOMSAttributeForGroup(proxyparams.diracGroup))
    if not VOMSresult['OK']:
        print 'Warning : Cannot add voms attribute to the proxy'
        print '          Accessing data in the grid storage from the user interface will not be possible.'
        print '          The grid jobs will not be affected.'
    else:
        # dump VOMS proxy to file
        print VOMSresult['Value'].dumpAllToFile(proxyProps['path'])

    # set path of proxy so  AMGA client picks it up
    os.environ['X509_USER_PROXY'] = proxyProps['path']

  # FIXME - check for existing project name, and prevent additions unless forced

  # perform the metadata query
    asearch = AmgaSearch()
    asearch.setDataType(cliParams.getDataType())
    asearch.setExperiments([int(s) for s in
                           cliParams.getExperiments().split(',')])
    asearch.setQuery(cliParams.getQuery())
    asearch.setAttributes(['lfn', 'events'])
    results = asearch.executeAmgaQueryWithAttributes()
    if results == {}:
        print 'Query returned no results - no jobs to submit'
        DIRAC.exit(1)

  # create the input sandbox
    tar = make_tar(cliParams.getProject(), cliParams.getInputFiles())

  # keep track of the number of events submitted
    totalevents = 0
  # for each of the lfns, make a job and submit it
    for result in results:
        jdl = make_jdl(  # Events/sec into CPUSecs
                         # XXX
            cliParams.getSteeringFile(),
            cliParams.getProject(),
            int(float(results[result]['events']) / (cliParams.getEvtPerMin()
                / 60.0)),
            cliParams.getJobPriority(),
            results[result]['lfn'].replace('belle2', 'belle'),
            cliParams.getSysConfig(),
            cliParams.getSwVer(),
            tar,
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

  # print any errors encountered during submission
    for error in errorList:
        print 'ERROR %s' % error

  # clean temporary files
    if exitCode != 2 and tar is not None:
        os.remove(tar)
    else:
        print 'Something went wrong - leaving JDL and sandbox in place'

    print str(totalevents) + 'events to process!'
    print 'Now visit https://kek2-uidev.cc.kek.jp:15043/DIRAC/systems/projects/overview and select Project ' \
        + cliParams.getProject()
    DIRAC.exit(exitCode)


if __name__ == '__main__':
    main()
