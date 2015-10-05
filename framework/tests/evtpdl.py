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
default_evtpdl = os.path.join(os.environ["BELLE2_EXTERNALS_DIR"], "share", "evtgen", "evt.pdl")
# create a temporary one and compare
with NamedTemporaryFile() as tempfile:
    # write a evt.pdl from the EventGenDatabasePDG
    db.WriteEvtGenTable(tempfile.name)
    # and make sure that it's the same as the original one except for
    # whitespace, empty lines and lines starting with a "*"
    retcode = subprocess.call(["diff", "-w", "-B", "-I^\*.*$", default_evtpdl, tempfile.name])

    # create a new particle
    db.AddParticle("testparticle", "testparticle", 1, False, 2, 3, "custom", 10001, 4, 5, 6, 7, 8, 9)

    # also test creating particle with the python functions
    pdg.add_particle("test2", 10002, 5.28, 0.1, -1, 0.5)
    # write the table again
    db.WriteEvtGenTable(tempfile.name)
    # and make sure that it has the additional particle
    subprocess.call(["diff", "-w", "-B", "-I^\*.*$", default_evtpdl, tempfile.name])

db.ReadEvtGenTable("/no/such/file/(I hope)")
# and also test the python function
pdg.load("/no/such/file/(again)")

print("now we should have zero entries: ", db.ParticleList().GetEntries())
print("so reread default file...")
db.ReadEvtGenTable()
print("number of entries is now", db.ParticleList().GetEntries() and "positive" or "zero")
sys.exit(retcode)
