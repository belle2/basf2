#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Produce the wire-by-wire CDC tracking efficiency canvas from a DQM ROOT file.

Reads CDC/hTrackingWireEff from the input file (produced by CDCDQM) and
writes a 4-panel canvas:
  (a) observed wire hits
  (b) expected wire hits from track extrapolation
  (c) per-wire efficiency map (TH2Poly, XY view)
  (d) one-dimensional efficiency distribution

Output: <output>.pdf and <output>.root

Usage:
    basf2 CDCWireEfficiencyCanvas.py [-- --input dqm_cdc.root --output wire_eff]
"""

import argparse
import math
import basf2 as b2

parser = argparse.ArgumentParser()
parser.add_argument('--input',    default='dqm_cdc.root',
                    help='DQM ROOT file containing CDC/hTrackingWireEff (default: dqm_cdc.root)')
parser.add_argument('--output',   default='wire_eff',
                    help='output base name without extension (default: wire_eff)')
parser.add_argument('--label',    default='Belle II',
                    help='primary label drawn on the canvas (default: Belle II)')
parser.add_argument('--sublabel', default='',
                    help='secondary label, e.g. event type or run range')
parser.add_argument('--release',  default='',
                    help='software release label drawn on the canvas')
args, _ = parser.parse_known_args()


def draw_label(pad, label, sublabel, release, xshift=0.0, yshift=0.0):
    import ROOT
    pad.cd()
    tex = ROOT.TLatex()
    tex.SetNDC(True)
    tex.SetTextAlign(33)  # top-right
    tex.SetTextSize(0.036)
    dy = 0.050
    x, y = 0.84 + xshift, 0.92 + yshift
    if label:
        tex.SetTextFont(72)
        tex.DrawLatex(x, y, label)
        y -= dy
    tex.SetTextFont(42)
    if sublabel:
        tex.DrawLatex(x, y, sublabel)
        y -= dy
    if release:
        tex.DrawLatex(x, y, release)


def make_poly(name, title, geo):
    import ROOT
    nSL = geo.getNumberOfSenseLayers()
    maxR = geo.senseWireR(nSL - 1)
    h = ROOT.TH2Poly(name, title, -maxR * 1.05, maxR * 1.05, -maxR * 1.05, maxR * 1.05)
    for lay in range(nSL):
        nW = geo.nWiresInLayer(lay)
        off = geo.offset(lay)
        R = geo.senseWireR(lay)
        if lay == 0:
            dr = geo.senseWireR(1) - R
            r1, r2 = R - dr / 2.0, R + dr / 2.0
        elif lay == nSL - 1:
            dr = R - geo.senseWireR(lay - 1)
            r1, r2 = R - dr / 2.0, R + dr / 2.0
        else:
            r1 = R - (R - geo.senseWireR(lay - 1)) / 2.0
            r2 = R + (geo.senseWireR(lay + 1) - R) / 2.0
        dPhi = 2.0 * math.pi / nW
        for wire in range(nW):
            phi = dPhi * (wire + off)
            phi1 = phi - dPhi * 0.5
            phi2 = phi + dPhi * 0.5
            import array
            xx = array.array('d', [r1 * math.cos(phi1), r2 * math.cos(phi1),
                                   r2 * math.cos(phi2), r1 * math.cos(phi2)])
            yy = array.array('d', [r1 * math.sin(phi1), r2 * math.sin(phi1),
                                   r2 * math.sin(phi2), r1 * math.sin(phi2)])
            h.AddBin(4, xx, yy)
    return h


class CDCCanvasModule(b2.Module):

    def initialize(self):
        import ROOT
        from ROOT import Belle2

        ROOT.gROOT.SetBatch(True)
        ROOT.gStyle.SetOptStat(0)
        ROOT.gStyle.SetPalette(ROOT.kBird)

        fin = ROOT.TFile.Open(args.input, 'READ')
        if not fin or fin.IsZombie():
            b2.B2FATAL(f'Cannot open input file: {args.input}')
        h_src = fin.Get('CDC/hTrackingWireEff')
        if not h_src:
            b2.B2FATAL(f'CDC/hTrackingWireEff not found in {args.input}')
        h_src.SetDirectory(0)
        fin.Close()

        geo = Belle2.CDC.CDCGeometryPar.Instance()
        nSL = geo.getNumberOfSenseLayers()

        b2.B2INFO('CDCCanvas: building TH2Poly geometry ...')
        h_obs = make_poly('hist_attachedWires', ';X [cm];Y [cm]', geo)
        h_exp = make_poly('hist_expectedWires',  ';X [cm];Y [cm]', geo)
        h_eff = make_poly('hist_wireAttachEff',  ';X [cm];Y [cm]', geo)
        h_1d = ROOT.TH1F('hist_wire_attach_eff_1d', ';Efficiency;Wires / bin', 104, -0.02, 1.02)

        b2.B2INFO('CDCCanvas: filling histograms ...')
        for lay in range(nSL):
            nW = geo.nWiresInLayer(lay)
            off = geo.offset(lay)
            R = geo.senseWireR(lay)
            dPhi = 2.0 * math.pi / nW
            exp_ybin = lay + 1
            obs_ybin = lay + nSL + 1
            for wire in range(nW):
                phi = dPhi * (wire + off)
                x = R * math.cos(phi)
                y = R * math.sin(phi)
                exp_v = h_src.GetBinContent(wire + 1, exp_ybin)
                obs_v = h_src.GetBinContent(wire + 1, obs_ybin)
                h_obs.Fill(x, y, obs_v)
                h_exp.Fill(x, y, exp_v)
                if exp_v > 0:
                    eff = obs_v / exp_v
                    h_eff.Fill(x, y, eff)
                    h_1d.Fill(eff)

        # ── canvas layout ────────────────────────────────────────────────────
        c = ROOT.TCanvas('canvas_wire_eff', 'CDC Wire Tracking Efficiency', 1600, 1400)
        gap = 0.005

        def make_pad(name, x1, y1, x2, y2):
            p = ROOT.TPad(name, name, x1, y1, x2, y2)
            p.Draw()
            return p

        p1 = make_pad('p1', gap,       0.5 + gap, 0.5 - gap, 1.0 - gap)
        p2 = make_pad('p2', 0.5 + gap, 0.5 + gap, 1.0 - gap, 1.0 - gap)
        p3 = make_pad('p3', gap,       gap,        0.5 - gap, 0.5 - gap)
        p4 = make_pad('p4', 0.5 + gap, gap,        1.0 - gap, 0.5 - gap)

        def setup_circle_pad(p):
            p.SetLeftMargin(0.13)
            p.SetRightMargin(0.15)
            p.SetTopMargin(0.11)
            p.SetBottomMargin(0.07)

        lbl = ROOT.TLatex()
        lbl.SetTextSize(0.060)
        lbl.SetTextFont(62)
        lbl.SetNDC(True)

        # (a) observed
        p1.cd()
        setup_circle_pad(p1)
        h_obs.SetStats(0)
        h_obs.GetYaxis().SetTitleOffset(1.2)
        h_obs.GetZaxis().SetRangeUser(0, h_obs.GetMaximum())
        h_obs.Draw('COLZ')
        lbl.DrawLatex(0.15, 0.82, '(a)')
        draw_label(p1, args.label, args.sublabel, args.release)

        # (b) expected
        p2.cd()
        setup_circle_pad(p2)
        h_exp.SetStats(0)
        h_exp.GetYaxis().SetTitleOffset(1.2)
        h_exp.GetZaxis().SetRangeUser(0, h_exp.GetMaximum())
        h_exp.Draw('COLZ')
        lbl.DrawLatex(0.15, 0.82, '(b)')
        draw_label(p2, args.label, args.sublabel, args.release)

        # (c) efficiency
        p3.cd()
        setup_circle_pad(p3)
        p3.SetTopMargin(0.03)
        p3.SetBottomMargin(0.15)
        h_eff.SetStats(0)
        h_eff.GetYaxis().SetTitleOffset(1.2)
        h_eff.GetZaxis().SetTitle('Efficiency')
        h_eff.GetZaxis().SetTitleOffset(1.3)
        h_eff.GetZaxis().SetRangeUser(0, 1)
        h_eff.Draw('COLZ')
        lbl.DrawLatex(0.15, 0.87, '(c)')
        draw_label(p3, args.label, args.sublabel, args.release, 0.0, 0.08)

        # ── efficiency fractions ─────────────────────────────────────────────
        total = h_1d.Integral(1, h_1d.GetNbinsX())
        b08 = h_1d.FindBin(0.08)
        b72 = h_1d.FindBin(0.72)
        f_dead = h_1d.Integral(1,    b08 - 1) / total * 100.0 if total > 0 else 0.0
        f_mid = h_1d.Integral(b08,  b72 - 1) / total * 100.0 if total > 0 else 0.0
        f_good = h_1d.Integral(b72,  h_1d.GetNbinsX()) / total * 100.0 if total > 0 else 0.0
        mean_eff = h_1d.GetMean()

        # (d) 1D distribution
        p4.cd()
        p4.SetLeftMargin(0.14)
        p4.SetRightMargin(0.06)
        p4.SetTopMargin(0.03)
        p4.SetBottomMargin(0.15)
        h_1d.SetStats(0)
        h_1d.SetLineWidth(2)
        h_1d.Draw()
        lbl.DrawLatex(0.16, 0.87, '(d)')
        draw_label(p4, args.label, args.sublabel, args.release, -0.08)

        stats = ROOT.TLatex()
        stats.SetNDC(True)
        stats.SetTextSize(0.036)
        stats.SetTextFont(42)

        sx_def_r = 0.45
        sx_pct_l = 0.465
        sy = 0.58
        sdy = 0.045

        stats.SetTextAlign(11)
        stats.DrawLatex(0.20, sy, f'Mean #varepsilon = {mean_eff:.3f}')
        sy -= sdy

        stats.SetTextAlign(31)
        stats.DrawLatex(sx_def_r, sy,         '#varepsilon #geq 0.72')
        stats.DrawLatex(sx_def_r, sy - sdy,   '0.08 #leq #varepsilon < 0.72')
        stats.DrawLatex(sx_def_r, sy - 2*sdy, '#varepsilon < 0.08')

        stats.SetTextAlign(11)
        stats.DrawLatex(sx_pct_l, sy,         f': {f_good:.2f}%')
        stats.DrawLatex(sx_pct_l, sy - sdy,   f': {f_mid:.2f}%')
        stats.DrawLatex(sx_pct_l, sy - 2*sdy, f': {f_dead:.2f}%')

        c.cd()
        c.SaveAs(f'{args.output}.pdf')
        b2.B2INFO(f'CDCCanvas: saved {args.output}.pdf')

        fout = ROOT.TFile(f'{args.output}.root', 'RECREATE')
        h_obs.Write()
        h_exp.Write()
        h_eff.Write()
        h_1d.Write()
        fout.Close()
        b2.B2INFO(f'CDCCanvas: saved {args.output}.root')


b2.set_log_level(b2.LogLevel.WARNING)
main = b2.create_path()
main.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
main.add_module('Gearbox')
main.add_module('Geometry', useDB=True)
main.add_module(CDCCanvasModule())
b2.process(main)
