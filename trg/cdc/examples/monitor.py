#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2, TH2F, TFile, gStyle, TColor, TCanvas
import numpy
from math import pi
from interactive import embed
import os

# skip the event in the output if it doesn't contain trg data
skim_dummy_trg = True
# whether to save the output dst file
save_output = False

input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')

main = create_path()
main.add_module(root_input)

# unpack CDC data
# Set Database
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
cdc_unpacker = register_module('CDCUnpacker')
cdc_unpacker.param('enableStoreCDCRawHit', True)
main.add_module(cdc_unpacker)

unpacker = register_module('CDCTriggerUnpacker')
unpacker.logging.log_level = LogLevel.DEBUG
# increase this value to get debug mesages in more detail
unpacker.logging.debug_level = 10
unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
# size (number of words) of the Belle2Link header
unpacker.param('headerSize', 3)
# unpack the data from the 2D tracker and save its Bitstream
unpacker.param('unpackTracker2D', True)
# make CDCTriggerSegmentHit objects from the 2D input
unpacker.param('decode2DFinderInput', True)
# it seems the B2L for 2D0 and 2D1 are swapped
unpacker.param('2DNodeId', [
    [0x11000001, 1],
    [0x11000001, 0],
    [0x11000002, 0],
    [0x11000002, 1]])

main.add_module(unpacker)

if skim_dummy_trg:
    # skip if there are no trigger data in the event
    empty_path = create_path()
    unpacker.if_false(empty_path)

main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe',
                                        'PXD', 'SVD', 'CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
cdcdigitizer = register_module('CDCDigitizer')
# ...CDCDigitizer...
# set digitizer to no smearing
param_cdcdigi = {'Fraction': 1,
                 'Resolution1': 0.,
                 'Resolution2': 0.,
                 'Threshold': -10.0}
cdcdigitizer.param(param_cdcdigi)
cdcdigitizer.param('AddInWirePropagationDelay', True)
cdcdigitizer.param('AddTimeOfFlight', True)
cdcdigitizer.param('UseSimpleDigitization', True)
main.add_module(cdcdigitizer)
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"),
                TSHitCollectionName='TSimSegmentHits')


# make the plots

color1501 = TColor(1501, 35 / 255., 55 / 255., 59 / 255.)
color1502 = TColor(1502, 99 / 255., 120 / 255., 173 / 255.)
color1503 = TColor(1503, 235 / 255., 129 / 255., 27 / 255.)
color1504 = TColor(1504, 99 / 255., 173 / 255., 132 / 255.)
color1505 = TColor(1505, 123 / 255., 111 / 255., 37 / 255.)


def set_style(hist, color=1503, marker=20):
    hist.SetLineColor(color)
    hist.SetLineWidth(2)
    hist.SetFillStyle(0)
    hist.SetMarkerStyle(marker)
    hist.SetMarkerSize(2)
    hist.SetMarkerColor(color)


hall = TH2F('all', 'all', 5, 0, 5, 384, 0, 384)
hhit = TH2F('hit', 'hit', 5, 0, 5, 384, 0, 384)

used = numpy.zeros(37000)


class Monitor(Module):
    """
    Module to make some monitor plots for TSF
    """

    def initialize(self):
        self.event_info = Belle2.PyStoreObj('EventMetaData')
        self.tshits = Belle2.PyStoreArray('CDCTriggerSegmentHits')
        self.simhits = Belle2.PyStoreArray('TSimSegmentHits')
        self.first_run = self.event_info.getRun()

    def event(self):
        used.fill(0)
        for simhit in self.simhits:
            if simhit.getISuperLayer() % 2 == 0:
                iax, iw = simhit.getISuperLayer() // 2, simhit.getIWire()
                hall.Fill(iax, iw)
        for hit in self.tshits:
            ind = hit.getID()
            # only fill the histogram if the same ID is not found in the event
            # In other words, it can't be the same hit from another 2D
            # even if the unpacker does not eliminate repeated hits.
            if used[ind] == 0:
                used[ind] = 1
                iax, iw = hit.getISuperLayer() // 2, hit.getIWire()
                hhit.Fill(iax, iw)

    def terminate(self):
        if not os.path.exists('monitor_plots'):
            os.mkdir('monitor_plots')
        can = TCanvas('can2', 'can2', 800, 700)
        gStyle.SetOptStat(0)

        nts = [160, 192, 256, 320, 384]

        slall = [[]] * 5
        slhit = [[]] * 5
        quos = [[]] * 5

        # 1 bin corresponds to 8 TS (2 bins for a merger unit)
        width = 8
        for iax in range(5):
            ibin = iax + 1
            p = hall.ProjectionY('all{}'.format(iax), ibin, ibin)
            p.SetBins(nts[iax], 0, 2 * pi)
            p.Rebin(width)
            slall[iax] = p

            q = hhit.ProjectionY('hit{}'.format(iax), ibin, ibin)
            q.SetBins(nts[iax], 0, 2 * pi)
            q.Rebin(width)
            slhit[iax] = q

            r = q.Clone('quo{}'.format(iax))
            q.Sumw2()
            p.Sumw2()
            # get the ratio of (found hit / simulated hit) with binomial error
            r.Divide(q, p, 1.0, 1.0, 'B')
            quos[iax] = r

        for hits, name in [(slhit, 'data TS'),
                           (slall, 'TSIM')]:
            for i in range(5):
                h = hits[i]
                h.SetTitle('SL{}'.format(i * 2))
                h.Scale(1 / h.Integral('width'))
                set_style(h, 1501 + i, 20 + i)
            height = max([g.GetMaximum() for g in hits])
            for h in hits:
                h.SetMaximum(1.1 * height)
                h.SetMinimum(0)
                options = 'L M C'
                if h.GetTitle() != 'SL0':
                    options += ' same'
                h.Draw(options)
            can.BuildLegend(.85, .76, .95, .95)
            hits[0].SetTitle(name + ' hit distribution in run {}; #phi (rad)'.format(
                self.first_run))
            can.SaveAs('monitor_plots/' + name.split()[0] +
                       '_ts_hits_{:05d}.pdf'.format(self.first_run))

        upp = max([g.GetMaximum() for g in quos])
        low = min([g.GetMinimum() for g in quos])
        for i in range(5):
            h = quos[i]
            h.SetTitle('SL{}'.format(2 * i))
            h.SetMaximum(1.1 * upp)
            h.SetMinimum(0.9 * low)
            set_style(h, 1501 + i, 20 + i)
            options = 'e'
            if i != 0:
                options += ' same'
            h.Draw(options)
        can.BuildLegend(.85, .76, .95, .95)
        quos[0].SetTitle('TSF efficiency (w.r.t. fast TSIM) in run {};#phi (rad)'.format(
            self.first_run))
        can.SaveAs('monitor_plots/ts_eff_{:05d}.pdf'.format(self.first_run))


main.add_module(Monitor())

if save_output:
    # save the output root file with specified file name
    output_name = input_files.front().split('/')[-1]
    output_name = output_name[:output_name.rfind('.')] + '.unpacked.root'
    main.add_module('RootOutput',
                    outputFileName=output_name,
                    excludeBranchNames=['RawCDCs',
                                        'RawECLs',
                                        'RawKLMs',
                                        'RawSVDs',
                                        'RawPXDs',
                                        'RawTOPs'])

process(main)
print(statistics)
