#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Import channel mask from a histogram to conditions database
# ARICHImportChannelMaskToDB.py  -f a.root --hname ARICH/chHit --firstexp=3 --firstrun=2300 --lastexp=4 --lastrun=10
#
from basf2 import *
import ROOT
from ROOT.Belle2 import ARICHDatabaseImporter

from optparse import OptionParser


def GetChannelMask(h):
    # Explicitly copy
    nbins = h.GetNbinsX()
    xlow = h.GetBinLowEdge(1)
    xup = h.GetBinLowEdge(nbins + 1)
    mf = ROOT.TF1('mf', 'pol0')
    h.Fit(mf, 'LQ0')

    hmask = ROOT.TH1S("ARICHChannelMask", "ARICHChannelMask", nbins, xlow, xup)
    nhot = 0
    ndead = 0
    maxHits = int(mf.GetParameter(0) * 20)
    minHits = 1
    for bin in range(nbins + 1):
        value = 1
        if (h.GetBinContent(bin) > maxHits):
            value = 0
            nhot += 1
        if (h.GetBinContent(bin) < minHits):
            value = 0
            ndead += 1
        hmask.SetBinContent(bin, value)
    print('<mean>{}</mean>'.format(mf.GetParameter(0)))
    print('<max>{}</max>'.format(h.GetMaximum()))
    print('<rms>{}</rms>'.format(h.GetRMS()))
    print('<maxHits>{}<maxHits>'.format(maxHits))
    print('<minHits>{}</minHits>'.format(minHits))
    print('<nall>{}</nall>'.format(nbins))
    print('<nhot>{}</nhot>'.format(nhot))
    print('<ndead>{}</ndead>'.format(ndead))
    return hmask


# parameters
parser = OptionParser()
parser.add_option('-f', '--file', dest='filename', default='DQMhistograms.root')
parser.add_option('--hname', dest='hname', default='ARICHDQM/chHit')
parser.add_option('-e', '--firstexp', dest='firstexp', default=0, type="int", help="IntevalOfValidity first experiment")
parser.add_option('--lastexp', dest='lastexp', default=-1, type="int", help="IntevalOfValidity last experiment")
parser.add_option('-r', '--firstrun', dest='firstrun', default=0, type="int", help="IntevalOfValidity first run")
parser.add_option('--lastrun', dest='lastrun', default=-1, type="int", help="IntevalOfValidity last run")
parser.add_option('-d', '--dryrun', dest='dryrun', default=0, type="int", help="do not export to database")
(options, args) = parser.parse_args()

# set local database folder
use_local_database("localdb/database.txt", "localdb", 0, LogLevel.INFO)

print('<runinfo>')
print('<run>{}</run>'.format(options.firstrun))
print('<exp>{}</exp>'.format(options.firstexp))
print('<fname>{}</fname>'.format(options.filename))


# open file, get histogram
f = ROOT.TFile(options.filename)
h = f.Get(options.hname)
print('</runinfo>')

# get channel mask
hmask = GetChannelMask(h)

# run the importer
if (options.dryrun == 0):
    ARICHDatabaseImporter().importChannelMask(hmask, options.firstexp, options.firstrun, options.lastexp, options.lastrun)
