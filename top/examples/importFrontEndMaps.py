#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------------------
# Import front-end mappings given by top/data/FrontEndMapping.xml to a local database.
#
# usage: basf2 importFrontEndMaps.py [localDB]
#        localDB: path/name of local database (D = "localDB-FEMaps/localDB.txt")
# -------------------------------------------------------------------------------------

from basf2 import conditions, create_path, process, Module, set_log_level, LogLevel
from ROOT import Belle2
import sys

localDBName = "localDB-FEMaps/localDB.txt"
if len(sys.argv) > 1:
    localDBName = sys.argv[1]

# define a local database (will be created automatically, if doesn't exist)
conditions.expert_settings(save_payloads=localDBName)


class ImportFrontEndMaps(Module):
    """ database importer for TOPFrontEndMaps """

    def initialize(self):
        """ initialize method: import a payload to local database """

        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
        mapper = Belle2.TOP.TOPGeometryPar.Instance().getFrontEndMapper()
        if not mapper.isValid():
            return
        mapper.importPayload(iov)

        print()
        print('Mapping of boardstacks to SCROD IDs')
        for slot in range(1, 17):
            print('slot', slot)
            for bs in range(4):
                femap = mapper.getMap(slot, bs)
                if femap:
                    print('  BS' + str(bs) + ':', femap.getScrodID())
                else:
                    print('  BS' + str(bs) + ': *not available*')
        print('--> FrontEndMaps imported to local database ' + localDBName)


set_log_level(LogLevel.ERROR)

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('TOPGeometryParInitializer', useDB=False)
main.add_module(ImportFrontEndMaps())

# process single event
process(main)
