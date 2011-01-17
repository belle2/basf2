#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# gBasf2 - http://b2comp.kek.jp/~twiki/bin/view/Computing/GBasf2
# gBasf2output is the script that uploads files to the grid and registers them into file and
# metadata catalogs
#
# Tom Fifield (fifieldt@unimelb.edu.au) - 2011-01
#

# we require Amga tools from Polish devs for metadata
from AmgaClient import AmgaClient
# handy file/path features
import os
import glob
# we're part of DIRAC, yay
import DIRAC
from DIRAC.Core.Base import Script
from DIRAC.Core.Security.Misc import *
from DIRAC.Core.Security import Properties
from DIRAC.ResourceStatusSystem.Utilities.CS import *
from DIRAC.FrameworkSystem.Client.ProxyGeneration import generateProxy
from DIRAC.FrameworkSystem.Client.ProxyManagerClient import ProxyManagerClient
from DIRAC.DataManagementSystem.Client.ReplicaManager import ReplicaManager

# used for commandline and steeringfile option parsing
from gbasf2util import CLIParams


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
    Script.enableCS()

  # AMGAClient for metadata
    aclient = AmgaClient()

    proxyinfo = getProxyInfo()
    if not proxyinfo['OK']:
        print 'Problem with proxy'
        DIRAC.exit(1)
    else:
        outputpath = '/belle2/user/belle/' + proxyinfo['Value']['username'] \
            + '/' + cliParams.getProject()

    entries = {}
    if aclient.checkDirectory(outputpath):
        repman = ReplicaManager()
      # loop through the output files, uploading and registering
        se = repman._getSEProximity(getStorageElements()['Value'])['Value'][0]
        print 'trying to use SE: ' + se
        for outputfile in glob.glob('*.txt'):
            repman.putAndRegister(outputpath + '/' + outputfile, outputfile,
                                  se)
        # entries[outputfile] = (['exp', 'run', 'guid', 'lfn', 'status', 'md5', 'adler32'],[cliParams.getExperiments()[0],'0', 'guid:', 'lfn:', 'good', 'abcdef0', 'abcdef0']
            print outputfile
            if not aclient.bulkInsert(outputpath, entries):
                print 'Error inserting metadata'
                DIRAC.exit(1)
    else:
        print 'Error with metadata path'
        DIRAC.exit(1)

  # print any errors encountered during submission
    for error in errorList:
        print 'ERROR %s' % error

  # clean temporary files
    if exitCode != 0:
        print 'Something went wrong'

    DIRAC.exit(exitCode)


if __name__ == '__main__':
    main()
