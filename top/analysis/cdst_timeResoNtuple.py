#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Makes ntuple w/ variable size arrays to study single photon time resolution with di-muon data
# by comparing photon times w.r.t leading PDF peak position.
# Provides also full information of a PDF peak assocoated with a detected photon.
# The following draw command must show a peak that represents an overall time resolution:
#    tree->Draw("time-t0>>h(100,-1,2)", "t0<10")
#
# Usage: basf2 cdst_timeResoNtuple.py -i <cdst_file-dimuon_skim.root> [mc]
# ---------------------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2
from ROOT import TH1F, TH2F
from ROOT import AddressOf, gROOT
from ROOT import TLorentzVector
import math
import ROOT
import sys

MC = False
if len(sys.argv) > 1:
    MC = True
    file_num = sys.argv[1]


gROOT.ProcessLine('struct TreeStruct {\
   int run;       /* run number */ \
   int itrk;      /* track counter */ \
   float offset;  /* current bunch offset */ \
   int usedTrk;   /* number of tracks used for bunch reconstruction */ \
   int slot;      /* slot ID */ \
   float p;       /* extHit momentum */ \
   float cth;     /* extHit local cos(theta) (global must be pretty much the same) */ \
   float phi;     /* extHit local phi */ \
   float z;       /* extHit local z */ \
   float x;       /* extHit local x */ \
   float tof;     /* extHit time-of-flight */ \
   int charge;    /* particle charge */ \
   float poca_x;  /* POCA x */ \
   float poca_y;  /* POCA y */ \
   float poca_z;  /* POCA z */ \
   int hitsCDC;   /* number of CDC hits */ \
   float dEcms;   /* delta Ecms for muon */ \
   float rec_t0;   /* t0 of this track determined with bunch finder */ \
   int valid_t0;   /* reconstruction status of t0 */ \
   int nfot;      /* number of photons */ \
   int channel[1000];  /* channel */ \
   int pixel[1000] ;    /* pixel ID */ \
   float time[1000];    /* photon time */ \
   float timeErr[1000]; /* photon time uncertainty */ \
   int pulseHeight[1000]; /* photon pulse height */ \
   float pulseWidth[1000]; /* photon pulse width */ \
   int sample[1000];    /* sample number modulo 256 */ \
   int status[1000];    /* calibration status bits */ \
   float t0[1000];      /* leading PDF peak: position */ \
   float wid0[1000];    /* leading PDF peak: width w/o TTS */ \
   float t1[1000];      /* next to leading PDF peak: position */ \
   float t_pdf[1000];   /* associated pdf peak time */ \
   int type[1000];      /* 0 bkg, 1 direct, 2 reflected */ \
   int nx[1000];    /* total number of reflections in x */ \
   int ny[1000];    /* total number of reflections in y */ \
   int nxm[1000];    /* number of reflections in x before mirror */ \
   int nym[1000];    /* number of reflections in y before mirror */ \
   int nxe[1000];    /* number of reflections in x in prism */ \
   int nye[1000];    /* number of reflections in y in prism */ \
   int nys[1000];    /* number of reflections on slanted surface of prism */ \
   float xd[1000];  /* unfolded x coordinate of a pixel */ \
   float yd[1000];  /* unfolded y coordinate of a pixel */ \
   float xm[1000];  /* unfolded x coordinate of a reconstructed point on mirror */ \
   float kx[1000];  /* reconstructed photon direction in x at emission */ \
   float ky[1000];  /* reconstructed photon direction in y at emission */ \
   float alpha[1000];   /* impact angle on photo-cathode [degrees] */ \
};')

int_arrays = ['channel', 'pixel', 'pulseHeight', 'sample', 'status', 'type', 'nx', 'ny',
              'nxm', 'nym', 'nxe', 'nye', 'nys']
float_arrays = ['time', 'timeErr', 'pulseWidth', 't0', 'wid0', 't1', 't_pdf', 'alpha',
                'xd', 'yd', 'xm', 'kx', 'ky']

from ROOT import TreeStruct  # noqa


class Ntuple(b2.Module):
    ''' Makes a flat ntuple '''

    #: histogram counter
    nhisto = 0

    def initialize(self):
        ''' initialize: open root file, construct ntuple '''

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        expNo = evtMetaData.obj().getExperiment()
        runNo = evtMetaData.obj().getRun()
        exp_run = '-e' + '{:0=4d}'.format(expNo) + '-r' + '{:0=5d}'.format(runNo)
        if not MC:
            outName = 'out_data/timeResoNtuple' + exp_run + '.root'
        else:
            outName = 'out_mc/timeResoNtuple' + exp_run + '-' + file_num + '.root'

        #: file object
        self.file = ROOT.TFile(outName, 'recreate')

        #: histogram of bunch offset
        self.bunchOffset = TH1F("bunchOffset", "bunch offset", 100, -1.0, 1.0)
        self.bunchOffset.SetXTitle("bunch offset [ns]")
        #: histogram cos(theta) vs, momentum
        self.h_cth_vs_p = TH2F("cth_vs_p", "local cos #theta vs. p", 100, 0.0, 10.0,
                               100, -1.0, 1.0)
        self.h_cth_vs_p.SetXTitle("p [GeV/c]")
        self.h_cth_vs_p.SetYTitle("cos #theta")
        #: histogram of momentum
        self.h_momentum = TH1F("momentum", "momentum", 100, 0.0, 10.0)
        self.h_momentum.SetXTitle("p [GeV/c]")
        #: histogram of cos(theta)
        self.h_cth = TH1F("cos_theta", "local cos #theta", 100, -1.0, 1.0)
        self.h_cth.SetXTitle("cos #theta")
        #: histogram of local phi
        self.h_phi = TH1F("local_phi", "local phi", 100, -math.pi, math.pi)
        self.h_phi.SetXTitle("local #phi")
        #: histogram of local z
        self.h_z = TH1F("local_z", "local z", 100, -140.0, 140.0)
        self.h_z.SetXTitle("local z [cm]")
        #: histogram of local x
        self.h_x = TH1F("local_x", "local x", 100, -23.0, 23.0)
        self.h_x.SetXTitle("local x [cm]")
        #: histogram of charge
        self.h_charge = TH1F("charge", "charge", 3, -1.5, 1.5)
        self.h_charge.SetXTitle("charge")
        #: histogram of POCA x-y
        self.h_poca_xy = TH2F("poca_xy", "POCA distribution in x-y", 100, -1.0, 1.0,
                              100, -1.0, 1.0)
        self.h_poca_xy.SetXTitle("x [cm]")
        self.h_poca_xy.SetYTitle("y [cm]")
        #: histogram of POCA z
        self.h_poca_z = TH1F("poca_z", "POCA distribution in z", 100, -2.0, 2.0)
        self.h_poca_z.SetXTitle("z [cm]")
        #: histogram of number of hits in CDC
        self.h_hitsCDC = TH1F("cdc_hits", "CDC hits", 100, 0.0, 100.0)
        self.h_hitsCDC.SetXTitle("number of CDC hits")
        #: histogram of Ecms
        self.h_Ecms = TH1F("Ecms", "c.m.s. energy of muon", 300, 4.5, 6.0)
        self.h_Ecms.SetXTitle("E_{cm} [GeV/c]")

        #: tree object
        self.tree = ROOT.TTree('tree', 'time resolution')
        #: data structure
        self.data = TreeStruct()
        self.data.itrk = 0

        for key in TreeStruct.__dict__.keys():
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                if key in int_arrays:
                    formstring = '[nfot]/I'
                elif key in float_arrays:
                    formstring = '[nfot]/F'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def sortPeaks(self, unsortedPeaks):
        ''' sort PDF peaks according to their positions '''

        py_list = [x for x in unsortedPeaks]
        return sorted(py_list, key=lambda x: (x.mean))

    def pdfHistogram(self, pdf):
        ''' make histogram of PDF peak positions for the first 20 tracks '''

        self.nhisto += 1
        if self.nhisto > 20:
            return
        h = TH2F('pdf' + str(self.nhisto), 'muon PDF, itrk = ' + str(self.data.itrk),
                 512, 0.0, 512.0, 1000, 0.0, 75.0)
        h.SetXTitle('pixelID - 1')
        h.SetYTitle('peak positions [ns]')
        for x in range(512):
            peaks = pdf[x]
            for peak in peaks:
                h.Fill(x, peak.mean)
        h.Write()

    def getEcms(self, tfit, chargedStable):
        ''' returns c.m.s. energy of a particle '''
        mom = tfit.getMomentum()
        lorentzLab = TLorentzVector()
        mass = chargedStable.getMass()
        lorentzLab.SetXYZM(mom.X(), mom.Y(), mom.Z(), mass)
        T = Belle2.PCmsLabTransform()
        lorentzCms = T.labToCms(lorentzLab)
        return [lorentzCms.Energy(), T.getCMSEnergy() / 2]

    def trackSelected(self):
        ''' returns true if track fulfills selection criteria  '''

        if abs(self.data.dEcms) > 0.1:
            return False
        if abs(self.data.poca_x) > 0.2:
            return False
        if abs(self.data.poca_y) > 0.2:
            return False
        if abs(self.data.poca_z) > 0.5:
            return False
        return True

    def getTOF(self, track, pdg, slot):
        ''' returns timo-of-flight of extrapolated track '''

        extHits = track.getRelationsWith('ExtHits')
        extEnter = None
        extExit = None
        for extHit in extHits:
            if abs(extHit.getPdgCode()) != pdg:
                continue
            if extHit.getDetectorID() != Belle2.Const.TOP:
                continue
            if extHit.getCopyID() != slot:
                continue
            if extHit.getStatus() == Belle2.EXT_ENTER:
                extEnter = extHit
            elif extHit.getStatus() == Belle2.EXT_EXIT:
                extExit = extHit
                if extEnter:
                    return (extEnter.getTOF() + extExit.getTOF()) / 2
        return 0

    def event(self):
        ''' event processing '''

        recBunch = Belle2.PyStoreObj('TOPRecBunch')
        if not recBunch:
            b2.B2ERROR('no TOPRecBunch')
            return
        if not recBunch.isReconstructed():
            return
        self.bunchOffset.Fill(recBunch.getCurrentOffset())

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        self.data.run = evtMetaData.getRun()
        self.data.offset = recBunch.getCurrentOffset()
        self.data.usedTrk = recBunch.getUsedTracks()

        for track in Belle2.PyStoreArray('Tracks'):
            pdfs = track.getRelated('TOPPDFCollections')
            if not pdfs:
                continue
            self.data.slot = pdfs.getModuleID()
            momentum = pdfs.getAssociatedLocalMomentum()
            position = pdfs.getAssociatedLocalHit()
            self.data.p = momentum.Mag()
            self.data.cth = momentum.CosTheta()
            self.data.phi = momentum.Phi()
            self.data.z = position.Z()
            self.data.x = position.X()
            self.data.tof = self.getTOF(track, 13, self.data.slot)
            try:
                tfit = track.getTrackFitResultWithClosestMass(Belle2.Const.muon)
            except BaseException:
                b2.B2ERROR("No trackFitResult available")
                continue
            self.data.charge = tfit.getChargeSign()
            pocaPosition = tfit.getPosition()
            self.data.poca_x = pocaPosition.X()
            self.data.poca_y = pocaPosition.Y()
            self.data.poca_z = pocaPosition.Z()
            self.data.hitsCDC = tfit.getHitPatternCDC().getNHits()
            Ecms = self.getEcms(tfit, Belle2.Const.muon)
            self.data.dEcms = Ecms[0] - Ecms[1]
            if not self.trackSelected():
                continue
            try:
                topll = track.getRelated('TOPLikelihoods')
                extHit = topll.getRelated('ExtHits')
                timeZero = extHit.getRelated('TOPTimeZeros')
                self.data.rec_t0 = timeZero.getTime()
                self.data.valid_t0 = timeZero.isValid()
            except BaseException:
                self.data.rec_t0 = 0
                self.data.valid_t0 = 0

            self.h_cth_vs_p.Fill(self.data.p, self.data.cth)
            self.h_momentum.Fill(self.data.p)
            self.h_cth.Fill(self.data.cth)
            self.h_phi.Fill(self.data.phi)
            self.h_z.Fill(self.data.z)
            self.h_x.Fill(self.data.x)
            self.h_charge.Fill(self.data.charge)
            self.h_poca_xy.Fill(self.data.poca_x, self.data.poca_y)
            self.h_poca_z.Fill(self.data.poca_z)
            self.h_hitsCDC.Fill(self.data.hitsCDC)
            self.h_Ecms.Fill(Ecms[0])
            try:
                pdf = pdfs.getHypothesisPDF(13)
            except BaseException:
                b2.B2ERROR("No PDF available for PDG = 13")
                continue
            self.data.itrk += 1
            self.pdfHistogram(pdf)
            x0 = position.X() - momentum.X() / momentum.Y() * position.Y()  # emission
            z0 = position.Z() - momentum.Z() / momentum.Y() * position.Y()  # emission
            self.data.nfot = 0
            for digit in Belle2.PyStoreArray('TOPDigits'):
                if digit.getModuleID() == self.data.slot and digit.getHitQuality() == 1:
                    k = self.data.nfot
                    if k >= 1000:
                        continue
                    self.data.nfot += 1
                    self.data.channel[k] = digit.getChannel()
                    self.data.pixel[k] = digit.getPixelID()
                    self.data.time[k] = digit.getTime()
                    self.data.timeErr[k] = digit.getTimeError()
                    self.data.pulseHeight[k] = digit.getPulseHeight()
                    self.data.pulseWidth[k] = digit.getPulseWidth()
                    self.data.sample[k] = digit.getModulo256Sample()
                    self.data.status[k] = digit.getStatus()
                    peaks = pdf[digit.getPixelID() - 1]
                    if peaks.empty():
                        self.data.t0[k] = 0
                        self.data.wid0[k] = 0
                        self.data.t1[k] = 0
                    else:
                        sorted_peaks = self.sortPeaks(peaks)
                        self.data.t0[k] = sorted_peaks[0].mean
                        self.data.wid0[k] = sorted_peaks[0].width
                        self.data.t1[k] = self.data.t0[k] + 100
                        if peaks.size() > 1:
                            self.data.t1[k] = sorted_peaks[1].mean
                    self.data.t_pdf[k] = 0
                    self.data.type[k] = 0
                    self.data.nx[k] = 0
                    self.data.ny[k] = 0
                    self.data.nxm[k] = 0
                    self.data.nym[k] = 0
                    self.data.nxe[k] = 0
                    self.data.nye[k] = 0
                    self.data.nys[k] = 0
                    self.data.xd[k] = 0
                    self.data.yd[k] = 0
                    self.data.xm[k] = 0
                    self.data.kx[k] = 0
                    self.data.ky[k] = 0
                    self.data.alpha[k] = 0
                    assocPDF = digit.getRelated('TOPAssociatedPDFs')
                    if assocPDF:
                        pik = assocPDF.getSinglePeak()
                        if pik:
                            self.data.t_pdf[k] = pik.position
                            self.data.type[k] = pik.type
                            self.data.nx[k] = pik.nx
                            self.data.ny[k] = pik.ny
                            self.data.nxm[k] = pik.nxm
                            self.data.nym[k] = pik.nym
                            self.data.nxe[k] = pik.nxe
                            self.data.nye[k] = pik.nye
                            if pik.kyd > 0:
                                self.data.nys[k] = int(pik.nye / 2)
                            else:
                                self.data.nys[k] = int((pik.nye + 1) / 2)
                            self.data.xd[k] = pik.xd
                            self.data.yd[k] = pik.yd
                            if pik.kze > 0:
                                self.data.xm[k] = x0 + pik.kxe / pik.kze * (130.0 - z0)
                            self.data.kx[k] = pik.kxe
                            self.data.ky[k] = pik.kye
                            self.data.alpha[k] = math.degrees(math.acos(abs(pik.kzd)))

            self.tree.Fill()

    def terminate(self):
        ''' terminate: close root file '''

        self.file.cd()
        self.file.Write()
        self.file.Close()


# Create path
main = b2.create_path()

# Input: cdst file(s), use -i option
main.add_module('RootInput')

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Channel masking
main.add_module('TOPChannelMasker')

# Make a muon PDF available at datastore
main.add_module('TOPPDFDebugger', pdgCodes=[13])  # default

# Write ntuple
main.add_module(Ntuple())

# Print progress
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
