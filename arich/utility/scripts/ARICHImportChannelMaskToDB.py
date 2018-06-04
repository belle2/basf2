#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Import channel mask from a histogram to conditions database
# ARICHImportChannelMaskToDB.py  -f a.root --hname ARICH/chHit --firstexp=3 --firstrun=2300 --lastexp=4 --lastrun=10
#
from basf2 import *
import ROOT
from ROOT.Belle2 import ARICHDatabaseImporter

from optparse import OptionParser

# parameters
parser = OptionParser()
parser.add_option('-f', '--file', dest='filename', default='DQMhistograms.root')
parser.add_option('--hname', dest='hname', default='ARICHDQM/chHit')
parser.add_option('-e', '--firstexp', dest='firstexp', default=0, type="int", help="IntevalOfValidity first experiment")
parser.add_option('--lastexp', dest='lastexp', default=-1, type="int", help="IntevalOfValidity last experiment")
parser.add_option('-r', '--firstrun', dest='firstrun', default=0, type="int", help="IntevalOfValidity first run")
parser.add_option('--lastrun', dest='lastrun', default=-1, type="int", help="IntevalOfValidity last run")
(options, args) = parser.parse_args()

# set local database folder
use_local_database("localdb/database.txt", "localdb", 0, LogLevel.INFO)

f = ROOT.TFile(options.filename)
f.ls()
h = f.Get(options.hname)
h.Print()
print(type(h))
# and run the importer
ARICHDatabaseImporter().importChannelMask(h, options.firstexp, options.firstrun, options.lastexp, options.lastrun)
