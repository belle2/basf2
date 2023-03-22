#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

def scanTTree(filename):
    from ROOT import TFile

    tfile = TFile(filename, "READ")
    print(f"TFile: {filename}")

    # get lists of TTree in the filename
    ttrees = [key.GetName() for key in tfile.GetListOfKeys() if key.GetClassName() == "TTree"]

    for ttree_name in ttrees:
        print(f"TTree: {ttree_name}")

        # get TTree object
        ttree = tfile.Get(ttree_name)
        ttree.GetEntry(0)
        # print name and value of all TBranch in the TTree
        for branch in ttree.GetListOfBranches():
            branch_name = branch.GetName()
            branch_value = getattr(ttree, branch.GetName())
            print(f"TBranch: {branch_name}, {branch_value}")

    tfile.Close()
