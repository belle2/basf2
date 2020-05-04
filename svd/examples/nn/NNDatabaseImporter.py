#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Simple example for testing the import to database.
"""

from basf2 import *
import ROOT
from ROOT import Belle2

use_local_database("localDB_NN_toImport/database_NN_toImport.txt", "localDB_NN_toImport")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# process single event
process(main)

# call the importer class
dbImporter = Belle2.SVDDatabaseImporter()

# import the noises
dbImporter.importSVDHitTimeNeuralNetwork('svd/scripts/SVDTimeNet.xml')
dbImporter.importSVDHitTimeNeuralNetwork('svd/scripts/SVDTimeNet3.xml', True)

print("Neural network data imported.")
