#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# gBasf2 - http://b2comp.kek.jp/~twiki/bin/view/Computing/GBasf2
# gBasf2output is the script that uploads files to the grid and registers them into file and
# metadata catalogs
#
# Tom Fifield (fifieldt@unimelb.edu.au) - 2011-01
#

import os
import glob
from DIRAC.Core.Base import Script
from gbasf2util import CLIParams

# parse options
cliParams = CLIParams()
if os.environ.has_key('BELLE2_RELEASE'):
    cliParams.setSwVer(os.environ['BELLE2_RELEASE'])
cliParams.registerCLISwitches()
Script.parseCommandLine(ignoreErrors=True)
cliParams.registerSteeringOptions()

import DIRAC
from DIRAC import gLogger
from DIRAC.Core.Security.Misc import *
from DIRAC.ConfigurationSystem.Client.Config import gConfig
from DIRAC.DataManagementSystem.Client.ReplicaManager import ReplicaManager
from AmgaClient import AmgaClient

gLogger.setLevel(cliParams.getLogLevel())


def main():
    exitCode = 0
    errorList = []
    lfns = []

    # get the proxy info and check whether it's OK.
    proxyinfo = getProxyInfo()
    if not proxyinfo['OK']:
        gLogger.error('Problem with proxy')
        DIRAC.exit(1)
    else:
        outputpath = '/belle2/user/belle/' + proxyinfo['Value']['username'] \
            + '/' + cliParams.getProject()
        gLogger.debug('The output path is %s' % outputpath)
    # AMGAClient for metadata
    entries = {}
    aclient = AmgaClient()
    if aclient.checkDirectory(outputpath):
        repman = ReplicaManager()
        # loop through the output files, uploading and registering
        ses = gConfig.getSections('/Resources/StorageElements')['Value']
        # ses.remove('SandboxSE')  #hanyl
        # ses.remove('CentralSE')  #hanyl
        ses.remove('ProductionSandboxSE')
        selist = repman._getSEProximity(ses)
        se = selist['Value'][0]
        print 'trying to use SE: ' + se
        inputfiles = []
        try:
            for jdlline in open(glob.glob('../*.jdl')[0].rstrip()):
                if 'LFN' in jdlline:
                    inputfiles.append(os.path.basename(jdlline)[0:-3])
            gLogger.debug('The inputfiles with lfn are %s' % str(inputfiles))
            gLogger.debug('The inputfiles written in the jdl is %s'
                          % str(cliParams.getInputFiles()))
        except:
            gLogger.info('There is no jdl file in the parent directory')
            pass

        # XXX need a better way to determine output files
        for outputfile in glob.glob('*.root'):
            gLogger.debug('the outputfile is %s' % outputfile)
            if outputfile in cliParams.getInputFiles() or outputfile \
                in inputfiles:
                continue
            lfn = outputpath + '/' + outputfile
            print 'trying to upload/register ' + lfn
            # XXX - we're using belle VO for belle2 data. Badness.
            cr_result = repman.putAndRegister(lfn.replace('belle2', 'belle'),
                    outputfile, se)
            if not cr_result['OK']:
                gLogger.error(cr_result)
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
        # if this isn't an existing dataset, we need to set the attributes
        if len(aclient.getAttributes(outputpath)[0]) == 0:
            aclient.prepareUserDataset(outputpath)
        if not aclient.bulkInsert(outputpath, entries):
            gLogger.error('Error inserting metadata')
            DIRAC.exit(1)
    else:
        gLogger.error('Error with metadata path')
        DIRAC.exit(1)

  # print any errors encountered during submission
    for error in errorList:
        gLogger.error('ERROR %s' % error)

  # clean temporary files
    if exitCode != 0:
        gLogger.error('Something went wrong')

    DIRAC.exit(exitCode)


if __name__ == '__main__':
    main()
