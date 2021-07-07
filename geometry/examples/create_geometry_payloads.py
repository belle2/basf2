#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond no_doxygen

"""
Create a full set of consistent geometry payloads for nominal geometry, phase 2
geometry, and early phase 3 gometry from XML files.

Optionally one can give a list of payload names to keep only a subset of payloads
"""

import basf2
import shutil
import sys
import re
import os
import subprocess

# if we have extra arguments only keep the payloads given as arguments
interested = sys.argv[1:]

# remove existing local database
shutil.rmtree("localdb", ignore_errors=True)

# create phase3 geometry: This is the default in Belle2.xml
phase3 = basf2.Path()
phase3.add_module("EventInfoSetter")
phase3.add_module("Gearbox")
phase3.add_module("Geometry", createPayloads=True, payloadIov=[0, 0, -1, -1])
basf2.process(phase3)

# create phase2 geometry. For this we need to manually add all the phase2
# detectors and remove the phase3-only detectors.
phase2_detectors = "BeamPipe PXD SVD ServiceGapsMaterial MICROTPC PINDIODE BEAMABORT HE3TUBE CLAWS FANGS PLUME QCSMONITOR".split()
phase2 = basf2.create_path()
phase2.add_module("EventInfoSetter")
phase2.add_module("Gearbox")
phase2.add_module("Geometry", createPayloads=True, payloadIov=[1002, 0, 1002, -1],
                  excludedComponents=["BeamPipe", "PXD", "SVD", "VXDService", "ServiceGapsMaterial"],
                  additionalComponents=[e + "-phase2" for e in phase2_detectors])
basf2.process(phase2)

# create early phase3 geometry: same as phase 3 but different PXD configuration
early3 = basf2.Path()
early3.add_module("EventInfoSetter")
early3.add_module("Gearbox")
early3.add_module("Geometry", createPayloads=True, payloadIov=[1003, 0, 1003, -1],
                  excludedComponents=['PXD', "ServiceGapsMaterial"],
                  additionalComponents=['PXD-earlyPhase3', 'ServiceGapsMaterial-earlyPhase3'])
basf2.process(early3)

# most of the components are identical so we avoid uploading two
# revisions so we remove most of them. We only need separate payloads for the
# for some of them
phase2 = ["GeoConfiguration", "PXDGeometryPar", "SVDGeometryPar", "BeamPipeGeo", "ServiceGapsMaterialsPar"]
# once we have a different ServiceGapsMaterial configuration for phase 3 we
# want to put that here as well
early3 = ["PXDGeometryPar"]
database_content = []
line_match = re.compile(r"^dbstore/(.*?) ([0-9a-f]+) ([0-9\-,]*)$")
keep = set()
with open("localdb/database.txt") as dbfile:
    for line in dbfile:
        match = line_match.search(line)
        name, revision, iov = match.groups()
        # do we want to keep that payload at all?
        if interested and name not in interested:
            continue
        # if so check whether we can unify the payloads
        iov = tuple(int(e) for e in iov.split(','))
        if iov[0] == 0:
            if name in phase2:
                keep.add((name, str(revision)))  # we keep all revision one payloads somehow
                database_content.append(f'dbstore/{name} {revision} 0,0,0,-1\n')
                if name not in early3:
                    database_content.append(f'dbstore/{name} {revision} 1003,0,1003,-1\n')
                # luckily nothing we have in early phase 3 is identical between
                # phase 3 and phase 2 so need for extra checks if in early3 but
                # not phase2
                continue
        elif iov[0] == 1002 and name not in phase2:
            continue
        elif iov[0] == 1003 and name not in early3:
            continue

        # otherwise keep as it is ...
        keep.add((name, str(revision)))
        database_content.append(line)

# and write new database file
database_content.sort()
with open("localdb/database.txt", "w") as dbfile:
    dbfile.writelines(database_content)

# Ok, finally remove all unneeded payload files ...
for filename in os.scandir('localdb/'):
    match = re.match(r"dbstore_(.*?)_rev_(\d*).root", filename.name)
    if not match:
        continue
    if match and match.groups() not in keep:
        print(f"Removing {filename.name}: not needed")
        os.unlink(filename.path)
    else:
        print(f"Normalizing {filename.name} as '{match.group(1)}'")
        subprocess.call(["b2file-normalize", "-i", "-n", match.group(1), filename.path])

# @endcond
