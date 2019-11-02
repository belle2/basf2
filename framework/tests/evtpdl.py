#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Check reading and writing of evt.pdl files as well as errors when creating new particles.

This script tests that
 - the ROOT.TDatabasePDG instance is in fact an EvtGenDatabasePDG
 - reading a pdl file raises a warning if it's not the original one
 - creating a particle which already exists is forbidden
 - creating particles with whitespace in the name is forbidden
 - writing an evt.pdl results in the same output as the original pdl, except for
   whitespace and additionally defined particles
"""


from tempfile import NamedTemporaryFile
import basf2
import ROOT
from ROOT import Belle2
import re
import os
import sys
import subprocess
import pdg

db = ROOT.TDatabasePDG.Instance()
# create particle which already exists, should print error
db.AddParticle("e-", "electron", 0, False, 0, 0, "duplicate", 11, 0, 0, 0, 0, 0, 0)
# create particle with whitespace in the name, should print error
db.AddParticle("invalid name", "invalid name", 0, False, 0, 0, "custom", 2345, 0, 0, 0, 0, 0, 0)
# and also create a particle with the python function: should give error due to
# dublicate pdg code
pdg.add_particle("test2", 1, 5.28, 0.1, 0, 0.5)
pdg.add_particle("foo\tbar", 10001, 0, 0, 0, 0)

# default evt.pdl filename
default_evtpdl = Belle2.FileSystem.findFile(os.path.join("decfiles", "dec", "evt.pdl"))
# create a temporary one and compare
retcode = 0
with NamedTemporaryFile() as tempfile:
    # write a evt.pdl from the EventGenDatabasePDG
    db.WriteEvtGenTable(tempfile.name)
    # and make sure that it's the same as the original one except for
    # whitespace, empty lines and lines starting with a "*"
    file1 = open(default_evtpdl, 'r')
    lines1 = list(filter(lambda x: x[0] != '*' and x != 'end', file1.readlines()))
    file2 = open(tempfile.name, 'r')
    lines2 = list(filter(lambda x: x[0] != '*' and x != 'end', file2.readlines()))
    if len(lines1) != len(lines2):
        retcode = 1
    else:
        for i in range(len(lines1)):
            fields1 = lines1[i].split(None)
            fields2 = lines2[i].split(None)
            if (len(fields1) != len(fields2)):
                retcode = 1
            else:
                if (fields1[0] == 'end' and fields2[0] == 'end'):
                    continue
                if (fields1[0] != fields2[0] or  # add
                    fields1[1] != fields2[1] or  # p
                    fields1[2] != fields2[2] or  # Particle
                    fields1[3] != fields2[3] or  # name
                    int(fields1[4]) != int(fields2[4]) or  # code
                    float(fields1[5]) != float(fields2[5]) or  # mass
                    float(fields1[6]) != float(fields2[6]) or  # width
                    float(fields1[7]) != float(fields2[7]) or  # Delta m
                    int(fields1[8]) != int(fields2[8]) or  # charge
                    int(fields1[9]) != int(fields2[9]) or  # spin
                    float(fields1[10]) != float(fields2[10]) or  # c * tau
                        int(fields1[11]) != int(fields2[11])):  # Pythia code
                    print("Particles differ: %s %s" % (lines1[i], lines2[i]))
                    retcode = 1
    file1.close()
    file2.close()
    # create a new particle
    db.AddParticle("testparticle", "testparticle", 1, False, 2, 3, "custom", 10001, 4, 5, 6, 7, 8, 9)

    # also test creating particle with the python functions
    pdg.add_particle("test2", 10002, 5.28, 0.1, -1, 0.5)
    # write the table again
    tempfile2 = NamedTemporaryFile()
    db.WriteEvtGenTable(tempfile2.name)
    # and make sure that it has the additional particle
    subprocess.call(["diff", tempfile.name, tempfile2.name])
    tempfile2.close()

db.ReadEvtGenTable("/no/such/file/(I hope)")
# and also test the python function
pdg.load("/no/such/file/(again)")

print("now we should have zero entries: ", db.ParticleList().GetEntries())
print("so reread default file...")
db.ReadEvtGenTable()
print("number of entries is now", db.ParticleList().GetEntries() and "positive" or "zero")
sys.exit(retcode)
