#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#     Create several bklm validation plots                      #
#     and store them in a root file                             #
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
ROOT.gROOT.SetBatch(ROOT.kTRUE)

from ROOT import TFile, TChain, TH1F, TH2F, TNamed, gStyle, PyConfig
PyConfig.IgnoreCommandLineOptions = True

import sys
from optparse import OptionParser

# contact person information
# is added to the plot descriptions
CONTACT_PERSON = {'Name': 'Leo Piilonen',
                  'Email': 'piilonen@vt.edu'}


def main():
    """
    Create validation plots for BKLM.
    """

    print('Creating the BKLM validation plots...')

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

    print('BKLM validation plots created.')


# NOTE: *.Draw() must precede *.GetListOfFunctions().Add() or the latter will be discarded!
def draw_bklmhists(file_chain):
    """
    Draw the BKLMHit2d-related distributions.
    """

    # Shifter plots

    inRPC = TH1F('InRPC', 'InRPC for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.inRPC()>>InRPC', '')
    inRPC.GetXaxis().SetTitle('0=scintillator  1=RPC')
    inRPC.GetListOfFunctions().Add(TNamed('Description', 'Flag indicating if a muon hit is in scintillator (0) or RPC (1)'))
    inRPC.GetListOfFunctions().Add(TNamed('Check', 'Mostly in RPC'))
    inRPC.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    inRPC.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    inRPC.SetMinimum(0.0)
    inRPC.Write()

    section = TH1F('Forward', 'Section for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.getSection()>>Forward', '')
    section.GetXaxis().SetTitle('0=backward  1=forward')
    section.GetListOfFunctions().Add(TNamed('Description',
                                            'Flag indicating if a muon hit is in backward (0) or forward (1) BKLM'))
    section.GetListOfFunctions().Add(TNamed('Check', 'Somewhat more hits in the backward'))
    section.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    section.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    section.SetMinimum(0.0)
    section.Write()

    isOnTrack = TH1F('IsOnTrack', 'IsOnTrack for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.isOnTrack()>>IsOnTrack', '')
    isOnTrack.GetXaxis().SetTitle('0=not associated with Track  1=associated with Track')
    isOnTrack.GetListOfFunctions().Add(TNamed('Description',
                                              'Flag indicating if a muon hit is associated with a CDC Track by Muid'))
    isOnTrack.GetListOfFunctions().Add(TNamed('Check', 'Mostly associated'))
    isOnTrack.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    isOnTrack.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.50,pvalue-error=0.10'))
    isOnTrack.SetMinimum(0.0)
    isOnTrack.Write()

    sector = TH1F('Sector', 'Sector for BKLMHit2ds', 10, -0.5, 9.5)
    file_chain.Draw('BKLMHit2ds.getSector()>>Sector', '')
    sector.GetXaxis().SetTitle('Sector #')
    sector.GetListOfFunctions().Add(TNamed('Description', 'Sector number of muon hit'))
    sector.GetListOfFunctions().Add(TNamed('Check', 'Roughly flat in sectors 1-8'))
    sector.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    sector.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    sector.SetMinimum(0.0)
    sector.Write()

    layer = TH1F('Layer', 'Layer for BKLMHit2ds', 16, -0.5, 15.5)
    file_chain.Draw('BKLMHit2ds.getLayer()>>Layer', '')
    layer.GetXaxis().SetTitle('Layer #')
    layer.GetListOfFunctions().Add(TNamed('Description', 'Layer number of muon hit'))
    layer.GetListOfFunctions().Add(TNamed('Check',
                                          'First peak at layer 1 and second (higher) peak at layer 3, with tail above those'))
    layer.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    layer.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    layer.SetMinimum(0.0)
    layer.Write()

    phistrip = TH1F('PhiStrip', 'PhiStrip for BKLMHit2ds', 50, -0.5, 49.5)
    file_chain.Draw('BKLMHit2ds.getPhiStripAve()>>PhiStrip', '')
    phistrip.GetXaxis().SetTitle('Phi strip #')
    phistrip.GetListOfFunctions().Add(TNamed('Description', 'Strip number in phi plane of muon hit'))
    phistrip.GetListOfFunctions().Add(TNamed('Check',
                                             'Roughly flat for 1-36 (all layers) and then for 37-48 (layers 6-15)'))
    phistrip.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    phistrip.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    phistrip.SetMinimum(0.0)
    phistrip.Write()

    zstrip = TH1F('ZStrip', 'ZStrip for BKLMHit2ds', 60, -0.5, 59.5)
    file_chain.Draw('BKLMHit2ds.getZStripAve()>>ZStrip', '')
    zstrip.GetXaxis().SetTitle('Z strip #')
    zstrip.GetListOfFunctions().Add(TNamed('Description', 'Strip number in z plane of muon hit'))
    zstrip.GetListOfFunctions().Add(TNamed('Check',
                                           'Downward-sloping for 1-48 (all layers), shoulder for 49-54 (layers 1-2)'))
    zstrip.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    zstrip.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    zstrip.SetMinimum(0.0)
    zstrip.Write()

    timeRPC = TH1F('TimeRPC', 'Hit time for BKLMHit2ds in RPCs', 200, -5.0, 5.0)
    file_chain.Draw('BKLMHit2ds.getTime()>>TimeRPC', 'BKLMHit2ds.inRPC()==1')
    timeRPC.GetXaxis().SetTitle('t (ns)')
    timeRPC.GetListOfFunctions().Add(TNamed('Description', 'Time of muon hit in RPCs'))
    timeRPC.GetListOfFunctions().Add(TNamed('Check', 'Narrow peak at 0 ns'))
    timeRPC.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    timeRPC.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    timeRPC.Write()

    timeSci = TH1F('TimeSci', 'Hit time for BKLMHit2ds in scintillators', 100, -5.0, 15.0)
    file_chain.Draw('BKLMHit2ds.getTime()>>TimeSci', 'BKLMHit2ds.inRPC()==0')
    timeSci.GetXaxis().SetTitle('t (ns)')
    timeSci.GetListOfFunctions().Add(TNamed('Description', 'Time of muon hit in scintillators'))
    timeSci.GetListOfFunctions().Add(TNamed('Check',
                                            'Broad peak mainly between 2 ns and 8 ns, with the mean around 3.5 ns'))
    timeSci.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    timeSci.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    timeSci.Write()

    r = TH1F('r', 'r for BKLMHit2ds', 30, 200.0, 350.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Perp()>>r', '')
    r.GetXaxis().SetTitle('r (cm)')
    r.GetListOfFunctions().Add(TNamed('Description', 'Radial position of muon hit'))
    r.GetListOfFunctions().Add(TNamed('Check', 'Comb-like downward-sloping distribution (a la layers)'))
    r.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    r.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    r.Write()

    z = TH1F('z', 'z for BKLMHit2ds', 100, -200.0, 300.0)
    file_chain.Draw('BKLMHit2ds.getGlobalPosition().Z()>>z', '')
    z.GetXaxis().SetTitle('z (cm)')
    z.GetListOfFunctions().Add(TNamed('Description', 'Axial position of muon hit'))
    z.GetListOfFunctions().Add(TNamed('Check', 'Broad peak near zero with dip at 47 cm (forward-backward boundary)'))
    z.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    z.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.99,pvalue-error=0.90'))
    z.Write()

    # Expert plots

    edep = TH1F('EDep', 'Energy deposition for BKLMHit2ds', 50, 0.0, 25.0)
    file_chain.Draw('BKLMHit2ds.getEDep()*1000.0>>EDep', '')
    edep.GetXaxis().SetTitle('E (keV)')
    edep.GetListOfFunctions().Add(TNamed('Description', 'dE/dx energy deposition of muon hit'))
    edep.GetListOfFunctions().Add(TNamed('Check', 'Peak near 3 keV'))
    edep.GetListOfFunctions().Add(TNamed('Contact', 'piilonen@vt.edu'))
    edep.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=0.99,pvalue-error=0.90'))
    edep.Write()

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


# Entry point of this script: call the main() function
if __name__ == '__main__':
    main()
