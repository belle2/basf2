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
    <input>muon-KLMValidation.root</input>
    <contact>martina.laurenza@roma3.infn.it</contact>
    <description>Create validation plots for BKLM</description>
</header>
"""

import ROOT
ROOT.gROOT.SetBatch(ROOT.kTRUE)  # noqa

from ROOT import TFile, TChain, TH1F, TH2F, TNamed, gStyle, PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa

import sys
import argparse

# contact person information
# is added to the plot descriptions
CONTACT_PERSON = {'Name': 'Martina Laurenza',
                  'Email': 'martina.laurenza@roma3.infn.it'}


def main():
    """
    Create validation plots for BKLM.
    """
    print('Creating the BKLM validation plots...')

    parser = argparse.ArgumentParser()

    parser.add_argument('-i', '--input-file', dest='input_file',
                        default='../muon-KLMValidation.root',
                        help='Root file with Ext/Muid/BKLM/EKLM validation data.'
                        )
    parser.add_argument('-o', '--output-file', dest='output_file',
                        default='BKLMMuon.root',
                        help='Root file with BKLM validation histograms.')

    args = parser.parse_args()

    # load chain of input files
    file_chain = TChain('tree')
    file_chain.Add(args.input_file)

    number_entries = 0
    try:
        number_entries = file_chain.GetEntries()
    except AttributeError:
        print('Could not load input file(s) %s.' % args.input_file)

    if number_entries == 0:
        print('Data tree is empty or does not exist in file(s) %s. Exit.' % args.input_file)
        sys.exit(0)

    # open the output root file
    output_root_file = TFile(args.output_file, 'recreate')

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

    contact = 'Martina Laurenza (martina.laurenza@roma3.infn.it)'

    # Shifter plots

    inRPC = TH1F('InRPC', 'InRPC for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.inRPC()>>InRPC', '')
    inRPC.GetXaxis().SetTitle('0=scintillator  1=RPC')
    inRPC.GetListOfFunctions().Add(TNamed('Description', 'Flag indicating if a muon hit is in scintillator (0) or RPC (1)'))
    inRPC.GetListOfFunctions().Add(TNamed('Check', 'Mostly in RPC'))
    inRPC.GetListOfFunctions().Add(TNamed('Contact', contact))
    inRPC.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.50,pvalue-error=0.10'))
    inRPC.SetMinimum(0.0)
    inRPC.Write()

    section = TH1F('Forward', 'Section for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.getSection()>>Forward', '')
    section.GetXaxis().SetTitle('0=backward  1=forward')
    section.GetListOfFunctions().Add(TNamed('Description',
                                            'Flag indicating if a muon hit is in backward (0) or forward (1) BKLM'))
    section.GetListOfFunctions().Add(TNamed('Check', 'Somewhat more hits in the backward'))
    section.GetListOfFunctions().Add(TNamed('Contact', contact))
    section.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.50,pvalue-error=0.10'))
    section.SetMinimum(0.0)
    section.Write()

    isOnTrack = TH1F('IsOnTrack', 'IsOnTrack for BKLMHit2ds', 2, -0.5, 1.5)
    file_chain.Draw('BKLMHit2ds.isOnTrack()>>IsOnTrack', '')
    isOnTrack.GetXaxis().SetTitle('0=not associated with Track  1=associated with Track')
    isOnTrack.GetListOfFunctions().Add(TNamed('Description',
                                              'Flag indicating if a muon hit is associated with a CDC Track by Muid'))
    isOnTrack.GetListOfFunctions().Add(TNamed('Check', 'Mostly associated'))
    isOnTrack.GetListOfFunctions().Add(TNamed('Contact', contact))
    isOnTrack.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.50,pvalue-error=0.10'))
    isOnTrack.SetMinimum(0.0)
    isOnTrack.Write()

    sector = TH1F('Sector', 'Sector for BKLMHit2ds', 10, -0.5, 9.5)
    file_chain.Draw('BKLMHit2ds.getSector()>>Sector', '')
    sector.GetXaxis().SetTitle('Sector #')
    sector.GetListOfFunctions().Add(TNamed('Description', 'Sector number of muon hit'))
    sector.GetListOfFunctions().Add(TNamed('Check', 'Roughly flat in sectors 1-8'))
    sector.GetListOfFunctions().Add(TNamed('Contact', contact))
    sector.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=1.00,pvalue-error=0.01'))
    sector.SetMinimum(0.0)
    sector.Write()

    layer = TH1F('Layer', 'Layer for BKLMHit2ds', 16, -0.5, 15.5)
    file_chain.Draw('BKLMHit2ds.getLayer()>>Layer', '')
    layer.GetXaxis().SetTitle('Layer #')
    layer.GetListOfFunctions().Add(TNamed('Description', 'Layer number of muon hit'))
    layer.GetListOfFunctions().Add(TNamed('Check',
                                          'First peak at layer 1 and second (higher) peak at layer 2, with tail above those'))
    layer.GetListOfFunctions().Add(TNamed('Contact', contact))
    layer.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=1.0,pvalue-error=0.01'))
    layer.SetMinimum(0.0)
    layer.Write()

    phistrip = TH1F('PhiStrip', 'PhiStrip for BKLMHit2ds', 50, -0.5, 49.5)
    file_chain.Draw('BKLMHit2ds.getPhiStripAve()>>PhiStrip', '')
    phistrip.GetXaxis().SetTitle('Phi strip #')
    phistrip.GetListOfFunctions().Add(TNamed('Description', 'Strip number in phi plane of muon hit'))
    phistrip.GetListOfFunctions().Add(TNamed('Check',
                                             'Roughly flat for 1-36 (all layers) and then for 37-48 (layers 6-15)'))
    phistrip.GetListOfFunctions().Add(TNamed('Contact', contact))
    phistrip.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=1.00,pvalue-error=0.01'))
    phistrip.SetMinimum(0.0)
    phistrip.Write()

    zstrip = TH1F('ZStrip', 'ZStrip for BKLMHit2ds', 60, -0.5, 59.5)
    file_chain.Draw('BKLMHit2ds.getZStripAve()>>ZStrip', '')
    zstrip.GetXaxis().SetTitle('Z strip #')
    zstrip.GetListOfFunctions().Add(TNamed('Description', 'Strip number in z plane of muon hit'))
    zstrip.GetListOfFunctions().Add(TNamed('Check',
                                           'Downward-sloping for 1-48 (all layers), shoulder for 49-54 (layers 1-2)'))
    zstrip.GetListOfFunctions().Add(TNamed('Contact', contact))
    zstrip.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=1.00,pvalue-error=0.01'))
    zstrip.SetMinimum(0.0)
    zstrip.Write()

    RPC_bklmdigits_tres = ROOT.TH1F('RPC_bklmdigits_tres', 'KLM Digits time resolution in RPC', 250, -25, 25)
    file_chain.Draw('KLMDigits.getTime()-KLMDigits.getMCTime()>>RPC_bklmdigits_tres',
                    'KLMDigits.getSubdetector()==1 && KLMDigits.getLayer() > 3')
    RPC_bklmdigits_tres.SetXTitle('ns')
    RPC_bklmdigits_tres.GetListOfFunctions().Add(TNamed('Description', 'KLMDigits Time resolution in RPC'))
    RPC_bklmdigits_tres.GetListOfFunctions().Add(TNamed('Contact', contact))
    RPC_bklmdigits_tres.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.50,pvalue-error=0.10'))
    RPC_bklmdigits_tres.Write()

    timeRPC = TH1F('TimeRPC', 'Hit time for BKLMHit2ds in RPCs', 100, -2.0, 2.0)
    file_chain.Draw('BKLMHit2ds.getTime()>>TimeRPC', 'BKLMHit2ds.inRPC()==1')
    timeRPC.GetXaxis().SetTitle('t (ns)')
    timeRPC.GetListOfFunctions().Add(TNamed('Description', 'Time of muon hit in RPCs'))
    timeRPC.GetListOfFunctions().Add(TNamed('Check', 'Narrow peak at 0 ns'))
    timeRPC.GetListOfFunctions().Add(TNamed('Contact', contact))
    timeRPC.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=1.00,pvalue-error=0.01'))
    timeRPC.Write()

    Sci_bklmdigits_tres = ROOT.TH1F('Sci_bklmdigits_tres', 'KLM Digits time resolution in Scintillators', 250, -25, 25)
    file_chain.Draw('KLMDigits.getTime()-KLMDigits.getMCTime()>>Sci_bklmdigits_tres',
                    'KLMDigits.getSubdetector()==1 && KLMDigits.getLayer() <= 3')
    Sci_bklmdigits_tres.SetXTitle('ns')
    Sci_bklmdigits_tres.GetListOfFunctions().Add(TNamed('Description', 'KLMDigits Time resolution in Scintillators'))
    Sci_bklmdigits_tres.GetListOfFunctions().Add(TNamed('Contact', contact))
    Sci_bklmdigits_tres.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=0.50,pvalue-error=0.10'))
    Sci_bklmdigits_tres.Write()

    timeSci = TH1F('TimeSci', 'Hit time for BKLMHit2ds in scintillators', 100, -5.0, 15.0)
    file_chain.Draw('BKLMHit2ds.getTime()>>TimeSci', 'BKLMHit2ds.inRPC()==0')
    timeSci.GetXaxis().SetTitle('t (ns)')
    timeSci.GetListOfFunctions().Add(TNamed('Description', 'Time of muon hit in scintillators'))
    timeSci.GetListOfFunctions().Add(TNamed('Check',
                                            'Broad peak mainly between 2 ns and 8 ns, with the mean around 3.5 ns'))
    timeSci.GetListOfFunctions().Add(TNamed('Contact', contact))
    timeSci.GetListOfFunctions().Add(TNamed('MetaOptions', 'shifter,pvalue-warn=1.00,pvalue-error=0.01'))
    timeSci.Write()

    nPE = TH1F('nGenPE', 'Generated PE in BKLM', 50, 0.0, 100)
    file_chain.Draw('KLMDigits.getNGeneratedPhotoelectrons()>>nGenPE', 'KLMDigits.getSubdetector()==1 && KLMDigits.m_Layer < 3')
    nPE.GetXaxis().SetTitle('# generated PE')
    nPE.GetListOfFunctions().Add(TNamed('Description', 'Number of generated photoelectrons in BKLM'))
    nPE.GetListOfFunctions().Add(TNamed('Check', 'Mean around 46'))
    nPE.GetListOfFunctions().Add(TNamed('Contact', contact))
    nPE.GetListOfFunctions().Add(TNamed('MetaOptions', ''))
    nPE.Write()

    # Expert plots

    edep = TH1F('EnergyDeposit', 'Energy deposition for BKLMHit2ds', 50, 0.0, 25.0)
    file_chain.Draw('BKLMHit2ds.getEnergyDeposit()*1000.0>>EnergyDeposit', '')
    edep.GetXaxis().SetTitle('E (keV)')
    edep.GetListOfFunctions().Add(TNamed('Description', 'dE/dx energy deposition of muon hit'))
    edep.GetListOfFunctions().Add(TNamed('Check', 'Peak near 3 keV'))
    edep.GetListOfFunctions().Add(TNamed('Contact', contact))
    edep.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert,pvalue-warn=1.00,pvalue-error=0.01'))
    edep.Write()


# Entry point of this script: call the main() function
if __name__ == '__main__':
    main()
