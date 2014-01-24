#!/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#     script creates efficiency vs pt plots and stores them     #
#     in a root file                                            #
#                                                               #
#    written by Michael Ziegler, KIT                            #
#    michael.ziegler2@kit.edu                                   #
#                                                               #
#################################################################

from __future__ import division
from ROOT import TFile, TTree, TH1F, TCanvas, TGraphErrors, TGraph, gStyle, \
    TNamed
import sys
import math

DELTA_PT = 0.0001


def main():
    '''Function which is executed'''

    print 'Tracking validation plots.'

    gStyle.SetOptStat(0)

    # load data file
    input_file_name = '../trackingEfficiency_FinalData.root'
    print 'Load file %s' % input_file_name
    input_root_file = TFile(input_file_name, 'read')
    # raise exception if file could not be opened
    if input_root_file.IsZombie():
        raise OSError('File %s could not be opened.' % input_file_name)

    # load data tree
    tree_name = 'data'
    data_tree = input_root_file.Get(tree_name)

    number_entries = 0
    try:
        number_entries = data_tree.GetEntries()
    except AttributeError:
        print 'Could not load tree with name %s.' % tree_name

    if number_entries == 0:
        print 'Data tree \'%s\'is empty or does not exist. Exit.' % tree_name
        sys.exit()

    # output root file
    output_file_name = 'TrackingValidation.root'
    output_root_file = TFile(output_file_name, 'recreate')

    # create efficiency in bins of pt plot
    calculate_efficiency_in_pt(data_tree)

    pt_values = [  # 0.05,
                   # 0.1,
                   # 0.15,
        0.25,
        0.5,
        0.75,
        1.0,
        1.5,
        2.0,
        2.5,
        3.0,
        ]

    for pt_value in pt_values:
        # create plots of efficiency in bins of cos Theta for different pt
        generate_cos_theta_plot(data_tree, pt_value)

    # create momentum resolution plot
    create_momentum_resolution_plot(data_tree, pt_values)

    # close output file
    output_root_file.Close()


def calculate_efficiency_in_pt(data_tree):
    '''Calculate single track reconstruction efficiency in bins of pt'''

    number_bins = 62
    pt_lower = -0.025
    pt_upper = 3.075

    hist_pt_gen = TH1F('hPtGen', 'hPtGen', number_bins, pt_lower, pt_upper)
    hist_pt_rec = TH1F('hPtRec', 'hPtRec', number_bins, pt_lower, pt_upper)

    # draw data in defined histos
    data_tree.Draw('pt_gen>>hPtGen', '', 'goff')
    data_tree.Draw('pt_gen>>hPtRec', 'pt != -999', 'goff')

    efficiencies = []
    eff_errors = []

    # final hist
    efficiency_hist = TH1F('hEfficiency', 'hEfficiency', number_bins,
                           pt_lower, pt_upper)

    description = \
        'Events with 10 muon tracks with fixed transverse momentum are generated using the ParticleGun (500 events for each pt value). The events are reconstructed with VXDTF+Trasan+MCTrackCandCombiner. This plot shows the single track reconstruction efficiency over the transverse momentum.'
    check = 'The efficiency should be stable for higher pt values.'
    efficiency_hist.GetListOfFunctions().Add(TNamed('Description',
            description))
    efficiency_hist.GetListOfFunctions().Add(TNamed('Check', check))
    # loop over bins and calculate efficiency and error of efficiency
    for ibin in range(1, number_bins + 1):
        number_generated = hist_pt_gen.GetBinContent(ibin)
        efficiency = 0
        error = 0

        if number_generated > 0:
            number_reconstructed = hist_pt_rec.GetBinContent(ibin)
            efficiency = number_reconstructed / number_generated
            error = math.sqrt(number_reconstructed * (number_generated
                              - number_reconstructed) / pow(number_generated,
                              3))
        # set according bin to the efficiency value and add errorbar
        efficiency_hist.SetBinContent(ibin, efficiency)
        efficiency_hist.SetBinError(ibin, error)

    efficiency_hist.SetTitle('Tracking efficiency in bins of transverse momentum pt.'
                             )
    efficiency_hist.SetXTitle('pt in GeV')
    efficiency_hist.SetYTitle('efficiency')

    # write hist to the output file
    efficiency_hist.Write()


def generate_cos_theta_plot(data_tree, pt_value):
    '''Creates a efficiency histo in bins of cos theta'''

    number_bins = 100
    cos_lower = -1
    cos_upper = 1

    hist_cos_gen = TH1F('hCosGen', 'hCosGen', number_bins, cos_lower,
                        cos_upper)
    hist_cos_rec = TH1F('hCosRec', 'hCosRec', number_bins, cos_lower,
                        cos_upper)

    data_tree.Draw('cosTheta_gen>>hCosGen',
                   'pt_gen>(%.2f - %f) &&pt_gen<(%.2f + %f)' % (pt_value,
                   DELTA_PT, pt_value, DELTA_PT), 'goff')
    data_tree.Draw('cosTheta_gen>>hCosRec',
                   'pt_gen>(%.2f - %f) &&pt_gen<(%.2f + %f) && pt != -999'
                   % (pt_value, DELTA_PT, pt_value, DELTA_PT), 'goff')

    description = \
        'Events with 10 muon tracks with fixed transverse momentum are generated using the ParticleGun (500 events for each pt value). The events are reconstructed with VXDTF+Trasan+MCTrackCandCombiner. The plot shows the single track reconstruction efficiency in bins of the polar angle for the fixed transverse momentum pt = %.2f GeV.' \
        % pt_value
    check = 'Stable efficiency over the hole range of the polar angle.'

    efficiency_hist = TH1F('hEfficiencyPt%.2fGeV' % pt_value,
                           'hEfficiencyPt%.2fGeV' % pt_value, number_bins,
                           cos_lower, cos_upper)
    efficiency_hist.GetListOfFunctions().Add(TNamed('Description',
            description))
    efficiency_hist.GetListOfFunctions().Add(TNamed('Check', check))
    for ibin in range(1, number_bins + 1):
        efficiency = 0
        error = 0
        number_generated = hist_cos_gen.GetBinContent(ibin)

        if number_generated > 0:
            number_reconstructed = hist_cos_rec.GetBinContent(ibin)
            efficiency = number_reconstructed / number_generated
            error = math.sqrt(number_reconstructed * (number_generated
                              - number_reconstructed) / pow(number_generated,
                              3))

        efficiency_hist.SetBinContent(ibin, efficiency)
        efficiency_hist.SetBinError(ibin, error)

    efficiency_hist.SetTitle('Tracks with pt = %.2f GeV' % pt_value)
    efficiency_hist.SetXTitle('cos #Theta')
    efficiency_hist.SetYTitle('efficiency')
    efficiency_hist.Write()


def calculate_momentum_resolution(data_tree, pt_value_condition):
    '''
    Create histogram of the relative resolution of transverse momentum in
    bins of transverse momentum
    '''

    number_entries = data_tree.GetEntries()

    pt_values = []
    pt_values_gen = []
    residuums = []

    nbins = 200
    pt_lower = -0.1
    pt_upper = 0.1

    hist_residuum = TH1F('hResiduumPt%.2f' % pt_value_condition,
                         'hResiduumPt%.2f' % pt_value_condition, nbins,
                         pt_lower, pt_upper)
    hist_residuum.SetTitle('Momentum residuals for pt = %.2f'
                           % pt_value_condition)
    hist_residuum.SetXTitle('(pt_{rec} - pt_{gen}) in GeV')
    hist_residuum.SetYTitle('number of entries/(%.3f GeV)' % ((pt_upper
                            - pt_lower) / nbins))

    description = \
        'Events with 10 muon tracks with fixed transverse momentum are generated using the ParticleGun (500 events for each pt value). The events are reconstructed with VXDTF+Trasan+MCTrackCandCombiner. The plot shows the difference of the generated transverse momentum pt_{gen} = %.2f GeV and the reconstructed transverse momentum pt_{rec} for all successfully fitted tracks.' \
        % pt_value_condition
    check = 'Residuals should be distributed around 0 GeV.'
    hist_residuum.GetListOfFunctions().Add(TNamed('Description', description))
    hist_residuum.GetListOfFunctions().Add(TNamed('Check', check))
    # loop over all entries in data_tree
    for ientry in range(number_entries):
        data_tree.GetEntry(ientry)  # load entry

        pt_value_gen = data_tree.GetLeaf('pt_gen').GetValue()
        # generated value is equal chosen pt value
        if pt_value_gen < pt_value_condition + DELTA_PT and pt_value_gen \
            > pt_value_condition - DELTA_PT:

            # load reconstructed pt value
            pt_value = data_tree.GetLeaf('pt').GetValue()

            # if the track was not fitted pt == -999
            if pt_value != -999:
                residuums.append(pt_value - pt_value_gen)
                pt_values.append(pt_value)
                pt_values_gen.append(pt_value_gen)
                hist_residuum.Fill(pt_value - pt_value_gen)

    # no track was fitted or chosen pt value is not in data_tree
    if len(residuums) == 0:
        return (pt_value_condition, -999, -999)

    hist_residuum.Write()
    # calculate width of residuum distribution == momentum resolution
    sigma_pt = hist_residuum.GetRMS(1)
    sigma_pt_error = hist_residuum.GetRMSError(1)

    return (pt_value_condition, sigma_pt, sigma_pt_error)


def create_momentum_resolution_plot(data_tree, pt_condition_values):
    '''Create momentum resolution plot'''

    number_bins = 62
    pt_lower = -0.025
    pt_upper = 3.075

    sigma_pt_values = []
    for pt_condition_value in pt_condition_values:
        sigma_pt_values.append(calculate_momentum_resolution(data_tree,
                               pt_condition_value))

    hist_resolution = TH1F('hPtResolution', 'hPtResolution', number_bins,
                           pt_lower, pt_upper)

    for ibin in range(1, number_bins + 1):
        bin_center = hist_resolution.GetBinCenter(ibin)

        for sigma_pt_value in sigma_pt_values:
            if bin_center < sigma_pt_value[0] + DELTA_PT and bin_center \
                > sigma_pt_value[0] - DELTA_PT:
                sigma_pt_over_pt = 0
                error = 0
                if sigma_pt_value[1] != -999:
                    sigma_pt_over_pt = sigma_pt_value[1] / sigma_pt_value[0]

                hist_resolution.SetBinContent(ibin, sigma_pt_over_pt)
                hist_resolution.SetBinError(ibin, sigma_pt_value[2])

    hist_resolution.SetTitle('Momentum resolution')
    hist_resolution.SetXTitle('pt in GeV')
    hist_resolution.SetYTitle('#sigma_{pt}/pt')

    description = \
        'Events with 10 muon tracks with fixed transverse momentum are generated using the ParticleGun (500 events for each pt value). The events are reconstructed with VXDTF+Trasan+MCTrackCandCombiner. The plot shows the relative momentum resolution of the transverse momentum over transverse momentum.'
    check = ''
    hist_resolution.GetListOfFunctions().Add(TNamed('Description',
            description))
    hist_resolution.GetListOfFunctions().Add(TNamed('Check', check))

    hist_resolution.Write()


if __name__ == '__main__':
    main()
