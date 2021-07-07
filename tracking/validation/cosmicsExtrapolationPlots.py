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

# X resolution histogram.
hist_xres = ROOT.TH1F('xres', 'Extrapolated hit X resolution', 100, -20, 20)
hist_xres.SetXTitle('cm')
hist_xres.SetYTitle('Events')
function_list = hist_xres.GetListOfFunctions()
function_list.Add(TNamed('Description', 'X resolution'))
function_list.Add(TNamed('Check', 'No bias, resolution ~ 2 cm.'))
function_list.Add(TNamed('Contact', contact))
function_list.Add(TNamed('MetaOptions', 'shifter'))
# Y resolution histogram.
hist_yres = ROOT.TH1F('yres', 'Extrapolated hit Y resolution', 100, -20, 20)
hist_yres.SetXTitle('cm')
hist_yres.SetYTitle('Events')
function_list = hist_yres.GetListOfFunctions()
function_list.Add(TNamed('Description', 'Y resolution'))
function_list.Add(TNamed('Check', 'No bias, resolution ~ 2 cm.'))
function_list.Add(TNamed('Contact', contact))
function_list.Add(TNamed('MetaOptions', 'shifter'))
# Z resolution histogram.
hist_zres = ROOT.TH1F('zres', 'Extrapolated hit Z resolution', 100, -20, 20)
hist_zres.SetXTitle('cm')
hist_zres.SetYTitle('Events')
function_list = hist_zres.GetListOfFunctions()
function_list.Add(TNamed('Description', 'Z resolution'))
function_list.Add(TNamed('Check', 'No bias, resolution ~ 2 cm.'))
function_list.Add(TNamed('Contact', contact))
function_list.Add(TNamed('MetaOptions', 'shifter'))

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
            hist_xres.Fill(ext_position.X() - bklmhit2d.getGlobalPositionX())
            hist_yres.Fill(ext_position.Y() - bklmhit2d.getGlobalPositionY())
            hist_zres.Fill(ext_position.Z() - bklmhit2d.getGlobalPositionZ())
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
            hist_xres.Fill(ext_position.X() - eklmhit2d.getPositionX())
            hist_yres.Fill(ext_position.Y() - eklmhit2d.getPositionY())
            hist_zres.Fill(ext_position.Z() - eklmhit2d.getPositionZ())

output_file.cd()
hist_xres.Write()
hist_yres.Write()
hist_zres.Write()
output_file.Close()
input_file.Close()
