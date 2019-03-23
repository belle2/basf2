#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   Analyze a DST file and write a PDF of "interesting" event displays.
#   This script cannot analyze MDST files because they don't contain RawKLMs.
#
# Prerequisites (on kekcc):
#   Before running this script, type
#     source /cvmfs/belle.cern.ch/tools/b2setup release-02-01-00 <or higher release>
#   then verify that the corresponding proper global tag is used near the end of this script.
#   (Global tags are tabulated at https://confluence.desy.de/display/BI/Global+Tag+%28GT%29+page)
#   The external python script bklmDB.py must be in the same folder as this script.
#
# Usage:
#   basf2 bklm-eventdisplay.py -- -e # -r # -i infilename -c # -n # -m #
#      You need the '--' before these options to tell basf2 that these are options to this script.
#   Required arguments:
#      either -i infilename or -e # -r #
#      -i infilename  to specify the full pathname of the input ROOT DST file
#      -e #   to specify the experiment number, e.g., -e 1 (default is 3)
#      -r #   to specify the run number, e.g., -r 4794 (default is 4794)
#   Optional arguments:
#      -c #   to specify the maximum number of events to write to the PDF file (default = 100)
#      -n #   to specify the minimum number of RPC hits in one sector (default = 4)
#      -m #   to specify the minimum number of Muid hits in the event (default = 1)
#
# Input:
#   ROOT DST file written by basf2 (may include multiple folios for one expt/run). For example,
#   /ghi/fs01/belle2/bdata/Data/Raw/e0003/r04794/sub00/physics.0003.r04794.HLT2.f*.root
#   /ghi/fs01/belle2/bdata/Data/Raw/e0004/r06380/sub00/cosmic.0004.r06380.HLT2.f00000.root
#
# Output:
#   PDF file named bklmEvents-e#r#.pdf, using the experiment number and run number
#

from basf2 import *
import bklmDB
import simulation
import reconstruction
import tracking
from tracking import add_tracking_reconstruction
import rawdata
import math
import ctypes
import ROOT
from ROOT import Belle2, TH1F, TH2F, TCanvas, THistPainter, TPad
from optparse import Option, OptionValueError, OptionParser
import glob

# =========================================================================
#
#   EventCountLimiter basf2 module (must be defined before use)
#
# =========================================================================


class EventCountLimiter(Module):

    def __init__(self):
        """ init """
        super(EventCountLimiter, self).__init__()

    def initialize(self):
        pass

    def terminate(self):
        pass

    def beginRun(self):
        pass

    def endRun(self):
        pass

    def event(self):
        global eventDisplayCounter
        global eventCounterMax
        super(EventCountLimiter, self).return_value(eventDisplayCounter < eventCounterMax)

# =========================================================================
#
#   EventInspectorBKLM basf2 module (must be defined before use)
#
# =========================================================================


class EventInspectorBKLM(Module):

    BKLM_ID = 0x07000000
    EKLM_ID = 0x08000000

    BKLM_STRIP_BIT = 0
    BKLM_PLANE_BIT = 6
    BKLM_LAYER_BIT = 7
    BKLM_SECTOR_BIT = 11
    BKLM_END_BIT = 14
    BKLM_MAXSTRIP_BIT = 15
    BKLM_OUTOFTIME_BIT = 24
    BKLM_ONTRACK_BIT = 27
    BKLM_ONSTATRACK_BIT = 29

    BKLM_STRIP_MASK = 0x3f
    BKLM_PLANE_MASK = (1 << BKLM_PLANE_BIT)
    BKLM_LAYER_MASK = (15 << BKLM_LAYER_BIT)
    BKLM_SECTOR_MASK = (7 << BKLM_SECTOR_BIT)
    BKLM_END_MASK = (1 << BKLM_END_BIT)
    BKLM_MAXSTRIP_MASK = (63 << BKLM_MAXSTRIP_BIT)
    BKLM_ONTRACK_MASK = (1 << BKLM_ONTRACK_BIT)
    BKLM_ONSTATRACK_MASK = (1 << BKLM_ONSTATRACK_BIT)
    BKLM_MODULEID_MASK = (BKLM_END_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK)

    def __init__(self):
        """ init """
        super(EventInspectorBKLM, self).__init__()

    def makeGraph(self, x, y):
        graph = ROOT.TGraph()
        for i in range(0, len(x)):
            graph.SetPoint(i, x[i], y[i])
        graph.SetLineColor(2)
        graph.SetLineWidth(1)
        return graph

    def makeText(self, x, y, s):
        text = ROOT.TLatex(x, y, s)
        text.SetTextSize(0.04)
        text.SetTextColor(2)
        text.SetTextAlign(22)
        text.SetTextAngle(90)
        return text

    def initialize(self):

        global exp
        global run
        global eventPdfName
        global minRPCHitsPerSector
        global minMuids

        expRun = 'e{0:02d}r{1}: '.format(int(exp), int(run))
        self.eventCanvas = ROOT.TCanvas("eventCanvas", eventPdfName, 3200, 1600)
        title = '{0}['.format(eventPdfName)
        self.eventCanvas.SaveAs(title)
        self.eventCanvas.Clear()
        self.eventCanvas.Divide(2, 1)
        self.cosine = [0, 0, 0, 0, 0, 0, 0, 0]
        self.sine = [0, 0, 0, 0, 0, 0, 0, 0]
        for sector in range(0, 8):
            phi = math.pi * sector / 4
            self.cosine[sector] = math.cos(phi)
            self.sine[sector] = math.sin(phi)
        self.hist_XY = ROOT.TH2F('XY', ' ;x;y', 10, -345.0, 345.0, 10, -345.0, 345.0)
        self.hist_XY.SetStats(False)
        self.hist_ZY = ROOT.TH2F('ZY', ' ;z;y', 10, -345.0, 345.0, 10, -345.0, 345.0)
        self.hist_ZY.SetStats(False)
        # 300x300 cm^2 grid for each octant
        u1 = 65
        u2 = 365
        u3 = 100
        u4 = 400
        u5 = 150
        self.hist_XYS = [0, 0, 0, 0, 0, 0, 0, 0]
        self.hist_XYS[0] = ROOT.TH2F('XYS0', ' ;x;y', 10, +u3, +u4, 10, -u5, +u5)
        self.hist_XYS[0].SetStats(False)
        self.hist_XYS[1] = ROOT.TH2F('XYS1', ' ;x;y', 10, +u1, +u2, 10, +u1, +u2)
        self.hist_XYS[1].SetStats(False)
        self.hist_XYS[2] = ROOT.TH2F('XYS2', ' ;x;y', 10, -u5, +u5, 10, +u3, +u4)
        self.hist_XYS[2].SetStats(False)
        self.hist_XYS[3] = ROOT.TH2F('XYS3', ' ;x;y', 10, -u2, -u1, 10, +u1, +u2)
        self.hist_XYS[3].SetStats(False)
        self.hist_XYS[4] = ROOT.TH2F('XYS4', ' ;x;y', 10, -u4, -u3, 10, -u5, +u5)
        self.hist_XYS[4].SetStats(False)
        self.hist_XYS[5] = ROOT.TH2F('XYS5', ' ;x;y', 10, -u2, -u1, 10, -u2, -u1)
        self.hist_XYS[5].SetStats(False)
        self.hist_XYS[6] = ROOT.TH2F('XYS6', ' ;x;y', 10, -u5, +u5, 10, -u4, -u3)
        self.hist_XYS[6].SetStats(False)
        self.hist_XYS[7] = ROOT.TH2F('XYS7', ' ;x;y', 10, +u1, +u2, 10, -u2, -u1)
        self.hist_XYS[7].SetStats(False)
        self.hist_ZYS = ROOT.TH2F('ZYS', ' ;z;y', 10, -150.0, 150.0, 10, 125.0, 425.0)
        self.hist_ZYS.SetStats(False)
        ROOT.gStyle.SetOptStat(10)
        self.bklmXY = []
        r0 = 201.9 + 0.5 * 4.4  # cm
        dr = 9.1  # cm
        tan0 = math.tan(math.pi / 8.0)
        g = ROOT.TGraph()
        g.SetPoint(0, -200.0, 0.0)
        g.SetPoint(1, +200.0, 0.0)
        g.SetLineColor(19)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmXY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, 0.0, -200.0)
        g.SetPoint(1, 0.0, +200.0)
        g.SetLineColor(19)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmXY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -5.0, 0.0)
        g.SetPoint(1, +5.0, 0.0)
        g.SetPoint(2, 0.0, 0.0)
        g.SetPoint(3, 0.0, +5.0)
        g.SetPoint(4, 0.0, -5.0)
        g.SetLineColor(1)
        g.SetLineWidth(1)
        self.bklmXY.append(g)
        for layer in range(0, 15):
            r = r0 + layer * dr
            x = r * tan0
            g = ROOT.TGraph()
            g.SetPoint(0, +r, -x)
            g.SetPoint(1, +r, +x)
            g.SetPoint(2, +x, +r)
            g.SetPoint(3, -x, +r)
            g.SetPoint(4, -r, +x)
            g.SetPoint(5, -r, -x)
            g.SetPoint(6, -x, -r)
            g.SetPoint(7, +x, -r)
            g.SetPoint(8, +r, -x)
            if layer < 2:
                g.SetLineColor(18)
            else:
                g.SetLineColor(17)
                if (layer % 5) == 0:
                    g.SetLineStyle(3)
            g.SetLineWidth(1)
            self.bklmXY.append(g)
        self.bklmZY = []
        rF = r0 + 14 * dr
        x0 = r0 * tan0
        z0 = 47.0  # cm
        zL = 220.0  # cm
        g = ROOT.TGraph()
        g.SetPoint(0, -zL + z0 - 140.0, 0.0)
        g.SetPoint(1, +zL + z0 + 70.0, 0.0)
        g.SetLineColor(19)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, 0.0, -315.0)
        g.SetPoint(1, 0.0, +340.0)
        g.SetLineColor(19)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -5.0, 0.0)
        g.SetPoint(1, +5.0, 0.0)
        g.SetPoint(2, 0.0, 0.0)
        g.SetPoint(3, 0.0, +5.0)
        g.SetPoint(4, 0.0, -5.0)
        g.SetLineColor(1)
        g.SetLineWidth(1)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -zL + z0, +x0)
        g.SetPoint(1, -zL + z0, +r0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -zL + z0, -x0)
        g.SetPoint(1, -zL + z0, -r0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, +zL + z0, +x0)
        g.SetPoint(1, +zL + z0, +r0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, +zL + z0, -x0)
        g.SetPoint(1, +zL + z0, -r0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        g.SetLineStyle(3)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -zL + z0, r0)
        g.SetPoint(1, +zL + z0, r0)
        g.SetPoint(2, +zL + z0, rF)
        g.SetPoint(3, -zL + z0, rF)
        g.SetPoint(4, -zL + z0, r0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -zL + z0, -r0)
        g.SetPoint(1, +zL + z0, -r0)
        g.SetPoint(2, +zL + z0, -rF)
        g.SetPoint(3, -zL + z0, -rF)
        g.SetPoint(4, -zL + z0, -r0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        self.bklmZY.append(g)
        g = ROOT.TGraph()
        g.SetPoint(0, -zL + z0, -x0)
        g.SetPoint(1, +zL + z0, -x0)
        g.SetPoint(2, +zL + z0, +x0)
        g.SetPoint(3, -zL + z0, +x0)
        g.SetPoint(4, -zL + z0, -x0)
        g.SetLineColor(18)
        g.SetLineWidth(1)
        self.bklmZY.append(g)
        self.bklmZYL = []
        for layer in range(0, 15):
            r = r0 + layer * dr
            z0 = 47.0  # cm
            zL = 220.0  # cm
            g = ROOT.TGraph()
            g.SetPoint(0, -zL + z0, r)
            g.SetPoint(1, +zL + z0, r)
            g.SetLineColor(19)
            g.SetLineWidth(1)
            self.bklmZYL.append(g)
        # fill the readout <-> detector map from the information retrieved from the conditions database
        self.electIdToModuleId = bklmDB.fillDB()
        # maps for sectorFB <-> data concentrator
        self.sectorFBToDC = [11, 15, 2, 6, 10, 14, 3, 7, 9, 13, 0, 4, 8, 12, 1, 5]
        self.dcToSectorFB = [10, 14, 2, 6, 11, 15, 3, 7, 12, 8, 4, 0, 13, 9, 5, 1]
        # per-sector calibration adjustments (ns)
        # old self.t0Cal = 312.62 # for rawKLMs
        # old self.t0Cal2d = 293.2 # for BKLMHit2ds
        # old self.ct0Cal = -1148.01
        # old self.t0RPC = [7.85, -14.04, -5.58, -17.45, -6.34, 10.03, 13.29, 8.61, -3.3, -11.0, 0, -16.04, 4.97, 8.99, 9.64, 7.26]
        # old self.ct0Scint = [0.70, -7.40, -11.54, -8.30, 0.50, 9.11, 11.12, 8.41, 0.69, -7.64, 0, -9.10, 0.82, 6.55, 10.57, 7.19]
        self.t0Cal = 312.77  # for rawKLMs
        self.t0Cal2d = 293.64  # for BKLMHit2ds
        self.ct0Cal = 905.61
        self.ct0Cal2d = 905.61
        self.t0RPC = [8.07, -13.98, -6.73, -17.52, -4.91, 9.24, 12.83, 8.92, -1.44, -10.46, 0, -15.57, 2.44, 7.68, 9.92, 8.23]
        self.ct0Scint = [0.67, -7.70, -12.70, -9.05, 0.14, 7.12, 10.14, 7.73, 0.17, -7.92, 0, -9.10, 0.19, 5.86, 10.67, 5.51]

    def terminate(self):

        global eventPdfName

        self.eventCanvas.Clear()
        self.eventCanvas.SaveAs(eventPdfName)
        title = '{0}]'.format(eventPdfName)
        self.eventCanvas.SaveAs(title)
        print('Goodbye')

    def beginRun(self):
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        print('beginRun', EventMetaData.getRun())

    def endRun(self):
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        print('endRun', EventMetaData.getRun())

    def event(self):

        global minRPCHitsPerSector
        global minMuids
        global eventDisplayCounter
        global eventPdfName

        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        event = EventMetaData.getEvent()
        rawklms = Belle2.PyStoreArray('RawKLMs')
        hit2ds = Belle2.PyStoreArray('BKLMHit2ds')
        exthits = Belle2.PyStoreArray('ExtHits')
        muidhits = Belle2.PyStoreArray('MuidHits')

        rawFb = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawSector = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawLayer = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawPlane = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawStrip = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawCtime = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]

        for copper in range(0, len(rawklms)):
            rawklm = rawklms[copper]
            nodeID = rawklm.GetNodeID(0) - self.BKLM_ID
            if nodeID >= self.EKLM_ID - self.BKLM_ID:
                nodeID = nodeID - (self.EKLM_ID - self.BKLM_ID) + 4
            if (nodeID < 0) or (nodeID > 4):  # skip EKLM nodes
                continue
            for finesse in range(0, 4):
                dc = (finesse << 2) + copper
                n = rawklm.GetDetectorNwords(0, finesse)
                bufSlot = rawklm.GetDetectorBuffer(0, finesse)
                if n > 0:
                    n = n >> 1
                    for j in range(0, n):
                        word0 = bufSlot[j * 2]
                        word1 = bufSlot[j * 2 + 1]
                        ctime = word0 & 0xffff
                        channel = (word0 >> 16) & 0x7f
                        axis = (word0 >> 23) & 0x01
                        lane = (word0 >> 24) & 0x1f  # crate's slot number
                        flag = (word0 >> 30) & 0x03
                        fb = -1
                        sector = -1
                        layer = -1
                        plane = -1
                        strip = -1
                        electId = (channel << 12) | (axis << 11) | (lane << 6) | (finesse << 4) | nodeID
                        if electId in self.electIdToModuleId:
                            moduleId = self.electIdToModuleId[electId]
                            fb = (moduleId & self.BKLM_END_MASK) >> self.BKLM_END_BIT
                            sector = (moduleId & self.BKLM_SECTOR_MASK) >> self.BKLM_SECTOR_BIT
                            layer = (moduleId & self.BKLM_LAYER_MASK) >> self.BKLM_LAYER_BIT
                            plane = (moduleId & self.BKLM_PLANE_MASK) >> self.BKLM_PLANE_BIT
                            strip = (moduleId & self.BKLM_STRIP_MASK) >> self.BKLM_STRIP_BIT
                        rawFb[dc].append(fb)
                        rawSector[dc].append(sector)
                        rawLayer[dc].append(layer)
                        rawPlane[dc].append(plane)
                        rawStrip[dc].append(strip)
                        rawCtime[dc].append(ctime)

        tCal2d = []
        for hit2d in hit2ds:
            tCal2d.append(hit2d.getTime())

        # Process the ExtHits for event display

        xold = 0
        yold = 0
        zold = 0
        sold = 0
        epositions = []
        sumx = 0
        sumy = 0
        sumz = 0
        sumn = 0
        for exthit in exthits:
            sector = (exthit.getCopyID() & self.BKLM_SECTOR_MASK) >> self.BKLM_SECTOR_BIT
            extPosition = exthit.getPosition()
            x = extPosition[0]
            y = extPosition[1]
            z = extPosition[2]
            dx = x - xold
            dy = y - yold
            dz = z - zold
            if (dx * dx + dy * dy + dz * dz > 36) and (sumn > 0):
                eposition = [sumx / sumn, sumy / sumn, sumz / sumn, sold]
                epositions.append(eposition)
                sumx = 0
                sumy = 0
                sumz = 0
                sumn = 0
            else:
                if sumn == 0:
                    xold = x
                    yold = y
                    zold = z
                    sold = sector
                sumx = sumx + x
                sumy = sumy + y
                sumz = sumz + z
                sumn = sumn + 1
        if sumn > 0:
            eposition = [sumx / sumn, sumy / sumn, sumz / sumn, sold]
            epositions.append(eposition)
        extXYGraph = ROOT.TGraph()
        extXYGraph.SetMarkerColor(30)
        extXYGraph.SetMarkerSize(2.25)
        extXYGraph.SetMarkerStyle(21)
        extZYGraph = ROOT.TGraph()
        extZYGraph.SetMarkerColor(30)
        extZYGraph.SetMarkerSize(2.25)
        extZYGraph.SetMarkerStyle(21)
        extZYSGraph = [0, 0, 0, 0, 0, 0, 0, 0]
        for sector in range(0, 8):
            extZYSGraph[sector] = ROOT.TGraph()
            extZYSGraph[sector].SetMarkerColor(30)
            extZYSGraph[sector].SetMarkerSize(2.25)
            extZYSGraph[sector].SetMarkerStyle(21)
        j = -1
        for eposition in epositions:
            j = j + 1
            x = eposition[0]
            y = eposition[1]
            z = eposition[2]
            extXYGraph.SetPoint(j, x, y)
            extZYGraph.SetPoint(j, z, y)
            sector = int(eposition[3])
            nPoint = extZYSGraph[sector].GetN()
            extZYSGraph[sector].SetPoint(nPoint, z, abs(x * self.cosine[sector] + y * self.sine[sector]))

        # Process the MuidHits for event display

        zMuids = [0, 0, 0, 0, 0, 0, 0, 0]
        nMuids = [0, 0, 0, 0, 0, 0, 0, 0]
        muidXYGraph = ROOT.TGraph()
        muidXYGraph.SetMarkerColor(5)
        muidXYGraph.SetMarkerSize(2.0)
        muidXYGraph.SetMarkerStyle(20)
        muidZYGraph = ROOT.TGraph()
        muidZYGraph.SetMarkerColor(5)
        muidZYGraph.SetMarkerSize(2.0)
        muidZYGraph.SetMarkerStyle(20)
        muidZYSGraph = [0, 0, 0, 0, 0, 0, 0, 0]
        for sector in range(0, 8):
            muidZYSGraph[sector] = ROOT.TGraph()
            muidZYSGraph[sector].SetMarkerColor(5)
            muidZYSGraph[sector].SetMarkerSize(2.0)
            muidZYSGraph[sector].SetMarkerStyle(20)
        j = -1
        for muidhit in muidhits:
            j = j + 1
            muidPosition = muidhit.getExtPosition()
            x = muidPosition[0]
            y = muidPosition[1]
            z = muidPosition[2]
            muidXYGraph.SetPoint(j, x, y)
            muidZYGraph.SetPoint(j, z, y)
            sector = muidhit.getSector()
            nPoint = muidZYSGraph[sector].GetN()
            muidZYSGraph[sector].SetPoint(nPoint, z, abs(x * self.cosine[sector] + y * self.sine[sector]))
            if nMuids[sector] == 0:
                zMuids[sector] = z
            nMuids[sector] = nMuids[sector] + 1

        # Process the BKLMHit2ds for event display

        rpcHitCount = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        promptXYGraph = ROOT.TGraph()
        promptXYGraph.SetMarkerColor(4)
        promptXYGraph.SetMarkerSize(2.0)
        promptXYGraph.SetMarkerStyle(29)
        promptZYGraph = ROOT.TGraph()
        promptZYGraph.SetMarkerColor(4)
        promptZYGraph.SetMarkerSize(2.0)
        promptZYGraph.SetMarkerStyle(29)
        bkgdXYGraph = ROOT.TGraph()
        bkgdXYGraph.SetMarkerColor(2)
        bkgdXYGraph.SetMarkerSize(2.0)
        bkgdXYGraph.SetMarkerStyle(29)
        bkgdZYGraph = ROOT.TGraph()
        bkgdZYGraph.SetMarkerColor(2)
        bkgdZYGraph.SetMarkerSize(2.0)
        bkgdZYGraph.SetMarkerStyle(29)
        promptZYSGraph = [0, 0, 0, 0, 0, 0, 0, 0]
        bkgdZYSGraph = [0, 0, 0, 0, 0, 0, 0, 0]
        for sector in range(0, 8):
            promptZYSGraph[sector] = ROOT.TGraph()
            promptZYSGraph[sector].SetMarkerColor(4)
            promptZYSGraph[sector].SetMarkerSize(2.0)
            promptZYSGraph[sector].SetMarkerStyle(29)
            bkgdZYSGraph[sector] = ROOT.TGraph()
            bkgdZYSGraph[sector].SetMarkerColor(2)
            bkgdZYSGraph[sector].SetMarkerSize(2.0)
            bkgdZYSGraph[sector].SetMarkerStyle(29)
        jPrompt = -1
        jBkgd = -1
        for hit2d in hit2ds:
            key = hit2d.getModuleID()
            layer = (key & self.BKLM_LAYER_MASK) >> self.BKLM_LAYER_BIT
            sector = (key & self.BKLM_SECTOR_MASK) >> self.BKLM_SECTOR_BIT
            fb = (key & self.BKLM_END_MASK) >> self.BKLM_END_BIT
            phiStripMin = hit2d.getPhiStripMin() - 1
            phiStripMax = hit2d.getPhiStripMax() - 1
            zStripMin = hit2d.getZStripMin() - 1
            zStripMax = hit2d.getZStripMax() - 1
            sectorFB = sector if fb == 0 else sector + 8
            if layer >= 2:
                rpcHitCount[sectorFB] = rpcHitCount[sectorFB] + 1
            dc = self.sectorFBToDC[sectorFB]
            copper = dc & 0x03
            finesse = dc >> 2
            n = rawklms[copper].GetDetectorNwords(0, finesse) >> 1
            trigCtime = (rawklms[copper].GetTTCtime(0) & 0x07ffffff) << 3
            ctDiffMax = 99999
            tCal = -1
            jZ = -1
            jPhi = -1
            ctZ = 0
            ctPhi = 0
            for j in range(0, n):
                if layer != rawLayer[dc][j]:
                    continue
                if sector != rawSector[dc][j]:
                    continue
                if fb != rawFb[dc][j]:
                    continue
                strip = rawStrip[dc][j]
                plane = rawPlane[dc][j]
                if plane == 0:  # it's a z strip
                    if strip < zStripMin:
                        continue
                    if strip > zStripMax:
                        continue
                    ctZ = rawCtime[dc][j] << 3  # in ns, range is only 8 bits in SCROD (??)
                    jZ = j
                else:  # it's a phi strip
                    if strip < phiStripMin:
                        continue
                    if strip > phiStripMax:
                        continue
                    ctPhi = rawCtime[dc][j] << 3  # in ns, range is only 8 bits in SCROD (??)
                    jPhi = j
                if (jZ >= 0) and (jPhi >= 0):
                    if layer < 2:  # it's a scint layer
                        if abs(ctZ - ctPhi) > 40:
                            continue
                        ct = int((ctZ + ctPhi) * 0.5 - trigCtime - self.ct0Scint[sectorFB]) & 0x3ff
                        if abs(ct - self.ct0Cal) < ctDiffMax:
                            ctDiffMax = int(abs(ct - self.ct0Cal))
                            tCal = ct
                            if ctDiffMax == 0:
                                break
                    else:  # it's an RPC layer
                        tCal = ((int(hit2d.getTime()) - trigCtime) & 0x03ff) - self.t0RPC[sectorFB] - 0.75 * jPhi - 0.75 * jZ
                        break
            x = hit2d.getGlobalPositionX()
            y = hit2d.getGlobalPositionY()
            z = hit2d.getGlobalPositionZ()
            isPromptHit = False
            if layer < 2:
                if abs(tCal - self.ct0Cal2d) < 20:
                    isPromptHit = True
            else:
                if abs(tCal - self.t0Cal2d) < 20:
                    isPromptHit = True
            if isPromptHit:
                jPrompt = jPrompt + 1
                promptXYGraph.SetPoint(jPrompt, x, y)
                promptZYGraph.SetPoint(jPrompt, z, y)
                nPoint = promptZYSGraph[sector].GetN()
                promptZYSGraph[sector].SetPoint(nPoint, z, abs(x * self.cosine[sector] + y * self.sine[sector]))
            else:
                jBkgd = jBkgd + 1
                bkgdXYGraph.SetPoint(jBkgd, x, y)
                bkgdZYGraph.SetPoint(jBkgd, z, y)
                nPoint = bkgdZYSGraph[sector].GetN()
                bkgdZYSGraph[sector].SetPoint(nPoint, z, abs(x * self.cosine[sector] + y * self.sine[sector]))

        hasManyRPCHits = False
        for count in rpcHitCount:
            if count > minRPCHitsPerSector:
                hasManyRPCHits = True
                break
        if hasManyRPCHits and (len(muidhits) > minMuids):
            eventDisplayCounter = eventDisplayCounter + 1
            title = 'e{0:02d}r{1}: event {2}'.format(int(exp), int(run), event)
            self.hist_XY.SetTitle(title)
            self.hist_ZY.SetTitle(title)
            self.eventCanvas.cd(1)
            self.hist_XY.Draw()
            for g in self.bklmXY:
                g.Draw("L")
            if extXYGraph.GetN() > 0:
                extXYGraph.Draw("P")
            if muidXYGraph.GetN() > 0:
                muidXYGraph.Draw("P")
            if bkgdXYGraph.GetN() > 0:
                bkgdXYGraph.Draw("P")
            if promptXYGraph.GetN() > 0:
                promptXYGraph.Draw("P")
            self.eventCanvas.cd(2)
            self.hist_ZY.Draw()
            for g in self.bklmZY:
                g.Draw("L")
            if extZYGraph.GetN() > 0:
                extZYGraph.Draw("P")
            if muidZYGraph.GetN() > 0:
                muidZYGraph.Draw("P")
            if bkgdZYGraph.GetN() > 0:
                bkgdZYGraph.Draw("P")
            if promptZYGraph.GetN() > 0:
                promptZYGraph.Draw("P")
            self.eventCanvas.Print(eventPdfName, "Title:{0}".format(event))
            for sector in range(0, 8):
                if nMuids[sector] > 0:
                    title = 'e{0:02d}r{1}: event {2} sector {3}'.format(int(exp), int(run), event, sector)
                    self.hist_XYS[sector].SetTitle(title)
                    self.eventCanvas.cd(1)
                    self.hist_XYS[sector].Draw()
                    for g in self.bklmXY:
                        g.Draw("L")
                    if extXYGraph.GetN() > 0:
                        extXYGraph.Draw("P")
                    if muidXYGraph.GetN() > 0:
                        muidXYGraph.Draw("P")
                    if bkgdXYGraph.GetN() > 0:
                        bkgdXYGraph.Draw("P")
                    if promptXYGraph.GetN() > 0:
                        promptXYGraph.Draw("P")
                    self.eventCanvas.cd(2)
                    z0 = zMuids[sector]
                    self.hist_ZYS.SetTitle(title)
                    self.hist_ZYS.SetBins(10, z0 - 150.0, z0 + 150.0, 10, 125.0, 425.0)
                    self.hist_ZYS.Draw()
                    for g in self.bklmZY:
                        g.Draw("L")
                    for g in self.bklmZYL:
                        g.Draw("L")
                    if extZYSGraph[sector].GetN() > 0:
                        extZYSGraph[sector].Draw("P")
                    if muidZYSGraph[sector].GetN() > 0:
                        muidZYSGraph[sector].Draw("P")
                    if bkgdZYSGraph[sector].GetN() > 0:
                        bkgdZYSGraph[sector].Draw("P")
                    if promptZYSGraph[sector].GetN() > 0:
                        promptZYSGraph[sector].Draw("P")
                    self.eventCanvas.Print(eventPdfName, "Title:{0}".format(event))

# =========================================================================
#
#   Main routine; called by basf2
#
# =========================================================================


parser = OptionParser()
parser.add_option('-i', '--inputfile',
                  dest='infilename', default='',
                  help='Input ROOT filename [no default]')
parser.add_option('-e', '--experiment',
                  dest='eNumber', default='',
                  help='Experiment number [no default]')
parser.add_option('-r', '--run',
                  dest='rNumber', default='',
                  help='Run number [no default]')
parser.add_option('-c', '--count',
                  dest='counter', default='100',
                  help='Maximum # of displayed events [default=100]')
parser.add_option('-n', '--nrpchits',
                  dest='minRPCHits', default='4',
                  help='Minimum # of RPC hits in one sector [default=4]')
parser.add_option('-m', '--muids',
                  dest='minMuids', default='1',
                  help='Minimum # of Muid hits in the event [default=1]')
(options, args) = parser.parse_args()
eventCounterMax = int(options.counter)
if eventCounterMax <= 0:
    print('Maximum number of events to display is', eventCounterMax, ' - nothing to do.')
    sys.exit()
eventDisplayCounter = 0
minRPCHitsPerSector = int(options.minRPCHits)
minMuids = int(options.minMuids)

inputName = ''
exp = ''
run = ''
if options.infilename != '':
    inputName = options.infilename
    fileList = glob.glob(inputName)
    if len(fileList) == 0:
        print('No file(s) match {0}'.format(inputName))
        sys.exit()
if options.eNumber != '':
    if not options.eNumber.isdecimal():
        print('Experiment number ({0}) is not valid'.format(options.eNumber))
        sys.exit()
    exp = '{0:04d}'.format(int(options.eNumber))
else:
    eStart = inputName.find('/e') + 2
    if eStart < 0:
        print('Input filename does not contain the required experiment number')
        sys.exit()
    eEnd = inputName.find('/', eStart)
    exp = inputName[eStart:eEnd]
    if not exp.isdecimal():
        print('Input filename experiment number({0}) is not valid'.format(exp))
        sys.exit()
if options.rNumber != '':
    if not options.rNumber.isdecimal():
        print('Run number ({0}) is not valid'.format(options.rNumber))
        sys.exit()
    run = '{0:05d}'.format(int(options.rNumber))
else:
    rStart = inputName.find('/r') + 2
    if rStart < 0:
        print('Input filename does not contain the required run number')
        sys.exit()
    rEnd = inputName.find('/', rStart)
    run = inputName[rStart:rEnd]
    if not run.isdecimal():
        print('Input filename run number({0}) is not valid'.format(run))
        sys.exit()
if len(inputName) == 0:
    fileList = glob.glob('/ghi/fs01/belle2/bdata/Data/Raw/e{0}/r{1}/sub00/*.{0}.{1}.HLT2.f00000.root'.format(exp, run))
    if len(fileList) == 0:
        print('No file(s) found for experiment <{0}> run <{1}>'.format(options.eNumber, options.rNumber))
        sys.exit()
    inputName = fileList[0].replace('f00000', 'f*')
eventPdfName = 'bklmEvents-e{0}r{1}.pdf'.format(exp, run)

print('bklm-eventDisplay: exp=', exp, 'run=', run, 'input=', inputName, '. Write at most', eventCounterMax,
      ' events. Criteria: # RPC hits per sector >=', minRPCHitsPerSector, '  # Muids in event >=', minMuids)

reset_database()
use_database_chain()
# use proper global tag for data - depends on the basf2 release
use_central_database('data_reprocessing_prod6')  # for release-02-01-00
# use_central_database('data_reprocessing_proc8')  # for release-03-01-00

main = create_path()
main.add_module('RootInput', inputFileName=inputName)
main.add_module('ProgressBar')

child = create_path()
eventCountLimiter = EventCountLimiter()
eventCountLimiter.if_true(child, AfterConditionPath.CONTINUE)
main.add_module(eventCountLimiter)

rawdata.add_unpackers(child)
child.add_module('BKLMReconstructor')
add_tracking_reconstruction(child)
ext = child.add_module('Ext')
ext.param('pdgCodes', [13])
muid = child.add_module('Muid')
# muid.param('MaxDistSigma', 10.0)
child.add_module(EventInspectorBKLM())

process(main)
print(statistics)
