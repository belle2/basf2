# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2, TH2F, TH1F, TFile, gStyle, TColor, TCanvas
import numpy
from math import pi
from interactive import embed


# make the monitoring plots
file_type = 'png'

color1501 = TColor(1501, 35 / 255., 55 / 255., 59 / 255.)
color1502 = TColor(1502, 99 / 255., 120 / 255., 173 / 255.)
color1503 = TColor(1503, 235 / 255., 129 / 255., 27 / 255.)
color1504 = TColor(1504, 99 / 255., 173 / 255., 132 / 255.)
color1505 = TColor(1505, 123 / 255., 111 / 255., 37 / 255.)

nWiresInLayer = [160, 192, 256, 320, 384]


def set_style(hist, color=1503, marker=20):
    hist.SetLineColor(color)
    hist.SetLineWidth(2)
    hist.SetFillStyle(0)
    hist.SetMarkerStyle(marker)
    hist.SetMarkerSize(2)
    hist.SetMarkerColor(color)


hall = TH2F('all', 'all', 5, 0, 5, 384, 0, 384)
hhit = TH2F('hit', 'hit', 5, 0, 5, 384, 0, 384)
hno_ghost = TH2F('ng', 'hit', 5, 0, 5, 384, 0, 384)
hghost = TH2F('ghost', 'hit', 5, 0, 5, 384, 0, 384)

hlr = TH1F('lr', 'left right', 4, 0, 4)
hpri = TH1F('pri', 'priority position', 4, 0, 4)
hpritime = TH1F('pritime', 'priority time', 512, 0, 512)
hftime = TH1F('found time', 'found time', 48, -35, 13)

used = numpy.zeros(2336, numpy.int)
in_tsim = numpy.zeros(2336, numpy.int)


class Monitor(Module):
    """
    Module to make some monitor plots for TSF
    """

    def initialize(self):
        self.event_info = Belle2.PyStoreObj('EventMetaData')
        self.tshits = Belle2.PyStoreArray('CDCTriggerSegmentHits')
        self.simhits = Belle2.PyStoreArray('TSimSegmentHits')
        self.first_run = self.event_info.getRun()
        self.nevents = 0

    def event(self):
        self.nevents += 1
        used.fill(0)
        in_tsim.fill(0)
        for simhit in self.simhits:
            if simhit.getISuperLayer() % 2 == 0:
                iax, iw = simhit.getISuperLayer() // 2, simhit.getIWire()
                if simhit.getPriorityPosition() == 1:
                    iw += 1
                if iw == nWiresInLayer[iax]:
                    iw = 0
                hall.Fill(iax, iw)
                in_tsim[simhit.getSegmentID()] = 1
        for hit in self.tshits:
            ind = hit.getSegmentID()
            # only fill the histogram if the same ID is not found in the event
            # In other words, it can't be the same hit from another 2D
            # even if the unpacker does not eliminate repeated hits.
            iax, iw = hit.getISuperLayer() // 2, hit.getIWire()
            if hit.getPriorityPosition() == 1:
                iw += 1
            if iw == nWiresInLayer[iax]:
                iw = 0
            if not used[ind]:
                used[ind] = 1
                hhit.Fill(iax, iw)
                if in_tsim[ind]:
                    hno_ghost.Fill(iax, iw)
            if not in_tsim[ind]:
                hghost.Fill(iax, iw)
                hlr.Fill(hit.getLeftRight())
                hpri.Fill(hit.getPriorityPosition())
                hpritime.Fill(hit.priorityTime())
                hftime.Fill(hit.foundTime())

    def terminate(self):
        if self.nevents == 0:
            B2WARNING("The monitor module is never called.\n" +
                      "There seems to be no CDC Trigger data at all!")
            return
        elif hhit.GetEntries() == 0:
            B2WARNING("No recorded TS hits at all!")
            return
        elif hall.GetEntries() == 0:
            B2ERROR("No simulated TS hits at all!")
            return
        if not os.path.exists('monitor_plots'):
            os.mkdir('monitor_plots')
        can = TCanvas('can2', 'can2', 800, 700)
        gStyle.SetOptStat(0)

        nts = [160, 192, 256, 320, 384]

        slall = [[]] * 5
        slhit = [[]] * 5
        slghost = [[]] * 5
        quos = [[]] * 5
        slng = [[]] * 5
        quong = [[]] * 5
        quo_ghost = [[]] * 5

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

            g = hghost.ProjectionY('ghost{}'.format(iax), ibin, ibin)
            g.SetBins(nts[iax], 0, 2 * pi)
            g.Rebin(width)
            slghost[iax] = g

            s = hno_ghost.ProjectionY('ng{}'.format(iax), ibin, ibin)
            s.SetBins(nts[iax], 0, 2 * pi)
            s.Rebin(width)
            slng[iax] = s

            r = g.Clone('ghost{}'.format(iax))
            r2 = s.Clone('quong{}'.format(iax))
            g.Sumw2()
            p.Sumw2()
            s.Sumw2()
            # get the ratio of (found hit / simulated hit) with binomial error
            r.Divide(g, p, 1.0, 1.0, 'B')
            quos[iax] = r
            r2.Divide(s, p, 1.0, 1.0, 'B')
            quong[iax] = r2

        for hits, name in [(slhit, 'data TS'),
                           (slall, 'TSIM')]:
            for i in range(5):
                h = hits[i]
                h.SetTitle(f'SL{i * 2}')
                set_style(h, 1501 + i, 20 + i)
                if name in ['data TS', 'TSIM']:
                    try:
                        h.Scale(1 / h.Integral('width'))
                    except ZeroDivisionError:
                        B2WARNING(f'Not a single hit in SL{i * 2}!')
                        continue
            height = max([g.GetMaximum() for g in hits])
            for h in hits:
                h.SetMaximum(1.1 * height)
                h.SetMinimum(0)
                options = 'L M E0'
                if h.GetTitle() != 'SL0':
                    options += ' same'
                h.Draw(options)
            can.BuildLegend(.85, .76, .95, .95)
            hits[0].SetTitle(name + ' hit distribution in run {}; #phi (rad)'.format(
                self.first_run))
            can.SaveAs('monitor_plots/' + name.split()[0] +
                       '_ts_hits_{:05d}.{}'.format(self.first_run, file_type))

        for ratio, name in [(quos, 'ghost rate'), (quong, 'efficiency (w.r.t. fast TSIM)')]:
            upp = max([g.GetMaximum() for g in ratio])
            low = min([g.GetMinimum() for g in ratio])
            for i in range(5):
                h = ratio[i]
                h.SetTitle('SL{}'.format(2 * i))
                h.SetMaximum(1.1 * upp)
                h.SetMinimum(0.9 * low)
                set_style(h, 1501 + i, 20 + i)
                options = 'e0'
                if i != 0:
                    options += ' same'
                h.Draw(options)
            can.BuildLegend(.91, .76, .98, .95)
            ratio[0].SetTitle('TSF ' + name + ' in run {};#phi (rad)'.format(
                self.first_run))
            file_name = name.split()[0]
            can.SaveAs('monitor_plots/ts_{}_{:05d}.{}'.format(file_name, self.first_run, file_type))

        # ghost distribution
        for h in [hlr, hpri, hpritime, hftime]:
            set_style(h, 1503)
            h.Draw()
            name = h.GetTitle().replace(' ', '_')
            can.SaveAs('monitor_plots/ghost_{}_{:05d}.{}'.format(name, self.first_run, file_type))
