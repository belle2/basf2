#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import tarfile
from AmgaSearch import AmgaSearch
from AmgaClient import AmgaClient
from DIRAC import gLogger
import DIRAC

from DIRAC.FrameworkSystem.Client.ProxyGeneration import generateProxy
from DIRAC.FrameworkSystem.Client.ProxyManagerClient import ProxyManagerClient
from DIRAC.Core.Security.Misc import *
from DIRAC.Core.Security import CS
from DIRAC.Core.Security import Properties


def CheckAndRemoveProjectIfForce(username=None, project=None):

    from DIRAC.Interfaces.API import Dirac
    from DIRAC.Core.Base import Script

    projectStatus = None

    ac = AmgaClient()
    directory = '/belle2/user/belle/' + username + '/' + project
    if ac.checkDirectoryOnly(directory):
        print 'The project has been in the AMGA server. Do you want to give up(G), or ' \
            + 'force to execute it by remove the previous one(R), or add this project the previous(A)?'
        user_input = raw_input('Please enter G, R or A: ')
        if user_input.upper() == 'G':
            print 'You have terminated the project'
            DIRAC.exit(1)
        elif user_input.upper() != 'R' and user_input.upper() != 'A':
            print 'You must eneter G or R or A'
            DIRAC.exit(1)
        else:
            gLogger.debug('The existed directory is %s' % directory)
            files = ac.getSubdirectories(directory)
            gLogger.debug('The existed files in  AMGA is %s' % str(files))
            results = ac.directQueryWithAttributes(directory, '2>1', ['lfn'])
            gLogger.debug('The existed lfns is %s' % str(results))
            if user_input.upper() != 'R':
                Script.parseCommandLine(ignoreErrors=True)
                dirac = Dirac.Dirac()
                for result in results:
                    rm_result = dirac.removeFile(results[result]['lfn'
                            ].replace('belle2', 'belle'))
                    if rm_result['OK']:
                        print 'we have removed %s in the storage element' \
                            % results[result]['lfn'].replace('belle2', 'belle')
                for file in files:
                    if ac.rm(file):
                        print 'We have removed the file %s in AMGA' % file
                    else:
                        print 'Error occur when remove the file %s in AMGA' \
                            % file
                        DIRAC.exit(1)
                if ac.removeDir(directory):
                    print 'We have removed the diretory %s' % directory
                else:
                    print 'Error occur when remove the directory %s' \
                        % directory
                    DIRAC.exit(1)
                projectStatus = ('New', 0)
            else:
                projectStatus = ('Old', len(results))
        return projectStatus


def make_jdl(
    steering_file,
    project,
    CPUTime,
    priority,
    lfns,
    sysconfig,
    swver,
    tar,
    num=0,
    site='',
    ):
    '''make_jdl takes the options defined by the user and and lfn and makes a basic
       JDL file. This is then written into a temporary file in the same directory.
       Returns the path of the JDL
    '''

    if len(lfns) > 1:
        lfn = project + '-' + str(num)
    else:
        lfn = lfns[0]

    import os
    path = os.popen('which gbasf2').read()[:-11] + 'lib/'

    f = open(project + '-' + os.path.basename(lfn) + '.jdl', 'w')
    f.write('[\n')
    f.write('    Executable = "basf2helper.sh";\n')
    f.write('    Arguments = "' + steering_file + ' ' + swver + '";\n')
    f.write('    JobGroup = ' + project + ';\n')
    f.write('    JobName = ' + os.path.basename(lfn) + ';\n')
    f.write('    PilotType = "private";\n')
    f.write('    SystemConfig = ' + sysconfig + ';\n')
    if site != '':
        if site != None:
            f.write('    Site = ' + site + ';\n')
#    f.write('    Requirements = Member("VO-belle-' + swver
#            + '",other.GlueHostApplicationSoftwareRunTimeEnvironment);\n')
    f.write('    InputSandbox = \n')
    f.write('    {\n')
    f.write('      "' + steering_file + '",\n')
    f.write('     ' + path + '"basf2helper.sh",\n')
    f.write('     ' + path + '"gbasf2util.py",\n')
    f.write('     ' + path + '"gbasf2output.py",\n')
    f.write('     ' + path + '"AmgaClient.py",\n')
    f.write('     ' + path + '"mdinterface.py",\n')
    f.write('     ' + path + '"mdstandalone.py",\n')
    f.write('     ' + path + '"mdclient.py",\n')
    f.write('     ' + path + '"mdparser.py",\n')
    if tar is not None:
        f.write('      "' + tar + '",\n')
    # if lfn != 'None':
    for lf in lfns:
        if lf != 'None':
            f.write('      "LFN:' + lf.replace('belle2', 'belle') + '"\n')
    f.write('''    };
\
      OutputSandbox =
\
        {
\
            "basf2.err",
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


def prepareProxy():
    '''check for proxy prescence and if not present, make it, upload it, VOMS it
       FIXME - upload proxy for lifetime of certificate
       FIXME - warn on certificate validity
       return proxy infomation if proxy is generated correctly
    '''

    from DIRAC.Core.Base import Script
    Script.enableCS()
    Script.parseCommandLine(ignoreErrors=True)
    proxyinfo = getProxyInfo()
    timeleft = int(proxyinfo['Value']['secondsLeft'])
    timeleft /= 3600
    print "The proxy will be despired after %s hours. Do you want to continue \
or to generate a new proxy?" \
        % timeleft

    user_choice = raw_input('Please input C or G: ')
    if user_choice.upper() == 'C':
        return proxyinfo
    elif user_choice.upper() != 'G':
        print 'You should give C or G'
        import sys
        sys.exit(-1)

    if not proxyinfo['OK'] or 'username' not in proxyinfo['Value'].keys() \
        or user_choice.upper() == 'G':
        print 'No proxy found - trying to generate one'
        proxyparams = DIRAC.FrameworkSystem.Client.ProxyGeneration.CLIParams()
        proxyparams.diracGroup = 'belle'
        proxyparams.proxyLifeTime = 604800
        proxyparams.checkClock = True
        proxyparams.debug = False
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
            CS.getVOMSAttributeForGroup(proxyparams.diracGroup),
            proxyparams.diracGroup)
    if not VOMSresult['OK']:
        print VOMSresult['Message']
        print 'Warning : Cannot add voms attribute to the proxy'
        print '          Accessing data in the grid storage from the user interface will not be possible.'
        print '          The grid jobs will not be affected.'
    else:
        # dump VOMS proxy to file
        print VOMSresult['Value'].dumpAllToFile(proxyProps['path'])

    # set path of proxy so  AMGA client picks it up
    os.environ['X509_USER_PROXY'] = proxyProps['path']
    return proxyinfo


def make_tar(project, files):
    '''basic function to make a tar of input files, written to pwd
       FIXME - upload to SandboxSE, return an URL and reuse URL to speed up submission
    '''

    if files is not None:
        tar = tarfile.open(project + '-inputsandbox.tar.bz2', 'w:bz2')
        for file in files:
            try:
                tar.add(file)
            except OSError:
                print 'No such input file: ' + file
                DIRAC.exit(1)
        return project + '-inputsandbox.tar.bz2'
    else:
        return None


