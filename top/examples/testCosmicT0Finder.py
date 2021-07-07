#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------
# Test of performance and validation of TOPCosmicT0Finder with simulation
# Input file can be prepared with top/examples/simGCRData.py
# Output ntuple to cosmicT0FinderNtuple.root
# ---------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
from ROOT import TH1F
from ROOT import gROOT, AddressOf
import math
import ROOT

timeShift = 100  # ns (can be arbitrary chosen)


class ShiftDigits(b2.Module):
    ''' shift digits in time by timeShift'''

    def event(self):
        ''' event processing '''

        digits = Belle2.PyStoreArray('TOPDigits')
        for digit in digits:
            digit.setTime(digit.getTime() + timeShift)


gROOT.ProcessLine('struct TreeStruct {\
   int slot;        /* slot number */ \
   int nfot;        /* number of photons in the slot */ \
   int pdg;         /* PDG code from barHit */ \
   float x;         /* local x from barHit */ \
   float y;         /* local y from barHit */ \
   float z;         /* local z from barHit */ \
   float t;         /* time from barHit */ \
   float p;         /* momentum from barHit */ \
   float theta;     /* momentum theta [deg] from barHit */ \
   float phi;       /* momentum local phi [deg] from barHit */ \
   float dx;        /* extHit - barHit difference in local x */ \
   float dy;        /* extHit - barHit difference in local y */ \
   float dz;        /* extHit - barHit difference in local z */ \
   float dt;        /* extHit - barHit difference in time */ \
   float dp;        /* extHit - barHit difference in momentum */ \
   float dtheta;    /* extHit - barHit difference in theta [deg] */ \
   float dphi;      /* extHit - barHit difference in local phi [deg] */ \
   float t0;        /* t0 determined by TOPCosmicT0Finder */ \
   float t0err;     /* error on t0 */ \
   float timeShift; /* time shift applied to TOPDigits */ \
   float dt0;       /* difference: dt + t0 - timeShift */ \
};')

from ROOT import TreeStruct  # noqa


class Ntuple(b2.Module):
    ''' makes ntuple for MC studies of TOPCosmicT0Finder performance '''

    def initialize(self):
        ''' initialize '''

        #: root file
        self.file = ROOT.TFile('cosmicT0FinderNtuple.root', 'recreate')
        #: root tree
        self.tree = ROOT.TTree('tree', '')
        #: data structure to be written to tree
        self.data = TreeStruct()

        for key in TreeStruct.__dict__.keys():
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

        #: histogram of t0 residuals
        self.reso = TH1F('reso', 'T0 residuals', 200, -1.0, 1.0)
        self.reso.SetXTitle('#Delta t0 [ns]')
        #: histogram of t0 pulls
        self.pull = TH1F('pull', 'T0 pulls', 200, -10.0, 10.0)

    def event(self):
        ''' event processing: fill ntuple and histograms '''

        mcParticles = Belle2.PyStoreArray('MCParticles')
        timeZeros = Belle2.PyStoreArray('TOPTimeZeros')
        for timeZero in timeZeros:
            extHit = timeZero.getRelated('ExtHits')
            if not extHit:
                b2.B2WARNING('no related extHits')
                continue
            barHit = None
            for barhit in mcParticles[0].getRelationsWith('TOPBarHits'):
                if barhit.getModuleID() == extHit.getCopyID():
                    barHit = barhit
            if not barHit:
                b2.B2WARNING('no corresponding TOPBarHit')
                continue

            moduleID = timeZero.getModuleID()
            if extHit.getCopyID() != moduleID:
                b2.B2ERROR('moduleID in extHit differs: ' + str(extHit.getCopyID()) +
                           ' ' + str(moduleID))
            if barHit.getModuleID() != moduleID:
                b2.B2ERROR('moduleID in barHit differs: ' + str(barHit.getModuleID()) +
                           ' ' + str(moduleID))

            self.data.slot = moduleID
            self.data.nfot = timeZero.getNumPhotons()
            self.data.pdg = barHit.getPDG()
            pos = barHit.getLocalPosition()
            self.data.x = pos.x()
            self.data.y = pos.y()
            self.data.z = pos.z()
            self.data.t = barHit.getTime()
            phiBar = math.pi / 2 - math.pi / 8 * (moduleID - 0.5)
            mom = barHit.getMomentum()
            mom.RotateZ(phiBar)
            self.data.p = mom.Mag()
            self.data.theta = math.degrees(mom.Theta())
            self.data.phi = math.degrees(mom.Phi())
            dr = extHit.getPosition() - barHit.getPosition()
            dr.RotateZ(phiBar)
            self.data.dx = dr.x()
            self.data.dy = dr.y()
            self.data.dz = dr.z()
            self.data.dt = extHit.getTOF() - self.data.t
            momExt = extHit.getMomentum()
            momExt.RotateZ(phiBar)
            self.data.dp = momExt.Mag() - self.data.p
            self.data.dtheta = math.degrees(momExt.Theta()) - self.data.theta
            self.data.dphi = math.degrees(momExt.Phi()) - self.data.phi
            self.data.t0 = timeZero.getTime()
            self.data.t0err = timeZero.getError()
            self.data.timeShift = timeShift
            self.data.dt0 = self.data.dt + self.data.t0 - self.data.timeShift
            self.file.cd()
            self.tree.Fill()
            self.reso.Fill(self.data.dt0)
            self.pull.Fill(self.data.dt0/self.data.t0err)
            if timeZero.getChi2().GetEntries() > 0:
                timeZero.getChi2().Write()
                timeZero.getPDF().Write()
                timeZero.getHits().Write()

    def terminate(self):
        ''' terminate: write and close root file '''

        self.file.cd()
        self.file.Write()
        self.file.Close()


# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.WARNING)

# Define a global tag (note: the one given bellow will become out-dated!)
b2.use_central_database('data_reprocessing_proc8')

# Create path
main = b2.create_path()

# input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# shift digits in time
main.add_module(ShiftDigits())

# t0 finder
finder = b2.register_module('TOPCosmicT0Finder')
finder.param('useIncomingTrack', False)  # or True
# finder.param('saveHistograms', True)
main.add_module(finder)

# ntuple
main.add_module(Ntuple())

# Print progress
main.add_module('Progress')

# Process events
b2.process(main)

# Print statistics
print(b2.statistics)
