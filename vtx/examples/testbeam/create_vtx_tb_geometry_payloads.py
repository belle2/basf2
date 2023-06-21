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
Create a full set of consistent geometry payloads for VTX testbeam from XML files.
List of (exclusive) upgrade detector options: "VTX-CMOS-testbeam-2020-03-12", ...

Usage: basf2 create_vtx_tb_geometry_payloads.py (-- --magnetON --vtx=VTX-CMOS-testbeam-2020-03-12)
"""

import basf2
import shutil
import re
import os
import subprocess


import argparse
ap = argparse.ArgumentParser()
ap.add_argument("--vtx", default="VTX-CMOS-testbeam-2020-03-12", help="VTX geometry variant from xml")
ap.add_argument('--magnetON', default=False, action='store_true', help="Belle II Magneticfield ON")
args = vars(ap.parse_args())

print("INFO: Create geometry payloads for " + args['vtx'])

excluded_parts = [
    'PXD',
    'SVD',
    'CDC',
    'ECL',
    'ARICH',
    'TOP',
    'KLM',
    'COIL',
    'STR',
    'ServiceGapsMaterial',
    'BeamPipe',
    'Cryostat',
    'FarBeamLine',
    'HeavyMetalShield',
    'VXDService',
]

if not args['magnetON']:
    excluded_parts.append('MagneticField')


# only keep the payloads which are different from the
# default ones.
interested = ["GeoConfiguration", "VTXGeometryPar"]

# remove existing local database
shutil.rmtree("localdb", ignore_errors=True)


# create upgrade geometry. For this we need to manually add the upgrade
# detectors and remove the old PXD+SVD detectors. Only for exp=0.
upgrade = basf2.create_path()
upgrade.add_module("EventInfoSetter")
upgrade.add_module("Gearbox")
upgrade.add_module("Geometry", createPayloads=True, payloadIov=[0, 0, 0, -1],
                   excludedComponents=excluded_parts,
                   additionalComponents=[args['vtx']])
basf2.process(upgrade)

# most of the components are identical so we avoid uploading two
# revisions so we remove most of them. We only need separate payloads for the
# for some of them
upgrade = ["GeoConfiguration", "VTXGeometryPar"]

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
            if name in upgrade:
                keep.add((name, str(revision)))  # we keep all revision one payloads somehow
                database_content.append(f'dbstore/{name} {revision} 0,0,0,-1\n')
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
