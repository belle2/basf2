#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT
import sys
import argparse

# prevent most dictionary warnings
ROOT.gSystem.Load('libdataobjects.so')

parser = argparse.ArgumentParser()
parser.add_argument('--raw', dest='raw', action='store_true', help='Output raw information on all branches and sub-branches.')
parser.add_argument('ROOTFILE', type=str, help='Input .root file.')
parser.add_argument('TREENAME', default='tree', nargs='?', help='Name of the TTree to analyze')
args = parser.parse_args()

rootFile = ROOT.TFile(args.ROOTFILE)
tree = rootFile.Get(args.TREENAME)
if not tree:
    print("Couldn't find tree: " + args.TREENAME)
    sys.exit(1)


def convert_bytes(bytes):
    bytes = float(bytes)
    if bytes >= 1099511627776:
        terabytes = bytes / 1099511627776
        size = '\033[35m%.2fT' % terabytes
    elif bytes >= 1073741824:
        gigabytes = bytes / 1073741824
        size = '\033[31m%.2fG' % gigabytes
    elif bytes >= 1048576:
        megabytes = bytes / 1048576
        size = '\033[33m%.2fM' % megabytes
    elif bytes >= 1024:
        kilobytes = bytes / 1024
        size = '\033[32m%.2fK' % kilobytes
    else:
        size = '%.2fb' % bytes
    return size + '\033[39m'


if args.raw:
    tree.Print()
else:
    branchesList = tree.GetListOfBranches()
    # this is broken on some platforms?
    #    branchesList.sort(key=lambda branch: branch.GetName())

    # use TObjArray sort instead
    branchesList.Sort()

    print("%s:\t%30s\t%7s\t%7s" % ("#", "Branch Name", "Size", "Compressed Size"))
    print("====================================================================")
    for index, branch in enumerate(branchesList):
        size = convert_bytes(branch.GetTotBytes('*'))
        comp_size = convert_bytes(branch.GetZipBytes('*'))
        print("%i:\t%30s\t%7s\t%7s" % (index + 1, branch.GetName(), size, comp_size))
    print("====================================================================")
    tot = tree.GetTotBytes()
    totzip = tree.GetZipBytes()
    nentries = tree.GetEntries()
    print("\t%30s\t%7s\t%7s" % ("Total", convert_bytes(tot), convert_bytes(totzip)))
    print("\t%30s\t%7s\t%7s" % ("Per entry", convert_bytes(tot / nentries), convert_bytes(totzip / nentries)))
    print("'%s' contains %d entries." % (args.TREENAME, nentries))
