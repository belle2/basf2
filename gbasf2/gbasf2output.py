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
# from DIRAC.ResourceStatusSystem.Utilities.CS import *
from DIRAC.ConfigurationSystem.Client.Config import gConfig
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
      #  ses = getStorageElements()['Value']
        ses = gConfig.getSections('/Resources/StorageElements')['Value']
        ses.remove('SandboxSE')
        ses.remove('CentralSE')
        print ses
        selist = repman._getSEProximity(ses)
        print selist
        se = selist['Value'][0]
        print 'trying to use SE: ' + se
        # XXX need a better way to determine output files
        for outputfile in glob.glob('*.root'):
            lfn = outputpath + '/' + outputfile
            print 'trying to upload/register ' + lfn
            # XXX - we're using belle VO for belle2 data. Badness.
            cr_result = repman.putAndRegister(lfn.replace('belle2', 'belle'),
                    outputfile, se)
            if not cr_result['OK']:
                print cr_result
                DIRAC.exit(1)
            else:
                cr_result = cr_result['Value']['Successful'
                        ][lfn.replace('belle2', 'belle')]
                # XXX - need to get guid/checksum in another call, they are not
                #       returned by putAndRegister
                # entries[lfn] = (['lfn', 'guid', 'adler32'], [lfn,
                #                cr_result['GUID'], cr_result['Addler']])
                entries[outputfile] = (['lfn'], [lfn])
                try:
                    mfile = open(outputfile.rsplit('.', 1)[0] + '.metadata')
                    for line in mfile:
                        line_parts = line.split(': ', 1)
                        # make sure we have both a key and a value!
                        if len(line_parts) == 2:
                            entries[outputfile][0].append(line_parts[0])
                            entries[outputfile][1].append(line_parts[1].rstrip())
                except IOError:
                    print 'no metadata file, using defaults'
        print entries
        # if this isn't an existing dataset, we need to set the attributes
        if len(aclient.getAttributes(outputpath)[0]) == 0:
            aclient.prepareUserDataset(outputpath)
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
