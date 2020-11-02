#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Simple example for testing the import to database.
"""

import basf2 as b2
from ROOT import Belle2

main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# process single event
b2.process(main)

# call the importer class
dbImporter = Belle2.SVDDatabaseImporter()

# import the noises
dbImporter.importSVDHitTimeNeuralNetwork('svd/scripts/SVDTimeNet.xml')
dbImporter.importSVDHitTimeNeuralNetwork('svd/scripts/SVDTimeNet3.xml', True)

print("Neural network data imported.")
