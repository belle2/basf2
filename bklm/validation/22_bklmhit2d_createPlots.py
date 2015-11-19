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
from ROOT import TFile, TChain, TTree, TH1F, TH2F, TCanvas, TGraphErrors, TGraph, \
    gStyle, TNamed, TF1, TProfile
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

    inRPC = TH1F('InRPC', 'InRPC for BKLMHit2ds', 2, -0.5, 1.5)
    inRPC.GetXaxis().SetTitle('0=scint  1=RPC')
    inRPC.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.inRPC()>>InRPC', '')
    inRPC.Write()

    isForward = TH1F('IsForward', 'IsForward for BKLMHit2ds', 2, -0.5, 1.5)
    isForward.GetXaxis().SetTitle('0=backward  1=forward')
    isForward.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.isForward()>>IsForward', '')
    isForward.Write()

    isOnTrack = TH1F('IsOnTrack', 'IsOnTrack for BKLMHit2ds', 2, -0.5, 1.5)
    isOnTrack.GetXaxis().SetTitle('0=not associated with track  1=associated with track')
    isOnTrack.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.isOnTrack()>>IsOnTrack', '')
    isOnTrack.Write()

    sector = TH1F('Sector', 'Sector for BKLMHit2ds', 10, -0.5, 9.5)
    sector.GetXaxis().SetTitle('Sector #')
    sector.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.getSector()>>Sector', '')
    sector.Write()

    layer = TH1F('Layer', 'Layer for BKLMHit2ds', 16, -0.5, 15.5)
    layer.GetXaxis().SetTitle('Layer #')
    layer.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.getLayer()>>Layer', '')
    layer.Write()

    phistrip = TH1F('PhiStrip', 'PhiStrip for BKLMHit2ds', 50, -0.5, 49.5)
    phistrip.GetXaxis().SetTitle('Phi strip #')
    phistrip.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.getPhiStripAve()>>PhiStrip', '')
    phistrip.Write()

    zstrip = TH1F('ZStrip', 'ZStrip for BKLMHit2ds', 50, -0.5, 49.5)
    zstrip.GetXaxis().SetTitle('Z strip #')
    zstrip.SetMinimum(0.0)
    file_chain.Draw('BKLMHit2ds.getZStripAve()>>ZStrip', '')
    zstrip.Write()

    time = TH1F('Time', 'Hit time for BKLMHit2ds', 100, 0.0, 25.0)
    time.GetXaxis().SetTitle('t (ns)')
    file_chain.Draw('BKLMHit2ds.getTime()>>Time', '')
    time.Write()

    edep = TH1F('EDep', 'Energy deposition for BKLMHit2ds', 100, 0.0, 25.0)
    edep.GetXaxis().SetTitle('E (keV)')
    file_chain.Draw('BKLMHit2ds.getEDep()*1000.0>>EDep', '')
    edep.Write()

    xy = TH2F('xy', 'y vs x for BKLMHit2ds', 140, -350.0, 350.0, 140, -350.0, 350.0)
    xy.GetXaxis().SetTitle('x (cm)')
    xy.GetYaxis().SetTitle('y (cm)')
    xy.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Y():BKLMHit2ds.getGlobalPosition().X()>>xy', '')
    xy.Write()

    xz = TH2F('xz', 'x vs z for BKLMHit2ds', 140, -300.0, 400.0, 140, -350.0, 350.0)
    xz.GetXaxis().SetTitle('z (cm)')
    xz.GetYaxis().SetTitle('x (cm)')
    xz.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().X():BKLMHit2ds.getGlobalPosition().Z()>>xz', '')
    xz.Write()

    yz = TH2F('yz', 'y vs z for BKLMHit2ds', 140, -300.0, 400.0, 140, -350.0, 350.0)
    yz.GetXaxis().SetTitle('z (cm)')
    yz.GetYaxis().SetTitle('y (cm)')
    yz.GetListOfFunctions().Add(TNamed('MetaOptions', 'box'))
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Y():BKLMHit2ds.getGlobalPosition().Z()>>yz', '')
    yz.Write()

####################################################################
# Entry point of this script: call the main() function             #
####################################################################
if __name__ == '__main__':
    main()
