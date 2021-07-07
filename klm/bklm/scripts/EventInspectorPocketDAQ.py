#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Purpose:
#   basf2 module to histogram useful values in PocketDAQ data created by the
#   test stand at Indiana University.
#
import ROOT


class EventInspectorPocketDAQ:
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

    def __init__(self, exp, run, histName, pdfName):
        """Constructor

        Arguments:
            exp (str): formatted experiment number
            run (str): formatter run number
            histName (str): path name of the output histogram ROOT file
            pdfName (str): path name of the output histogram PDF file
            eventPdfName (str): path name of the output event-display PDF file
            maxDisplays (int): max # of events displays to write
            minRPCHits (int): min # of RPC BKLMHit2ds in any sector for event display
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
        """Handle job initialization: create histograms"""

        expRun = 'e{0:02d}r{1}: '.format(int(self.exp), int(self.run))

        #: Output ROOT TFile that will contain the histograms/scatterplots
        self.histogramFile = ROOT.TFile.Open(self.histName, "RECREATE")

        # create the rawKLM histograms

        #: histogram of the tt_ctime relative to triggertime
        self.hist_ttc_trigtime = ROOT.TH1F('ttc_trigtime',
                                           expRun + 'tt_ctime relative to triggertime;' +
                                           'tt_ctime - triggertime (ns)',
                                           256, -0.5, 2047.5)
        #: histogram of the hit's lane
        self.hist_rawKLMlane = ROOT.TH1F('rawKLMlane',
                                         expRun + 'RawKLM lane;Lane (scint: 1..7, RPC: 8..20)',
                                         21, -0.5, 20.5)
        #: histogram of number of hits, including multiple entries on one readout channel
        self.hist_rawKLMsizeMultihit = ROOT.TH1F('rawKLMsizeMultihit',
                                                 expRun + 'RawKLM word count (N/channel)',
                                                 200, -0.5, 199.5)
        #: histogram of number of hits, at most one entry per readout channel
        self.hist_rawKLMsize = ROOT.TH1F('rawKLMsize',
                                         expRun + 'RawKLM word count (1/channel)',
                                         200, -0.5, 199.5)
        #: scatterplot of multiplicity of entries in one readout channel vs lane/axis
        self.hist_PerChannelMultiplicity = ROOT.TH2F(
            'PerChannelMultiplicity',
            expRun + 'Per-channel multiplicity (N/channel > 1);' +
                     'Per-channel multiplicity;' +
                     '(Lane #) * 2 + (Axis #)',
            30, -0.5, 29.5, 42, -0.5, 41.5)
        #: scatterplot of number of mapped RPC hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_RPCLaneAxisOccupancy = ROOT.TH2F(
            'RPCLaneAxisOccupancy',
            expRun + 'Lane/axis occupancy of RPC channels (1/channel);' +
                     'Sector # (always 0);' +
                     '(Lane #) * 2 + (Axis #)',
            3, -1.5, 1.5, 42, -0.5, 41.5)
        #: scatterplot of number of mapped scint hits by lane/axis vs sector, at most one entry per readout channel
        self.hist_ScintLaneAxisOccupancy = ROOT.TH2F(
            'ScintLaneAxisOccupancy',
            expRun + 'Lane/axis occupancy of scint channels (1/channel);' +
                     'Sector # (always 0);' +
                     '(Lane #) * 2 + (Axis #)',
            3, -1.5, 1.5, 42, -0.5, 41.5)
        #: scatterplots of channel occupancy (1 hit per readout channel) for each axis
        self.hist_ChannelOccupancy = [0, 0]
        self.hist_ChannelOccupancy[0] = ROOT.TH2F('ChannelOccupancy_a0',
                                                  expRun + 'Channel occupancy for axis 0;lane;channel',
                                                  42, -0.25, 20.75, 128, -0.25, 63.75)
        self.hist_ChannelOccupancy[1] = ROOT.TH2F('ChannelOccupancy_a1',
                                                  expRun + 'Channel occupancy for axis 1;lane;channel',
                                                  42, -0.25, 20.75, 128, -0.25, 63.75)
        #: histograms of channel occupancy (1 hit per readout channel), indexed by axis/lane
        self.hist_ChannelOccupancyAL = [
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0],
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0],
            [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]
        for lane in range(0, 21):
            nChannels = 64 if (lane > 2) else 128
            label = 'ChannelOccupancy_A0L{0}'.format(lane)
            title = '{0}Channel occupancy for axis 0 lane {1};channel'.format(expRun, lane)
            self.hist_ChannelOccupancyAL[lane][0] = ROOT.TH1F(label, title, nChannels, -0.5, nChannels - 0.5)
            label = 'ChannelOccupancy_A1L{0}'.format(lane)
            title = '{0}Channel occupancy for axis 1 lane {1};channel'.format(expRun, lane)
            self.hist_ChannelOccupancyAL[lane][1] = ROOT.TH1F(label, title, nChannels, -0.5, nChannels - 0.5)
        #: scatterplot of RPC TDC low-order bits vs sector (should be 0 since granularity is 4 ns)
        self.hist_RPCTimeLowBitsBySector = ROOT.TH2F('RPCTimeLowBitsBySector',
                                                     expRun + 'RPC TDC lowest-order bits;' +
                                                     'Sector # (always 0);' +
                                                     'TDC % 4 (ns) [should be 0]',
                                                     3, -1.5, 1.5, 8, -0.25, 3.75)
        #: histogram of RPC TDC value relative to event's REVO9 trigger time in last word of event
        self.hist_RPCTime = ROOT.TH1F('RPCTime',
                                      expRun + 'RPC tdc relative to event trigtime;tdc - triggerTime (ns)',
                                      256, -0.5, 1023.5)
        #: histogram of RPC TDC value relative to event's ctime in event header
        self.hist_RPCTime2 = ROOT.TH1F('RPCTime_Ctime',
                                       expRun + 'RPC tdc relative to event ctime;tdc - trigCtime (ns)',
                                       256, -0.5, 1023.5)
        #: histograms of RPC TDC value relative to event's trigger time for axis 0, indexed by lane
        self.hist_RPCTimePerLayerA0 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for lane in range(0, 21):
            label = 'RPCTimeA0L{0:02d}'.format(lane)
            title = '{0}RPC axis 0 lane {1} time relative to trigtime;t - triggerTime (ns)'.format(expRun, lane)
            self.hist_RPCTimePerLayerA0[lane] = ROOT.TH1F(label, title, 256, -0.5, 1023.5)
        #: histograms of RPC TDC value relative to event's trigger time for axis 1, indexed by lane
        self.hist_RPCTimePerLayerA1 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for lane in range(0, 21):
            label = 'RPCTimeA1L{0:02d}'.format(lane)
            title = '{0}RPC axis 1 lane {1} time relative to trigtime;t - triggerTime (ns)'.format(expRun, lane)
            self.hist_RPCTimePerLayerA1[lane] = ROOT.TH1F(label, title, 256, -0.5, 1023.5)
        #: histograms of RPC TDC value relative to event's ctime for axis 0, indexed by lane
        self.hist_RPCTime2PerLayerA0 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for lane in range(0, 21):
            label = 'RPCTime2A0L{0:02d}'.format(lane)
            title = '{0}RPC axis 0 lane {1} time relative to trigCtime;t - trigCtime (ns)'.format(expRun, lane)
            self.hist_RPCTime2PerLayerA0[lane] = ROOT.TH1F(label, title, 256, -0.5, 1023.5)
        #: histograms of RPC TDC value relative to event's ctime for axis 1, indexed by lane
        self.hist_RPCTime2PerLayerA1 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for lane in range(0, 21):
            label = 'RPCTime2A1L{0:02d}'.format(lane)
            title = '{0}RPC axis 1 lane {1} time relative to trigCtime;t - trigCtime (ns)'.format(expRun, lane)
            self.hist_RPCTime2PerLayerA1[lane] = ROOT.TH1F(label, title, 256, -0.5, 1023.5)
        #: histogram of RPC TDC range in event
        self.hist_RPCTdcRange = ROOT.TH1F('RPCTdcRange', expRun + 'RPC TDC-range in event;TDCMax - TDCMin (ns)', 512, -0.5, 2047.5)
        #: histogram of RPC REVO9 range in event
        self.hist_RPCRevotimeRange = ROOT.TH1F('RPCRevotimeRange',
                                               expRun + 'RPC revotime-range in event;revotimeMax - revotimeMin (ns)',
                                               128, -0.5, 8191.5)
        #: scatterplot of RPC REVO9 range vs TDC value in event
        self.hist_revotimeRPCtdc = ROOT.TH2F('revotimeRPCtdc',
                                             expRun + 'RPC TDC vs revotime;tdc (ns);revotime - minRevotime',
                                             64, 767.5, 1023.5, 10, -0.5, 159.5)
        #: scatterplot of RPC REVO9 range vs TDC value corrected for DC-processing delay in event
        self.hist_revotimeRPCtdc2 = ROOT.TH2F(
            'revotimeRPCtdc2',
            expRun + 'RPC TDC vs revotime;' +
            'tdc - dt(index) (ns);' +
            'revotime - minRevotime',
            64, 767.5, 1023.5, 10, -0.5, 159.5)
        #: scatterplot of RPC calibrated time vs hit's index
        self.hist_jRPCtdc = ROOT.TH2F('jRPCtdc',
                                      expRun + 'RPC TDC vs hit index;tdc (ns);Hit index',
                                      64, 767.5, 1023.5, 60, -0.5, 59.5)
        #: scatterplot of RPC calibrated time vs hit's index, corrected for DC-processing delay
        self.hist_jRPCtdc2 = ROOT.TH2F('jRPCtdc2',
                                       expRun + 'RPC TDC vs hit index;tdc - dt(index) (ns);Hit index',
                                       64, 767.5, 1023.5, 60, -0.5, 59.5)
        #: scatterplot of scint TDC low-order bits vs sector
        self.hist_ScintTimeLowBitsBySector = ROOT.TH2F(
            'ScintTimeLowBitsBySector',
            expRun + 'Scint TDC lowest-order bits;' +
            'Sector # (always 0);' +
            'TDC % 4 (ns)',
            3, -1.5, 1.5, 8, -0.25, 3.75)
        #: histogram of scint TDC value relative to event's trigger time
        self.hist_ScintTime = ROOT.TH1F('ScintTime',
                                        expRun + 'Scint tdc distribution;tdc - triggerTime (ns)',
                                        256, -0.5, 1023.5)
        #: histogram of scint CTIME value relative to event's ctime
        self.hist_ScintCtime = ROOT.TH1F('ScintCtime',
                                         expRun + 'Scint ctime distribution;ctime - triggerCtime (ns)',
                                         32, -0.5, 1023.5)
        #: histogram of scint CTIME value relative to event's trigger time
        self.hist_ScintCtime0 = ROOT.TH1F('ScintCtime0',
                                          expRun + 'Scint ctime distribution;ctime - triggerTime (ns)',
                                          32, -0.5, 1023.5)
        #: histogram of scint CTIME range in event
        self.hist_ScintCtimeRange = ROOT.TH1F(
            'ScintCtimeRange', expRun + 'Scint ctime-range in event;ctimeMax - ctimeMin (ns)', 128, -0.5, 8191.5)

    def terminate(self):
        """Handle job termination: draw histograms, close output files"""

        canvas = ROOT.TCanvas("canvas", self.pdfName, 1600, 1600)
        title = '{0}['.format(self.pdfName)
        canvas.SaveAs(title)
        canvas.Clear()
        canvas.GetPad(0).SetGrid(1, 1)
        canvas.GetPad(0).Update()
        self.hist_rawKLMlane.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMlane.GetName()))
        self.hist_rawKLMsizeMultihit.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMsizeMultihit.GetName()))
        self.hist_rawKLMsize.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_rawKLMsize.GetName()))
        self.hist_PerChannelMultiplicity.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_PerChannelMultiplicity.GetName()))
        canvas.Clear()
        canvas.Divide(2, 1)
        canvas.GetPad(0).SetGrid(1, 1)
        canvas.GetPad(1).SetGrid(1, 1)
        canvas.GetPad(2).SetGrid(1, 1)
        for sectorFB in range(0, 1):
            canvas.cd(1)
            self.hist_ChannelOccupancy[0].Draw("colz")
            canvas.cd(2)
            self.hist_ChannelOccupancy[1].Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ChannelOccupancy[0].GetName()))
        for lane in range(0, 21):
            n0 = self.hist_ChannelOccupancyAL[lane][0].GetEntries()
            n1 = self.hist_ChannelOccupancyAL[lane][1].GetEntries()
            if n0 + n1 > 0:
                canvas.cd(1)
                self.hist_ChannelOccupancyAL[lane][0].Draw()
                canvas.cd(2)
                self.hist_ChannelOccupancyAL[lane][1].Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ChannelOccupancyAL[lane][0].GetName()))
        canvas.Clear()
        canvas.Divide(1, 1)
        self.hist_ttc_trigtime.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ttc_trigtime.GetName()))
        self.hist_RPCTimeLowBitsBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCTimeLowBitsBySector.GetName()))
        self.hist_RPCTime.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCTime.GetName()))
        for lane in range(0, 21):
            if self.hist_RPCTimePerLayerA0[lane].GetEntries() > 0:
                self.hist_RPCTimePerLayerA0[lane].Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCTimePerLayerA0[lane].GetName()))
        for lane in range(0, 21):
            if self.hist_RPCTimePerLayerA1[lane].GetEntries() > 0:
                self.hist_RPCTimePerLayerA1[lane].Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCTimePerLayerA1[lane].GetName()))
        self.hist_RPCTdcRange.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCTdcRange.GetName()))
        self.hist_RPCRevotimeRange.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_RPCRevotimeRange.GetName()))
        self.hist_revotimeRPCtdc.Draw("colz")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_revotimeRPCtdc.GetName()))
        # self.hist_revotimeRPCtdc2.Draw("colz")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_revotimeRPCtdc2.GetName()))
        self.hist_jRPCtdc.Draw("colz")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_jRPCtdc.GetName()))
        # self.hist_jRPCtdc2.Draw("colz")
        # canvas.Print(self.pdfName, "Title:{0}".format(self.hist_jRPCtdc2.GetName()))
        self.hist_ScintTimeLowBitsBySector.Draw("box")
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ScintTimeLowBitsBySector.GetName()))
        self.hist_ScintTime.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ScintTime.GetName()))
        self.hist_ScintCtime.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ScintCtime.GetName()))
        self.hist_ScintCtime0.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ScintCtime0.GetName()))
        self.hist_ScintCtimeRange.Draw()
        canvas.Print(self.pdfName, "Title:{0}".format(self.hist_ScintCtimeRange.GetName()))
        pdfNameLast = '{0}]'.format(self.pdfName)
        canvas.Print(pdfNameLast, "Title:{0}".format(self.hist_ScintCtimeRange.GetName()))
        self.histogramFile.Write()
        self.histogramFile.Close()
        print('Goodbye')

    def beginRun(self):
        """Handle begin of run: print diagnostic message"""
        print('beginRun', self.run)

    def endRun(self):
        """Handle end of run: print diagnostic message"""
        print('endRun', self.run)

    def event(self, eventHits, tt_ctime, raw_time):
        """Process one event: fill histograms"""

        sectorFB = 0
        n = len(eventHits)
        countAllMultihit = 2 * n + 1 if (n > 0) else 0
        countAll = 1
        channelMultiplicity = {}
        minCtime = 99999
        minRPCTdc = 99999
        maxRPCTdc = 0
        minRPCCtime = 99999
        maxRPCCtime = 0
        minScintCtime = 99999
        maxScintCtime = 0
        self.hist_ttc_trigtime.Fill((tt_ctime - raw_time) & 0x0fff)
        for j in range(0, n):
            items = eventHits[j]
            lane = items[0]
            channel = items[1]
            axis = items[2]
            ctime = items[3]  # this is revo9 time for RPCs, ctime for scintillators
            flag = 1 if (lane > 2) else 2
            isRPC = (flag == 1)
            isScint = (flag == 2)
            laneAxisChannel = (((lane << 1) + axis) << 7) + channel
            if laneAxisChannel not in channelMultiplicity:
                countAll = countAll + 2
                channelMultiplicity[laneAxisChannel] = 0
            channelMultiplicity[laneAxisChannel] = channelMultiplicity[laneAxisChannel] + 1
            if ctime < minCtime:
                minCtime = ctime
            self.hist_rawKLMlane.Fill(lane)
        for j in range(0, n):
            items = eventHits[j]
            lane = items[0]
            channel = items[1]
            axis = items[2]
            ctime = items[3]  # this is revo9 time for RPCs, ctime for scintillators
            tdc = items[4]
            charge = items[5]
            flag = 1 if (lane > 2) else 2
            isRPC = (flag == 1)
            isScint = (flag == 2)
            laneAxisChannel = (((lane << 1) + axis) << 7) + channel
            laneAxis = axis if ((lane < 1) or (lane > 20)) else ((lane << 1) + axis)
            if laneAxisChannel in channelMultiplicity:
                if channelMultiplicity[laneAxisChannel] > 1:
                    self.hist_PerChannelMultiplicity.Fill(channelMultiplicity[laneAxisChannel], laneAxis)
                # DIVOT del channelMultiplicity[laneAxisChannel] # consider only first hit in the channel/axis/lane of this dc
                t = (tdc - raw_time) & 0x03ff  # in ns, range is 0..1023
                t2 = (tdc - tt_ctime) & 0x03ff  # in ns, range is 0..1023
                ct = ((ctime << 3) - tt_ctime) & 0x3ff
                if isRPC:
                    if tdc < minRPCTdc:
                        minRPCTdc = tdc
                    if tdc > maxRPCTdc:
                        maxRPCTdc = tdc
                    if ctime < minRPCCtime:
                        minRPCCtime = ctime
                    if ctime > maxRPCCtime:
                        maxRPCCtime = ctime
                    self.hist_RPCTimeLowBitsBySector.Fill(sectorFB, (tdc & 3))
                    self.hist_RPCLaneAxisOccupancy.Fill(sectorFB, laneAxis)
                    self.hist_RPCTime.Fill(t)
                    self.hist_RPCTime2.Fill(t2)
                    if axis == 0:
                        self.hist_RPCTimePerLayerA0[lane].Fill(t)
                        self.hist_RPCTime2PerLayerA0[lane].Fill(t2)
                    else:
                        self.hist_RPCTimePerLayerA1[lane].Fill(t)
                        self.hist_RPCTime2PerLayerA1[lane].Fill(t2)
                    if n > 0:
                        t0j = 0.75 * j
                        self.hist_revotimeRPCtdc.Fill(t, ctime - minCtime)
                        self.hist_revotimeRPCtdc2.Fill(t - t0j, ctime - minCtime)
                        self.hist_jRPCtdc.Fill(t, j)
                        self.hist_jRPCtdc2.Fill(t - t0j, j)
                else:
                    if ctime < minScintCtime:
                        minScintCtime = ctime
                    if ctime > maxScintCtime:
                        maxScintCtime = ctime
                    self.hist_ScintTimeLowBitsBySector.Fill(sectorFB, (tdc & 3))
                    self.hist_ScintLaneAxisOccupancy.Fill(sectorFB, laneAxis)
                    self.hist_ScintTime.Fill(t)
                    self.hist_ScintCtime.Fill(ct)
                    self.hist_ScintCtime0.Fill(((ctime << 3) - raw_time) & 0x3ff)
                self.hist_ChannelOccupancy[axis].Fill(lane, channel)
                self.hist_ChannelOccupancyAL[lane][axis].Fill(channel)
        if n > 1:
            if maxRPCTdc > 0:
                self.hist_RPCTdcRange.Fill(maxRPCTdc - minRPCTdc)
            if maxRPCCtime > 0:
                self.hist_RPCRevotimeRange.Fill((maxRPCCtime - minRPCCtime) << 3)
            if maxScintCtime > 0:
                self.hist_ScintCtimeRange.Fill((maxScintCtime - minScintCtime) << 3)
        self.hist_rawKLMsizeMultihit.Fill(countAllMultihit)
        self.hist_rawKLMsize.Fill(countAll)
