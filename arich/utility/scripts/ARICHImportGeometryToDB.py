#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2 as b2
import os

home = os.environ['BELLE2_LOCAL_DIR']

# change if you use different local DB folder (see arich/examples/database... examples of database usage)
# use_local_database(home + "/arich/database/cosmicTest_payloads/cosmicTest_database.txt",
#                    home + "/arich/database/cosmicTest_payloads")
# use_local_database(home + "/arich/database/newGeo/database.txt",
#                   home + "/arich/database/newGeo")
b2.use_local_database("localdb/database.txt", "localdb")


gearbox = b2.register_module('Gearbox')

main = b2.create_path()
main.add_module('EventInfoSetter', evtNumList=1, logLevel=b2.LogLevel.DEBUG)
main.add_module(gearbox)

# import geometry for ARICH only
geometry = b2.register_module('Geometry')
geometry.param('components', ['ARICH'])
geometry.param('payloadIov', [0, 0, -1, -1])
geometry.param('createPayloads', 1)
main.add_module(geometry)
print("Geometry Imported to the database!")
b2.process(main)
