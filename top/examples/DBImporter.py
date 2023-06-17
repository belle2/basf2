#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------------
# Example of using Database importer
# Note: this example is not meant to be executed, but to see how to write the code
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter

# define local database with write access
b2.conditions.expert_settings(save_payloads="localDB/localDB.txt")


class PayloadImporter(b2.Module):
    ''' Payload importer using TOPDatabaseImporter '''

    def initialize(self):
        ''' Import dummy payloads '''

        dbImporter = TOPDatabaseImporter()
        dbImporter.importDummyCalTimebase(0, 0, 0, -1)
        dbImporter.importDummyCalChannelT0(0, 0, 0, -1)
        dbImporter.importDummyCalModuleT0(0, 0, 0, -1)


# create path
main = b2.create_path()

# Event info setter - execute single event
main.add_module('EventInfoSetter')

# Gearbox
main.add_module('Gearbox')

# Geometry parameters
main.add_module('TOPGeometryParInitializer', useDB=False)

# Importer
main.add_module(PayloadImporter())

# process single event
b2.process(main)
