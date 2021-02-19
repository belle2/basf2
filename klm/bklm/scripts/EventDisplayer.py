#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   basf module to create BKLM event displays from BKLMHit2ds, ExtHits, and MuidHits.
#

import basf2
import bklmDB
import math
import ROOT
from ROOT import Belle2

# =========================================================================
#
#   EventDisplayer
#
# =========================================================================


class EventDisplayer(basf2.Module):
    """Draw BKLM event displays from BKLMHit2ds, ExtHits, and MuidHits."""

    #: COPPER base identifier for BKLM readout
    BKLM_ID = 0x07000000
    #: COPPER base identifier for EKLM readout
    EKLM_ID = 0x08000000
    #: bit position for strip-1 [0..47]
    BKLM_STRIP_BIT = 0
    #: bit position for plane-1 [0..1]; 0 is inner-plane
    BKLM_PLANE_BIT = 6
    #: bit position for layer-1 [0..14]; 0 is innermost
    BKLM_LAYER_BIT = 7
    #: bit position for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
    BKLM_SECTOR_BIT = 11
    #: bit position for section [0..1]; forward is 0
    BKLM_SECTION_BIT = 14
    #: bit position for maxStrip-1 [0..47]
    BKLM_MAXSTRIP_BIT = 15
    #: bit mask for strip-1 [0..47]
    BKLM_STRIP_MASK = 0x3f
    #: bit mask for plane-1 [0..1]; 0 is inner-plane
    BKLM_PLANE_MASK = (1 << BKLM_PLANE_BIT)
    #: bit mask for layer-1 [0..15]; 0 is innermost and 14 is outermost
    BKLM_LAYER_MASK = (15 << BKLM_LAYER_BIT)
    #: bit mask for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
    BKLM_SECTOR_MASK = (7 << BKLM_SECTOR_BIT)
    #: bit mask for section [0..1]; forward is 0
    BKLM_SECTION_MASK = (1 << BKLM_SECTION_BIT)
    #: bit mask for maxStrip-1 [0..47]
    BKLM_MAXSTRIP_MASK = (63 << BKLM_MAXSTRIP_BIT)
    #: bit mask for unique module identifier (end, sector, layer)
    BKLM_MODULEID_MASK = (BKLM_SECTION_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK)

    def __init__(self, exp, run, eventPdfName, maxDisplays, minRPCHits, minMuidHits):
        """Constructor

        Arguments:
            exp (str): formatted experiment number
            run (str): formatter run number
            eventPdfName (str): path name of the output event-display PDF file
            maxDisplays (int): max # of events displays to write
            minRPCHits (int): min # of RPC BKLMHit2ds in any sector for event display
            minMuidHits (int): min # of MuidHits in the event for event display
        """
        super().__init__()
        #: internal copy of experiment number
        self.exp = exp
        #: internal copy of run number
        self.run = run
        #: internal copy of the pathname of the output event-display PDF file
        self.eventPdfName = eventPdfName
        #: internal copy of the maximum number of event displays to write
        self.maxDisplays = maxDisplays
        #: internal copy of the minimum number of RPC BKLMHit2ds in any sector for event display
        self.minRPCHits = minRPCHits
        #: internal copy of the minimum number of MuidHits in the event for event display
        self.minMuidHits = minMuidHits
        #: event counter (needed for PDF table of contents' ordinal event#)
        self.eventCounter = 0
        #: event-display counter
        self.eventDisplays = 0
        #: title of the last-drawn event display (needed for PDF table of contents' last event)
        self.lastTitle = ''

    def initialize(self):
        """Handle job initialization: fill the mapping database, create histograms, open the event-display file"""

        # expRun = 'e{0:02d}r{1}: '.format(int(self.exp), int(self.run))

        # Open the output PDF file for event displays

        #: TCanvas on which event displays will be drawn
        self.eventCanvas = ROOT.TCanvas("eventCanvas", self.eventPdfName, 3200, 1600)
        title = '{0}['.format(self.eventPdfName)
        self.eventCanvas.SaveAs(title)
        self.eventCanvas.Clear()
        self.eventCanvas.Divide(2, 1)

        # Create the boilerplate for the end- and side-views of the event display

        #: table of cosines for the BKLM-sector normals
        self.cosine = [0, 0, 0, 0, 0, 0, 0, 0]
        #: table of sines for the BKLM-sector normals
        self.sine = [0, 0, 0, 0, 0, 0, 0, 0]
        for sector in range(0, 8):
            phi = math.pi * sector / 4
            self.cosine[sector] = math.cos(phi)
            self.sine[sector] = math.sin(phi)
        #: blank scatterplot to define the bounds of the BKLM end view
        self.hist_XY = ROOT.TH2F('XY', ' ;x;y', 10, -345.0, 345.0, 10, -345.0, 345.0)
        self.hist_XY.SetStats(False)
        #: blank scatterplot to define the bounds of the BKLM side view
        self.hist_ZY = ROOT.TH2F('ZY', ' ;z;y', 10, -345.0, 345.0, 10, -345.0, 345.0)
        self.hist_ZY.SetStats(False)
        # 300x300 cm^2 grid for each octant
        u1 = 65
        u2 = 365
        u3 = 100
        u4 = 400
        u5 = 150
        #: list of blank scatterplots to define the per-sector bounds of the BKLM end view
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
        #: blank scatterplot to define the per-sector bounds of the rotated BKLM side view
        self.hist_ZYS = ROOT.TH2F('ZYS', ' ;z;y', 10, -150.0, 150.0, 10, 125.0, 425.0)
        self.hist_ZYS.SetStats(False)
        ROOT.gStyle.SetOptStat(10)
        #: list of line-segment (x,y) points for the BKLM end view
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
        #: list of line-segment (z,y) points for the BKLM side view
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
        #: list of line-segment (z,y) points for the BKLM sector's zoomed and rotated side view
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
        #: readout <-> detector map (from the information retrieved from the conditions database)
        self.electIdToModuleId = bklmDB.fillDB()
        #: map for sectorFB -> data concentrator
        self.sectorFBToDC = [11, 15, 2, 6, 10, 14, 3, 7, 9, 13, 0, 4, 8, 12, 1, 5]
        #: map for data concentrator -> sectorFB
        self.dcToSectorFB = [10, 14, 2, 6, 11, 15, 3, 7, 12, 8, 4, 0, 13, 9, 5, 1]
        #: RPC-time calibration adjustment (ns) for rawKLMs
        self.t0Cal = 312
        #: RPC-time calibration adjustment (ns) for BKLMHit2ds
        self.t0Cal2d = 308
        #: scint-ctime calibration adjustment (ns) for rawKLMs
        self.ct0Cal = 455
        #: scint-ctime calibration adjustment (ns) for BKLMHit2ds
        self.ct0Cal2d = 520
        #: per-sector variations in RPC-time calibration adjustment (ns) for rawKLMs
        self.t0RPC = [8, -14, -6, -14, -2, 10, 9, 13, 0, -10, -14, -20, 2, 6, 14, 11]
        #: per-sector variations in scint-ctime calibration adjustment (ns) for rawKLMs
        self.ct0Scint = [-1, -33, -46, -33, -2, 32, 51, 32, 0, -32, -45, -33, -4, 34, 45, 27]

    def terminate(self):
        """Handle job termination: close event-display file"""

        pdfNameLast = '{0}]'.format(self.eventPdfName)
        self.eventCanvas.Print(pdfNameLast, self.lastTitle)
        print('Goodbye')

    def beginRun(self):
        """Handle begin of run: print diagnostic message"""
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        print('beginRun', EventMetaData.getRun())

    def endRun(self):
        """Handle end of run: print diagnostic message"""
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        print('endRun', EventMetaData.getRun())

    def event(self):
        """Process one event: (optionally) draw event display"""

        super().return_value(self.eventDisplays < self.maxDisplays)

        if self.eventDisplays >= self.maxDisplays:
            return

        self.eventCounter += 1
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        event = EventMetaData.getEvent()
        rawklms = Belle2.PyStoreArray('RawKLMs')  # to determine if BKLMHit2d is prompt
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
                        # word1 = bufSlot[j * 2 + 1]
                        ctime = word0 & 0xffff
                        channel = (word0 >> 16) & 0x7f
                        axis = (word0 >> 23) & 0x01
                        lane = (word0 >> 24) & 0x1f  # crate's slot number
                        # flag = (word0 >> 30) & 0x03
                        fb = -1
                        sector = -1
                        layer = -1
                        plane = -1
                        strip = -1
                        electId = (channel << 12) | (axis << 11) | (lane << 6) | (finesse << 4) | nodeID
                        if electId in self.electIdToModuleId:
                            moduleId = self.electIdToModuleId[electId]
                            fb = (moduleId & self.BKLM_SECTION_MASK) >> self.BKLM_SECTION_BIT
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
            j += 1
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
            nMuids[sector] += 1

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
            fb = (key & self.BKLM_SECTION_MASK) >> self.BKLM_SECTION_BIT
            phiStripMin = hit2d.getPhiStripMin() - 1
            phiStripMax = hit2d.getPhiStripMax() - 1
            zStripMin = hit2d.getZStripMin() - 1
            zStripMax = hit2d.getZStripMax() - 1
            sectorFB = sector if fb == 0 else sector + 8
            if layer >= 2:
                rpcHitCount[sectorFB] += 1
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
                jPrompt += 1
                promptXYGraph.SetPoint(jPrompt, x, y)
                promptZYGraph.SetPoint(jPrompt, z, y)
                nPoint = promptZYSGraph[sector].GetN()
                promptZYSGraph[sector].SetPoint(nPoint, z, abs(x * self.cosine[sector] + y * self.sine[sector]))
            else:
                jBkgd += 1
                bkgdXYGraph.SetPoint(jBkgd, x, y)
                bkgdZYGraph.SetPoint(jBkgd, z, y)
                nPoint = bkgdZYSGraph[sector].GetN()
                bkgdZYSGraph[sector].SetPoint(nPoint, z, abs(x * self.cosine[sector] + y * self.sine[sector]))

        hasEnoughRPCHits = False
        for count in rpcHitCount:
            if count > self.minRPCHits:
                hasEnoughRPCHits = True
                break
        if hasEnoughRPCHits and (len(muidhits) > self.minMuidHits):
            self.eventDisplays += 1
            title = 'e{0:02d}r{1}: event {2}'.format(int(self.exp), int(self.run), event)
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
            self.lastTitle = "Title:E{0} (#{1})".format(event, self.eventCounter)
            self.eventCanvas.Print(self.eventPdfName, self.lastTitle)
            for sector in range(0, 8):
                if nMuids[sector] > 0:
                    title = 'e{0:02d}r{1}: event {2} sector {3}'.format(int(self.exp), int(self.run), event, sector)
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
                    self.lastTitle = "Title:E{0} sector {1}".format(event, sector)
                    self.eventCanvas.Print(self.eventPdfName, self.lastTitle)
