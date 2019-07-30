#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#     Create bklm validation plots and store them               #
#     in a root file                                            #
#                                                               #
#    written by Leo Piilonen, VT                                #
#    piilonen@vt.edu                                            #
#                                                               #
#################################################################

"""
<header>
    <input>muon-BKLMValidation.root</input>
    <contact>piilonen@vt.edu</contact>
    <description>Create validation plots for BKLM</description>
</header>
"""

import ROOT

ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.gROOT.SetBatch(True)

from ROOT import TFile, TChain, TTree, TH1F, TH2F, TCanvas, TNamed, gStyle
import sys
import math
import numpy as np
from optparse import OptionParser

# contact person information
# is added to the plot descriptions
CONTACT_PERSON = {'Name': 'Leo Piilonen',
                  'Email': 'piilonen@vt.edu'}


def main():
    """Create validation plots for BKLM"""

    print('BKLM validation plots.')

    option_parser = OptionParser()
    option_parser.add_option('-i', '--input-file', dest='input_file',
                             default='../muon-BKLMValidation.root',
                             help='Root file with Ext/Muid/BKLM/EKLM validation data.'
                             )
    option_parser.add_option('-o', '--output-file', dest='output_file',
                             default='BKLMValidation.root',
                             help='Root file with BKLM validation histograms.')

    (options, args) = option_parser.parse_args()

    # load chain of input files
    file_chain = TChain('tree')
    file_chain.Add(options.input_file)

    # for file in file_chain:

    number_entries = 0
    try:
        number_entries = file_chain.GetEntries()
    except AttributeError:
        print('Could not load input file(s) %s.' % options.input_file)

    if number_entries == 0:
        print('Data tree is empty or does not exist in file(s) %s. Exit.' % options.input_file)
        sys.exit(0)

    # open the output root file
    output_root_file = TFile(options.output_file, 'recreate')

    # create and draw histograms
    gStyle.SetOptStat(0)
    draw_bklmhists(file_chain)

    # close the output file
    output_root_file.Write()
    output_root_file.Close()


def draw_bklmhists(file_chain):
    """
    Draw the BKLMHit2d-related distributions.
    """

    # NOTE: *.Draw() must precede *.GetListOfFunctions().Add() or the latter will be discarded!
    inRPC = TH1F('InRPC', 'InRPC for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.inRPC()>>InRPC', '')
    inRPC.GetXaxis().SetTitle('0=scint  1=RPC')
    inRPC.GetListOfFunctions().Add(TNamed('Description', 'Flag indicating if muon hit is in scintillator (0) or RPC (1)'))
    inRPC.GetListOfFunctions().Add(TNamed('Check', 'Mostly in RPC'))
    inRPC.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    inRPC.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    inRPC.SetMinimum(0.0)
    inRPC.Write()

    section = TH1F('Forward', 'Section for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.getSection()>>Forward', '')
    section.GetXaxis().SetTitle('0=backward  1=section')
    section.GetListOfFunctions().Add(TNamed('Description', 'Flag indicating if muon hit is at z < 47 cm (0) or z > 47 cm (1)'))
    section.GetListOfFunctions().Add(TNamed('Check', 'Somewhat more at backward end'))
    section.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    section.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    section.SetMinimum(0.0)
    section.Write()

    isOnTrack = TH1F('IsOnTrack', 'IsOnTrack for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.isOnTrack()>>IsOnTrack', '')
    isOnTrack.GetXaxis().SetTitle('0=not associated with track  1=associated with track')
    isOnTrack.GetListOfFunctions().Add(TNamed('Description', 'Flag indicating if muon hit associated with a CDC track by Muid'))
    isOnTrack.GetListOfFunctions().Add(TNamed('Check', 'Mostly associated'))
    isOnTrack.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    isOnTrack.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.50,pvalue-error=0.10'))
    isOnTrack.SetMinimum(0.0)
    isOnTrack.Write()

    sector = TH1F('Sector', 'Sector for BKLMHit2ds', 10, -0.5, 9.5)
    file_chain.Draw('BKLMHit2ds.getSector()>>Sector', '')
    sector.GetXaxis().SetTitle('Sector #')
    sector.GetListOfFunctions().Add(TNamed('Description', 'Sector number of muon hit'))
    sector.GetListOfFunctions().Add(TNamed('Check', 'Roughly flat in sectors 1-8'))
    sector.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    sector.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    sector.SetMinimum(0.0)
    sector.Write()

    layer = TH1F('Layer', 'Layer for BKLMHit2ds', 16, -0.5, 15.5)
    file_chain.Draw('BKLMHit2ds.getLayer()>>Layer', '')
    layer.GetXaxis().SetTitle('Layer #')
    layer.GetListOfFunctions().Add(TNamed('Description', 'Layer number of muon hit'))
    layer.GetListOfFunctions().Add(TNamed('Check', 'Peak at layer 1 with tail above that'))
    layer.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    layer.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    layer.SetMinimum(0.0)
    layer.Write()

    phistrip = TH1F('PhiStrip', 'PhiStrip for BKLMHit2ds', 50, -0.5, 49.5)
    file_chain.Draw('BKLMHit2ds.getPhiStripAve()>>PhiStrip', '')
    phistrip.GetXaxis().SetTitle('Phi strip #')
    phistrip.GetListOfFunctions().Add(TNamed('Description', 'Phi strip number of muon hit'))
    phistrip.GetListOfFunctions().Add(TNamed('Check', 'Roughly flat for 1-36 (all layers) and then for 37-48 (layers 6-15)'))
    phistrip.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    phistrip.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    phistrip.SetMinimum(0.0)
    phistrip.Write()

    zstrip = TH1F('ZStrip', 'ZStrip for BKLMHit2ds', 60, -0.5, 59.5)
    file_chain.Draw('BKLMHit2ds.getZStripAve()>>ZStrip', '')
    zstrip.GetXaxis().SetTitle('Z strip #')
    zstrip.GetListOfFunctions().Add(TNamed('Description', 'Z strip number of muon hit'))
    zstrip.GetListOfFunctions().Add(TNamed('Check', 'Downward-sloping for 1-48 (all layers), shoulder for 49-54 (layers 1-2)'))
    zstrip.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    zstrip.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    zstrip.SetMinimum(0.0)
    zstrip.Write()

    time = TH1F('Time', 'Hit time for BKLMHit2ds', 200, -10.0, 10.0)
    file_chain.Draw('BKLMHit2ds.getTime()>>Time', '')
    time.GetXaxis().SetTitle('t (ns)')
    time.GetListOfFunctions().Add(TNamed('Description', 'Time of muon hit'))
    time.GetListOfFunctions().Add(TNamed('Check', 'Peak near 0'))
    time.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    time.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    time.Write()

    edep = TH1F('EDep', 'Energy deposition for BKLMHit2ds', 50, 0.0, 25.0)
    file_chain.Draw('BKLMHit2ds.getEDep()*1000.0>>EDep', '')
    edep.GetXaxis().SetTitle('E (keV)')
    edep.GetListOfFunctions().Add(TNamed('Description', 'dE/dx energy deposition of muon hit'))
    edep.GetListOfFunctions().Add(TNamed('Check', 'Peak near 3 keV'))
    edep.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    edep.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    edep.Write()

    r = TH1F('r', 'r for BKLMHit2ds', 30, 200.0, 350.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Perp()>>r', '')
    r.GetXaxis().SetTitle('r (cm)')
    r.GetListOfFunctions().Add(TNamed('Description', 'Distance from z axis in transverse plane of muon hit'))
    r.GetListOfFunctions().Add(TNamed('Check', 'Comb-like downward-sloping distribution (a la layers)'))
    r.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    r.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    r.Write()

    z = TH1F('z', 'z for BKLMHit2ds', 100, -200.0, 300.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Z()>>z', '')
    z.GetXaxis().SetTitle('z (cm)')
    z.GetListOfFunctions().Add(TNamed('Description', 'Axial position of muon hit'))
    z.GetListOfFunctions().Add(TNamed('Check', 'Broad peak near zero with dip at 47 cm (forward-backward boundary)'))
    z.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    z.GetListOfFunctions().Add(TNamed('MetaOptions', 'pvalue-warn=0.99,pvalue-error=0.90'))
    z.Write()

    xy = TH2F('xy', 'y vs x for BKLMHit2ds', 140, -350.0, 350.0, 140, -350.0, 350.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Y():BKLMHit2ds.getGlobalPosition().X()>>xy', '')
    xy.GetXaxis().SetTitle('x (cm)')
    xy.GetYaxis().SetTitle('y (cm)')
    xy.GetListOfFunctions().Add(TNamed('Description', 'Position projected into transverse plane of muon hit'))
    xy.GetListOfFunctions().Add(TNamed('Check', 'Octagonal pattern with layers'))
    xy.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    xy.GetListOfFunctions().Add(TNamed('MetaOptions', 'box, expert'))
    xy.Write()

    xz = TH2F('xz', 'x vs z for BKLMHit2ds', 140, -300.0, 400.0, 140, -350.0, 350.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().X():BKLMHit2ds.getGlobalPosition().Z()>>xz', '')
    xz.GetXaxis().SetTitle('z (cm)')
    xz.GetYaxis().SetTitle('x (cm)')
    xz.GetListOfFunctions().Add(TNamed('Description', 'Position projected into x-z plane of muon hit'))
    xz.GetListOfFunctions().Add(TNamed('Check', ' '))
    xz.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    xz.GetListOfFunctions().Add(TNamed('MetaOptions', 'box, expert'))
    xz.Write()

    yz = TH2F('yz', 'y vs z for BKLMHit2ds', 140, -300.0, 400.0, 140, -350.0, 350.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Y():BKLMHit2ds.getGlobalPosition().Z()>>yz', '')
    yz.GetXaxis().SetTitle('z (cm)')
    yz.GetYaxis().SetTitle('y (cm)')
    yz.GetListOfFunctions().Add(TNamed('Description', 'Position projected into y-z plane of muon hit'))
    yz.GetListOfFunctions().Add(TNamed('Check', ' '))
    yz.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    yz.GetListOfFunctions().Add(TNamed('MetaOptions', 'box, expert'))
    yz.Write()

####################################################################
# Entry point of this script: call the main() function             #
####################################################################
if __name__ == '__main__':
    main()
