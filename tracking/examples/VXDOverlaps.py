##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import ROOT
import numpy
import root_numpy
from array import array
from ROOT import TCanvas, TH1F, TLine


'''Takes the output of combined OverlapResiduals and HistoManager modules,
   as input, providess hit-maps for overlapping VXD hits in Layer:Sensor
   plots and computes statistics for 2D monitoring plots.'''


def Median_plots_phi(filename, lyr_num, phi_bins, phi_inf, phi_sup):
    """
    Function to compute the median of the projected DeltaResU(V)
    distributions for each azimuthal overlap in DeltaResU(V) vs phi plots
    """
    # Accesses the TDirectory containing VXD histograms of residuals differences for overlapping hits
    f = ROOT.TFile.Open(filename, 'read')
    mn = f.Get('Monitoring_VXDOverlaps')
    # Gets 2D DeltaRes_u vs phi histograms stored in histofile.root
    h_PhiU = mn.Get('h_DeltaResUPhi_Lyr' + str(lyr_num))
    # Gets 2D DeltaRes_v vs phi histograms stored in histofile.root
    h_PhiV = mn.Get('h_DeltaResVPhi_Lyr' + str(lyr_num))
    # Defines histograms of projected DeltaRes_u distributions
    h_UMedians = TH1F(
        'h_UMedians_Lyr' +
        str(lyr_num),
        'Layer' +
        str(lyr_num) +
        ': medians of #Deltares_{u} for each overlap',
        phi_bins,
        phi_inf,
        phi_sup)
    # Defines histograms of projected DeltaRes_u distributions
    h_VMedians = TH1F(
        'h_VMedians_Lyr' +
        str(lyr_num),
        'Layer' +
        str(lyr_num) +
        ': medians of #Deltares_{v} for each overlap',
        phi_bins,
        phi_inf,
        phi_sup)
    # Sets the number of resampled samples for bootstrap
    Nrs = 100
    # Initializes parameters to compute the median of ROOT TH1
    q_U = array('d', [0])
    p_U = array('d', [0.5])
    q_V = array('d', [0])
    p_V = array('d', [0.5])
    # Lists storing medians of toy MC distributions for statistical bootstrap
    l_U_median = []
    l_V_median = []
    # Lists storing estimated medians of projected DeltaRes distribution for every azimuthal overlap
    l_U_median_pos = []
    l_V_median_pos = []
    # Computes medians and related uncertainties (statistical bootstrapping) and produces plots
    c_PhiU = TCanvas('c_PhiU_' + str(lyr_num), 'DeltaResUPhi_' + str(lyr_num), 700, 500)
    c_PhiV = TCanvas('c_PhiV_' + str(lyr_num), 'DeltaResVPhi_' + str(lyr_num), 700, 500)
    if(lyr_num == 1 or lyr_num == 3):
        c_PhiU.Divide(4, 2)
        c_PhiV.Divide(4, 2)
    if(lyr_num == 4):
        c_PhiU.Divide(5, 2)
        c_PhiV.Divide(5, 2)
    if(lyr_num == 5):
        c_PhiU.Divide(5, 3)
        c_PhiV.Divide(5, 3)
    if(lyr_num == 6):
        c_PhiU.Divide(5, 4)
        c_PhiV.Divide(5, 4)
    c_PhiUMedians = TCanvas('c_PhiUMedians_' + str(lyr_num), 'UMedians_' + str(lyr_num), 700, 500)
    c_PhiVMedians = TCanvas('c_PhiVMedians_' + str(lyr_num), 'VMedians_' + str(lyr_num), 700, 500)
    for i in range(0, phi_bins):
        xinf = phi_inf + i * (phi_sup - phi_inf) / phi_bins
        xsup = phi_inf + (i + 1) * (phi_sup - phi_inf) / phi_bins
        h_PhiU.GetXaxis().SetRangeUser(xinf, xsup)
        h_PhiV.GetXaxis().SetRangeUser(xinf, xsup)
        h_PhiU.ProjectionY().GetQuantiles(1, q_U, p_U)
        h_PhiV.ProjectionY().GetQuantiles(1, q_V, p_V)
        h_U = h_PhiU.ProjectionY()
        h_V = h_PhiV.ProjectionY()
        h_U.SetTitle(str(round(xinf, 3)) + ' < #phi < ' + str(round(xsup, 3)))
        h_V.SetTitle(str(round(xinf, 3)) + ' < #phi < ' + str(round(xsup, 3)))
        h_U.GetXaxis().SetRangeUser(-200, 200)
        h_V.GetXaxis().SetRangeUser(-200, 200)
        if(lyr_num == 1):
            h_U.GetYaxis().SetRangeUser(0, 300)
            h_V.GetYaxis().SetRangeUser(0, 300)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 300)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 300)
        if(lyr_num == 3):
            h_U.GetYaxis().SetRangeUser(0, 500)
            h_V.GetYaxis().SetRangeUser(0, 500)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 500)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 500)
        if(lyr_num == 4):
            h_U.GetYaxis().SetRangeUser(0, 2000)
            h_V.GetYaxis().SetRangeUser(0, 2000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 2000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 2000)
        if(lyr_num == 5):
            h_U.GetYaxis().SetRangeUser(0, 1000)
            h_V.GetYaxis().SetRangeUser(0, 1000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 1000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 1000)
        if(lyr_num == 6):
            h_U.GetYaxis().SetRangeUser(0, 1000)
            h_V.GetYaxis().SetRangeUser(0, 1000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 1000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 1000)
        median_pos_U.SetLineWidth(2)
        median_pos_V.SetLineWidth(2)
        median_pos_U.SetLineColor(2)
        median_pos_V.SetLineColor(2)
        l_U_median_pos.append(median_pos_U)
        l_V_median_pos.append(median_pos_V)
        h_U.GetYaxis().SetTitle('counts')
        h_V.GetYaxis().SetTitle('counts')
        meas_U = root_numpy.hist2array(h_U)
        meas_V = root_numpy.hist2array(h_V)
        bs_U = numpy.random.poisson(1., (len(meas_U), Nrs))
        bs_V = numpy.random.poisson(1., (len(meas_V), Nrs))
        for j in range(Nrs):
            toy_U = numpy.repeat(meas_U, bs_U[:, j])
            toy_V = numpy.repeat(meas_V, bs_V[:, j])
            median_U_toy = numpy.median(toy_U)
            median_V_toy = numpy.median(toy_V)
            l_U_median.append(median_U_toy)
            l_V_median.append(median_V_toy)
        median_U_rs = numpy.array(l_U_median)
        median_V_rs = numpy.array(l_V_median)
        median_U_dev = numpy.std(median_U_rs)
        median_V_dev = numpy.std(median_V_rs)
        h_UMedians.SetBinContent(i + 1, q_U[0])
        h_UMedians.SetBinError(i + 1, median_U_dev)
        h_VMedians.SetBinContent(i + 1, q_V[0])
        h_VMedians.SetBinError(i + 1, median_V_dev)
        c_PhiU.cd(i + 1)
        h_U.DrawCopy()
        l_U_median_pos[i].Draw("SAME")
        c_PhiV.cd(i + 1)
        h_V.DrawCopy()
        l_V_median_pos[i].Draw("SAME")
        c_PhiUMedians.cd()
        h_UMedians.GetXaxis().SetTitle('#phi (rad)')
        h_UMedians.GetYaxis().SetTitle('Median of #Deltares_{u} (#mum)')
        h_UMedians.Draw()
        c_PhiVMedians.cd()
        h_VMedians.GetXaxis().SetTitle('#phi (rad)')
        h_VMedians.GetYaxis().SetTitle('Median of #Deltares_{V} (#mum)')
        h_VMedians.Draw()
        # If not existing, creates a dedicated folder
        if not os.path.exists('Median_plots_OverlapsPhi'):
            os.mkdir('Median_plots_OverlapsPhi')
        c_PhiU.SaveAs('Median_plots_OverlapsPhi/Median_and_DeltaResUPhi_Lyr' + str(lyr_num) + '.root')
        c_PhiU.SaveAs('Median_plots_OverlapsPhi/Median_and_DeltaResUPhi_Lyr' + str(lyr_num) + '.pdf')
        c_PhiV.SaveAs('Median_plots_OverlapsPhi/Median_and_DeltaResVPhi_Lyr' + str(lyr_num) + '.root')
        c_PhiV.SaveAs('Median_plots_OverlapsPhi/Median_and_DeltaResVPhi_Lyr' + str(lyr_num) + '.pdf')
        c_PhiUMedians.SaveAs('Median_plots_OverlapsPhi/Lyr' + str(lyr_num) + '_DeltaResUMedians_vs_phi.root')
        c_PhiUMedians.SaveAs('Median_plots_OverlapsPhi/Lyr' + str(lyr_num) + '_DeltaResUMedians_vs_phi.pdf')
        c_PhiVMedians.SaveAs('Median_plots_OverlapsPhi/Lyr' + str(lyr_num) + '_DeltaResVMedians_vs_phi.root')
        c_PhiVMedians.SaveAs('Median_plots_OverlapsPhi/Lyr' + str(lyr_num) + '_DeltaResVMedians_vs_phi.pdf')
    return


def Median_plots_z(filename, lyr_num, z_bins, z_inf, z_sup):
    """
    Function to compute the median of the projected DeltaResU(V) distributions
    for each sensor in DeltaResU(V) vs z plots
    """
    # Accesses the TDirectory containing VXD histograms of residuals differences for overlapping hits
    f = ROOT.TFile.Open(filename, 'read')
    mn = f.Get('Monitoring_VXDOverlaps')
    # Gets 2D DeltaRes_u vs z histograms stored in histofile.root
    h_ZU = mn.Get('h_DeltaResUz_Lyr' + str(lyr_num))
    # Gets 2D DeltaRes_v vs z histograms stored in histofile.root
    h_ZV = mn.Get('h_DeltaResVz_Lyr' + str(lyr_num))
    # Defines histograms of projected DeltaRes_u distributions
    h_UMedians = TH1F('h_UMedians_Lyr' + str(lyr_num), 'Layer' + str(lyr_num) +
                      ': medians of #Deltares_{u} for each sensor', z_bins, z_inf, z_sup)
    # Defines histograms of projected DeltaRes_u distributions
    h_VMedians = TH1F('h_VMedians_Lyr' + str(lyr_num), 'Layer' + str(lyr_num) +
                      ': medians of #Deltares_{v} for each sensor', z_bins, z_inf, z_sup)
    # Sets the number of resampled samples for bootstrap
    Nrs = 100
    # Initializes parameters to compute the median of ROOT TH1
    q_U = array('d', [0])
    p_U = array('d', [0.5])
    q_V = array('d', [0])
    p_V = array('d', [0.5])
    # Lists storing medians of toy MC distributions for statistical bootstrap
    l_U_median = []
    l_V_median = []
    # Lists storing estimated medians of projected DeltaRes distribution for every sensor along z
    l_U_median_pos = []
    l_V_median_pos = []
    # Computes medians and related uncertainties (statistical bootstrapping) and produces plots
    c_ZU = TCanvas('c_ZU_' + str(lyr_num), 'DeltaResUZ_' + str(lyr_num), 700, 500)
    c_ZV = TCanvas('c_ZV_' + str(lyr_num), 'DeltaResVZ_' + str(lyr_num), 700, 500)
    if(lyr_num == 1 or lyr_num == 3):
        c_ZU.Divide(2, 1)
        c_ZV.Divide(2, 1)
    if(lyr_num == 4):
        c_ZU.Divide(3, 1)
        c_ZV.Divide(3, 1)
    if(lyr_num == 5 or lyr_num == 6):
        c_ZU.Divide(3, 2)
        c_ZV.Divide(3, 2)
    c_ZUMedians = TCanvas('c_ZUMedians_' + str(lyr_num), 'UMedians_' + str(lyr_num), 700, 500)
    c_ZVMedians = TCanvas('c_ZVMedians_' + str(lyr_num), 'VMedians_' + str(lyr_num), 700, 500)
    for i in range(0, z_bins):
        xinf = z_inf + i * (z_sup - z_inf) / z_bins
        xsup = z_inf + (i + 1) * (z_sup - z_inf) / z_bins
        h_ZU.GetXaxis().SetRangeUser(xinf, xsup)
        h_ZV.GetXaxis().SetRangeUser(xinf, xsup)
        h_ZU.ProjectionY().GetQuantiles(1, q_U, p_U)
        h_ZV.ProjectionY().GetQuantiles(1, q_V, p_V)
        h_U = h_ZU.ProjectionY()
        h_V = h_ZV.ProjectionY()
        h_U.SetTitle(str(round(xinf, 3)) + ' (cm) < z < ' + str(round(xsup, 3)) + ' (cm)')
        h_V.SetTitle(str(round(xinf, 3)) + ' (cm) < z < ' + str(round(xsup, 3)) + ' (cm)')
        h_U.GetXaxis().SetRangeUser(-200, 200)
        h_V.GetXaxis().SetRangeUser(-200, 200)
        if(lyr_num == 1):
            h_U.GetYaxis().SetRangeUser(0, 500)
            h_V.GetYaxis().SetRangeUser(0, 500)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 500)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 500)
        if(lyr_num == 3):
            h_U.GetYaxis().SetRangeUser(0, 2000)
            h_V.GetYaxis().SetRangeUser(0, 2000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 2000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 2000)
        if(lyr_num == 4):
            h_U.GetYaxis().SetRangeUser(0, 7000)
            h_V.GetYaxis().SetRangeUser(0, 7000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 7000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 7000)
        if(lyr_num == 5):
            h_U.GetYaxis().SetRangeUser(0, 3000)
            h_V.GetYaxis().SetRangeUser(0, 3000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 3000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 3000)
        if(lyr_num == 6):
            h_U.GetYaxis().SetRangeUser(0, 3000)
            h_V.GetYaxis().SetRangeUser(0, 3000)
            median_pos_U = TLine(q_U[0], 0, q_U[0], 3000)
            median_pos_V = TLine(q_V[0], 0, q_V[0], 3000)
        median_pos_U.SetLineWidth(2)
        median_pos_V.SetLineWidth(2)
        median_pos_U.SetLineColor(2)
        median_pos_V.SetLineColor(2)
        l_U_median_pos.append(median_pos_U)
        l_V_median_pos.append(median_pos_V)
        h_U.GetYaxis().SetTitle('counts')
        h_V.GetYaxis().SetTitle('counts')
        meas_U = root_numpy.hist2array(h_U)
        meas_V = root_numpy.hist2array(h_V)
        bs_U = numpy.random.poisson(1., (len(meas_U), Nrs))
        bs_V = numpy.random.poisson(1., (len(meas_V), Nrs))
        for j in range(Nrs):
            toy_U = numpy.repeat(meas_U, bs_U[:, j])
            toy_V = numpy.repeat(meas_V, bs_V[:, j])
            median_U_toy = numpy.median(toy_U)
            median_V_toy = numpy.median(toy_V)
            l_U_median.append(median_U_toy)
            l_V_median.append(median_V_toy)
        median_U_rs = numpy.array(l_U_median)
        median_V_rs = numpy.array(l_V_median)
        median_U_dev = numpy.std(median_U_rs)
        median_V_dev = numpy.std(median_V_rs)
        h_UMedians.SetBinContent(i + 1, q_U[0])
        h_UMedians.SetBinError(i + 1, median_U_dev)
        h_VMedians.SetBinContent(i + 1, q_V[0])
        h_VMedians.SetBinError(i + 1, median_V_dev)
        c_ZU.cd(i + 1)
        h_U.DrawCopy()
        l_U_median_pos[i].Draw("SAME")
        c_ZV.cd(i + 1)
        h_V.DrawCopy()
        l_V_median_pos[i].Draw("SAME")
        c_ZUMedians.cd()
        h_UMedians.GetXaxis().SetTitle('z (cm)')
        h_UMedians.GetYaxis().SetTitle('Median of #Deltares_{u} (#mum)')
        h_UMedians.Draw()
        c_ZVMedians.cd()
        h_VMedians.GetXaxis().SetTitle('z (cm)')
        h_VMedians.GetYaxis().SetTitle('Median of #Deltares_{V} (#mum)')
        h_VMedians.Draw()
        # If not existing, creates a dedicated folder
        if not os.path.exists('Median_plots_OverlapsZ'):
            os.mkdir('Median_plots_OverlapsZ')
        c_ZU.SaveAs('Median_plots_OverlapsZ/Median_and_DeltaResUZ_Lyr' + str(lyr_num) + '.root')
        c_ZU.SaveAs('Median_plots_OverlapsZ/Median_and_DeltaResUZ_Lyr' + str(lyr_num) + '.pdf')
        c_ZV.SaveAs('Median_plots_OverlapsZ/Median_and_DeltaResVZ_Lyr' + str(lyr_num) + '.root')
        c_ZV.SaveAs('Median_plots_OverlapsZ/Median_and_DeltaResVZ_Lyr' + str(lyr_num) + '.pdf')
        c_ZUMedians.SaveAs('Median_plots_OverlapsZ/Lyr' + str(lyr_num) + '_DeltaResUMedians_vs_z.root')
        c_ZUMedians.SaveAs('Median_plots_OverlapsZ/Lyr' + str(lyr_num) + '_DeltaResUMedians_vs_z.pdf')
        c_ZVMedians.SaveAs('Median_plots_OverlapsZ/Lyr' + str(lyr_num) + '_DeltaResVMedians_vs_z.root')
        c_ZVMedians.SaveAs('Median_plots_OverlapsZ/Lyr' + str(lyr_num) + '_DeltaResVMedians_vs_z.pdf')
    return


def LayerSensorPlots(filename, lyr_num, lddr_num, snsr_num):
    """
    Creates and saves Layer.Sensor plots for overlapping hits hitmaps
    """
    # Accesses the TDirectory containing VXD hit-maps for overlapping hits
    f = ROOT.TFile.Open(filename, 'read')
    hm = f.Get('HitMaps_VXDOverlaps')
    # If not existing, creates a dedicated directory
    if not os.path.exists('HitMaps_plots_Overlaps'):
        os.mkdir('HitMaps_plots_Overlaps')
    # Produces Layer.Sensor plots containing hit-maps for overlapping hits
    # in all the ladders of a specific layer
    for i in range(1, snsr_num + 1):
        c_Meas = TCanvas('c_Meas_' + str(lyr_num) + ':' + str(i), 'Layer:Sensor = ' + str(lyr_num) + ':' + str(i), 500, 700)
        if(lyr_num == 6):
            c_Meas.Divide(4, 4)
        elif(lyr_num == 5):
            c_Meas.Divide(4, 3)
        elif(lyr_num == 4):
            c_Meas.Divide(5, 2)
        elif(lyr_num == 3):
            c_Meas.Divide(7, 1)
        elif(lyr_num == 2):
            c_Meas.Divide(6, 2)
        elif(lyr_num == 1):
            c_Meas.Divide(4, 2)
        for k in range(1, lddr_num + 1):
            histo = hm.Get('h_' + str(lyr_num) + str(k) + str(i))
            c_Meas.cd(k)
            histo.Draw('COLZ')
            c_Meas.SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(lyr_num) + str(i) + '.root')
            c_Meas.SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(lyr_num) + str(i) + '.pdf')
    return


# Root output of module OverlapResiduals
filename = 'histofile.root'

if __name__ == "__main__":
    # Dictionary for VXD layers with overlaps
    VXDLayers = {1: {'Layer': 1, 'Ladders': 8, 'Sensors': 2, 'Phi_bins': 8,
                     'Phi_inf': -3.2, 'Phi_sup': 3.2, 'Z_bins': 2, 'Z_inf': -3.2, 'Z_sup': 5.9},
                 3: {'Layer': 3, 'Ladders': 7, 'Sensors': 2, 'Phi_bins': 7,
                     'Phi_inf': -3.0, 'Phi_sup': 3.0, 'Z_bins': 2, 'Z_inf': -9.5, 'Z_sup': 15.5},
                 4: {'Layer': 4, 'Ladders': 10, 'Sensors': 3, 'Phi_bins': 10,
                     'Phi_inf': -3.0, 'Phi_sup': 3.0, 'Z_bins': 3, 'Z_inf': -16.5, 'Z_sup': 21.5},
                 5: {'Layer': 5, 'Ladders': 12, 'Sensors': 4, 'Phi_bins': 13,
                     'Phi_inf': -3.2, 'Phi_sup': 3.2, 'Z_bins': 4, 'Z_inf': -20.5, 'Z_sup': 29.5},
                 6: {'Layer': 6, 'Ladders': 16, 'Sensors': 5, 'Phi_bins': 17,
                     'Phi_inf': -3.3, 'Phi_sup': 3.3, 'Z_bins': 5, 'Z_inf': -25.5, 'Z_sup': 36.5}}
    # Calls the defined functions
    for i in range(1, 7):
        if(i == 2):
            continue  # No overlaps for layer 2 in Phase3
        else:
            LayerSensorPlots(
                filename,
                lyr_num=VXDLayers[i]['Layer'],
                lddr_num=VXDLayers[i]['Ladders'],
                snsr_num=VXDLayers[i]['Sensors'])
            Median_plots_phi(
                filename,
                lyr_num=VXDLayers[i]['Layer'],
                phi_bins=VXDLayers[i]['Phi_bins'],
                phi_inf=VXDLayers[i]['Phi_inf'],
                phi_sup=VXDLayers[i]['Phi_sup'])
            Median_plots_z(
                filename,
                lyr_num=VXDLayers[i]['Layer'],
                z_bins=VXDLayers[i]['Z_bins'],
                z_inf=VXDLayers[i]['Z_inf'],
                z_sup=VXDLayers[i]['Z_sup'])
