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
Create a full set of consistent geometry payloads for reduced CDC geometry from XML files.
"""

import basf2
import shutil
import re
import os
import subprocess

# only keep the payloads which are different from the default ones.
interestedPayloads = ["GeoConfiguration", "CDCGeometry"]

# remove existing local database
shutil.rmtree("localdb", ignore_errors=True)

# create new geometry with reduced CDC. For this we need to manually
# remove the default CDC and add the reduced CDC instead. Only for exp=0.
reducedCDCpath = basf2.create_path()
reducedCDCpath.add_module("EventInfoSetter")
reducedCDCpath.add_module("Gearbox")
reducedCDCpath.add_module("Geometry", createPayloads=True, payloadIov=[0, 0, 0, -1],
                          excludedComponents=['CDC'], additionalComponents=['CDCReduced'])
basf2.process(reducedCDCpath)

# most of the components are identical so we avoid uploading two revisions
# so we remove most of them. We only need separate payloads for the for
# some of them
replacementPayloads = ["GeoConfiguration", "CDCGeometry"]

database_content = []
line_match = re.compile(r"^dbstore/(.*?) ([0-9a-f]+) ([0-9\-,]*)$")
keep = set()
with open("localdb/database.txt") as dbfile:
    for line in dbfile:
        match = line_match.search(line)
        name, revision, iov = match.groups()
        # do we want to keep that payload at all?
        if interestedPayloads and name not in interestedPayloads:
            continue
        # if so check whether we can unify the payloads
        iov = tuple(int(e) for e in iov.split(','))
        if iov[0] == 0:
            if name in replacementPayloads:
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

# @endcond
