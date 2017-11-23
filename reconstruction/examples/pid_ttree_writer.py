#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file creates a ROOT file called PID_TTree.root
# that contains some useful information on LL differences,
# momenta, and MC truth for TOP and dE/dx.
#
# Usage: basf2 pid_ttree_writer.py -i output.root
# --> saves PID information from ROOT file generated using
#     example.py (must run example.py first) to ROOT file
#
# Input: mdst file specified by command line arguments
#        (example above uses output.root from example.py)
# Output: PID_TTree.root
#
# Example steering file - 2011 Belle II Collaboration
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
    float lld_dedx;\
    float lld_top;\
    float p;\
    float phi;\
    float costheta;\
    float trackmomentum;\
    int iskaon;\
};'
                  )

from ROOT import TreeStruct

# define the python module to save the PID information


class TreeWriterModule(Module):

    """
    This module writes its output to a ROOT tree.
    Adapted from pxd/validation/PXDValidationTTreeSimHit.py
    """

    def __init__(self):
        """Initialize the module"""

        super(TreeWriterModule, self).__init__()

        #: Output ROOT file.
        self.file = ROOT.TFile('PID_TTree.root', 'recreate')
        #: TTree for output data
        self.tree = ROOT.TTree('tree', '')
        #: Instance of EventData class
        self.data = TreeStruct()
        """ Declare tree branches """
        for key in TreeStruct.__dict__.keys():
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def event(self):
        """Store TOP and dE/dx info in tree"""

        pids = Belle2.PyStoreArray('PIDLikelihoods')
        for pid in pids:
            track = pid.getRelatedFrom('Tracks')
            if track:
                mcpart = track.getRelatedTo('MCParticles')
            try:
                pdg = abs(mcpart.getPDG())
                if pdg != 211 and pdg != 321:
                    continue
                momentumVec = mcpart.getMomentum()
                momentum = momentumVec.Mag()
                if momentum > 3.5:  # cut off
                    continue
                costheta = momentumVec.CosTheta()
                phi = momentumVec.Phi()
                fitresult = track.getTrackFitResultWithClosestMass(Belle2.Const.pion)
                if fitresult:
                    trackmomentum = fitresult.getMomentum().Mag()
                else:
                    trackmomentum = 0.0

                # particle to compare with pions
                selectedpart = Belle2.Const.kaon
                pid_dedx = Belle2.Const.PIDDetectorSet(Belle2.Const.CDC)
                pid_top = Belle2.Const.PIDDetectorSet(Belle2.Const.TOP)
                logl_sel = pid.getLogL(selectedpart, pid_dedx)
                logl_pi = pid.getLogL(Belle2.Const.pion, pid_dedx)
                dedx_DLL = logl_pi - logl_sel

                logl_sel = pid.getLogL(selectedpart, pid_top)
                logl_pi = pid.getLogL(Belle2.Const.pion, pid_top)
                top_DLL = logl_pi - logl_sel

                self.data.lld_dedx = dedx_DLL
                self.data.lld_top = top_DLL
                self.data.p = momentum
                self.data.phi = phi
                self.data.costheta = costheta
                self.data.trackmomentum = trackmomentum
                self.data.iskaon = pdg == 321

                # Fill tree
                self.file.cd()
                self.tree.Fill()
            except:

                # some tracks don't have an mcparticle (fixed now)
                B2WARNING('problems with track <-> mcparticle relations')
                event = Belle2.PyStoreObj('EventMetaData').obj().getEvent()
                print('event: %d, track: %d' % (event, track.getArrayIndex()))

    def terminate(self):
        """ Close the output file."""

        self.file.cd()
        self.file.Write()
        self.file.Close()

# create path
main = create_path()

# use the input file defined via command line
main.add_module(register_module('RootInput'))

# add the python module defined above
main.add_module(TreeWriterModule())

# process events and print call statistics
process(main)
print(statistics)
