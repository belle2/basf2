#!/usr/bin/env python3

import basf2
import shutil
import re

# remove existing local database
shutil.rmtree("localdb", ignore_errors=True)

# create phase3 geometry: This is the default in Belle2.xml
phase3 = basf2.create_path()
phase3.add_module("EventInfoSetter")
phase3.add_module("Gearbox")
phase3.add_module("Geometry", createPayloads=True, payloadIov=[0, 0, -1, -1])
basf2.process(phase3)

# create phase2 geometry. For this we need to manually add all the phase2
# detectors and remove the phase3-only detectors.
phase2_detectors = "BeamPipe PXD SVD MICROTPC PINDIODE BEAMABORT HE3TUBE CLAWS FANGS PLUME QCSMONITOR".split()
phase2 = basf2.create_path()
phase2.add_module("EventInfoSetter")
phase2.add_module("Gearbox")
phase2.add_module("Geometry", createPayloads=True, payloadIov=[1002, 0, 1002, -1],
                  excludedComponents=["BeamPipe", "PXD", "SVD", "VXDService"],
                  additionalComponents=[e + "-phase2" for e in phase2_detectors])
basf2.process(phase2)

# most of the components are identical so we avoid uploading two
# revisions so we remove most of them. We only need separate payloads for the
# general configuration and PXD and SVD
varying = ["GeoConfiguration", "PXDGeometryPar", "SVDGeometryPar"]
database_content = []
line_match = re.compile("^dbstore/(.*?) (\d+)")
with open("localdb/database.txt") as dbfile:
    for line in dbfile:
        match = line_match.search(line)
        if (match.group(1) in varying):
            if match.group(2) == "1":
                # limit phase3 to max experiment 999
                database_content.append("%s 0,0,999,-1\n" % match.group(0))
                continue
        elif match.group(2) != "1":
            continue
        database_content.append(line)

with open("localdb/database.txt", "w") as dbfile:
    dbfile.writelines(database_content)
