#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------------------
# Check front-end mappings (boardstacks to SCROD IDs)
#
# usage: basf2 checkFrontEndMaps.py expNo runNo [globalTag or localDB]
# -------------------------------------------------------------------------------------

from basf2 import conditions, create_path, process, Module, set_log_level, LogLevel
from ROOT import Belle2
import sys

argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "expNo runNo [globalTag or localDB]")
    sys.exit()

expNo = int(argvs[1])
runNo = int(argvs[2])
tag = '(main)'
tagtxt = 'globalTag:'
if len(argvs) == 4:
    tag = argvs[3]
    if '.txt' in tag:
        conditions.append_globaltag('online')
        conditions.append_testing_payloads(tag)
        tagtxt = 'localDB:'
    else:
        conditions.append_globaltag(tag)


class CheckFrontEndMaps(Module):
    """ Print SCROD to boardstack mapping given by TOPFrontEndMaps """

    def initialize(self):
        """ initialize method: prints SCROD mappings """

        mapper = Belle2.TOP.TOPGeometryPar.Instance().getFrontEndMapper()
        if not mapper.isValid():
            return

        print()
        print('expNo:', expNo, 'runNo:', runNo, tagtxt, tag)
        print('Mapping of boardstacks to SCROD IDs')
        for slot in range(1, 17):
            print('slot', slot)
            for bs in range(4):
                femap = mapper.getMap(slot, bs)
                if femap:
                    print('  BS' + str(bs) + ':', femap.getScrodID())
                else:
                    print('  BS' + str(bs) + ': *not available*')


set_log_level(LogLevel.ERROR)

main = create_path()
main.add_module('EventInfoSetter', expList=[expNo], runList=[runNo])
main.add_module('TOPGeometryParInitializer')
main.add_module(CheckFrontEndMaps())

# process single event
process(main)
