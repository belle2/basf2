#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>giacomo.pietro@kit.edu</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>BasicSimulationAndReconstruction.root</output>
  <description>
  This validation script checks the basic of our detector simulation (digits)
  and reconstruction (clusters etc) in a simulation without background
  </description>
</header>
"""

from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed
import basf2
from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction

NAME = 'Basic detector simulation and reconstruction'
CONTACT = 'giacomo.pietro@kit.edu'
INPUT_FILE = '../EvtGenSimNoBkg.root'
OUTPUT_FILE = 'BasicSimulationAndReconstruction.root'

# If False, the script will be called but no events will be processed
ACTIVE = True


def run():
    """
    Check the number of hits that are produced by add_simulation and the basic reconstruction in SVD and CDC.
    """
    class BasicSimulationAndReconstructionValidation(basf2.Module):
        """
        Module to collect information about the number of
        * PXDDigits
        * PXDClusters
        * PXDSpacePoints
        * SVDShaperDigits
        * SVDClusters
        * SVDSpacePoints
        * CDCHits
        * CDCHits above threshold
        * TOPDigits
        * ARICHDigits
        * ECLDigits
        * KLMDigits
        """

        def __init__(
            self,
            name='',
            contact='',
            output_file_name=None,
        ):
            """Constructor"""

            super().__init__()
            #: name of this validation output
            self.validation_name = NAME
            #: contact person
            self.contact = CONTACT
            #: #: name of the output ROOT file
            self.output_file_name = OUTPUT_FILE

        def initialize(self):
            ''' Initialize the Module: book histograms and set descriptions and checks'''

            self.hists = []
            self.hists.append(TH1F('PXDDigits', 'PXDDigits (no data reduction)',
                                   100, 0, 100))
            self.hists.append(TH1F('PXDClusters', 'PXDClusters (no data reduction)',
                                   50, 0, 50))
            self.hists.append(TH1F('PXDSpacePoints', 'PXDSpacePoints (no data reduction)',
                                   50, 0, 50))
            self.hists.append(TH1F('SVDShaperDigits', 'SVDShaperDigits', 200, 0, 800))
            self.hists.append(TH1F('SVDClusters', 'SVDClusters', 200, 0, 400))
            self.hists.append(TH1F('SVDSpacePoints', 'SVDSpacePoints', 200, 0, 600))
            self.hists.append(TH1F('CDCHits', 'CDCHits (all)', 200, 0, 2000))
            self.hists.append(TH1F('CDCHitsAboveThreshold', 'CDCHits (above threshold)', 200, 0, 2000))
            self.hists.append(TH1F('TOPDigits', 'TOPDigits', 200, 0, 800))
            self.hists.append(TH1F('ARICHDigits', 'ARICHDigits', 200, 0, 200))
            self.hists.append(TH1F('ECLDigits', 'ECLDigits, m_Amp > 500 (roughly 25 MeV)',
                                   100, 0, 100))
            self.hists.append(TH1F('KLMDigits', 'KLMDigits', 200, 0, 200))

            for h in self.hists:
                h.SetXTitle(f'Number of {h.GetName()} in event')
                option = 'shifter'
                descr = TNamed('Description', 'Number of ' + h.GetName() +
                               ' per event (no BG overlay, just result of add_simulation and basic reconstruction)')

                h.SetYTitle('entries per bin')
                h.GetListOfFunctions().Add(descr)
                check = TNamed('Check', 'Distribution must agree with its reference')
                h.GetListOfFunctions().Add(check)
                contact = TNamed('Contact', self.contact)
                h.GetListOfFunctions().Add(contact)
                options = TNamed('MetaOptions', option)
                h.GetListOfFunctions().Add(options)

            #: number of L3 strips (u+v)
            self.nSVDL3 = 768 * 7 * 2 * 2

        def event(self):
            ''' Event processor: fill histograms '''

            for h in self.hists:
                digits = Belle2.PyStoreArray(h.GetName())
                if h.GetName() == 'ECLDigits':
                    n = 0
                    for digit in digits:
                        if digit.getAmp() > 500:  # roughly 25 MeV
                            n += 1
                    h.Fill(n)
                elif h.GetName() == 'CDCHitsAboveThreshold':
                    pass
                else:
                    h.Fill(digits.getEntries())

            # CDC
            cdcHits = Belle2.PyStoreArray('CDCHits')
            nCDC_above_threshold = 0
            for cdcHit in cdcHits:  # count wires of inner/outer layers
                if cdcHit.getISuperLayer() == 0 and cdcHit.getADCCount() > 15:
                    nCDC_above_threshold += 1
                if cdcHit.getISuperLayer() != 0 and cdcHit.getADCCount() > 18:
                    nCDC_above_threshold += 1
            for h in self.hists:
                if h.GetName() == 'CDCHitsAboveThreshold':
                    h.Fill(nCDC_above_threshold)

        def terminate(self):
            """ Write histograms to file."""

            tfile = TFile(self.output_file_name, 'recreate')
            for h in self.hists:
                h.Write()
            tfile.Close()

    """ Steering """
    basf2.set_random_seed(1509)

    path = basf2.create_path()

    path.add_module('RootInput', inputFileName=INPUT_FILE)
    path.add_module('Gearbox')
    path.add_module('Geometry')
    add_svd_reconstruction(path, isROIsimulation=False)
    add_pxd_reconstruction(path)

    path.add_module(BasicSimulationAndReconstructionValidation())

    path.add_module('Progress')

    print(path)
    basf2.process(path)


if __name__ == '__main__':
    if ACTIVE:
        run()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
