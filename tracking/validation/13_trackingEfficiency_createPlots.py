#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#     script creates efficiency vs pt plots and stores them     #
#     in a root file                                            #
#                                                               #
#################################################################

"""
<header>
    <input>trackingEfficiency_pt_0.05GeV.root,
    trackingEfficiency_pt_0.10GeV.root,trackingEfficiency_pt_0.25GeV.root,
    trackingEfficiency_pt_0.40GeV.root,trackingEfficiency_pt_0.60GeV.root,
    trackingEfficiency_pt_1.00GeV.root,trackingEfficiency_pt_1.50GeV.root,
    trackingEfficiency_pt_2.00GeV.root,trackingEfficiency_pt_3.00GeV.root,
    trackingEfficiency_pt_4.00GeV.root</input>
    <output>TrackingValidation.root</output>
    <contact>software-tracking@belle2.org</contact>
    <description>Create momentum resolution, impact parameter resolution and efficiency plots.</description>
</header>
"""


import ROOT

ROOT.PyConfig.IgnoreCommandLineOptions = True  # noqa
from ROOT import TFile, TChain, TH1F, \
    gStyle, TNamed, TProfile
import sys
import math
import numpy as np
from optparse import OptionParser
from tracking.validation.tracking_efficiency_helpers import get_generated_pt_values

DELTA_PT = 0.0001

PT_VALUES = get_generated_pt_values()
# PT_VALUES = [0.05, 0.1, 0.15, 0.25, 0.5, 1.0, 2.0, 3.0]

# contact person information
# is added to the plot descriptions
CONTACT_PERSON = {'Name': 'tracking software mailing list',
                  'Email': 'software-tracking@belle2.org'}


def make_expert_plot(hist):
    hist.GetListOfFunctions().Add(TNamed('MetaOptions', 'expert'))


def main():
    """Function which is executed"""

    print('Tracking validation plots.')

    option_parser = OptionParser()
    option_parser.add_option('-i', '--input-file', dest='input_file',
                             default='../trackingEfficiency_pt_*.root',
                             help='Root file with StandardTrackingPerformance output.'
                             )
    option_parser.add_option('-o', '--output-file', dest='output_file',
                             default='TrackingValidation.root',
                             help='Root file with all histograms.')

    options = option_parser.parse_args()[0]

    gStyle.SetOptStat(0)

    # load data tree
    tree_name = 'data'
    data_tree = TChain(tree_name)  # input_root_file.Get(tree_name)
    data_tree.Add(options.input_file)

    number_entries = 0
    try:
        number_entries = data_tree.GetEntries()
    except AttributeError:
        print('Could not load tree with name %s.' % tree_name)

    if number_entries == 0:
        print('Data tree \'%s\'is empty or does not exist. Exit.' % tree_name)
        sys.exit(0)

    # output root file
    output_file_name = options.output_file
    output_root_file = TFile(output_file_name, 'recreate')

    # create efficiency in bins of pt plot
    calculate_efficiency_in_pt(data_tree)

    pt_of_interest = [0.05, 0.25, 1.]
    # pt_of_interest = PT_VALUES

    for pt_value in pt_of_interest:
        # create plots of efficiency in bins of cos Theta for different pt
        generate_cos_theta_plot(data_tree, pt_value)

    # create momentum resolution plot
    create_momentum_resolution_plot(data_tree)

    draw_impact_parameter(data_tree)

    draw_pvalue(data_tree)

    draw_hit_counts(data_tree, pt_of_interest)

    draw_residua(data_tree, 'x', 'x_gen', pt_of_interest=pt_of_interest)
    draw_residua(data_tree, 'y', 'y_gen', pt_of_interest=pt_of_interest)
    draw_residua(data_tree, 'z', 'z_gen', pt_of_interest=pt_of_interest)

    draw_residua(
        data_tree,
        'pt',
        'pt_gen',
        bins=200,
        ledge=-0.1,
        uedge=0.1,
        normalize=1,
        pt_of_interest=pt_of_interest,
    )

    # write additional plots in sub dir
    sub_dir = 'additionalPlots'
    output_root_file.mkdir(sub_dir)
    output_root_file.cd(sub_dir)

    for pt_value in PT_VALUES:
        # create plots of efficiency in bins of cos Theta for different pt
        generate_cos_theta_plot(data_tree, pt_value)

    draw_residua(data_tree, 'x', 'x_gen')
    draw_residua(data_tree, 'y', 'y_gen')
    draw_residua(data_tree, 'z', 'z_gen')

    draw_residua(
        data_tree,
        'pt',
        'pt_gen',
        bins=200,
        ledge=-0.1,
        uedge=0.1,
        normalize=1,
    )

    # close output file
    output_root_file.Write()
    output_root_file.Close()


def draw_hit_counts(data_tree, pt_values):
    """
    Draw the hit count distribution.
    """

    pt_lower = -0.025
    pt_upper = 4.025
    # 50 MeV wide bins.                            + 0.5 serves for correct rounding.
    number_bins = int((pt_upper - pt_lower) / 0.05 + 0.5)

    hists = {}
    for det in ['PXD', 'SVD', 'CDC']:
        hists[det] = TProfile('hHitCounts%s' % det,
                              'Hit count profile for the %s;pT;nHits' % det,
                              number_bins, pt_lower, pt_upper)

        description = 'Distribution of Hit Counts in %s (Contact: %s).' \
            % (det, CONTACT_PERSON['Email'])
        check = ''

        hists[det].GetListOfFunctions().Add(TNamed('Description', description))
        hists[det].GetListOfFunctions().Add(TNamed('Check', check))
        make_expert_plot(hists[det])

    hNweights = TH1F('hNweights', 'number of weights stored', 201, 0, 201)
    hWeightsProfile = TProfile('hWeightsProfile', 'profile of weights', 201,
                               0, 201)
    hWeightsProfile.SetMinimum(0)
    hWeightsProfile.SetMaximum(1.1)
    hWeights = TH1F('hWeights', 'DAF weights', 50, 0, 1)

    for track in data_tree:
        if not track.pValue == -999:
            hists['PXD'].Fill(track.pt_gen, track.nPXDhits)
            hists['SVD'].Fill(track.pt_gen, track.nSVDhits)
            hists['CDC'].Fill(track.pt_gen, track.nCDChits)
            hNweights.Fill(track.nWeights)
            for i in range(track.nWeights):
                hWeights.Fill(track.weights[i])
                hWeightsProfile.Fill(i, track.weights[i])

    for key in hists:
        hists[key].Write()
    # hNweights.Write()
    # hWeights.Write()
    make_expert_plot(hWeightsProfile)
    hWeightsProfile.Write()


def draw_pvalue(data_tree):
    """
    Create a histogram of the pValue of the track fits
    """

    number_entries = data_tree.GetEntries()
    # Normalize number of bins to number of events to keep plots comparable.
    numBins = math.trunc(number_entries / 200)
    hist_pvalue = TH1F('hpValue', 'p-Value Distribution', numBins, 0., 1.)
    # Make axis range independent of first bin contents for more pleasant look
    hist_pvalue.SetMinimum(0)
    # hist_pvalue.SetMaximum(number_entries / 50.)

    for entry in range(number_entries):
        data_tree.GetEntry(entry)

        try:
            pvalue = data_tree.GetLeaf('pValue').GetValue()
        except ReferenceError:
            print('The variable "pValue" doesn\'t exist in the tree "%s".\nLeave this function without plotting the variable.'
                  % data_tree.GetName())
            return

        if pvalue is -999:
            continue
        else:
            hist_pvalue.Fill(pvalue)

    hist_pvalue.SetTitle('Distribution of pValue')
    hist_pvalue.SetXTitle('pValue')
    hist_pvalue.SetYTitle('number of entries')

    description = 'Distribution of pValues of the tracks (Contact: %s).' \
        % CONTACT_PERSON['Email']
    check = 'Should be a flat distribution.'

    hist_pvalue.GetListOfFunctions().Add(TNamed('Description', description))
    hist_pvalue.GetListOfFunctions().Add(TNamed('Check', check))
    make_expert_plot(hist_pvalue)

    hist_pvalue.Write()


def calculate_efficiency_in_pt(data_tree):
    """Calculate single track reconstruction efficiency in bins of pt"""

    pt_lower = -0.025
    pt_upper = 4.025
    # 50 MeV wide bins.                            + 0.5 serves for correct rounding.
    number_bins = int((pt_upper - pt_lower) / 0.05 + 0.5)

    hist_pt_gen = TH1F('hPtGen', 'hPtGen', number_bins, pt_lower, pt_upper)
    hist_pt_rec = TH1F('hPtRec', 'hPtRec', number_bins, pt_lower, pt_upper)

    # draw data in defined histos
    data_tree.Draw('pt_gen>>hPtGen', '', 'goff')
    data_tree.Draw('pt_gen>>hPtRec', 'pt != -999', 'goff')

    # final hist
    efficiency_hist = TH1F('hEfficiency', 'hEfficiency', number_bins,
                           pt_lower, pt_upper)
    efficiency_hist.SetMinimum(0)
    efficiency_hist.SetMaximum(1.05)

    description = ('Events with 10 muon tracks with fixed transverse '
                   'momentum are generated using the ParticleGun(500 '
                   'events for each pt value). The events are reconstructed '
                   'with VXDTF + CDC Track Finder + VXDCDCMerger.'
                   'plot shows the single track reconstruction efficiency '
                   'over the transverse momentum.')

    check = 'The efficiency should be stable for higher pt values.'
    efficiency_hist.GetListOfFunctions().Add(TNamed('Description',
                                                    description))
    efficiency_hist.GetListOfFunctions().Add(TNamed('Check', check))
    efficiency_hist.GetListOfFunctions().Add(TNamed('Contact',
                                                    CONTACT_PERSON['Email']))

    # loop over bins and calculate efficiency and error of efficiency
    for ibin in range(1, number_bins + 1):
        number_generated = hist_pt_gen.GetBinContent(ibin)
        efficiency = 0
        error = 0

        if number_generated > 0:
            number_reconstructed = hist_pt_rec.GetBinContent(ibin)
            efficiency = number_reconstructed / number_generated
            error = math.sqrt(number_reconstructed * (number_generated - number_reconstructed) / pow(number_generated, 3))
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
    """Creates a efficiency histo in bins of cos theta"""

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

    description = ('Events with 10 muon tracks with fixed transverse '
                   'momentum are generated using the ParticleGun(500 '
                   'events for each pt value). The events are reconstructed '
                   'with VXDTF + CDCTF + MCTrackCandCombiner. The plot '
                   'shows the single track reconstruction efficiency in '
                   'bins of the polar angle for the fixed transverse '
                   'momentum pt = %.2f GeV.')
    check = 'Stable efficiency over the hole range of the polar angle.'

    efficiency_hist = TH1F('hEfficiencyPt%.2fGeV' % pt_value,
                           'hEfficiencyPt%.2fGeV' % pt_value, number_bins,
                           cos_lower, cos_upper)
    efficiency_hist.GetListOfFunctions().Add(TNamed('Description',
                                                    description))
    efficiency_hist.GetListOfFunctions().Add(TNamed('Check', check))
    efficiency_hist.GetListOfFunctions().Add(TNamed('Contact',
                                                    CONTACT_PERSON['Email']))

    for ibin in range(1, number_bins + 1):
        efficiency = 0
        error = 0
        number_generated = hist_cos_gen.GetBinContent(ibin)

        if number_generated > 0:
            number_reconstructed = hist_cos_rec.GetBinContent(ibin)
            efficiency = number_reconstructed / number_generated
            error = math.sqrt(number_reconstructed * (number_generated - number_reconstructed) / pow(number_generated, 3))

        efficiency_hist.SetBinContent(ibin, efficiency)
        efficiency_hist.SetBinError(ibin, error)

    efficiency_hist.SetTitle('Tracks with pt = %.2f GeV' % pt_value)
    efficiency_hist.SetXTitle('cos #Theta')
    efficiency_hist.SetYTitle('efficiency')
    make_expert_plot(efficiency_hist)
    efficiency_hist.Write()


def create_momentum_resolution_plot(data_tree):
    """Create momentum resolution plot"""

    pt_lower = -0.025
    pt_upper = 4.025
    # 50 MeV wide bins.                            + 0.5 serves for correct rounding.
    number_bins = int((pt_upper - pt_lower) / 0.05 + 0.5)

    sigma_pt_values = calculate_momentum_resolution2(data_tree)

    hist_resolution = TH1F('hPtResolution', 'hPtResolution', number_bins,
                           pt_lower, pt_upper)

    fit_pt_values = []
    fit_pt_res_values = []
    fit_pt_res_errors = []

    for (pt_value, sigma_pt_value) in sigma_pt_values.items():
        ibin = hist_resolution.FindBin(pt_value)
        bin_center = hist_resolution.GetBinCenter(ibin)

        sigma_pt_over_pt = 0
        if sigma_pt_value[0] != -999:
            sigma_pt_over_pt = sigma_pt_value[0] / pt_value
            sigma_pt_error_over_pt = sigma_pt_value[1] / pt_value

            fit_pt_res_values.append(sigma_pt_over_pt)
            fit_pt_res_errors.append(sigma_pt_error_over_pt)
            fit_pt_values.append(bin_center)

        hist_resolution.SetBinContent(ibin, sigma_pt_over_pt)
        hist_resolution.SetBinError(ibin, sigma_pt_error_over_pt)

    hist_resolution.SetTitle('Transverse Momentum resolution')
    hist_resolution.SetXTitle('pt in GeV/c')
    hist_resolution.SetYTitle('#sigma_{pt}/pt')

    description = ('Events with 10 muon tracks with fixed transverse '
                   'momentum are generated using the ParticleGun(200 '
                   'events for each pt value). The events are reconstructed '
                   'with VXDTF + CDCTF + MCTrackCandCombiner. The plot '
                   'shows the relative momentum resolution of the '
                   'transverse momentum over transverse momentum.')

    check = ''
    hist_resolution.GetListOfFunctions().Add(TNamed('Description',
                                                    description))
    hist_resolution.GetListOfFunctions().Add(TNamed('Check', check))
    hist_resolution.GetListOfFunctions().Add(TNamed('Contact',
                                                    CONTACT_PERSON['Email']))
    hist_resolution.GetListOfFunctions().Add(TNamed('MetaOptions', 'logy'))
    hist_resolution.Write()

    # fit_parameters = fit_resolution(fit_pt_values, fit_pt_res_values, 1, fit_pt_res_errors)


def get_scaled_rms_90(values, scale_factor=0.79):
    """
    Calculate the RMS90

    param values: list of numbers
    return scaled RMS90 of the distribution in values
    """

    (vMin, vMax) = (np.nanpercentile(values, 5), np.nanpercentile(values, 95))
    final_list = np.array(values)[np.logical_and(values > vMin, values < vMax)]

    rms_90 = np.sqrt(np.mean(np.square(final_list))) / scale_factor

    rms_error = rms_90 / (scale_factor * np.sqrt(2 * len(final_list)))

    return (rms_90, rms_error)


def value_in_list(test_value, value_list):
    """
    Function checks, if a given value is in the list values
    """

    for value in value_list:
        if value - DELTA_PT < test_value < value + DELTA_PT:
            return (True, value)

    return (False, -999)


def calculate_momentum_resolution2(data_tree):
    """
    Calculate momentum resolution
    """

    delta_pt_dict = {}

    for key in PT_VALUES:
        delta_pt_dict[key] = []

    for entry_index in range(data_tree.GetEntries()):
        # load entry
        data_tree.GetEntry(entry_index)

        pt_gen = data_tree.GetLeaf('pt_gen').GetValue()

        test_object = value_in_list(pt_gen, PT_VALUES)

        if test_object[0]:
            pt = data_tree.GetLeaf('pt').GetValue()
            if pt != -999:
                try:
                    delta_pt_dict[test_object[1]].append(pt - pt_gen)
                except KeyError:
                    print('pt = %0.2f is not generated and not used as key. Abort!'
                          % test_object[1])
                    sys.exit(1)
    pt_resolutions = {}

    print('********************************')
    print('Momentum resolution:')
    for key in sorted(delta_pt_dict):
        (rms90, rms_error) = get_scaled_rms_90(delta_pt_dict[key])
        pt_resolutions[key] = [rms90, rms_error]
        print('pt = %0.2f: sigma/pt = %0.4f' % (key, rms90 / key))

    return pt_resolutions


def fit_resolution(
    x_value_list,
    y_value_list,
    degree,
    y_value_errors=None,
):
    """
    Fit a polynomial to data

    @param x_value_list list of x values

    @param y_value_list list of y values

    @param degree degree of the fitting polynomial

    @param y_value_errors list of corresponding errors on y x_values
    """

    try:
        import scipy.optimize
    except ImportError:
        print('Module scipy.optimize is not installed. Fit cannot be done.')
        return []

    # weights = None
    # if y_value_errors is not None:
    #    weights = [1 / x ** 2 for x in y_value_errors]
    #
    # parameters = np.polyfit(x_value_list, y_value_list, degree, w=weights,
    #                        cov=True)

    x_value_list = np.array(x_value_list)
    y_value_list = np.array(y_value_list)
    y_value_errors = np.array(y_value_errors)

    (par_opt, par_cov) = \
        scipy.optimize.curve_fit(fit_function_sigma_pt_over_pt, x_value_list,
                                 y_value_list, [0.002, 0.003],
                                 sigma=y_value_errors)

    print(par_opt, par_cov)

    return par_opt


def draw_impact_parameter(data_tree):
    """
    Create a histogram with the impact parameter resolution

    @param data_tree ROOT tree with the input data
    """

    print('********************************')
    print('Impact parameter:')

    impact_param_d = {}
    impact_param_z = {}
    for key in PT_VALUES:
        impact_param_d[key] = []
        impact_param_z[key] = []

    try:
        number_of_entries = data_tree.GetEntries()
    except AttributeError:
        print('ERROR: data_tree in function "draw_impact_parameter" is no TTree.')
        sys.exit(2)

    for index_entry in range(number_of_entries):
        data_tree.GetEntry(index_entry)

        pt_gen = data_tree.GetLeaf('pt_gen').GetValue()

        test_object = value_in_list(pt_gen, PT_VALUES)
        if test_object[0] is True:
            poca_x = data_tree.GetLeaf('x').GetValue()

            if poca_x == -999:
                continue

            poca_y = data_tree.GetLeaf('y').GetValue()
            poca_z = data_tree.GetLeaf('z').GetValue()

            z_gen = data_tree.GetLeaf('z_gen').GetValue()

            px = data_tree.GetLeaf('px').GetValue()
            py = data_tree.GetLeaf('py').GetValue()

            d0 = d0_sign([poca_x, poca_y], [px, py]) * np.hypot(poca_x, poca_y)

            impact_param_d[test_object[1]].append(d0)
            impact_param_z[test_object[1]].append(poca_z - z_gen)
            # estimate sign of d0

    d0_resolutions = {}
    z_resolutions = {}
    for key in sorted(impact_param_d):
        (d0_resolution, d0_error) = get_scaled_rms_90(impact_param_d[key])
        d0_resolutions[key] = [d0_resolution, d0_error]
        (z_resolution, z_error) = get_scaled_rms_90(impact_param_z[key])
        z_resolutions[key] = [z_resolution, z_error]
        print('pt = %0.2f: sigma_d0 = %0.4f, sigma_z = %0.4f' % (key,
                                                                 d0_resolution, z_resolution))

    number_bins = 62
    lower_edge = -0.025
    upper_edge = 4.025

    hist_impact_parameter_d0 = TH1F('hd0resolution', 'hd0resolution',
                                    number_bins, lower_edge, upper_edge)
    hist_impact_parameter_d0.SetTitle('d0 resolution')
    hist_impact_parameter_d0.SetXTitle('pt in GeV/c')
    hist_impact_parameter_d0.SetYTitle('#sigma_{d0} [cm]')
    hist_impact_parameter_d0.GetListOfFunctions().Add(TNamed('MetaOptions', 'logy'))
    hist_impact_parameter_d0.GetListOfFunctions().Add(TNamed('Contact',
                                                             CONTACT_PERSON['Email']))

    hist_impact_parameter_z = TH1F('hzresolution', 'hzresolution',
                                   number_bins, lower_edge, upper_edge)
    hist_impact_parameter_z.SetTitle('z resolution')
    hist_impact_parameter_z.SetXTitle('pt in GeV/c')
    hist_impact_parameter_z.SetYTitle('#sigma_{z} [cm]')
    hist_impact_parameter_z.GetListOfFunctions().Add(TNamed('MetaOptions', 'logy'))
    hist_impact_parameter_z.GetListOfFunctions().Add(TNamed('Contact',
                                                            CONTACT_PERSON['Email']))

    for pt in PT_VALUES:
        ibin = hist_impact_parameter_d0.FindBin(pt)
        if d0_resolutions[pt][0] > 0 and d0_resolutions[pt][1] > 0:
            hist_impact_parameter_d0.SetBinContent(ibin, d0_resolutions[pt][0])
            hist_impact_parameter_d0.SetBinError(ibin, d0_resolutions[pt][1])
        if z_resolutions[pt][0] > 0 and z_resolutions[pt][1] > 0:
            hist_impact_parameter_z.SetBinContent(ibin, z_resolutions[pt][0])
            hist_impact_parameter_z.SetBinError(ibin, z_resolutions[pt][1])

    hist_impact_parameter_d0.Write()
    hist_impact_parameter_z.Write()


def scale_histogram(hist):
    """
    Scales a TH1F using the TH1::Scale function
    """

    if isinstance(hist, TH1F):
        if hist.GetEntries() > 0:
            hist.Scale(1 / hist.GetEntries())
        else:
            print('Cannot scale. No entries in histogram ' + hist.GetName())
    else:
        print('Not a TH1F. Not able to scale.')


def d0_sign(vector_d, momentum):
    """
    Estimate the sign of the impact parameter d0_sign

    @return -1 or +1
    """

    if len(vector_d) != 2 or len(momentum) != 2:
        print('Need x and y values of d and p_d.')
        sys.exit(1)

    phi_momentum = np.arctan2(momentum[1], momentum[0])
    phi_d = np.arctan2(vector_d[1], vector_d[0])

    diff = phi_momentum - phi_d
    # Catch corner cases.  For clarity, don't return early.
    if diff > np.pi:
        diff = diff - 2 * np.pi
    elif diff < -np.pi:
        diff = 2 * np.pi + diff

    return np.sign(diff)


def draw_residua(
    data_tree,
    variable_name,
    gen_variable_name,
    bins=100,
    ledge=-0.01,
    uedge=0.01,
    normalize=0,
    pt_of_interest=None,
):
    """
    Write histogram of difference of variable_name (reconstructed) and
    gen_variable_name (generated) in gDirectory
    """

    if pt_of_interest is None:
        used_pts = PT_VALUES
    else:
        used_pts = [pt for pt in pt_of_interest if pt in PT_VALUES]

    number_entries = data_tree.GetEntries()

    histograms = {}
    for pt_value in used_pts:
        histograms[pt_value] = TH1F('h%sResiduum_%0.2fGeV' % (variable_name,
                                                              pt_value), 'h%sResiduum_%0.2fGeV'
                                    % (variable_name, pt_value), bins, ledge,
                                    uedge)

    for index in range(number_entries):
        data_tree.GetEntry(index)

        variable_reconstructed = data_tree.GetLeaf(variable_name).GetValue()
        if variable_reconstructed != -999:
            variable_generated = \
                data_tree.GetLeaf(gen_variable_name).GetValue()
            pt_gen = round(data_tree.GetLeaf('pt_gen').GetValue(), 2)
            if pt_gen in used_pts:
                if normalize:
                    histograms[pt_gen].Fill((variable_reconstructed - variable_generated) / variable_generated)
                else:
                    histograms[pt_gen].Fill(variable_reconstructed - variable_generated)

    for (pt, hist) in histograms.items():
        scale_histogram(hist)
        if normalize:
            hist.SetXTitle('(%s - %s) / %s' % (variable_name,
                                               gen_variable_name, gen_variable_name))
        else:
            hist.SetXTitle('(%s - %s)' % (variable_name, gen_variable_name))
        hist.GetListOfFunctions().Add(TNamed('Contact', CONTACT_PERSON['Email'
                                                                       ]))
        make_expert_plot(hist)
        hist.Write()


def tf1_fit_function(x, par):
    return fit_function_sigma_pt_over_pt(x, par[0], par[1])


def fit_function_sigma_pt_over_pt(x, a, b):
    return np.sqrt((a * x) ** 2 + b * b)


if __name__ == '__main__':
    main()
