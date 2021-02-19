#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   basf module to histogram useful values in RawKLM, KLMDigit, BKLMHit1d, and BKLMHit2d
#   data-objects in a DST ROOT file and to create BKLM event displays from these data-objects.
#

import basf2
import bklmDB
import math
import ROOT
from ROOT import Belle2


class EventInspector(basf2.Module):
    """Fill BKLM histograms of values from RawKLMs, KLMDigits, BKLMHit1ds, and BKLMHit2ds;
    (optionally) draw event displays from these data-objects."""

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

    def __init__(self, exp, run, histName, pdfName, eventPdfName, verbosity,
                 maxDisplays, minRPCHits, legacyTimes, singleEntry, view):
        """Constructor

        Arguments:
            exp (str): formatted experiment number
            run (str): formatter run number
            histName (str): path name of the output histogram ROOT file
            pdfName (str): path name of the output histogram PDF file
            eventPdfName (str): path name of the output event-display PDF file
            verbosity (int): determines how many histograms are written to the histogram PDF file
            maxDisplays (int): max # of events displays to write
            minRPCHits (int): min # of RPC BKLMHit2ds in any sector for event display
            legacyTimes (bool): true to correct BKLMHit{1,2}d times in legacy reconstruction, False otherwise
            singleEntry (int): select events with any (0) or exactly one (1) or more than one (2) entries/channel
            view (int): view event displays using one-dimensional (1) or two-dimensional (2) BKLMHits
        """
        super().__init__()
        #: internal copy of experiment number
        self.exp = exp
        #: internal copy of run number
        self.run = run
        #: internal copy of the pathname of the output histogram ROOT file
        self.histName = histName
        #: internal copy of the pathname of the output histogram PDF file
        self.pdfName = pdfName
        #: internal copy of the pathname of the output event-display PDF file
        self.eventPdfName = eventPdfName
        #: internal copy of the histogram verbosity in the histogram PDF file
        self.verbosity = verbosity
        #: internal copy of the maximum number of event displays to write
        self.maxDisplays = maxDisplays
        #: internal copy of the minimum number of RPC BKLMHit2ds in any sector for event display
        self.minRPCHits = minRPCHits
        #: calculate prompt time for legacy BKLMHit1ds and BKLMHit2ds (True) or use stored time (False)
        self.legacyTimes = legacyTimes
        #: select events with any (0) or exactly one (1) or more than one (2) entries/channel
        self.singleEntry = singleEntry
        #: view event displays using one-dimensional (1) or two-dimensional (2) hits
        self.view = view
        #: event counter (needed for PDF table of contents' ordinal event#)
        self.eventCounter = 0
        #: event-display counter
        self.eventDisplays = 0
        #: title of the last-drawn event display (needed for PDF table of contents' last event)
        self.lastTitle = ''

    def makeGraph(self, x, y):
        """Create and return a ROOT TGraph

        Arguments:
          x[] (real): x coordinates
          y[] (real): y coordinates
        """
        graph = ROOT.TGraph()
        for i in range(0, len(x)):
            graph.SetPoint(i, x[i], y[i])
        graph.SetLineColor(2)
        graph.SetLineWidth(1)
        return graph

    def makeText(self, x, y, s):
        """Create and return a ROOT TLatex with the following properties:
        size = 0.04, color = red, alignment = middle centre, angle = 90 degrees

        Arguments:
          x (real): x coordinate
          y (real): y coordinate
          s (str):  character string
        """
        text = ROOT.TLatex(x, y, s)
        text.SetTextSize(0.04)
        text.SetTextColor(2)
        text.SetTextAlign(22)
        text.SetTextAngle(90)
        return text

    def initialize(self):
        """Handle job initialization: fill the mapping database, create histograms, open the event-display file"""

        expRun = 'e{0:02d}r{1}: '.format(int(self.exp), int(self.run))
        #: blank scatterplot to define the bounds of the BKLM end view
        self.hist_XY = ROOT.TH2F('XY', ' ;x;y', 10, -345.0, 345.0, 10, -345.0, 345.0)
        self.hist_XY.SetStats(False)
        #: blank scatterplot to define the bounds of the BKLM side view for 1D hits
        self.hist_ZY1D = [0, 0]
        self.hist_ZY1D[0] = ROOT.TH2F('ZY0', ' ;z;y', 10, -200.0, 300.0, 10, -150.0, 350.0)
        self.hist_ZY1D[1] = ROOT.TH2F('ZY1', ' ;z;y', 10, -200.0, 300.0, 10, -150.0, 350.0)
        self.hist_ZY1D[0].SetStats(False)
        self.hist_ZY1D[0].SetStats(False)
        #: blank scatterplot to define the bounds of the BKLM side view for 2D hits
        self.hist_ZY = ROOT.TH2F('ZY', ' ;z;y', 10, -345.0, 345.0, 10, -345.0, 345.0)
        self.hist_ZY.SetStats(False)

        # All histograms/scatterplots in the output file will show '# of events' only
        ROOT.gStyle.SetOptStat(10)
        #: readout <-> detector map (from the information retrieved from the conditions database)
        self.electIdToModuleId = bklmDB.fillDB()
        #: map for sectorFB -> data concentrator
        self.sectorFBToDC = [11, 15, 2, 6, 10, 14, 3, 7, 9, 13, 0, 4, 8, 12, 1, 5]
        #: map for data concentrator -> sectorFB
        self.dcToSectorFB = [10, 14, 2, 6, 11, 15, 3, 7, 12, 8, 4, 0, 13, 9, 5, 1]
        #: Time-calibration constants obtained from experiment 7 run 1505
        #: RPC-time calibration adjustment (ns) for rawKLMs
        self.t0Cal = 312
        #: RPC-time calibration adjustment (ns) for BKLMHit1ds
        self.t0Cal1d = 325
        #: RPC-time calibration adjustment (ns) for BKLMHit2ds
        self.t0Cal2d = 308
        #: scint-ctime calibration adjustment (ns) for rawKLMs
        self.ct0Cal = 455
        #: scint-ctime calibration adjustment (ns) for BKLMHit1ds
        self.ct0Cal1d = 533
        #: scint-ctime calibration adjustment (ns) for BKLMHit2ds
        self.ct0Cal2d = 520
        #: per-layer variations in RPC z- and phi-time calibration adjustment (ns) for rawKLMs
        self.t0RPC = [[[0, 0, 17, 16, 15, 13, 14, 14, 16, 17, 18, 16, 16, 18, 16],
                       [0, 0, 0, -2, -1, -5, -6, -5, -5, -3, -4, -6, -8, -6, -8],
                       [0, 0, 6, 3, 5, 0, -1, -1, -2, 0, 0, -2, -3, -2, -4],
                       [0, 0, -4, -3, -6, -8, -8, -9, -9, -7, -7, -10, -10, -10, -12],
                       [0, 0, 6, 8, 4, 4, 3, 8, 4, 7, 7, 3, 5, 5, 4],
                       [0, 0, 18, 20, 18, 18, 16, 21, 17, 20, 20, 19, 21, 19, 20],
                       [0, 0, 19, 19, 19, 18, 17, 18, 22, 24, 25, 22, 22, 24, 22],
                       [0, 0, 19, 19, 18, 17, 16, 18, 18, 20, 21, 19, 20, 20, 20],
                       [0, 0, 6, 7, 9, 5, 4, 6, 6, 9, 9, 6, 7, 8, 7],
                       [0, 0, 3, 2, 2, -4, -1, -2, -2, 1, 0, -4, 246, -3, -4],
                       [0, 0, -1, -1, -1, -5, -6, -6, -8, -5, -4, -7, -7, -7, -8],
                       [0, 0, -5, -5, -5, -12, -9, -10, -8, -6, -10, -8, -8, -11, -12],
                       [0, 0, 12, 12, 13, 8, 6, 6, 7, 9, 10, 7, 6, 8, 5],
                       [0, 0, 14, 15, 43, 12, 10, 12, 11, 15, 16, 28, 14, 15, 14],
                       [0, 0, 22, 22, 21, 19, 19, 19, 21, 23, 24, 21, 22, 22, 22],
                       [0, 0, 18, 18, 17, 16, 16, 18, 18, 20, 21, 18, 18, 20, 19]],
                      [[0, 0, 6, 5, 4, 1, 1, 2, 3, 2, 2, -1, 0, 1, -1],
                       [0, 0, -11, -12, -11, -15, -18, -18, -18, -18, -19, -22, -23, -22, -24],
                       [0, 0, -4, -7, -6, -11, -12, -12, -14, -15, -15, -18, -19, -18, -20],
                       [0, 0, -15, -15, -16, -19, -22, -21, -22, -22, -22, -25, -26, -26, -27],
                       [0, 0, -5, -3, -6, -7, -9, -9, -9, -8, -8, -13, -12, -10, -13],
                       [0, 0, 6, 7, 5, 5, 3, 9, 4, 5, 6, 3, 5, 3, 4],
                       [0, 0, 9, 10, 10, 7, 7, 7, 9, 9, 9, 6, 6, 8, 8],
                       [0, 0, 7, 8, 7, 6, 4, 5, 4, 5, 5, 4, 3, 4, 3],
                       [0, 0, -5, -3, -1, -4, -8, -7, -7, -6, -6, -6, -9, -9, -9],
                       [0, 0, -8, -8, -11, -10, -14, -15, -16, -14, -15, -20, -20, -13, -20],
                       [0, 0, -12, -12, -14, -16, -16, -15, -21, -19, -19, -23, -23, -23, -24],
                       [0, 0, -15, -15, -15, -21, -22, -22, -22, -21, -23, -25, -25, -26, -27],
                       [0, 0, 0, 0, 2, -4, -5, -5, -4, -2, -1, -5, -5, -3, -7],
                       [0, 0, 3, 3, 32, 1, 0, -1, -3, 2, 1, 13, -1, 0, -2],
                       [0, 0, 11, 11, 10, 9, 6, 7, 6, 8, 8, 5, 6, 7, 6],
                       [0, 0, 7, 8, 7, 5, 3, 5, 7, 5, 5, 2, 7, 4, 3]]]

        #: per-layer variations in scint-ctime calibration adjustment (ns) for rawKLMs
        self.ct0Scint = [[[5, 7], [-27, -24], [-29, -45], [-27, -32], [3, 6], [34, 35], [48, 44], [33, 38],
                          [4, 7], [-28, -27], [-39, -34], [-36, -33], [2, 5], [25, 30], [46, 49], [41, 31]],
                         [[0, 0], [-32, -32], [-29, -54], [-31, -40], [-1, -1], [29, 27], [41, 41], [28, 28],
                          [-2, -1], [-32, -34], [-38, -45], [-40, -41], [-3, -3], [21, 20], [41, 42], [41, 19]]]

        #: Output ROOT TFile that will contain the histograms/scatterplots
        self.histogramFile = ROOT.TFile.Open(self.histName, "RECREATE")
        # All histograms/scatterplots in the output file will show '# of events' only
        ROOT.gStyle.SetOptStat(10)
        ROOT.gStyle.SetOptFit(111)

        # create the rawKLM histograms

        #: histogram of the number of BKLMDigits in the event
        self.hist_nDigit = ROOT.TH1F('NDigit', expRun + '# of BKLMDigits', 500, -0.5, 499.5)
        #: histogram of the number of RawKLMs in the event (should be 1)
        self.hist_nRawKLM = ROOT.TH1F('NRawKLM', expRun + '# of RawKLMs', 10, -0.5, 9.5)
        #: histogram of the RawKLM's NumEvents (should be 1)
        self.hist_rawKLMnumEvents = ROOT.TH1F('RawKLMnumEvents', expRun + 'RawKLM NumEvents;(should be 1)', 10, -0.5, 9.5)
        #: histogram of the RawKLM's NumNodes (should be 1)
        self.hist_rawKLMnumNodes = ROOT.TH1F('RawKLMnumNodes', expRun + 'RawKLM NumNodes;(should be 1)', 10, -0.5, 9.5)
        #: scatterplot of the RawKLM's COPPER index vs NodeID relative to the base BKLM/EKLM values
        self.hist_rawKLMnodeID = ROOT.TH2F('RawKLMnodeID',
                                           expRun + 'RawKLM NodeID;' +
                                           'NodeID (bklm: 1..4, eklm:5..8);' +
                                           'Copper index',
                                           10, -0.5, 9.5, 10, -0.5, 9.5)
        #: scatterplot of the RawKLM hit's lane vs flag (1=RPC, 2=Scint)
        self.hist_rawKLMlaneFlag = ROOT.TH2F('rawKLMlaneFlag',
                                             expRun + 'RawKLM lane vs flag;' +
                                             'Flag (1=RPC, 2=Scint);' +
                                             'Lane (scint: 1..7, RPC: 8..20)',
                                             4, -0.5, 3.5, 21, -0.5, 20.5)
        #: scatterplot of the RawKLM RPC hit's extra bits vs sector in the third (time) word
        self.hist_rawKLMtdcExtraRPC = ROOT.TH2F('rawKLMtdcExtraRPC',
                                                expRun + 'RawKLM RPC tdcExtra bits;' +
                                                'Sector # (0-7 = backward, 8-15 = forward);' +
                                                'tdcExtra [should be 0]',
                                                16, -0.5, 15.5, 32, -0.5, 31.5)
        #: scatterplot of the RawKLM RPC hit's extra bits vs sector in the fourth (adc) word
        self.hist_rawKLMadcExtraRPC = ROOT.TH2F('rawKLMadcExtraRPC',
                                                expRun + 'RawKLM RPC adcExtra bits;' +
                                                'Sector # (0-7 = backward, 8-15 = forward);' +
                                                'adcExtra [should be 0]',
                                                16, -0.5, 15.5, 16, -0.5, 15.5)
        #: scatterplot of the RawKLM scint hit's extra bits vs sector in the third (time) word
        self.hist_rawKLMtdcExtraScint = ROOT.TH2F('rawKLMtdcExtraScint',
                                                  expRun + 'RawKLM Scint tdcExtra bits;' +
                                                  'Sector # (0-7 = backward, 8-15 = forward);' +
                                                  'tdcExtra',
                                                  16, -0.5, 15.5, 32, -0.5, 31.5)
        #: scatterplot of the RawKLM scint hit's extra bits vs sector in the fourth (adc) word
        self.hist_rawKLMadcExtraScint = ROOT.TH2F('rawKLMadcExtraScint',
                                                  expRun + 'RawKLM Scint adcExtra bits;' +
                                                  'Sector # (0-7 = backward, 8-15 = forward);' +
                                                  'adcExtra',
                                                  16, -0.5, 15.5, 16, -0.5, 15.5)
        #: histogram of number of hits, including multiple entries on one readout channel
        self.hist_rawKLMsizeMultihit = ROOT.TH1F('rawKLMsizeMultihit', expRun + 'RawKLM word count (N/channel)', 400, -0.5, 799.5)
        #: histogram of number of hits, at most one entry per readout channel
        self.hist_rawKLMsize = ROOT.TH1F('rawKLMsize', expRun + 'RawKLM word count (1/channel)', 250, -0.5, 499.5)
        #: histograms of number of hits, including multiple entries on one readout channel, indexed by sector#
        self.hist_rawKLMsizeByDCMultihit = []
        #: histograms of number of hits, at most one entry per readout channel, indexed by sector#
        self.hist_rawKLMsizeByDC = []
        for sectorFB in range(0, 16):
            dc = self.sectorFBToDC[sectorFB]
            copper = dc & 0x03
            finesse = dc >> 2
            label = 'rawKLM_S{0:02d}_sizeMultihit'.format(sectorFB)
            title = '{0}sector {1} [COPPER {2} finesse {3}] word count (N/channel)'.format(expRun, sectorFB, copper, finesse)
            self.hist_rawKLMsizeByDCMultihit.append(ROOT.TH1F(label, title, 100, -0.5, 199.5))
            label = 'rawKLM_S{0:02d}_size'.format(sectorFB)
            title = '{0}sector {1} [COPPER {2} finesse {3}] word count (1/channel)'.format(expRun, sectorFB, copper, finesse)
            self.hist_rawKLMsizeByDC.append(ROOT.TH1F(label, title, 100, -0.5, 199.5))
        #: scatterplots of multiplicity of entries in one readout channel vs lane/axis, indexed by sector#
        self.hist_rawKLMchannelMultiplicity = []
        #: scatterplots of multiplicity of entries in one readout channel vs lane/axis/channel, indexed by sector#
        self.hist_rawKLMchannelMultiplicityFine = []
        for sectorFB in range(0, 16):
            dc = self.sectorFBToDC[sectorFB]
            copper = dc & 0x03
            finesse = dc >> 2
            label = 'rawKLM_S{0:02d}_channelMultiplicity'.format(sectorFB)
            title = '{0}sector {1} [COPPER {2} finesse {3}] per-channel multiplicity (N/channel > 1);'.format(
                expRun, sectorFB, copper, finesse) + 'Per-channel multiplicity;(Lane #) * 2 + (Axis #)'
            self.hist_rawKLMchannelMultiplicity.append(ROOT.TH2F(label, title, 30, -0.5, 29.5, 42, -0.5, 41.5))
            label = 'rawKLM_S{0:02d}_channelMultiplicityFine'.format(sectorFB)
            title = '{0}sector {1} [COPPER {2} finesse {3}] per-channel multiplicity (N/channel > 1);'.format(
                expRun, sectorFB, copper, finesse) + 'Per-channel multiplicity;(Lane #) * 256 + (Axis #) * 128 + (Channel #)'
            self.hist_rawKLMchannelMultiplicityFine.append(ROOT.TH2F(label, title, 30, -0.5, 29.5, 8192, -0.5, 8191.5))
        #: histogram of number of mapped hits by sector, including multiple entries on one readout channel
        self.hist_mappedSectorOccupancyMultihit = ROOT.TH1F(
            'mappedSectorOccupancyMultihit',
            expRun + 'Sector occupancy of mapped channels (N/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: histogram of number of unmapped hits by sector, including multiple entries on one readout channel
        self.hist_unmappedSectorOccupancyMultihit = ROOT.TH1F(
            'unmappedSectorOccupancyMultihit',
            expRun + 'Sector occupancy of unmapped channels (N/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: histogram of number of mapped hits by sector, at most one entry per readout channel
        self.hist_mappedSectorOccupancy = ROOT.TH1F(
            'mappedSectorOccupancy',
            expRun + 'Sector occupancy of mapped channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: histogram of number of unmapped hits by sector, at most one entry per readout channel
        self.hist_unmappedSectorOccupancy = ROOT.TH1F(
            'unmappedSectorOccupancy',
            expRun + 'Sector occupancy of unmapped channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: histogram of number of mapped RPC hits by sector, at most one entry per readout channel
        self.hist_mappedRPCSectorOccupancy = ROOT.TH1F(
            'mappedRPCSectorOccupancy',
            expRun + 'Sector occupancy of mapped RPC channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: scatterplot of number of mapped RPC hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_mappedRPCLaneAxisOccupancy = ROOT.TH2F(
            'mappedRPCLaneAxisOccupancy',
            expRun + 'Lane/axis occupancy of mapped RPC channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward);' +
            '(Lane #) * 2 + (Axis #)',
            16, -0.5, 15.5, 42, -0.5, 41.5)
        #: histogram of number of unmapped RPC hits by sector, at most one entry per readout channel
        self.hist_unmappedRPCSectorOccupancy = ROOT.TH1F(
            'unmappedRPCSectorOccupancy',
            expRun + 'Sector occupancy of unmapped RPC channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: scatterplot of number of unmapped RPC hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_unmappedRPCLaneAxisOccupancy = ROOT.TH2F(
            'unmappedRPCLaneAxisOccupancy',
            expRun + 'Lane/axis occupancy of unmapped RPC channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward);' +
            '(Lane #) * 2 + (Axis #)',
            16, -0.5, 15.5, 42, -0.5, 41.5)
        #: scatterplot of number of mapped scint hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_mappedScintSectorOccupancy = ROOT.TH1F(
            'mappedScintSectorOccupancy',
            expRun + 'Sector occupancy of mapped scint channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: scatterplot of number of mapped scint hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_mappedScintLaneAxisOccupancy = ROOT.TH2F(
            'mappedScintLaneAxisOccupancy',
            expRun + 'Lane/axis occupancy of mapped scint channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward);' +
            '(Lane #) * 2 + (Axis #)',
            16, -0.5, 15.5, 42, -0.5, 41.5)
        #: histogram of number of unmapped scint hits by sector, at most one entry per readout channel
        self.hist_unmappedScintSectorOccupancy = ROOT.TH1F(
            'unmappedScintSectorOccupancy',
            expRun + 'Sector occupancy of unmapped scint channels (1/channel);' +
            'Sector # (0-7 = backward, 8-15 = forward)',
            16, -0.5, 15.5)
        #: scatterplot of number of unmapped scint hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_unmappedScintLaneAxisOccupancy = ROOT.TH2F(
            'unmappedScintLaneAxisOccupancy',
            expRun + 'Lane/axis occupancy of unmapped scint channels (1/channel);' +
                     'Sector # (0-7 = backward, 8-15 = forward);' +
                     '(Lane #) * 2 + (Axis #)',
            16, -0.5, 15.5, 42, -0.5, 41.5)
        #: scatterplots of in-time mapped channel occupancy (1 hit per readout channel), indexed by sector#
        self.hist_mappedChannelOccupancyPrompt = [
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0],
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]
        #: scatterplots of out-of-time mapped channel occupancy (1 hit per readout channel), indexed by sector#
        self.hist_mappedChannelOccupancyBkgd = [
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0],
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]
        #: scatterplots of unmapped channel occupancy (1 hit per readout channel), indexed by sector#
        self.hist_unmappedChannelOccupancy = [
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0],
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]
        for sectorFB in range(0, 16):
            label = 'mappedChannelOccupancy_S{0:02d}ZPrompt'.format(sectorFB)
            title = '{0}In-time mapped channel occupancy for sector {1} z hits;lane;channel'.format(expRun, sectorFB)
            self.hist_mappedChannelOccupancyPrompt[sectorFB][0] = ROOT.TH2F(label, title, 42, -0.25, 20.75, 128, -0.25, 63.75)
            label = 'mappedChannelOccupancy_S{0:02d}ZBkgd'.format(sectorFB)
            title = '{0}Out-of-time mapped channel occupancy for sector {1} z hits;lane;channel'.format(expRun, sectorFB)
            self.hist_mappedChannelOccupancyBkgd[sectorFB][0] = ROOT.TH2F(label, title, 42, -0.25, 20.75, 128, -0.25, 63.75)
            label = 'unmappedChannelOccupancy_S{0:02d}Z'.format(sectorFB)
            title = '{0}Unmapped channel occupancy for sector {1} z hits;lane;channel'.format(expRun, sectorFB)
            self.hist_unmappedChannelOccupancy[sectorFB][0] = ROOT.TH2F(label, title, 42, -0.25, 20.75, 128, -0.25, 63.75)
            label = 'mappedChannelOccupancy_S{0:02d}PhiPrompt'.format(sectorFB)
            title = '{0}In-time mapped occupancy for sector {1} phi hits;lane;channel'.format(expRun, sectorFB)
            self.hist_mappedChannelOccupancyPrompt[sectorFB][1] = ROOT.TH2F(label, title, 42, -0.25, 20.75, 128, -0.25, 63.75)
            label = 'mappedChannelOccupancy_S{0:02d}PhiBkgd'.format(sectorFB)
            title = '{0}Out-of-time mapped occupancy for sector {1} phi hits;lane;channel'.format(expRun, sectorFB)
            self.hist_mappedChannelOccupancyBkgd[sectorFB][1] = ROOT.TH2F(label, title, 42, -0.25, 20.75, 128, -0.25, 63.75)
            label = 'unmappedChannelOccupancy_S{0:02d}Phi'.format(sectorFB)
            title = '{0}Unmapped channel occupancy for sector {1} phi hits;lane;channel'.format(expRun, sectorFB)
            self.hist_unmappedChannelOccupancy[sectorFB][1] = ROOT.TH2F(label, title, 42, -0.25, 20.75, 128, -0.25, 63.75)
        #: scatterplot of RPC TDC low-order bits vs sector (should be 0 since granularity is 4 ns)
        self.hist_RPCTimeLowBitsBySector = ROOT.TH2F('RPCTimeLowBitsBySector',
                                                     expRun + 'RPC TDC lowest-order bits;' +
                                                     'Sector # (0-7 = backward, 8-15 = forward);' +
                                                     'TDC % 4 (ns) [should be 0]',
                                                     16, -0.5, 15.5, 4, -0.5, 3.5)
        #: histogram of RPC mapped-channel TDC value relative to event's trigger time
        self.hist_mappedRPCTime = ROOT.TH1F(
            'mappedRPCTime', expRun + 'RPC mapped-strip time distribution;t - t(trigger) (ns)', 256, -0.5, 1023.5)
        #: histogram of RPC mapped-channel TDC value relative to event's trigger time, corrected for inter-sector variation
        self.hist_mappedRPCTimeCal = ROOT.TH1F(
            'mappedRPCTimeCal', expRun + 'RPC mapped-strip time distribution;t - t(trigger) - dt(layer) (ns)', 256, -0.5, 1023.5)
        #: histograms of RPC mapped-channel phi-strip TDC value relative to event's trigger time, indexed by sector/layer
        self.hist_mappedRPCPhiTimePerLayer = []
        #: histograms of RPC mapped-channel z-strip TDC value relative to event's trigger time, indexed by sector/layer
        self.hist_mappedRPCZTimePerLayer = []
        for sectorFB in range(0, 16):
            self.hist_mappedRPCPhiTimePerLayer.append([])
            self.hist_mappedRPCZTimePerLayer.append([])
            for layer in range(0, 15):
                label = 'mappedRPCPhiTime_S{0:02d}L{1:02d}'.format(sectorFB, layer)
                title = '{0}RPC sector {1} layer {2} phi time distribution;t - t(trigger) (ns)'.format(expRun, sectorFB, layer)
                self.hist_mappedRPCPhiTimePerLayer[sectorFB].append(ROOT.TH1F(label, title, 256, -0.5, 1023.5))
                label = 'mappedRPCZTime_S{0:02d}L{1:02d}'.format(sectorFB, layer)
                title = '{0}RPC sector {1} layer {2} z time distribution;t - t(trigger) (ns)'.format(expRun, sectorFB, layer)
                self.hist_mappedRPCZTimePerLayer[sectorFB].append(ROOT.TH1F(label, title, 256, -0.5, 1023.5))
        #: scatterplot of RPC mapped-channel TDC value relative to event's trigger time vs sector
        self.hist_mappedRPCTimeBySector = ROOT.TH2F('mappedRPCTimeBySector',
                                                    expRun + 'RPC mapped-strip time;' +
                                                    'Sector # (0-7 = backward, 8-15 = forward);' +
                                                    't - t(trigger) (ns)',
                                                    16, -0.5, 15.5, 128, -0.5, 1023.5)
        #: scatterplot of RPC mapped-channel TDC relative to trigger time, corrected for inter-sector variation, by sector
        self.hist_mappedRPCTimeCalBySector = ROOT.TH2F('mappedRPCTimeCalBySector',
                                                       expRun + 'RPC mapped-strip time;' +
                                                       'Sector # (0-7 = backward, 8-15 = forward);' +
                                                       't - t(trigger) - dt(layer) (ns)',
                                                       16, -0.5, 15.5, 128, -0.5, 1023.5)
        #: histogram of RPC mapped-channel REVO9 range in event
        self.hist_mappedRPCCtimeRange = ROOT.TH1F('mappedRPCCtimeRange',
                                                  expRun + 'RPC Ctime-range in event;' +
                                                  'CtimeMax - CtimeMin (ns)',
                                                  128, -0.5, 8191.5)
        #: scatterplot of RPC mapped-channel REVO9 range in event vs sector
        self.hist_mappedRPCCtimeRangeBySector = ROOT.TH2F('mappedRPCCtimeRangeBySector',
                                                          expRun + 'RPC Ctime-range in event;' +
                                                          'Sector # (0-7 = backward, 8-15 = forward);' +
                                                          'CtimeMax - CtimeMin (ns)',
                                                          16, -0.5, 15.5, 128, -0.5, 8191.5)
        #: histogram of RPC unmapped-channel TDC value relative to event's trigger time
        self.hist_unmappedRPCTime = ROOT.TH1F('unmappedRPCTime',
                                              expRun + 'RPC unmapped-strip time distribution;' +
                                              't - t(trigger) (ns)',
                                              256, -0.5, 1023.5)
        #: scatterplot of RPC unmapped-channel TDC value relative to event's trigger time, by sector
        self.hist_unmappedRPCTimeBySector = ROOT.TH2F('unmappedRPCTimeBySector',
                                                      expRun + 'RPC unmapped-strip time;' +
                                                      'Sector # (0-7 = backward, 8-15 = forward);' +
                                                      't - t(trigger) (ns)',
                                                      16, -0.5, 15.5, 128, -0.5, 1023.5)
        #: scatterplot of scint TDC low-order bits vs sector
        self.hist_ScintTimeLowBitsBySector = ROOT.TH2F('ScintTimeLowBitsBySector',
                                                       expRun + 'Scint TDC lowest-order bits;' +
                                                       'Sector # (0-7 = backward, 8-15 = forward);' +
                                                       'TDC % 4 (ns)',
                                                       16, -0.5, 15.5, 4, -0.5, 3.5)
        #: histogram of scint mapped-channel CTIME value relative to event's trigger Ctime
        self.hist_mappedScintCtime = ROOT.TH1F('mappedScintCtime',
                                               expRun + 'Scint mapped-strip ctime distribution;' +
                                               'ctime - ct(trigger) (ns)',
                                               32, -0.5, 1023.5)
        #: scatterplot of scint mapped-channel CTIME value relative to event's trigger Ctime vs sector
        self.hist_mappedScintCtimeBySector = ROOT.TH2F('mappedScintCtimeBySector',
                                                       expRun + 'Scint mapped-strip ctime;' +
                                                       'Sector # (0-7 = backward, 8-15 = forward);' +
                                                       'ctime - ct(trigger) (ns)',
                                                       16, -0.5, 15.5, 32, -0.5, 1023.5)
        #: histogram of scint mapped-channel CTIME value relative to event's trigger Ctime, corrected for inter-sector variation
        self.hist_mappedScintCtimeCal = ROOT.TH1F('mappedScintCtimeCal',
                                                  expRun + 'Scint mapped-strip ctime distribution;' +
                                                  'ctime - ct(trigger) - dt(layer) (ns)',
                                                  32, -0.5, 1023.5)
        #: scatterplot of scint mapped-channel CTIME relative to trigger Ctime, corrected for inter-sector variation, by sector
        self.hist_mappedScintCtimeCalBySector = ROOT.TH2F('mappedScintCtimeCalBySector',
                                                          expRun + 'Scint mapped-strip ctime;' +
                                                          'Sector # (0-7 = backward, 8-15 = forward);' +
                                                          'ctime - ct(trigger) - dt(layer) (ns)',
                                                          16, -0.5, 15.5, 32, -0.5, 1023.5)
        #: histograms of scint mapped-channel phi-strip CTIME value relative to event's trigger Ctime, indexed by sector/layer
        self.hist_mappedScintPhiCtimePerLayer = []
        #: histograms of scint mapped-channel z-strip CTIME value relative to event's trigger Ctime, indexed by sector/layer
        self.hist_mappedScintZCtimePerLayer = []
        for sectorFB in range(0, 16):
            self.hist_mappedScintPhiCtimePerLayer.append([])
            self.hist_mappedScintZCtimePerLayer.append([])
            for layer in range(0, 2):
                label = 'mappedScintPhiCtime_S{0:02d}L{1:02d}'.format(sectorFB, layer)
                title = '{0}Scint sector {1} layer {2} phi ctime distribution;ctime - ct(trigger) (ns)'.format(expRun,
                                                                                                               sectorFB, layer)
                self.hist_mappedScintPhiCtimePerLayer[sectorFB].append(ROOT.TH1F(label, title, 32, -0.5, 1023.5))
                label = 'mappedScintZCtime_S{0:02d}L{1:02d}'.format(sectorFB, layer)
                title = '{0}Scint sector {1} layer {2} z ctime distribution;ctime - ct(trigger) (ns)'.format(expRun,
                                                                                                             sectorFB, layer)
                self.hist_mappedScintZCtimePerLayer[sectorFB].append(ROOT.TH1F(label, title, 32, -0.5, 1023.5))
        #: histogram of scint mapped-channel CTIME range in event
        self.hist_mappedScintCtimeRange = ROOT.TH1F('mappedScintCtimeRange',
                                                    expRun + 'Scint ctime-range in event;' +
                                                    'ctimeMax - ctimeMin (ns)',
                                                    128, -0.5, 1023.5)
        #: scatterplot of scint mapped-channel CTIME range in event vs sector
        self.hist_mappedScintCtimeRangeBySector = ROOT.TH2F('mappedScintCtimeRangeBySector',
                                                            expRun + 'Scint ctime-range in event;' +
                                                            'Sector # (0-7 = backward, 8-15 = forward);' +
                                                            'ctimeMax - ctimeMin (ns)',
                                                            16, -0.5, 15.5, 128, -0.5, 1023.5)
        #: histogram of scint unmapped-channel CTIME value relative to event's trigger Ctime
        self.hist_unmappedScintCtime = ROOT.TH1F('unmappedScintCtime',
                                                 expRun + 'Scint unmapped-strip ctime distribution;' +
                                                 'ctime - ct(trigger) (ns)',
                                                 32, -0.5, 1023.5)
        #: scatterplot of scint unmapped-channel CTIME value relative to event's trigger Ctime, by sector
        self.hist_unmappedScintCtimeBySector = ROOT.TH2F('unmappedScintCtimeBySector',
                                                         expRun + 'Scint unmapped-strip ctime;' +
                                                         'Sector # (0-7 = backward, 8-15 = forward);' +
                                                         'ctime - ct(trigger) (ns)',
                                                         16, -0.5, 15.5, 32, -0.5, 1023.5)
        #: histogram of scint mapped-channel TDC value (NOT relative to event's trigger Ctime)
        self.hist_mappedScintTDC = ROOT.TH1F('mappedScintTDC',
                                             expRun + 'Scint mapped-strip TDC distribution;' +
                                             't (ns)',
                                             32, -0.5, 31.5)
        #: histogram of scint mapped-channel TDC value relative to event's trigger Ctime
        self.hist_mappedScintTime = ROOT.TH1F('mappedScintTime',
                                              expRun + 'Scint mapped-strip time distribution;' +
                                              't - t(trigger) (ns)', 32, -0.5, 31.5)
        #: scatterplot of scint mapped-channel TDC value (NOT relative to event's trigger Ctime) vs sector
        self.hist_mappedScintTDCBySector = ROOT.TH2F('mappedScintTDCBySector',
                                                     expRun + 'Scint mapped-strip TDC;' +
                                                     'Sector # (0-7 = backward, 8-15 = forward);' +
                                                     't (ns)',
                                                     16, -0.5, 15.5, 32, -0.5, 31.5)
        #: scatterplot of scint mapped-channel TDC value relative to event's trigger Ctime vs sector
        self.hist_mappedScintTimeBySector = ROOT.TH2F('mappedScintTimeBySector',
                                                      expRun + 'Scint mapped-strip time;' +
                                                      'Sector # (0-7 = backward, 8-15 = forward);' +
                                                      't - t(trigger) (ns)',
                                                      16, -0.5, 15.5, 32, -0.5, 31.5)
        #: histogram of scint unmapped-channel TDC value relative to event's trigger Ctime
        self.hist_unmappedScintTime = ROOT.TH1F('unmappedScintTime',
                                                expRun + 'Scint unmapped-strip time distribution;' +
                                                't - t(trigger) (ns)',
                                                32, -0.5, 31.5)
        #: scatterplot of scint unmapped-channel TDC value relative to event's trigger Ctime vs sector
        self.hist_unmappedScintTimeBySector = ROOT.TH2F('unmappedScintTimeBySector',
                                                        expRun + 'Scint unmapped-strip time;' +
                                                        'Sector # (0-7 = backward, 8-15 = forward);' +
                                                        't - t(trigger) (ns)',
                                                        16, -0.5, 15.5, 32, -0.5, 31.5)

        # Create the RPC time-calibration/diagnostic histograms

        #: histogram of RawKLM[] header's trigger CTIME relative to its final-data-word trigger REVO9 time
        self.hist_trigCtimeVsTrigRevo9time = ROOT.TH1F('trigCtimeVsTrigRevo9time',
                                                       expRun + 'trigCtime - trigRevo9time (ns)',
                                                       256, -1024.5, 1023.5)
        #: histogram of RPC TDC range
        self.hist_tdcRangeRPC = ROOT.TH1F('tdcRangeRPC',
                                          expRun + 'RPC TDC range;' +
                                          'maxTDC - minTDC (ns)',
                                          128, -0.5, 1023.5)
        #: histogram of RPC Ctime range
        self.hist_ctimeRangeRPC = ROOT.TH1F('ctimeRangeRPC',
                                            expRun + 'RPC Ctime range;' +
                                            'maxCtime - minCtime (ns)',
                                            128, -0.5, 1023.5)
        #: scatterplot of RPC TDC range vs Ctime range
        self.hist_tdcRangeVsCtimeRangeRPC = ROOT.TH2F('tdcRangeVsCtimeRangeRPC',
                                                      expRun + 'RPC Ctime range vs TDC range;' +
                                                      'maxTDC - minTDC (ns);' +
                                                      'maxCtime - minCtime (ns)',
                                                      128, -0.5, 1023.5, 128, -0.5, 1023.5)
        #: scatterplot of RPC TDC range vs time
        self.hist_tdcRangeVsTimeRPC = ROOT.TH2F('tdcRangeVsTimeRPC',
                                                expRun + 'RPC TDC range vs time;' +
                                                't - t(trigger) (ns);' +
                                                'maxTDC - minTDC (ns)',
                                                128, -0.5, 1023.5, 128, -0.5, 1023.5)
        #: scatterplot of RPC Ctime range vs time
        self.hist_ctimeRangeVsTimeRPC = ROOT.TH2F('ctimeRangeVsTimeRPC',
                                                  expRun + 'RPC Ctime range vs time;' +
                                                  't - t(trigger) (ns);' +
                                                  'maxCtime - minCtime (ns)',
                                                  128, -0.5, 1023.5, 128, -0.5, 1023.5)

        # Create the BKLMHit1d-related histograms

        #: histogram of the number of BKLMHit1ds
        self.hist_nHit1d = ROOT.TH1F('NHit1d', expRun + '# of BKLMHit1ds', 100, -0.5, 99.5)
        #: histogram of the number of in-time RPC BKLMHit1ds
        self.hist_nHit1dRPCPrompt = ROOT.TH1F('NHit1dRPCPrompt', expRun + '# of prompt RPC BKLMHit1ds', 100, -0.5, 99.5)
        #: histogram of the number of out-of-time RPC BKLMHit1ds
        self.hist_nHit1dRPCBkgd = ROOT.TH1F('NHit1dRPCBkgd', expRun + '# of background RPC BKLMHit1ds', 100, -0.5, 99.5)
        #: histogram of the number of scint BKLMHit1ds
        self.hist_nHit1dScint = ROOT.TH1F('NHit1dScint', expRun + '# of scintillator BKLMHit1ds', 100, -0.5, 99.5)
        #: histogram of the number of in-time scint BKLMHit1ds
        self.hist_nHit1dPrompt = ROOT.TH1F('NHit1dPrompt', expRun + '# of prompt BKLMHit1ds', 100, -0.5, 99.5)
        #: histogram of the number of out-of-time scint BKLMHit1ds
        self.hist_nHit1dBkgd = ROOT.TH1F('NHit1dBkgd', expRun + '# of bkgd BKLMHit1ds', 100, -0.5, 99.5)
        #: scatterplot of #Z BKLMHit1ds vs #Phi BKLMHit1ds
        self.hist_n1dPhiZ = ROOT.TH2F('NHit1dPhiZ',
                                      expRun + 'Distribution of BKLMHit1ds;# of phi BKLMHit1ds;# of z BKLMHit1ds',
                                      60, -0.5, 59.5, 60, -0.5, 59.5)
        #: scatterplot of #Phi BKLMHit1ds vs sector
        self.hist_multiplicityPhiBySector = ROOT.TH2F('Hit1dMultiplicityPhiBySector',
                                                      expRun + 'BKLMHit1d phi-strip multiplicity;' +
                                                      'sector # (0-7 = backward, 8-15 = forward);' +
                                                      '# of strips',
                                                      16, -0.5, 15.5, 8, -0.5, 7.5)
        #: scatterplot of #Z BKLMHit1ds vs sector
        self.hist_multiplicityZBySector = ROOT.TH2F('Hit1dMultiplicityZBySector',
                                                    expRun + 'BKLMHit1d z-strip multiplicity;' +
                                                    'sector # (0-7 = backward, 8-15 = forward);' +
                                                    '# of strips',
                                                    16, -0.5, 15.5, 8, -0.5, 7.5)
        #: histogram of RPC-phi BKLMHit1d time relative to event's trigger time, corrected for inter-sector variation
        self.hist_tphiRPCCal1d = ROOT.TH1F('tphiRPCCal1d',
                                           expRun + 'RPC BKLMHit1d phi-strip time distribution;' +
                                           't(phi1D) - dt(layer) (ns)',
                                           256, -0.5, 1023.5)
        #: histogram of RPC-z BKLMHit1d time relative to event's trigger time, corrected for inter-sector variation
        self.hist_tzRPCCal1d = ROOT.TH1F('tzRPCCal1d',
                                         expRun + 'RPC BKLMHit1d z-strip time distribution;' +
                                         't(z1D) - dt(layer) (ns)',
                                         256, -0.5, 1023.5)
        #: histogram of RPC-phi and -z BKLMHit1d avg time relative to event's trigger time, corrected for inter-sector variation
        self.hist_tRPCCal1d = ROOT.TH1F('tRPCCal1d',
                                        expRun + 'RPC BKLMHit1d x 2 calibrated average-time distribution;' +
                                        '0.5*[t(phi1D) + t(z1D)] - dt(layer) (ns)',
                                        256, -0.5, 1023.5)
        #: histogram of RPC-phi and -z BKLMHit1d time difference
        self.hist_dtRPC1d = ROOT.TH1F('dtRPC1d',
                                      expRun + 'RPC BKLMHit1d x 2 time-difference distribution;' +
                                      't(phi1D) - t(z1D) (ns)',
                                      50, -100.0, 100.0)
        #: histogram of scint-phi BKLMHit1d time relative to event's trigger Ctime, corrected for inter-sector variation
        self.hist_ctphiScintCal1d = ROOT.TH1F('ctphiScintCal1d',
                                              expRun + 'Scintillator BKLMHit1d phi-strip ctime distribution;' +
                                              't(phi1D) - dt(layer) (ns)',
                                              128, -0.5, 1023.5)
        #: histogram of scint-z BKLMHit1d time relative to event's trigger Ctime, corrected for inter-sector variation
        self.hist_ctzScintCal1d = ROOT.TH1F('ctzScintCal1d',
                                            expRun + 'Scintillator BKLMHit1d z-strip ctime distribution;' +
                                            't(z1D) - dt(layer) (ns)',
                                            128, -0.5, 1023.5)
        #: histogram of scint-phi and -z BKLMHit1d avg time relative to event's trigger Ctime, corrected for inter-sector variation
        self.hist_ctScintCal1d = ROOT.TH1F('ctScintCal1d',
                                           expRun + 'Scintillator BKLMHit1d x 2 calibrated average-time distribution;' +
                                           '0.5*[t(phi1D) + t(z1D)] - dt(layer) (ns)',
                                           128, -0.5, 1023.5)
        #: histogram of scint-phi and -z BKLMHit1d time difference
        self.hist_dtScint1d = ROOT.TH1F('dtScint1d',
                                        expRun + 'Scintillator BKLMHit1d x 2 time-difference distribution;' +
                                        't(phi1D) - t(z1D) (ns)',
                                        50, -100.0, 100.0)

        # Create the BKLMHit2d-related histograms

        #: histogram of the number of BKLMHit2ds
        self.hist_nHit2d = ROOT.TH1F('NHit2d', expRun + '# of BKLMHit2ds', 50, -0.5, 49.5)
        #: scatterplot of end view of forward BKLM for in-time BKLMHit2ds
        self.hist_occupancyForwardXYPrompt = ROOT.TH2F('occupancyForwardXYPrompt',
                                                       expRun + 'Forward xy RPC occupancy for in-time hits;x(cm);y(cm)',
                                                       230, -345.0, 345.0, 230, -345.0, 345.0)
        #: scatterplot of end view of backward BKLM for in-time BKLMHit2ds
        self.hist_occupancyBackwardXYPrompt = ROOT.TH2F('occupancyBackwardXYPrompt',
                                                        expRun + 'Backward xy RPC occupancy for in-time hits;x(cm);y(cm)',
                                                        230, -345.0, 345.0, 230, -345.0, 345.0)
        #: scatterplot of end view of forward BKLM for out-of-time BKLMHit2ds
        self.hist_occupancyForwardXYBkgd = ROOT.TH2F('occupancyForwardXYBkgd',
                                                     expRun + 'Forward xy RPC occupancy for out-of-time hits;x(cm);y(cm)',
                                                     230, -345.0, 345.0, 230, -345.0, 345.0)
        #: scatterplot of end view of backward BKLM for out-of-time BKLMHit2ds
        self.hist_occupancyBackwardXYBkgd = ROOT.TH2F('occupancyBackwardXYBkgd',
                                                      expRun + 'Backward xy RPC occupancy for out-of-time hits;x(cm);y(cm)',
                                                      230, -345.0, 345.0, 230, -345.0, 345.0)
        #: scatterplot of side view of forward BKLM for in-time BKLMHit2ds
        self.hist_occupancyRZPrompt = ROOT.TH2F('occupancyRZPrompt',
                                                expRun + 'layer-z occupancy for in-time hits;z(cm);layer',
                                                48, -190.0, 290.0, 16, -0.5, 15.5)
        #: histogram of z coordinate for in-time BKLMHit2ds
        self.hist_occupancyZPrompt = ROOT.TH1F('occupancyZPrompt',
                                               expRun + 'z occupancy for in-time hits;z(cm)',
                                               48, -190.0, 290.0)
        #: histogram of layer# for in-time BKLMHit2ds
        self.hist_occupancyRPrompt = ROOT.TH1F('occupancyRPrompt',
                                               expRun + 'layer occupancy for in-time hits;layer',
                                               16, -0.5, 15.5)
        #: scatterplot of side view of forward BKLM for in-time BKLMHit2ds
        self.hist_occupancyRZBkgd = ROOT.TH2F('occupancyRZBkgd',
                                              expRun + 'layer-z occupancy for out-of-time hits;z(cm);layer',
                                              48, -190.0, 290.0, 16, -0.5, 15.5)
        #: histogram of z coordinate for out-of-time BKLMHit2ds
        self.hist_occupancyZBkgd = ROOT.TH1F('occupancyZBkgd',
                                             expRun + 'z occupancy for out-of-time hits;z(cm)',
                                             48, -190.0, 290.0)
        #: histogram of layer# for out-of-time BKLMHit2ds
        self.hist_occupancyRBkgd = ROOT.TH1F('occupancyRBkgd',
                                             expRun + 'layer occupancy for out-of-time hits;layer',
                                             16, -0.5, 15.5)
        #: histogram of RPC calibrated time in BKLMHit2ds
        self.hist_tRPCCal2d = ROOT.TH1F('tRPCCal2d',
                                        expRun + 'RPC BKLMHit2d time distribution;' +
                                        't(2D) - dt(layer) (ns)',
                                        256, -0.5, 1023.5)
        #: scatterplot of RPC calibrated time in BKLMHit2ds vs sector
        self.hist_tRPCCal2dBySector = ROOT.TH2F('tRPCCal2dBySector',
                                                expRun + 'RPC BKLMHit2d time distribution;' +
                                                'sector # (0-7 = backward, 8-15 = forward);' +
                                                't(2D) - dt(layer) (ns)',
                                                16, -0.5, 15.5, 256, -0.5, 1023.5)
        #: histogram of scint calibrated time in BKLMHit2ds
        self.hist_ctScintCal2d = ROOT.TH1F('ctScintCal2d',
                                           expRun + 'Scint BKLMHit2d ctime distribution;' +
                                           't(2D) - dt(layer) (ns)',
                                           128, -0.5, 1023.5)
        #: scatterplot of scint calibrated time in BKLMHit2ds vs sector
        self.hist_ctScintCal2dBySector = ROOT.TH2F('ctScintCal2dBySector',
                                                   expRun + 'Scint BKLMHit2d ctime distribution;' +
                                                   'sector # (0-7 = backward, 8-15 = forward);' +
                                                   't(2D) - dt(layer) (ns)',
                                                   16, -0.5, 15.5, 128, -0.5, 1023.5)

        #: profile histogram of BKLMHit2d RPC time vs z (forward)
        self.hist_tVsZFwd = ROOT.TProfile("tVsZForward",
                                          expRun + 'RPC BKLMHit2d time vs z (forward);' +
                                          'z (cm);' +
                                          't(2D) - dt(layer) (ns)',
                                          48, 0.0, 216.96)
        #: profile histogram of BKLMHit2d RPC time vs z (forward)
        self.hist_tVsZBwd = ROOT.TProfile("tVsZBackward",
                                          expRun + 'RPC BKLMHit2d time vs z (backward);' +
                                          'z (cm);' +
                                          't(2D) - dt(layer) (ns)',
                                          48, 0.0, 216.96)
        # Open the output PDF file for event displays

        if self.maxDisplays > 0:
            #: TCanvas on which event displays will be drawn
            self.eventCanvas = ROOT.TCanvas("eventCanvas", self.eventPdfName, 3200, 1600)
            title = '{0}['.format(self.eventPdfName)
            self.eventCanvas.SaveAs(title)
            self.eventCanvas.Clear()
            self.eventCanvas.Divide(2, 1)

        # Create the boilerplate for the end- and side-views of the event display

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

    def terminate(self):
        """Handle job termination: draw histograms, close output files"""

        if self.maxDisplays > 0:
            pdfNameLast = '{0}]'.format(self.eventPdfName)
            self.eventCanvas.Print(pdfNameLast, self.lastTitle)

        for sectorFB in range(0, 16):
            mappedScintSectorOccupancy = self.hist_mappedScintSectorOccupancy.GetBinContent(sectorFB + 1)
            if mappedScintSectorOccupancy > 0:
                for laneAxis in range(0, 42):
                    numerator = self.hist_mappedScintLaneAxisOccupancy.GetBinContent(sectorFB + 1, laneAxis + 1)
                    self.hist_mappedScintLaneAxisOccupancy.SetBinContent(
                        sectorFB + 1, laneAxis + 1, 100.0 * numerator / mappedScintSectorOccupancy)
            mappedRPCSectorOccupancy = self.hist_mappedRPCSectorOccupancy.GetBinContent(sectorFB + 1)
            if mappedRPCSectorOccupancy > 0:
                for laneAxis in range(0, 42):
                    numerator = self.hist_mappedRPCLaneAxisOccupancy.GetBinContent(sectorFB + 1, laneAxis + 1)
                    self.hist_mappedRPCLaneAxisOccupancy.SetBinContent(
                        sectorFB + 1, laneAxis + 1, 100.0 * numerator / mappedRPCSectorOccupancy)
            unmappedScintSectorOccupancy = self.hist_unmappedScintSectorOccupancy.GetBinContent(sectorFB + 1)
            if unmappedScintSectorOccupancy > 0:
                for laneAxis in range(0, 42):
                    numerator = self.hist_unmappedScintLaneAxisOccupancy.GetBinContent(sectorFB + 1, laneAxis + 1)
                    self.hist_unmappedScintLaneAxisOccupancy.SetBinContent(
                        sectorFB + 1, laneAxis + 1, 100.0 * numerator / unmappedScintSectorOccupancy)
            unmappedRPCSectorOccupancy = self.hist_unmappedRPCSectorOccupancy.GetBinContent(sectorFB + 1)
            if unmappedRPCSectorOccupancy > 0:
                for laneAxis in range(0, 42):
                    numerator = self.hist_unmappedRPCLaneAxisOccupancy.GetBinContent(sectorFB + 1, laneAxis + 1)
                    self.hist_unmappedRPCLaneAxisOccupancy.SetBinContent(
                        sectorFB + 1, laneAxis + 1, 100.0 * numerator / unmappedRPCSectorOccupancy)
        canvas = ROOT.TCanvas("canvas", self.pdfName, 1600, 1600)
        title = '{0}['.format(self.pdfName)
        canvas.SaveAs(title)
        canvas.Clear()
        canvas.GetPad(0).SetGrid(1, 1)
        canvas.GetPad(0).Update()
        self.hist_nDigit.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nDigit.GetName()))
        if self.verbosity > 0:
            self.hist_nRawKLM.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nRawKLM.GetName()))
            self.hist_rawKLMnumEvents.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMnumEvents.GetName()))
            self.hist_rawKLMnumNodes.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMnumNodes.GetName()))
            self.hist_rawKLMnodeID.Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMnodeID.GetName()))
            self.hist_rawKLMlaneFlag.Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMlaneFlag.GetName()))
        # self.hist_rawKLMtdcExtraRPC.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMtdcExtraRPC.GetName()))
        # self.hist_rawKLMadcExtraRPC.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMadcExtraRPC.GetName()))
        # self.hist_rawKLMtdcExtraScint.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMtdcExtraScint.GetName()))
        # self.hist_rawKLMadcExtraScint.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMadcExtraScint.GetName()))
        if self.verbosity > 0:
            self.hist_rawKLMsizeMultihit.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMsizeMultihit.GetName()))
            for dc in range(0, 16):
                self.hist_rawKLMsizeByDCMultihit[dc].Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMsizeByDCMultihit[dc].GetName()))
        self.hist_rawKLMsize.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMsize.GetName()))
        if self.verbosity > 0:
            for dc in range(0, 16):
                self.hist_rawKLMsizeByDC[dc].Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMsizeByDC[dc].GetName()))
        for dc in range(0, 16):
            self.hist_rawKLMchannelMultiplicity[dc].Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMchannelMultiplicity[dc].GetName()))
        # self.hist_mappedSectorOccupancy.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedSectorOccupancy.GetName()))
        # self.hist_unmappedSectorOccupancy.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedSectorOccupancy.GetName()))
        if self.verbosity > 0:
            self.hist_mappedRPCSectorOccupancy.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCSectorOccupancy.GetName()))
            self.hist_unmappedRPCSectorOccupancy.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedRPCSectorOccupancy.GetName()))
            self.hist_unmappedScintSectorOccupancy.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedScintSectorOccupancy.GetName()))
            self.hist_mappedScintSectorOccupancy.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintSectorOccupancy.GetName()))
        canvas.Clear()
        canvas.Divide(2, 1)
        canvas.GetPad(0).SetGrid(1, 1)
        canvas.GetPad(1).SetGrid(1, 1)
        canvas.GetPad(2).SetGrid(1, 1)
        # border of mapped z-readout channels for RPCs (axis=0)
        borderRPC0x = [7.5, 20.5, 20.5, 7.5, 7.5]
        borderRPC0y = [0.5, 0.5, 48.5, 48.5, 0.5]
        borderRPC0yChimney = [0.5, 0.5, 34.5, 34.5, 0.5]
        # border of mapped phi-readout channels for scints (axis=0)
        borderScint0x = [0.5, 1.5, 1.5, 2.5, 2.5, 1.5, 1.5, 0.5, 0.5]
        borderScint0y = [4.5, 4.5, 2.5, 2.5, 44.5, 44.5, 41.5, 41.5, 4.5]
        # border of mapped phi-readout channels for RPCs (axis=1)
        borderRPC1x = [7.5, 20.5, 20.5, 11.5, 11.5, 7.5, 7.5]
        borderRPC1y = [0.5, 0.5, 48.5, 48.5, 36.5, 36.5, 0.5]
        # border of mapped z-readout channels for scints (axis=1)
        borderScint1x = [0.5, 2.5, 2.5, 0.5, 0.5]
        borderScint1ay = [0.5, 0.5, 9.5, 9.5, 0.5]
        borderScint1by = [15.5, 15.5, 60.5, 60.5, 15.5]
        borderScint1xChimney = [0.5, 1.5, 1.5, 2.5, 2.5, 1.5, 1.5, 0.5, 0.5]
        borderScint1ayChimney = [0.5, 0.5, 0.5, 0.5, 9.5, 9.5, 8.5, 8.5, 0.5]
        borderScint1byChimney = [15.5, 15.5, 16.5, 16.5, 45.5, 45.5, 45.5, 45.5, 15.5]
        # graphs of z-readout-channel borders for RPCs (axis=0)
        graphRPC0 = self.makeGraph(borderRPC0x, borderRPC0y)
        graphRPC0Chimney = self.makeGraph(borderRPC0x, borderRPC0yChimney)
        # graph of phi-readout-channel border for scints (axis=0)
        graphScint0 = self.makeGraph(borderScint0x, borderScint0y)
        # graph of phi-readout-channel border for RPCs (axis=1)
        graphRPC1 = self.makeGraph(borderRPC1x, borderRPC1y)
        # graphs of z-readout-channel borders for scints (axis=1)
        graphScint1a = self.makeGraph(borderScint1x, borderScint1ay)
        graphScint1b = self.makeGraph(borderScint1x, borderScint1by)
        graphScint1aChimney = self.makeGraph(borderScint1xChimney, borderScint1ayChimney)
        graphScint1bChimney = self.makeGraph(borderScint1xChimney, borderScint1byChimney)
        # labels for the above borders
        textRPC0 = self.makeText(6.8, 25.0, "RPC z")
        textScint0 = self.makeText(3.2, 25.0, "Scint #phi")
        textRPC1 = self.makeText(6.8, 25.0, "RPC #phi")
        textScint1 = self.makeText(3.2, 25.0, "Scint z")
        for sectorFB in range(0, 16):
            zmax = 1
            for lane in range(8, 21):
                for channel in range(0, 64):
                    z = self.hist_mappedChannelOccupancyPrompt[sectorFB][0].GetBinContent(lane + 1, channel + 1)
                    zmax = z if z > zmax else zmax
            self.hist_mappedChannelOccupancyPrompt[sectorFB][0].SetMaximum(zmax)
            canvas.cd(1)
            self.hist_mappedChannelOccupancyPrompt[sectorFB][0].Draw("colz")  # z hits
            if sectorFB == 2:
                graphRPC0Chimney.Draw("L")
                graphScint1aChimney.Draw("L")
                graphScint1bChimney.Draw("L")
            else:
                graphRPC0.Draw("L")
                graphScint1a.Draw("L")
                graphScint1b.Draw("L")
            textRPC0.Draw()
            textScint1.Draw()
            zmax = 1
            for lane in range(8, 21):
                for channel in range(0, 64):
                    z = self.hist_mappedChannelOccupancyPrompt[sectorFB][1].GetBinContent(lane + 1, channel + 1)
                    zmax = z if z > zmax else zmax
            self.hist_mappedChannelOccupancyPrompt[sectorFB][1].SetMaximum(zmax)
            canvas.cd(2)
            self.hist_mappedChannelOccupancyPrompt[sectorFB][1].Draw("colz")  # phi hits
            graphRPC1.Draw("L")
            graphScint0.Draw("L")
            textRPC1.Draw()
            textScint0.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedChannelOccupancyPrompt[sectorFB][0].GetName()))
        if self.verbosity > 0:
            for sectorFB in range(0, 16):
                zmax = 1
                for lane in range(8, 21):
                    for channel in range(0, 64):
                        z = self.hist_mappedChannelOccupancyBkgd[sectorFB][0].GetBinContent(lane + 1, channel + 1)
                        zmax = z if z > zmax else zmax
                self.hist_mappedChannelOccupancyBkgd[sectorFB][0].SetMaximum(zmax)
                canvas.cd(1)
                self.hist_mappedChannelOccupancyBkgd[sectorFB][0].Draw("colz")  # z hits
                if sectorFB == 2:
                    graphRPC0Chimney.Draw("L")
                    graphScint1aChimney.Draw("L")
                    graphScint1bChimney.Draw("L")
                else:
                    graphRPC0.Draw("L")
                    graphScint1a.Draw("L")
                    graphScint1b.Draw("L")
                textRPC0.Draw()
                textScint1.Draw()
                zmax = 1
                for lane in range(8, 21):
                    for channel in range(0, 64):
                        z = self.hist_mappedChannelOccupancyBkgd[sectorFB][1].GetBinContent(lane + 1, channel + 1)
                        zmax = z if z > zmax else zmax
                self.hist_mappedChannelOccupancyBkgd[sectorFB][1].SetMaximum(zmax)
                canvas.cd(2)
                self.hist_mappedChannelOccupancyBkgd[sectorFB][1].Draw("colz")  # phi hits
                graphRPC1.Draw("L")
                graphScint0.Draw("L")
                textRPC1.Draw()
                textScint0.Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedChannelOccupancyBkgd[sectorFB][0].GetName()))
            for sectorFB in range(0, 16):
                canvas.cd(1)
                self.hist_unmappedChannelOccupancy[sectorFB][0].Draw("colz")  # z hits
                if sectorFB == 2:
                    graphRPC0Chimney.Draw("L")
                    graphScint1aChimney.Draw("L")
                    graphScint1bChimney.Draw("L")
                else:
                    graphRPC0.Draw("L")
                    graphScint1a.Draw("L")
                    graphScint1b.Draw("L")
                textRPC0.Draw()
                textScint1.Draw()
                canvas.cd(2)
                self.hist_unmappedChannelOccupancy[sectorFB][1].Draw("colz")  # phi hits
                graphRPC1.Draw("L")
                graphScint0.Draw("L")
                textRPC1.Draw()
                textScint0.Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedChannelOccupancy[sectorFB][0].GetName()))
        canvas.Clear()
        canvas.Divide(1, 1)
        # self.hist_RPCTimeLowBitsBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCTimeLowBitsBySector.GetName()))
        timeFit = ROOT.TF1("timeFit", "gausn(0)+pol0(3)", 100.0, 500.0)
        timeFit.SetParName(0, "Area")
        timeFit.SetParName(1, "Mean")
        timeFit.SetParName(2, "Sigma")
        timeFit.SetParName(3, "Bkgd")
        timeFit.SetParLimits(2, 10.0, 80.0)
        n = self.hist_mappedRPCTime.GetEntries()
        timeFit.SetParameter(0, n)
        timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        timeFit.SetParameter(1, 320.0)
        timeFit.SetParameter(2, 20.0)
        timeFit.SetParameter(3, 100.0)
        self.hist_mappedRPCTime.Fit("timeFit", "QR")
        self.hist_mappedRPCTime.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCTime.GetName()))
        self.hist_mappedRPCTimeBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCTimeBySector.GetName()))
        n = self.hist_mappedRPCTimeCal.GetEntries()
        timeFit.SetParameter(0, n)
        timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        timeFit.SetParameter(1, 320.0)
        timeFit.SetParameter(2, 20.0)
        timeFit.SetParameter(3, 100.0)
        self.hist_mappedRPCTimeCal.Fit("timeFit", "QR")
        self.hist_mappedRPCTimeCal.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCTimeCal.GetName()))
        self.hist_mappedRPCTimeCalBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCTimeCalBySector.GetName()))
        if self.verbosity > 0:
            self.hist_unmappedRPCTime.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedRPCTime.GetName()))
            self.hist_unmappedRPCTimeBySector.Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedRPCTimeBySector.GetName()))
            for sectorFB in range(0, 16):
                for layer in range(2, 15):
                    n = self.hist_mappedRPCPhiTimePerLayer[sectorFB][layer].GetEntries()
                    timeFit.SetParameter(0, n)
                    timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
                    timeFit.SetParameter(1, 320.0)
                    timeFit.SetParameter(2, 20.0)
                    timeFit.SetParameter(3, 100.0)
                    self.hist_mappedRPCPhiTimePerLayer[sectorFB][layer].Fit("timeFit", "QR")
                    self.hist_mappedRPCPhiTimePerLayer[sectorFB][layer].Draw()
                    canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCPhiTimePerLayer[sectorFB][layer].GetName()))
            for sectorFB in range(0, 16):
                for layer in range(2, 15):
                    n = self.hist_mappedRPCZTimePerLayer[sectorFB][layer].GetEntries()
                    timeFit.SetParameter(0, n)
                    timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
                    timeFit.SetParameter(1, 320.0)
                    timeFit.SetParameter(2, 20.0)
                    timeFit.SetParameter(3, 100.0)
                    self.hist_mappedRPCZTimePerLayer[sectorFB][layer].Fit("timeFit", "QR")
                    self.hist_mappedRPCZTimePerLayer[sectorFB][layer].Draw()
                    canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCZTimePerLayer[sectorFB][layer].GetName()))
        # self.hist_ScintTimeLowBitsBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ScintTimeLowBitsBySector.GetName()))
        # self.hist_mappedScintTDC.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintTDC.GetName()))
        # self.hist_mappedScintTDCBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintTDCBySector.GetName()))
        # self.hist_mappedScintTime.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintTime.GetName()))
        # self.hist_mappedScintTimeBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintTimeBySector.GetName()))
        # self.hist_unmappedScintTime.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedScintTime.GetName()))
        # self.hist_unmappedScintTimeBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedScintTimeBySector.GetName()))
        ctimeFit = ROOT.TF1("ctimeFit", "gausn(0)+pol0(3)", 300.0, 700.0)
        ctimeFit.SetParName(0, "Area")
        ctimeFit.SetParName(1, "Mean")
        ctimeFit.SetParName(2, "Sigma")
        ctimeFit.SetParName(3, "Bkgd")
        ctimeFit.SetParLimits(2, 10.0, 80.0)
        n = 32 * self.hist_mappedScintCtime.GetEntries()
        ctimeFit.SetParameter(0, n)
        ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        ctimeFit.SetParameter(1, 450.0)
        ctimeFit.SetParameter(2, 40.0)
        ctimeFit.SetParameter(3, 10.0)
        self.hist_mappedScintCtime.Fit("ctimeFit", "QR")
        self.hist_mappedScintCtime.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintCtime.GetName()))
        self.hist_mappedScintCtimeBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintCtimeBySector.GetName()))
        n = 32 * self.hist_mappedScintCtimeCal.GetEntries()
        ctimeFit.SetParameter(0, n)
        ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        ctimeFit.SetParameter(1, 450.0)
        ctimeFit.SetParameter(2, 40.0)
        ctimeFit.SetParameter(3, 10.0)
        self.hist_mappedScintCtimeCal.Fit("ctimeFit", "QR")
        self.hist_mappedScintCtimeCal.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintCtimeCal.GetName()))
        self.hist_mappedScintCtimeCalBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintCtimeCalBySector.GetName()))
        if self.verbosity > 0:
            self.hist_unmappedScintCtime.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedScintCtime.GetName()))
            self.hist_unmappedScintCtimeBySector.Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_unmappedScintCtimeBySector.GetName()))
            for sectorFB in range(0, 16):
                for layer in range(0, 2):
                    n = 32 * self.hist_mappedScintPhiCtimePerLayer[sectorFB][layer].GetEntries()
                    ctimeFit.SetParameter(0, n)
                    ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
                    ctimeFit.SetParameter(1, 440.0)
                    ctimeFit.SetParameter(2, 40.0)
                    ctimeFit.SetParameter(3, 10.0)
                    self.hist_mappedScintPhiCtimePerLayer[sectorFB][layer].Fit("ctimeFit", "QR")
                    self.hist_mappedScintPhiCtimePerLayer[sectorFB][layer].Draw()
                    canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintPhiCtimePerLayer[sectorFB][layer].GetName()))
            for sectorFB in range(0, 16):
                for layer in range(0, 2):
                    n = 32 * self.hist_mappedScintZCtimePerLayer[sectorFB][layer].GetEntries()
                    ctimeFit.SetParameter(0, n)
                    ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
                    ctimeFit.SetParameter(1, 440.0)
                    ctimeFit.SetParameter(2, 40.0)
                    ctimeFit.SetParameter(3, 10.0)
                    self.hist_mappedScintZCtimePerLayer[sectorFB][layer].Fit("ctimeFit", "QR")
                    self.hist_mappedScintZCtimePerLayer[sectorFB][layer].Draw()
                    canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintZCtimePerLayer[sectorFB][layer].GetName()))
            self.hist_mappedRPCCtimeRangeBySector.Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCCtimeRangeBySector.GetName()))
            canvas.SetLogy(0)
            self.hist_tdcRangeRPC.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tdcRangeRPC.GetName()))
            canvas.SetLogy(1)
            self.hist_tdcRangeRPC.Draw("")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tdcRangeRPC.GetName()))
            canvas.SetLogy(0)
            self.hist_ctimeRangeRPC.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctimeRangeRPC.GetName()))
            canvas.SetLogy(1)
            self.hist_ctimeRangeRPC.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctimeRangeRPC.GetName()))
            canvas.SetLogy(0)
            canvas.SetLogz(0)
            self.hist_tdcRangeVsCtimeRangeRPC.Draw("BOX")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tdcRangeVsCtimeRangeRPC.GetName()))
            canvas.SetLogz(1)
            self.hist_tdcRangeVsCtimeRangeRPC.Draw("COLZ")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tdcRangeVsCtimeRangeRPC.GetName()))
            canvas.SetLogz(0)
            self.hist_tdcRangeVsTimeRPC.Draw("BOX")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tdcRangeVsTimeRPC.GetName()))
            canvas.SetLogz(1)
            self.hist_tdcRangeVsTimeRPC.Draw("COLZ")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tdcRangeVsTimeRPC.GetName()))
            canvas.SetLogz(0)
            self.hist_ctimeRangeVsTimeRPC.Draw("BOX")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctimeRangeVsTimeRPC.GetName()))
            canvas.SetLogz(1)
            self.hist_ctimeRangeVsTimeRPC.Draw("COLZ")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctimeRangeVsTimeRPC.GetName()))
            canvas.SetLogz(0)
            self.hist_mappedScintCtimeRange.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintCtimeRange.GetName()))
            self.hist_mappedScintCtimeRangeBySector.Draw("box")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedScintCtimeRangeBySector.GetName()))

        self.hist_nHit1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit1d.GetName()))
        self.hist_n1dPhiZ.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_n1dPhiZ.GetName()))
        # self.hist_nHit1dRPCPrompt.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit1dRPCPrompt.GetName()))
        # self.hist_nHit1dRPCBkgd.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit1dRPCBkgd.GetName()))
        # self.hist_nHit1dScint.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit1dScint.GetName()))
        # self.hist_nHit1dPrompt.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit1dPrompt.GetName()))
        # self.hist_nHit1dBkgd.Draw()
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit1dBkgd.GetName()))
        # self.hist_multiplicityPhiBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_multiplicityPhiBySector.GetName()))
        # self.hist_multiplicityZBySector.Draw("box")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_multiplicityZBySector.GetName()))
        n = self.hist_tphiRPCCal1d.GetEntries()
        timeFit.SetParameter(0, n)
        timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        timeFit.SetParameter(1, 330.0)
        timeFit.SetParameter(2, 15.0)
        timeFit.SetParameter(3, 100.0)
        self.hist_tphiRPCCal1d.Fit("timeFit", "QR")
        self.hist_tphiRPCCal1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tphiRPCCal1d.GetName()))
        n = self.hist_tzRPCCal1d.GetEntries()
        timeFit.SetParameter(0, n)
        timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        timeFit.SetParameter(1, 330.0)
        timeFit.SetParameter(2, 15.0)
        timeFit.SetParameter(3, 100.0)
        self.hist_tzRPCCal1d.Fit("timeFit", "QR")
        self.hist_tzRPCCal1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tzRPCCal1d.GetName()))
        if self.verbosity > 0:
            n = self.hist_tRPCCal1d.GetEntries()
            timeFit.SetParameter(0, n)
            timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
            timeFit.SetParameter(1, 330.0)
            timeFit.SetParameter(2, 15.0)
            timeFit.SetParameter(3, 100.0)
            self.hist_tRPCCal1d.Fit("timeFit", "QR")
            self.hist_tRPCCal1d.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tRPCCal1d.GetName()))
        self.hist_dtRPC1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_dtRPC1d.GetName()))
        n = 32 * self.hist_ctphiScintCal1d.GetEntries()
        ctimeFit.SetParameter(0, n)
        ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        ctimeFit.SetParameter(1, 520.0)
        ctimeFit.SetParameter(2, 40.0)
        ctimeFit.SetParameter(3, 10.0)
        self.hist_ctphiScintCal1d.Fit("ctimeFit", "QR")
        self.hist_ctphiScintCal1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctphiScintCal1d.GetName()))
        n = 32 * self.hist_ctzScintCal1d.GetEntries()
        ctimeFit.SetParameter(0, n)
        ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        ctimeFit.SetParameter(1, 520.0)
        ctimeFit.SetParameter(2, 40.0)
        ctimeFit.SetParameter(3, 10.0)
        self.hist_ctzScintCal1d.Fit("ctimeFit", "QR")
        self.hist_ctzScintCal1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctzScintCal1d.GetName()))
        if self.verbosity > 0:
            n = 32 * self.hist_ctScintCal1d.GetEntries()
            ctimeFit.SetParameter(0, n)
            ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
            ctimeFit.SetParameter(1, 520.0)
            ctimeFit.SetParameter(2, 40.0)
            ctimeFit.SetParameter(3, 10.0)
            self.hist_ctScintCal1d.Fit("ctimeFit", "QR")
            self.hist_ctScintCal1d.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctScintCal1d.GetName()))
        self.hist_dtScint1d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_dtScint1d.GetName()))

        self.hist_nHit2d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_nHit2d.GetName()))
        self.hist_occupancyForwardXYPrompt.Draw("colz")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyForwardXYPrompt.GetName()))
        self.hist_occupancyBackwardXYPrompt.Draw("colz")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyBackwardXYPrompt.GetName()))
        if self.verbosity > 0:
            self.hist_occupancyForwardXYBkgd.Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyForwardXYBkgd.GetName()))
            self.hist_occupancyBackwardXYBkgd.Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyBackwardXYBkgd.GetName()))
            self.hist_occupancyRZPrompt.Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyRZPrompt.GetName()))
            self.hist_occupancyZPrompt.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyZPrompt.GetName()))
            self.hist_occupancyRPrompt.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyRPrompt.GetName()))
            self.hist_occupancyRZBkgd.Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyRZBkgd.GetName()))
            self.hist_occupancyZBkgd.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyZBkgd.GetName()))
            self.hist_occupancyRBkgd.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyRBkgd.GetName()))
            self.hist_tVsZFwd.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tVsZFwd.GetName()))
            self.hist_tVsZBwd.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tVsZFwd.GetName()))
        timeFit.SetParameter(0, self.hist_tRPCCal2d.GetEntries())
        n = self.hist_tRPCCal2d.GetEntries()
        timeFit.SetParameter(0, n)
        timeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        timeFit.SetParameter(1, 320.0)
        timeFit.SetParameter(2, 20.0)
        timeFit.SetParameter(3, 100.0)
        self.hist_tRPCCal2d.Fit("timeFit", "QR")
        self.hist_tRPCCal2d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tRPCCal2d.GetName()))
        self.hist_tRPCCal2dBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_tRPCCal2dBySector.GetName()))
        n = 32 * self.hist_ctScintCal2d.GetEntries()
        ctimeFit.SetParameter(0, n)
        ctimeFit.SetParLimits(0, 0.2 * n, 5.0 * n)
        ctimeFit.SetParameter(1, 500.0)
        ctimeFit.SetParameter(2, 40.0)
        ctimeFit.SetParameter(3, 10.0)
        self.hist_ctScintCal2d.Fit("ctimeFit", "QR")
        self.hist_ctScintCal2d.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctScintCal2d.GetName()))
        self.hist_ctScintCal2dBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ctScintCal2dBySector.GetName()))
        # the last page of the PDF file has to have "]" at the end of the PDF filename
        pdfNameLast = '{0}]'.format(self.pdfName)
        canvas.Print(pdfNameLast, "Title:{0}".format(self.hist_ctScintCal2dBySector.GetName()))
        self.histogramFile.Write()
        self.histogramFile.Close()
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
        """Process one event: fill histograms, (optionally) draw event display"""

        self.eventCounter += 1
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        event = EventMetaData.getEvent()
        rawklms = Belle2.PyStoreArray('RawKLMs')
        digits = Belle2.PyStoreArray('BKLMDigits')
        hit1ds = Belle2.PyStoreArray('BKLMHit1ds')
        hit2ds = Belle2.PyStoreArray('BKLMHit2ds')
        self.hist_nRawKLM.Fill(len(rawklms))
        self.hist_nDigit.Fill(len(digits))
        self.hist_nHit1d.Fill(len(hit1ds))
        self.hist_nHit2d.Fill(len(hit2ds))

        tCal1d = []
        for hit1d in hit1ds:
            tCal1d.append(hit1d.getTime())
        tCal2d = []
        for hit2d in hit2ds:
            tCal2d.append(hit2d.getTime())

        countAllMultihit = 0
        countAll = 0
        count = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        rawFb = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawSector = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawLayer = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawPlane = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawStrip = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        rawCtime = [[], [], [], [], [], [], [], [], [], [], [], [], [], [], [], []]
        for copper in range(0, len(rawklms)):
            rawklm = rawklms[copper]
            self.hist_rawKLMnumEvents.Fill(rawklm.GetNumEvents())
            self.hist_rawKLMnumNodes.Fill(rawklm.GetNumNodes())
            if rawklm.GetNumEntries() != 1:
                print('##0 Event', event, 'copper', copper, ' getNumEntries=', rawklm.GetNumEntries())
                continue
            nodeID = rawklm.GetNodeID(0) - self.BKLM_ID
            if nodeID >= self.EKLM_ID - self.BKLM_ID:
                nodeID = nodeID - (self.EKLM_ID - self.BKLM_ID) + 4
            self.hist_rawKLMnodeID.Fill(nodeID, copper)
            if (nodeID < 0) or (nodeID > 4):  # skip EKLM nodes
                continue
            trigCtime = (rawklm.GetTTCtime(0) & 0x7ffffff) << 3  # (ns)
            revo9time = trigCtime - 0x3b0
            for finesse in range(0, 4):
                dc = (finesse << 2) + copper
                nWords = rawklm.GetDetectorNwords(0, finesse)
                self.hist_rawKLMsizeByDCMultihit[dc].Fill(nWords)
                if nWords <= 0:
                    continue
                countAllMultihit = countAllMultihit + nWords
                bufSlot = rawklm.GetDetectorBuffer(0, finesse)
                lastWord = bufSlot[nWords - 1]
                if lastWord & 0xffff != 0:
                    print("##1 Event", event, 'copper', copper, 'finesse', finesse, 'n=', nWords, 'lastWord=', hex(lastWord))
                if (nWords % 2) == 0:
                    print("##2 Event", event, 'copper', copper, 'finesse', finesse, 'n=', nWords, 'should be odd -- skipping')
                    continue
                if int(self.exp) != 3:  # revo9time was not stored in the last word of the data-packet list?
                    revo9time = ((lastWord >> 16) << 3) & 0xffff
                dt = (trigCtime - revo9time) & 0x3ff
                if dt >= 0x200:
                    dt -= 0x400
                self.hist_trigCtimeVsTrigRevo9time.Fill(dt)
                countAll += 1
                count[dc] += 1
                sectorFB = self.dcToSectorFB[dc]
                n = nWords >> 1  # number of Data-Concentrator data packets
                channelMultiplicity = {}
                minRPCCtime = 99999
                maxRPCCtime = 0
                minRPCtdc = 99999
                maxRPCtdc = 0
                minScintCtime = 99999
                maxScintCtime = 0
                # first pass over this DC: determine per-channel multiplicities, event time ranges, and
                # fill dictionaries for accessing RawKLM hit information from BLKMHit1ds and BKLMHit2ds
                for j in range(0, n):
                    word0 = bufSlot[j * 2]
                    word1 = bufSlot[j * 2 + 1]
                    ctime = word0 & 0xffff
                    channel = (word0 >> 16) & 0x7f
                    axis = (word0 >> 23) & 0x01
                    lane = (word0 >> 24) & 0x1f  # 1..2 for scints, 8..20 for RPCs (=readout-board slot - 7)
                    flag = (word0 >> 30) & 0x03  # identifies scintillator or RPC hit
                    # adc = word1 & 0x0fff
                    tdc = (word1 >> 16) & 0x07ff
                    isRPC = (flag == 1)
                    isScint = (flag == 2)
                    laneAxisChannel = (word0 >> 16) & 0x1fff
                    if laneAxisChannel not in channelMultiplicity:
                        countAll = countAll + 2
                        count[dc] = count[dc] + 2
                        channelMultiplicity[laneAxisChannel] = 0
                    channelMultiplicity[laneAxisChannel] += 1
                    if isRPC:
                        if ctime < minRPCCtime:
                            minRPCCtime = ctime
                        if ctime > maxRPCCtime:
                            maxRPCCtime = ctime
                        if tdc < minRPCtdc:
                            minRPCtdc = tdc
                        if tdc > maxRPCtdc:
                            maxRPCtdc = tdc
                    elif isScint:
                        if int(self.exp) <= 3:  # fix the ctime for old SCROD firmware
                            trigCtx = trigCtime >> 3
                            ctime = trigCtx - ((trigCtx - ctime) << 2)
                        if ctime < minScintCtime:
                            minScintCtime = ctime
                        if ctime > maxScintCtime:
                            maxScintCtime = ctime
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
                    else:
                        rawFb[dc].append(-1)
                        rawSector[dc].append(-1)
                        rawLayer[dc].append(-1)
                        rawPlane[dc].append(-1)
                        rawStrip[dc].append(-1)
                        rawCtime[dc].append(-1)
                tdcRangeRPC = maxRPCtdc - minRPCtdc  # (ns)
                ctimeRangeRPC = (maxRPCCtime - minRPCCtime) << 3  # (ns)
                ctimeRangeScint = (maxScintCtime - minScintCtime) << 3  # (ns)
                if n > 1:
                    if maxRPCCtime > 0:
                        self.hist_mappedRPCCtimeRangeBySector.Fill(sectorFB, ctimeRangeRPC)
                    if maxScintCtime > 0:
                        self.hist_mappedScintCtimeRange.Fill(ctimeRangeScint)
                        self.hist_mappedScintCtimeRangeBySector.Fill(sectorFB, ctimeRangeScint)
                # second pass over this DC's hits: histogram everything
                for j in range(0, n):
                    word0 = bufSlot[j * 2]
                    word1 = bufSlot[j * 2 + 1]
                    ctime = word0 & 0xffff
                    channel = (word0 >> 16) & 0x7f
                    axis = (word0 >> 23) & 0x01
                    lane = (word0 >> 24) & 0x1f  # 1..2 for scints, 8..20 for RPCs (=readout-board slot - 7)
                    flag = (word0 >> 30) & 0x03  # 1 for RPCs, 2 for scints
                    electId = (channel << 12) | (axis << 11) | (lane << 6) | (finesse << 4) | nodeID
                    # adc = word1 & 0x0fff
                    tdc = (word1 >> 16) & 0x07ff
                    tdcExtra = (word1 >> 27) & 0x1f
                    adcExtra = (word1 >> 12) & 0x0f
                    isRPC = (flag == 1)
                    isScint = (flag == 2)
                    laneAxis = axis if ((lane < 1) or (lane > 20)) else ((lane << 1) + axis)
                    laneAxisChannel = (word0 >> 16) & 0x1fff
                    multiplicity = channelMultiplicity[laneAxisChannel]
                    if multiplicity > 1:  # histogram only if 2+ entries in the same channel
                        self.hist_rawKLMchannelMultiplicity[dc].Fill(multiplicity, laneAxis)
                        self.hist_rawKLMchannelMultiplicityFine[dc].Fill(multiplicity, laneAxisChannel)
                    if (self.singleEntry == 1 and multiplicity > 1) or (self.singleEntry == 2 and multiplicity == 1):
                        continue
                    self.hist_rawKLMlaneFlag.Fill(flag, lane)
                    if isRPC:
                        self.hist_rawKLMtdcExtraRPC.Fill(sectorFB, tdcExtra)
                        self.hist_rawKLMadcExtraRPC.Fill(sectorFB, adcExtra)
                    elif isScint:
                        self.hist_rawKLMtdcExtraScint.Fill(sectorFB, tdcExtra)
                        self.hist_rawKLMadcExtraScint.Fill(sectorFB, adcExtra)
                        if int(self.exp) <= 3:  # fix the ctime for old SCROD firmware
                            trigCtx = trigCtime >> 3
                            ctime = trigCtx - ((trigCtx - ctime) << 2)
                    t = (tdc - trigCtime) & 0x03ff  # in ns, range is 0..1023
                    ct = (ctime << 3) - (trigCtime & 0x7fff8)  # in ns, range is only 8 bits in SCROD (??)
                    ct = ct & 0x3ff
                    if electId in self.electIdToModuleId:  # mapped-channel histograms
                        self.hist_mappedSectorOccupancyMultihit.Fill(sectorFB)
                        if channelMultiplicity[laneAxisChannel] == 1:
                            self.hist_mappedSectorOccupancy.Fill(sectorFB)
                        if isRPC:
                            self.hist_RPCTimeLowBitsBySector.Fill(sectorFB, (tdc & 3))
                            tCal = int(tdc - trigCtime - self.t0RPC[axis][sectorFB][lane-6]) & 0x03ff  # in ns, range 0..1023
                            if j == 0:
                                self.hist_tdcRangeRPC.Fill(tdcRangeRPC)
                                self.hist_ctimeRangeRPC.Fill(ctimeRangeRPC)
                                self.hist_tdcRangeVsCtimeRangeRPC.Fill(tdcRangeRPC, ctimeRangeRPC)
                            self.hist_tdcRangeVsTimeRPC.Fill(tCal, tdcRangeRPC)
                            self.hist_ctimeRangeVsTimeRPC.Fill(tCal, ctimeRangeRPC)
                            if abs(tCal - self.t0Cal) < 50:
                                self.hist_mappedChannelOccupancyPrompt[sectorFB][axis].Fill(lane, channel)
                            else:
                                self.hist_mappedChannelOccupancyBkgd[sectorFB][axis].Fill(lane, channel)
                            self.hist_mappedRPCSectorOccupancy.Fill(sectorFB)
                            self.hist_mappedRPCLaneAxisOccupancy.Fill(sectorFB, laneAxis)
                            self.hist_mappedRPCTime.Fill(t)
                            self.hist_mappedRPCTimeCal.Fill(tCal)
                            self.hist_mappedRPCTimeBySector.Fill(sectorFB, t)
                            self.hist_mappedRPCTimeCalBySector.Fill(sectorFB, tCal)
                            if axis == 0:
                                self.hist_mappedRPCZTimePerLayer[sectorFB][lane - 6].Fill(t)
                            else:
                                self.hist_mappedRPCPhiTimePerLayer[sectorFB][lane - 6].Fill(t)
                        elif isScint:
                            self.hist_ScintTimeLowBitsBySector.Fill(sectorFB, (tdc & 3))
                            ctCal = int((ctime << 3) - trigCtime - self.ct0Scint[1-axis][sectorFB][lane-1]) & 0x03ff  # in ns
                            if abs(ctCal - self.ct0Cal) < 50:
                                self.hist_mappedChannelOccupancyPrompt[sectorFB][1 - axis].Fill(lane, channel)
                            else:
                                self.hist_mappedChannelOccupancyBkgd[sectorFB][1 - axis].Fill(lane, channel)
                            self.hist_mappedScintSectorOccupancy.Fill(sectorFB)
                            self.hist_mappedScintLaneAxisOccupancy.Fill(sectorFB, laneAxis)
                            self.hist_mappedScintTime.Fill(t & 0x1f)
                            self.hist_mappedScintTimeBySector.Fill(sectorFB, t & 0x1f)
                            self.hist_mappedScintTDC.Fill(tdc)
                            self.hist_mappedScintTDCBySector.Fill(sectorFB, tdc)
                            self.hist_mappedScintCtime.Fill(ct)
                            self.hist_mappedScintCtimeBySector.Fill(sectorFB, ct)
                            self.hist_mappedScintCtimeCal.Fill(ctCal)
                            self.hist_mappedScintCtimeCalBySector.Fill(sectorFB, ctCal)
                            if axis == 1:
                                self.hist_mappedScintZCtimePerLayer[sectorFB][lane - 1].Fill(ct)
                            else:
                                self.hist_mappedScintPhiCtimePerLayer[sectorFB][lane - 1].Fill(ct)
                    else:  # unmapped-channel histograms
                        self.hist_unmappedSectorOccupancyMultihit.Fill(sectorFB)
                        if channelMultiplicity[laneAxisChannel] == 1:
                            self.hist_unmappedSectorOccupancy.Fill(sectorFB)
                        if isRPC:
                            self.hist_unmappedChannelOccupancy[sectorFB][axis].Fill(lane, channel)
                            self.hist_RPCTimeLowBitsBySector.Fill(sectorFB, (tdc & 3))
                            self.hist_unmappedRPCSectorOccupancy.Fill(sectorFB)
                            self.hist_unmappedRPCLaneAxisOccupancy.Fill(sectorFB, laneAxis)
                            self.hist_unmappedRPCTime.Fill(t)
                            self.hist_unmappedRPCTimeBySector.Fill(sectorFB, t)
                        elif isScint:
                            self.hist_unmappedChannelOccupancy[sectorFB][1 - axis].Fill(lane, channel)
                            self.hist_ScintTimeLowBitsBySector.Fill(sectorFB, (tdc & 3))
                            self.hist_unmappedScintSectorOccupancy.Fill(sectorFB)
                            self.hist_unmappedScintLaneAxisOccupancy.Fill(sectorFB, laneAxis)
                            self.hist_unmappedScintTime.Fill(t & 0x1f)
                            self.hist_unmappedScintTimeBySector.Fill(sectorFB, t & 0x1f)
                            self.hist_unmappedScintCtime.Fill(ct)
                            self.hist_unmappedScintCtimeBySector.Fill(sectorFB, ct)
                self.hist_rawKLMsizeByDC[dc].Fill(count[dc])
        self.hist_rawKLMsizeMultihit.Fill(countAllMultihit)
        self.hist_rawKLMsize.Fill(countAll)

        # Process the BKLMHit1ds

        cosine = [0, 0, 0, 0, 0, 0, 0, 0]
        sine = [0, 0, 0, 0, 0, 0, 0, 0]
        for sector in range(0, 8):
            phi = math.pi * sector / 4
            cosine[sector] = math.cos(phi)
            sine[sector] = math.sin(phi)
        zyList = [[], [], [], [], [], [], [], []]
        xyList = [[], [], [], [], [], [], [], []]
        r0 = 201.9 + 0.5 * 4.4  # cm
        dr = 9.1  # cm
        z0 = 47.0  # cm
        dzScint = 4.0  # cm
        dzRPC = 4.52  # cm
        nPhiStrips = [37, 42, 36, 36, 36, 36, 48, 48, 48, 48, 48, 48, 48, 48, 48]
        dPhiStrips = [4.0, 4.0, 4.9, 5.11, 5.32, 5.53, 4.3, 4.46, 4.62, 4.77, 4.93, 5.09, 5.25, 5.4, 5.56]
        scintFlip = [[[-1, 1], [-1, 1], [1, 1], [1, -1], [1, -1], [1, -1], [-1, -1], [-1, 1]],
                     [[1, -1], [1, -1], [1, 1], [-1, 1], [-1, 1], [-1, 1], [-1, -1], [1, -1]]]
        promptColor = 3
        bkgdColor = 2
        phiTimes = {}
        zTimes = [{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}]
        nphihits = 0
        nzhits = 0
        nRPCPrompt = 0
        nRPCBkgd = 0
        nScint = 0
        for hit1d in hit1ds:
            key = hit1d.getModuleID()
            fb = (key & self.BKLM_SECTION_MASK) >> self.BKLM_SECTION_BIT
            sector = (key & self.BKLM_SECTOR_MASK) >> self.BKLM_SECTOR_BIT
            layer = (key & self.BKLM_LAYER_MASK) >> self.BKLM_LAYER_BIT
            plane = (key & self.BKLM_PLANE_MASK) >> self.BKLM_PLANE_BIT
            stripMin = (key & self.BKLM_STRIP_MASK) >> self.BKLM_STRIP_BIT
            stripMax = (key & self.BKLM_MAXSTRIP_MASK) >> self.BKLM_MAXSTRIP_BIT
            sectorFB = sector if fb == 0 else sector + 8
            if self.legacyTimes:
                dc = self.sectorFBToDC[sectorFB]
                copper = dc & 0x03
                finesse = dc >> 2
                n = rawklms[copper].GetDetectorNwords(0, finesse) >> 1
                trigCtime = (rawklms[copper].GetTTCtime(0) & 0x07ffffff) << 3
                tCal = -1
                ctDiffMax = 99999
                for j in range(0, n):
                    if layer != rawLayer[dc][j]:
                        continue
                    if sector != rawSector[dc][j]:
                        continue
                    if fb != rawFb[dc][j]:
                        continue
                    if plane != rawPlane[dc][j]:
                        continue
                    strip = rawStrip[dc][j]
                    if strip < stripMin:
                        continue
                    if strip > stripMax:
                        continue
                    if layer < 2:  # it's a scint layer
                        ctime = rawCtime[dc][j] << 3
                        ct = ctime - trigCtime - self.ct0Scint[plane][sectorFB][layer]
                        ctTrunc = int(ct) & 0x3ff
                        if abs(ctTrunc - self.ct0Cal) < ctDiffMax:
                            ctDiffMax = int(abs(ctTrunc - self.ct0Cal))
                            tCal = ct
                            if ctDiffMax == 0:
                                break
                    else:  # it's an RPC layer
                        tCal = tCal = hit1d.getTime() - trigCtime - self.t0RPC[plane][sectorFB][layer]
                        break
            else:
                if layer < 2:
                    tCal = hit1d.getTime() - self.ct0Scint[plane][sectorFB][layer]
                else:
                    tCal = hit1d.getTime() - self.t0RPC[plane][sectorFB][layer]
            tCalTrunc = int(tCal) & 0x3ff

            if self.view == 1:
                r = r0 + layer * dr
                yA = r
                zA = 500
                xB = 500
                yB = 500
                stripAverage = (stripMin + stripMax) * 0.5
                isPrompt = False
                if layer < 2:
                    nScint += 1
                    isPrompt = (abs(tCalTrunc - self.ct0Cal1d) < 50)
                    if plane == 0:
                        if fb == 0:
                            zA = z0 - stripAverage * dzScint
                        else:
                            zA = z0 + stripAverage * dzScint
                    else:
                        h = ((stripAverage - 0.5 * nPhiStrips[layer]) * dPhiStrips[layer]) * scintFlip[fb][sector][layer]
                        xB = r * cosine[sector] - h * sine[sector]
                        yB = r * sine[sector] + h * cosine[sector]
                else:
                    isPrompt = (abs(tCalTrunc - self.t0Cal1d) < 50)
                    if plane == 0:
                        if fb == 0:
                            zA = z0 - stripAverage * dzRPC
                        else:
                            zA = z0 + stripAverage * dzRPC
                    else:
                        h = ((stripAverage - 0.5 * nPhiStrips[layer]) * dPhiStrips[layer])  # * rpcFlip[fb][sector]
                        xB = r * cosine[sector] - h * sine[sector]
                        yB = r * sine[sector] + h * cosine[sector]
                    if abs(tCalTrunc - self.t0Cal) < 50:
                        nRPCPrompt += 1
                        if plane == 1:
                            self.hist_multiplicityPhiBySector.Fill(sectorFB, stripMax - stripMin + 1)
                        else:
                            self.hist_multiplicityZBySector.Fill(sectorFB, stripMax - stripMin + 1)
                    else:
                        nRPCBkgd += 1
            if plane == 1:
                nphihits += 1
                phiTimes[key] = tCal
                if layer < 2:
                    self.hist_ctphiScintCal1d.Fill(tCalTrunc)
                else:
                    self.hist_tphiRPCCal1d.Fill(tCalTrunc)
            else:
                nzhits += 1
                zTimes[layer][key] = tCal
                if layer < 2:
                    self.hist_ctzScintCal1d.Fill(tCalTrunc)
                else:
                    self.hist_tzRPCCal1d.Fill(tCalTrunc)
            # Add the hit to the event-display TGraph list (perhaps)
            if (self.view == 1) and (self.eventDisplays < self.maxDisplays):
                if zA != 500:
                    gZY = ROOT.TGraph()
                    gZY.SetPoint(0, zA - 1.0, yA - 1.0)
                    gZY.SetPoint(1, zA - 1.0, yA + 1.0)
                    gZY.SetPoint(2, zA + 1.0, yA + 1.0)
                    gZY.SetPoint(3, zA + 1.0, yA - 1.0)
                    gZY.SetPoint(4, zA - 1.0, yA - 1.0)
                    gZY.SetLineWidth(1)
                    if isPrompt:
                        gZY.SetLineColor(promptColor)
                    else:
                        gZY.SetLineColor(bkgdColor)
                    zyList[sector].append(gZY)
                if xB != 500:
                    gXY = ROOT.TGraph()
                    gXY.SetPoint(0, xB - 1.0, yB - 1.0)
                    gXY.SetPoint(1, xB - 1.0, yB + 1.0)
                    gXY.SetPoint(2, xB + 1.0, yB + 1.0)
                    gXY.SetPoint(3, xB + 1.0, yB - 1.0)
                    gXY.SetPoint(4, xB - 1.0, yB - 1.0)
                    gXY.SetLineWidth(1)
                    if isPrompt:
                        gXY.SetLineColor(promptColor)
                    else:
                        gXY.SetLineColor(bkgdColor)
                    xyList[sector].append(gXY)
        self.hist_nHit1dRPCPrompt.Fill(nRPCPrompt)
        self.hist_nHit1dRPCBkgd.Fill(nRPCBkgd)
        self.hist_nHit1dScint.Fill(nScint)
        if nRPCPrompt > 2:
            self.hist_nHit1dPrompt.Fill(nScint + nRPCBkgd + nRPCPrompt)
        else:
            self.hist_nHit1dBkgd.Fill(nScint + nRPCBkgd + nRPCPrompt)
        self.hist_n1dPhiZ.Fill(nphihits, nzhits)
        for phiKey in phiTimes:
            mphi = phiKey & self.BKLM_MODULEID_MASK
            layer = (mphi & self.BKLM_LAYER_MASK) >> self.BKLM_LAYER_BIT
            sector = (mphi & self.BKLM_SECTOR_MASK) >> self.BKLM_SECTOR_BIT
            fb = (mphi & self.BKLM_SECTION_MASK) >> self.BKLM_SECTION_BIT
            sectorFB = sector if fb == 0 else sector + 8
            tphi = phiTimes[phiKey]
            tphiTrunc = int(tphi) & 0x3ff
            for zKey in zTimes[layer]:
                mz = zKey & self.BKLM_MODULEID_MASK
                if mphi == mz:
                    tz = zTimes[layer][zKey]
                    tzTrunc = int(tz) & 0x3ff
                    dt = (tphiTrunc - tzTrunc) & 0x3ff
                    if dt >= 0x200:
                        dt -= 0x400
                    t = (tphi + tz) * 0.5
                    tTrunc = int(t) & 0x3ff
                    if layer < 2:
                        self.hist_dtScint1d.Fill(dt)
                    else:
                        self.hist_dtRPC1d.Fill(dt)
                    if abs(dt) < 4000:
                        if layer < 2:
                            self.hist_ctScintCal1d.Fill(tTrunc)
                        else:
                            self.hist_tRPCCal1d.Fill(tTrunc)

        # After processing all of the BKLMHit1ds in the event, draw the event display (perhaps)

        if (self.view == 1) and (self.eventDisplays < self.maxDisplays):
            drawnSectors = 0
            jCanvas = 1
            for sector in range(0, 8):
                if len(zyList[sector]) > self.minRPCHits:
                    drawnSectors += 1
                    self.eventCanvas.cd(jCanvas)
                    title = 'e{0:02d}r{1}: event {2} z-readout hits in S{3}'.format(int(self.exp), int(self.run), event, sector)
                    self.hist_ZY1D[jCanvas - 1].SetTitle(title)
                    self.hist_ZY1D[jCanvas - 1].Draw()
                    for g in self.bklmZY:
                        g.Draw("L")
                    for g in zyList[sector]:
                        g.Draw("L")
                    jCanvas += 1
                    if jCanvas > 2:
                        jCanvas = 1
                        self.lastTitle = "Title:E{0} (#{1})".format(event, self.eventCounter)
                        self.eventCanvas.Print(self.eventPdfName, self.lastTitle)
            enoughXYHits = False
            for sector in range(0, 8):
                if len(xyList[sector]) > self.minRPCHits:
                    enoughXYHits = True
                    break
            if enoughXYHits:
                drawnSectors += 1
                self.eventCanvas.cd(jCanvas)
                jCanvas += 1
                title = 'e{0:02d}r{1}: event {2} phi-readout hits'.format(int(self.exp), int(self.run), event)
                self.hist_XY.SetTitle(title)
                self.hist_XY.Draw()
                for g in self.bklmXY:
                    g.Draw("L")
                for sector in range(0, 8):
                    for g in xyList[sector]:
                        g.Draw("L")
                if jCanvas > 2:
                    jCanvas = 1
                    self.lastTitle = "Title:E{0} (#{1})".format(event, self.eventCounter)
                    self.eventCanvas.Print(self.eventPdfName, self.lastTitle)
            if jCanvas == 2:
                self.eventCanvas.cd(jCanvas)
                ROOT.gPad.Clear()
                self.lastTitle = "Title:E{0} (#{1})".format(event, self.eventCounter)
                self.eventCanvas.Print(self.eventPdfName, self.lastTitle)
            if drawnSectors > 0:
                self.eventDisplays += 1

        # Process the BKLMHit2ds

        xyList = []
        zyList = []
        rpcHits = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
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
                rpcHits[sectorFB] += 1
            if self.legacyTimes:
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
                            ct = int((ctZ + ctPhi) * 0.5 - trigCtime - self.ct0Scint[plane][sectorFB][layer]) & 0x3ff
                            if abs(ct - self.ct0Cal) < ctDiffMax:
                                ctDiffMax = int(abs(ct - self.ct0Cal))
                                tCal = ct
                                if ctDiffMax == 0:
                                    break
                        else:  # it's an RPC layer
                            tCal = hit2d.getTime() - trigCtime - self.t0RPC[plane][sectorFB][layer]
                            break
            else:
                if layer < 2:
                    tCal = hit2d.getTime() - self.ct0Scint[plane][sectorFB][layer]
                else:
                    tCal = hit2d.getTime() - self.t0RPC[plane][sectorFB][layer]
            tCalTrunc = int(tCal) & 0x3ff
            x = hit2d.getGlobalPositionX()
            y = hit2d.getGlobalPositionY()
            z = hit2d.getGlobalPositionZ()
            r = math.sqrt(x * x + y * y)
            isPromptHit = False
            promptColor = 3
            bkgdColor = 2
            if layer < 2:
                promptColor = 7
                bkgdColor = 4
                self.hist_ctScintCal2d.Fill(tCalTrunc)
                self.hist_ctScintCal2dBySector.Fill(sectorFB, tCalTrunc)
                if abs(tCalTrunc - self.ct0Cal2d) < 50:
                    isPromptHit = True
                    if fb == 0:  # backward
                        self.hist_occupancyBackwardXYPrompt.Fill(x, y)
                    else:  # forward
                        self.hist_occupancyForwardXYPrompt.Fill(x, y)
                else:
                    if fb == 0:  # backward
                        self.hist_occupancyBackwardXYBkgd.Fill(x, y)
                    else:  # forward
                        self.hist_occupancyForwardXYBkgd.Fill(x, y)
            else:
                self.hist_tRPCCal2d.Fill(tCalTrunc)
                self.hist_tRPCCal2dBySector.Fill(sectorFB, tCalTrunc)
                if abs(tCalTrunc - self.t0Cal2d) < 50:
                    isPromptHit = True
                    self.hist_occupancyRZPrompt.Fill(z, layer)
                    self.hist_occupancyZPrompt.Fill(z)
                    self.hist_occupancyRPrompt.Fill(layer)
                    if fb == 0:  # backward
                        self.hist_occupancyBackwardXYPrompt.Fill(x, y)
                        self.hist_tVsZBwd.Fill(-(z - 47.0), tCalTrunc)
                    else:  # forward
                        self.hist_occupancyForwardXYPrompt.Fill(x, y)
                        self.hist_tVsZFwd.Fill(+(z - 47.0), tCalTrunc)
                elif abs(tCalTrunc - self.t0Cal2d) >= 50:
                    self.hist_occupancyRZBkgd.Fill(z, layer)
                    self.hist_occupancyZBkgd.Fill(z)
                    self.hist_occupancyRBkgd.Fill(layer)
                    if fb == 0:  # backward
                        self.hist_occupancyBackwardXYBkgd.Fill(x, y)
                    else:  # forward
                        self.hist_occupancyForwardXYBkgd.Fill(x, y)

            # Add the hit to the event-display TGraph list (perhaps)
            if (self.view == 2) and (self.eventDisplays < self.maxDisplays):
                gXY = ROOT.TGraph()
                gXY.SetPoint(0, x - 1.0, y - 1.0)
                gXY.SetPoint(1, x - 1.0, y + 1.0)
                gXY.SetPoint(2, x + 1.0, y + 1.0)
                gXY.SetPoint(3, x + 1.0, y - 1.0)
                gXY.SetPoint(4, x - 1.0, y - 1.0)
                gXY.SetLineWidth(1)
                gZY = ROOT.TGraph()
                gZY.SetPoint(0, z - 1.0, y - 1.0)
                gZY.SetPoint(1, z - 1.0, y + 1.0)
                gZY.SetPoint(2, z + 1.0, y + 1.0)
                gZY.SetPoint(3, z + 1.0, y - 1.0)
                gZY.SetPoint(4, z - 1.0, y - 1.0)
                gZY.SetLineWidth(1)
                if isPromptHit:
                    gXY.SetLineColor(promptColor)
                    gZY.SetLineColor(promptColor)
                else:
                    gXY.SetLineColor(bkgdColor)
                    gZY.SetLineColor(bkgdColor)
                xyList.append(gXY)
                zyList.append(gZY)

        # After processing all of the hits in the event, draw the event display (perhaps)

        if (self.view == 2) and (self.eventDisplays < self.maxDisplays):
            hasEnoughRPCHits = False
            for count in rpcHits:
                if count > self.minRPCHits:
                    hasEnoughRPCHits = True
                    break
            if hasEnoughRPCHits:
                self.eventDisplays += 1
                title = 'e{0:02d}r{1}: event {2} 2D hits'.format(int(self.exp), int(self.run), event)
                self.hist_XY.SetTitle(title)
                self.hist_ZY.SetTitle(title)
                self.eventCanvas.cd(1)
                self.hist_XY.Draw()
                for g in self.bklmXY:
                    g.Draw("L")
                for g in xyList:
                    g.Draw("L")
                self.eventCanvas.cd(2)
                self.hist_ZY.Draw()
                for g in self.bklmZY:
                    g.Draw("L")
                for g in zyList:
                    g.Draw("L")
                self.lastTitle = "Title:E{0} (#{1})".format(event, self.eventCounter)
                self.eventCanvas.Print(self.eventPdfName, self.lastTitle)
