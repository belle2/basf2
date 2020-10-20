#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>CosmicsExtrapolation.root</input>
  <output>CosmicsExtrapolationPlots.root</output>
  <description>Validation of cosmic track extrapolation.</description>
</header>
"""

from ROOT import Belle2, TNamed
import ROOT
contact = 'Kirill Chilikin (chilikin@lebedev.ru)'


input_file = ROOT.TFile('../CosmicsExtrapolation.root')
output_file = ROOT.TFile('CosmicsExtrapolationPlots.root', 'recreate')

bklm_numbers = Belle2.BKLMElementNumbers()
eklm_numbers = Belle2.EKLMElementNumbers.Instance()


def set_options(histogram, description):
    histogram.SetXTitle('cm')
    histogram.SetYTitle('Events')
    function_list = histogram.GetListOfFunctions()
    function_list.Add(TNamed('Description', description))
    function_list.Add(TNamed('Check', ' No bias, no large background, resolution ~ 2 cm.'))
    function_list.Add(TNamed('Contact', contact))
    function_list.Add(TNamed('MetaOptions', 'shifter'))


# X resolution histograms (forward propagation).
hist_xres_forward_mum = ROOT.TH1F('xres_forward_mum',
                                  'X resolution (#mu^{-}, forward)',
                                  100, -20, 20)
set_options(hist_xres_forward_mum,
            'Extrapolated hit X resolution (mu-, forward propagation).')
hist_xres_forward_mup = ROOT.TH1F('xres_forward_mup',
                                  'X resolution (#mu^{+}, forward)',
                                  100, -20, 20)
set_options(hist_xres_forward_mup,
            'Extrapolated hit X resolution (mu+, forward propagation).')
# Y resolution histograms (forward propagation).
hist_yres_forward_mum = ROOT.TH1F('yres_forward_mum',
                                  'Y resolution (#mu^{-}, forward)',
                                  100, -20, 20)
set_options(hist_yres_forward_mum,
            'Extrapolated hit Y resolution (mu-, forward propagation).')
hist_yres_forward_mup = ROOT.TH1F('yres_forward_mup',
                                  'Y resolution (#mu^{+}, forward)',
                                  100, -20, 20)
set_options(hist_yres_forward_mup,
            'Extrapolated hit Y resolution (mu+, forward propagation).')
# Z resolution histograms (forward propagation).
hist_zres_forward_mum = ROOT.TH1F('zres_forward_mum',
                                  'Z resolution (#mu^{-}, forward)',
                                  100, -20, 20)
set_options(hist_zres_forward_mum,
            'Extrapolated hit Z resolution (mu-, forward propagation).')
hist_zres_forward_mup = ROOT.TH1F('zres_forward_mup',
                                  'Z resolution (#mu^{+}, forward)',
                                  100, -20, 20)
set_options(hist_zres_forward_mup,
            'Extrapolated hit Z resolution (mu+, forward propagation).')

# X resolution histograms (backward propagation).
hist_xres_backward_mum = ROOT.TH1F('xres_backward_mum',
                                   'X resolution (#mu^{-}, backward)',
                                   100, -20, 20)
set_options(hist_xres_backward_mum,
            'Extrapolated hit X resolution (mu-, backward propagation).')
hist_xres_backward_mup = ROOT.TH1F('xres_backward_mup',
                                   'X resolution (#mu^{+}, backward)',
                                   100, -20, 20)
set_options(hist_xres_backward_mup,
            'Extrapolated hit X resolution (mu+, backward propagation).')
# Y resolution histograms (backward propagation).
hist_yres_backward_mum = ROOT.TH1F('yres_backward_mum',
                                   'Y resolution (#mu^{-}, backward)',
                                   100, -20, 20)
set_options(hist_yres_backward_mum,
            'Extrapolated hit Y resolution (mu-, backward propagation).')
hist_yres_backward_mup = ROOT.TH1F('yres_backward_mup',
                                   'Y resolution (#mu^{+}, backward)',
                                   100, -20, 20)
set_options(hist_yres_backward_mup,
            'Extrapolated hit Y resolution (mu+, backward propagation).')
# Z resolution histograms (backward propagation).
hist_zres_backward_mum = ROOT.TH1F('zres_backward_mum',
                                   'Z resolution (#mu^{-}, backward)',
                                   100, -20, 20)
set_options(hist_zres_backward_mum,
            'Extrapolated hit Z resolution (mu-, backward propagation).')
hist_zres_backward_mup = ROOT.TH1F('zres_backward_mup',
                                   'Z resolution (#mu^{+}, backward)',
                                   100, -20, 20)
set_options(hist_zres_backward_mup,
            'Extrapolated hit Z resolution (mu+, backward propagation).')

for event in input_file.tree:
    bklmhit2ds = event.BKLMHit2ds
    eklmhit2ds = event.EKLMHit2ds
    exthits = event.ExtHits
    for bklmhit2d in bklmhit2ds:
        section = bklmhit2d.getSection()
        sector = bklmhit2d.getSector()
        layer = bklmhit2d.getLayer()
        for exthit in exthits:
            if exthit.getDetectorID() != Belle2.Const.BKLM:
                continue
            module = exthit.getCopyID()
            section_ext = bklm_numbers.getSectionByModule(module)
            sector_ext = bklm_numbers.getSectorByModule(module)
            layer_ext = bklm_numbers.getLayerByModule(module)
            if (section_ext != section or sector_ext != sector or
                    layer_ext != layer):
                continue
            ext_position = exthit.getPosition()
            if exthit.isBackwardPropagated():
                if exthit.getPdgCode() == 13:
                    hist_xres_backward_mum.Fill(
                        ext_position.X() - bklmhit2d.getGlobalPositionX())
                    hist_yres_backward_mum.Fill(
                        ext_position.Y() - bklmhit2d.getGlobalPositionY())
                    hist_zres_backward_mum.Fill(
                        ext_position.Z() - bklmhit2d.getGlobalPositionZ())
                elif exthit.getPdgCode() == -13:
                    hist_xres_backward_mup.Fill(
                        ext_position.X() - bklmhit2d.getGlobalPositionX())
                    hist_yres_backward_mup.Fill(
                        ext_position.Y() - bklmhit2d.getGlobalPositionY())
                    hist_zres_backward_mup.Fill(
                        ext_position.Z() - bklmhit2d.getGlobalPositionZ())
            else:
                if exthit.getPdgCode() == 13:
                    hist_xres_forward_mum.Fill(
                        ext_position.X() - bklmhit2d.getGlobalPositionX())
                    hist_yres_forward_mum.Fill(
                        ext_position.Y() - bklmhit2d.getGlobalPositionY())
                    hist_zres_forward_mum.Fill(
                        ext_position.Z() - bklmhit2d.getGlobalPositionZ())
                elif exthit.getPdgCode() == -13:
                    hist_xres_forward_mup.Fill(
                        ext_position.X() - bklmhit2d.getGlobalPositionX())
                    hist_yres_forward_mup.Fill(
                        ext_position.Y() - bklmhit2d.getGlobalPositionY())
                    hist_zres_forward_mup.Fill(
                        ext_position.Z() - bklmhit2d.getGlobalPositionZ())

    for eklmhit2d in eklmhit2ds:
        section = eklmhit2d.getSection()
        sector = eklmhit2d.getSector()
        layer = eklmhit2d.getLayer()
        for exthit in exthits:
            if exthit.getDetectorID() != Belle2.Const.EKLM:
                continue
            strip_global = exthit.getCopyID()
            section_ext = eklm_numbers.getSectionByGlobalStrip(strip_global)
            sector_ext = eklm_numbers.getSectorByGlobalStrip(strip_global)
            layer_ext = eklm_numbers.getLayerByGlobalStrip(strip_global)
            if (section_ext != section or sector_ext != sector or
                    layer_ext != layer):
                continue
            ext_position = exthit.getPosition()
            if exthit.isBackwardPropagated():
                if exthit.getPdgCode() == 13:
                    hist_xres_backward_mum.Fill(
                        ext_position.X() - eklmhit2d.getPositionX())
                    hist_yres_backward_mum.Fill(
                        ext_position.Y() - eklmhit2d.getPositionY())
                    hist_zres_backward_mum.Fill(
                        ext_position.Z() - eklmhit2d.getPositionZ())
                elif exthit.getPdgCode() == -13:
                    hist_xres_backward_mup.Fill(
                        ext_position.X() - eklmhit2d.getPositionX())
                    hist_yres_backward_mup.Fill(
                        ext_position.Y() - eklmhit2d.getPositionY())
                    hist_zres_backward_mup.Fill(
                        ext_position.Z() - eklmhit2d.getPositionZ())
            else:
                if exthit.getPdgCode() == 13:
                    hist_xres_forward_mum.Fill(
                        ext_position.X() - eklmhit2d.getPositionX())
                    hist_yres_forward_mum.Fill(
                        ext_position.Y() - eklmhit2d.getPositionY())
                    hist_zres_forward_mum.Fill(
                        ext_position.Z() - eklmhit2d.getPositionZ())
                elif exthit.getPdgCode() == -13:
                    hist_xres_forward_mup.Fill(
                        ext_position.X() - eklmhit2d.getPositionX())
                    hist_yres_forward_mup.Fill(
                        ext_position.Y() - eklmhit2d.getPositionY())
                    hist_zres_forward_mup.Fill(
                        ext_position.Z() - eklmhit2d.getPositionZ())

output_file.cd()
hist_xres_forward_mum.Write()
hist_xres_forward_mup.Write()
hist_yres_forward_mum.Write()
hist_yres_forward_mup.Write()
hist_zres_forward_mum.Write()
hist_zres_forward_mup.Write()
hist_xres_backward_mum.Write()
hist_xres_backward_mup.Write()
hist_yres_backward_mum.Write()
hist_yres_backward_mup.Write()
hist_zres_backward_mum.Write()
hist_zres_backward_mup.Write()
output_file.Close()
input_file.Close()
