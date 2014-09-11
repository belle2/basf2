#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################
# Usage: basf2 pid_ttree_writer.py -i someMDSTFile.root
#
# Creates DEDX_TTree.root with some useful information.
#############################################################

import sys
import math
from basf2 import *

# Some ROOT tools
import ROOT
from ROOT import Belle2
from ROOT import gROOT, AddressOf

# Define a ROOT struct to hold output data in the TTree.
gROOT.ProcessLine('struct TreeStruct {\
    int   event;\
    int   track;\
    int   layer;\
    float dx;\
    float de;\
    float dedx;\
    float doca;\
    float enta;\
    float cellHeight;\
    float cellHalfWidth;\
    float dpvp;\
    float pocaMom;\
    float pocaCos;\
    float pocaIP;\
};'
                  )

from ROOT import TreeStruct


class TreeWriterModule(Module):

    """
    This module writes its output to a ROOT tree.
    Adapted from pxd/validation/PXDValidationTTreeSimHit.py
    """

    def __init__(self):
        """Initialize the module"""

        super(TreeWriterModule, self).__init__()

        ## Output ROOT file.
        self.file = ROOT.TFile('DEDX_TTree.root', 'recreate')
        ## TTree for output data
        self.tree = ROOT.TTree('tree', '')
        ## Instance of EventData class
        self.data = TreeStruct()
        # Declare tree branches
        for key in TreeStruct.__dict__.keys():
            if not '__' in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key
                                 + formstring)

    def event(self):
        """Store TOP and dE/dx info in tree"""

        pids = Belle2.PyStoreArray('DedxCells')
        for pid in pids:
            ncells = pid.size()
            for i in range(0, ncells):
                m_event = pid.eventID()
                m_track = pid.trackID()

                m_layer = pid.getLayer(i)

                m_dx = pid.getDx(i)
                m_de = pid.getDE(i)
                m_dedx = pid.getDedx(i)
                m_doca = pid.getDoca(i)
                m_enta = pid.getEnta(i)
                m_cellHeight = pid.getCellHeight(i)
                m_cellHalfWidth = pid.getCellHalfWidth(i)
                m_dpvp = pid.getDpvp(i)
                m_pocaMom = pid.getPocaMom(i)
                m_pocaCos = pid.getPocaCos(i)
                m_pocaIP = pid.getPocaIP(i)

                self.data.event = m_event
                self.data.track = m_track

                self.data.layer = m_layer

                self.data.dx = m_dx
                self.data.de = m_de
                self.data.dedx = m_dedx
                self.data.doca = m_doca
                self.data.enta = m_enta
                self.data.dpvp = m_dpvp
                self.data.pocaMom = m_pocaMom
                self.data.pocaCos = m_pocaCos
                self.data.pocaIP = m_pocaIP

                self.data.cellHeight = m_cellHeight
                self.data.cellHalfWidth = m_cellHalfWidth

                # Fill tree
                self.file.cd()
                self.tree.Fill()

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()


main = create_path()

main.add_module(register_module('RootInput'))
main.add_module(TreeWriterModule())

process(main)
print statistics
