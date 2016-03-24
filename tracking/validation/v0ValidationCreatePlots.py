#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <input>V0ValidationHarvested.root</input>
  <description>This module creates efficiency plots for the V0 validation.</description>
</header>
"""


import numpy
import ROOT


class V0ValidationPlots:

    def __init__(self, input_file='../V0ValidationHarvested.root', output_file='V0Validation.root'):
        self.input_file = input_file
        self.output_file = output_file

        self.hist_r = ROOT.TH1F("", "True R", 20, 0, 20)
        self.hist_theta = ROOT.TH1F("", "True Theta", 26, 20, 150)
        self.hist_phi = ROOT.TH1F("", "True Phi", 36, -180, 180)
        self.hist_p = ROOT.TH1F("", "True P", 25, 0.0, 1.0)

        self.hist_r_found = ROOT.TH1F("", "Found R", 20, 0, 20)
        self.hist_theta_found = ROOT.TH1F("", "Found Theta", 26, 20, 150)
        self.hist_phi_found = ROOT.TH1F("", "Found Phi", 36, -180, 180)
        self.hist_p_found = ROOT.TH1F("", "Found P", 25, 0.0, 1.0)

        self.hist_invariant_mass = ROOT.TH1F("", "", 60, 0.470, 0.530)
        self.hist_invariant_mass_res = ROOT.TH1F("", "", 40, -0.02, 0.02)

        self.hist_chi2 = ROOT.TH1F("", "", 50, 0, 50)
        self.hist_chi2_inside = ROOT.TH1F("", "", 50, 0, 50)
        self.hist_chi2_outside = ROOT.TH1F("", "", 50, 0, 50)

        self.hist_mass_vs_mc_mass = ROOT.TH2F("", "", 80, 0, 0.8, 80, 0, 0.8)

        # Residuum histograms
        self.hist_invariant_mass_residuum = ROOT.TH1F("", "", 60, -0.05, 0.05)
        self.hist_r_residuum = ROOT.TH1F("", "", 60, -0.1, 0.1)
        self.hist_theta_residuum = ROOT.TH1F("", "", 60, -0.1, 0.1)
        self.hist_phi_residuum = ROOT.TH1F("", "", 60, -0.1, 0.1)
        self.hist_p_residuum = ROOT.TH1F("", "", 60, -0.05, 0.05)

    def collect_histograms(self):
        input_root_file = ROOT.TFile.Open(self.input_file, "READ")

        for event in input_root_file.V0Harvester_tree:
            self.hist_r.Fill(event.R_MC)
            self.hist_theta.Fill(numpy.rad2deg(event.THETA_MC))
            self.hist_phi.Fill(numpy.rad2deg(event.PHI_MC))
            self.hist_p.Fill(event.P_MC)

            if event.FOUND:
                self.hist_r_found.Fill(event.R_MC)
                self.hist_theta_found.Fill(numpy.rad2deg(event.THETA_MC))
                self.hist_phi_found.Fill(numpy.rad2deg(event.PHI_MC))
                self.hist_p_found.Fill(event.P_MC)

                self.hist_invariant_mass.Fill(event.M)

                # Residuum histograms
                self.hist_invariant_mass_residuum.Fill(event.M - event.M_MC)
                self.hist_r_residuum.Fill(event.R - event.R_MC)
                self.hist_theta_residuum.Fill(event.THETA - event.THETA_MC)
                self.hist_phi_residuum.Fill(event.PHI - event.PHI_MC)
                self.hist_p_residuum.Fill(event.P - event.P_MC)

                self.hist_chi2.Fill(event.CHI2)
                if event.R_MC > 1.0:
                    self.hist_chi2_outside.Fill(event.CHI2)
                else:
                    assert event.R_MC <= 1.0
                    self.hist_chi2_inside.Fill(event.CHI2)

                self.hist_mass_vs_mc_mass.Fill(event.M, event.M_MC)

        input_root_file.Close()
        return self

    @staticmethod
    def efficiency_plot(found, all, title, x_variable, x_unit, description='', check='', contact='', meta_options=''):
        efficiency = ROOT.TEfficiency(found, all)
        efficiency.SetName("".join(title.split()))
        ylabel = 'Efficiency / ({} {})'.format((found.GetXaxis().GetXmax() -
                                                found.GetXaxis().GetXmin()) / found.GetNbinsX(), x_unit)
        efficiency.SetTitle("{};{} / ({});{}".format(title, x_variable, x_unit, ylabel))
        efficiency.GetListOfFunctions().Add(ROOT.TNamed('Description', description))
        efficiency.GetListOfFunctions().Add(ROOT.TNamed('Check', check))
        efficiency.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
        efficiency.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', meta_options))
        return efficiency

    @staticmethod
    def histogram_plot(hist, title, x_variable, x_unit=None, description='', check='', contact='', meta_options=''):
        hist.SetName("".join(title.split()))
        xlabel = '{} / ({})'.format(x_variable, x_unit) if x_unit is not None else '{}'.format(x_variable)
        ylabel = 'Entries / ({} {})'.format((hist.GetXaxis().GetXmax() -
                                             hist.GetXaxis().GetXmin()) /
                                            hist.GetNbinsX(), x_unit) if x_unit is not None \
            else 'Entries / ({})'.format((hist.GetXaxis().GetXmax() - hist.GetXaxis().GetXmin()) / hist.GetNbinsX())
        hist.SetTitle("{};{};{}".format(title, xlabel, ylabel))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Description', description))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Check', check))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
        hist.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', meta_options))
        return hist

    @staticmethod
    def histogram_2d_plot(hist, title, x_variable, y_variable, x_unit=None, y_unit=None,
                          description='', check='', contact='', meta_options=''):
        hist.SetName("".join(title.split()))
        xlabel = '{} / ({})'.format(x_variable, x_unit) if x_unit is not None else '{}'.format(x_variable)
        ylabel = '{} / ({})'.format(y_variable, y_unit) if y_unit is not None else '{}'.format(y_variable)
        hist.SetTitle("{};{};{}".format(title, xlabel, ylabel))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Description', description))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Check', check))
        hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
        hist.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', meta_options))
        return hist

    def plot(self):
        output_root_file = ROOT.TFile.Open(self.output_file, "RECREATE")

        V0ValidationPlots.efficiency_plot(self.hist_r_found, self.hist_r, 'Efficiency vs R', 'r', 'cm',
                                          description='Reconstruction Efficiency vs. r (perpendicular)',
                                          check='',
                                          contact='markus.prim@kit.edu',
                                          meta_options='').Write()

        V0ValidationPlots.efficiency_plot(self.hist_theta_found, self.hist_theta, 'Efficiency vs Theta', 'Theta', 'deg',
                                          description='Reconstruction Efficiency vs. theta',
                                          check='',
                                          contact='markus.prim@kit.edu',
                                          meta_options='').Write()

        V0ValidationPlots.efficiency_plot(self.hist_phi_found, self.hist_phi, 'Efficiency vs Phi', 'Phi', 'deg',
                                          description='Reconstruction Efficiency vs phi',
                                          check='',
                                          contact='markus.prim@kit.edu',
                                          meta_options='').Write()

        V0ValidationPlots.efficiency_plot(self.hist_p_found, self.hist_p, 'Efficiency vs P', 'P', 'GeV',
                                          description='Reconstruction Efficiency vs momentum',
                                          check='',
                                          contact='markus.prim@kit.edu',
                                          meta_options='').Write()

        V0ValidationPlots.histogram_plot(self.hist_invariant_mass, "KShort Invariant Mass", "m", "GeV",
                                         description='Reconstructed invariant mass of KShorts with standard reconstruction',
                                         check='Invariant mass peak around KShort nominal mass 497.61 MeV.',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()

        V0ValidationPlots.histogram_plot(self.hist_invariant_mass_residuum, "KShort Invariant Mass Residuum", "Rec - MC", "GeV",
                                         description='Invariant mass residuum of KShorts with standard reconstruction',
                                         check='',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()

        V0ValidationPlots.histogram_plot(self.hist_chi2, "Chi2 of Vertex Fits.", "Chi2", None,
                                         description='Chi2 distributions of the vertex fits.',
                                         check='Check if distribution looks like a Chi2 distribution with 1 dof',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()

        V0ValidationPlots.histogram_plot(self.hist_chi2_inside, "Chi2 of Vertex Fits Inside Beampipe.", "Chi2", None,
                                         description='Chi2 distributions of the vertex fits inside the beampipe.',
                                         check='Check if distribution looks like a Chi2 distribution with 1 dof',
                                         contact='markus.prim@kit.edu',
                                         meta_options='expert').Write()

        V0ValidationPlots.histogram_plot(self.hist_chi2_outside, "Chi2 of Vertex Fits Outside Beampipe.", "Chi2", None,
                                         description='Chi2 distributions of the vertex fits outside the beampipe.',
                                         check='Check if distribution looks like a Chi2 distribution with 1 dof',
                                         contact='markus.prim@kit.edu',
                                         meta_options='expert').Write()

        V0ValidationPlots.histogram_2d_plot(self.hist_mass_vs_mc_mass, "Reconstructed vs MC Mass.",
                                            "Reconstructed Mass", "GeV", "MC Mass", "GeV",
                                            description="Reconstructed mass vs invariant Mass.",
                                            check="",
                                            contact="markus.prim@kit.edu",
                                            meta_options='expert').Write()

        V0ValidationPlots.histogram_plot(self.hist_r_residuum, "KShort R Residuum", "Rec - MC", "cm",
                                         description='R residuum of KShorts with standard reconstruction',
                                         check='',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()
        V0ValidationPlots.histogram_plot(self.hist_theta_residuum, "KShort Theta Residuum", "Rec - MC", "rad",
                                         description='Theta residuum of KShorts with standard reconstruction',
                                         check='',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()
        V0ValidationPlots.histogram_plot(self.hist_phi_residuum, "KShort Phi Residuum", "Rec - MC", "rad",
                                         description='Phi residuum of KShorts with standard reconstruction',
                                         check='',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()
        V0ValidationPlots.histogram_plot(self.hist_p_residuum, "KShort Momentum Residuum", "Rec - MC", "GeV",
                                         description='Momentum residuum of KShorts with standard reconstruction',
                                         check='',
                                         contact='markus.prim@kit.edu',
                                         meta_options='').Write()

        output_root_file.Write()
        output_root_file.Close()
        return self


if __name__ == '__main__':
    V0ValidationPlots().collect_histograms().plot()
