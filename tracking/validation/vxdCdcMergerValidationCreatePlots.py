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
  <contact>software-tracking@belle2.org</contact>
  <input>VxdCdcValidationHarvested.root</input>
  <output>VxdCdcMergerValidation.root</output>
  <description>This module creates efficiency plots for the V0 validation.</description>
</header>
"""


import ROOT

ACTIVE = True


class VxdCdcMergerValidationPlots:
    """Create efficiency plots for the V0 validation"""

    def __init__(self, input_file='../VxdCdcValidationHarvested.root', output_file='VxdCdcMergerValidation.root'):
        """Constructor"""
        #: cached name of the input file
        self.input_file = input_file
        #: cached name of the output file
        self.output_file = output_file

        #: 1D histogram of merged hits
        self.hist_merged_hits = ROOT.TH1F("merged_hits", "Merged Hits", 70, 0, 140)
        #: Profile histogram of good merged hits by transverse momentum
        self.hist_good_over_pt = ROOT.TProfile("good_over_pt", "Good Merge over Pt", 50, 0, 4)
        #: Profile histogram of good merged hits by polar angle
        self.hist_good_over_theta = ROOT.TProfile("good_over_theta", "Good Merge over Theta", 50, 0, 4)

    def collect_histograms(self):
        """Fill the histograms in each event"""
        input_root_file = ROOT.TFile.Open(self.input_file, "READ")

        for event in input_root_file.VxdCdcMergerHarvester_tree:
            self.hist_merged_hits.Fill(event.PR_NHITS)
            self.hist_good_over_pt.Fill(event.MC_PT, event.GOOD_MERGE)
            self.hist_good_over_theta.Fill(event.MC_THETA, event.GOOD_MERGE)

        input_root_file.Close()
        return self

    @staticmethod
    def histogram_plot(hist, title, x_variable, x_unit=None, description='', check='', contact='', meta_options=''):
        """Label and annotate the histograms"""
        hist.SetName("".join(title.split()))
        xlabel = f'{x_variable} / ({x_unit})' if x_unit is not None else f'{x_variable}'
        ylabel = f'Entries / ({(hist.GetXaxis().GetXmax() - hist.GetXaxis().GetXmin()) / hist.GetNbinsX()} {x_unit})' \
            if x_unit is not None \
            else f'Entries / ({(hist.GetXaxis().GetXmax() - hist.GetXaxis().GetXmin()) / hist.GetNbinsX()})'
        hist.SetTitle(f"{title};{xlabel};{ylabel}")
        hist.GetListOfFunctions().Add(ROOT.TNamed('Description', description))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Check', check))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
        hist.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', meta_options))
        return hist

    def plot(self):
        """Draw all of the histograms to the output ROOT file"""
        output_root_file = ROOT.TFile.Open(self.output_file, "RECREATE")

        VxdCdcMergerValidationPlots.histogram_plot(self.hist_merged_hits, "Number of hits of merged tracks", "Number of Hits", None,
                                                   description='Number of hits of merged tracks',
                                                   check='',
                                                   contact='software-tracking@belle2.org',
                                                   meta_options='').Write()

        VxdCdcMergerValidationPlots.histogram_plot(self.hist_good_over_pt, "Good Merge over Pt", "MC Track Pt (GeV)", None,
                                                   description='Good Merge over Pt',
                                                   check='',
                                                   contact='software-tracking@belle2.org',
                                                   meta_options='').Write()

        VxdCdcMergerValidationPlots.histogram_plot(self.hist_good_over_theta, "Good Merge over Theta", "MC Track Theta (1)", None,
                                                   description='Good Merge over Theta',
                                                   check='',
                                                   contact='software-tracking@belle2.org',
                                                   meta_options='').Write()

        output_root_file.Write()
        output_root_file.Close()
        return self


if __name__ == '__main__':
    if ACTIVE:
        VxdCdcMergerValidationPlots().collect_histograms().plot()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
