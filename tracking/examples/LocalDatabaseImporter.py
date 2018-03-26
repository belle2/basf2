from basf2 import *
import ROOT
from ROOT.Belle2 import TrackingDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

set_log_level(LogLevel.INFO)
reset_database()
use_local_database("localdb/database.txt", "localdb")
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()
paramloader = register_module('Gearbox')
main = create_path()
main.add_module(eventinfo)
main.add_module(paramloader)
process(main)

# and run the importer
dbImporter = TrackingDatabaseImporter()
# dbImporter.importMuidParameters()

dbImporter.exportMuidParameters()
