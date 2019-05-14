import os
import ROOT
import numpy
import root_numpy
from array import array
from ROOT import TCanvas, TH2F, TH1F, TH1D, TDirectory, TLine


'''Takes the output of combined OverlapResiduals and HistoManager modules,
   as input, providess hit-maps for overlapping VXD hits in Layer:Sensor
   plots and computes statistics for 2D monitoring plots.'''

''' Function to compute the median of the projected DeltaResU(V)
distributions for each azimuthal overlap in DeltaResU(V) vs phi plots '''


def Median_plots_phi(filename, lyr_num, phi_bins, phi_inf, phi_sup):
    f = ROOT.TFile.Open(filename, 'read')
    ''' Access the TDirectory containing VXD histograms of residuals differences for overlapping hits '''
    mn = f.Get('Monitoring_VXDOverlaps')
    ''' Get 2D DeltaRes_u vs phi histograms stored in histofile.root '''
    h_PhiU = mn.Get('h_DeltaResUPhi_Lyr' + str(lyr_num))
    ''' Get 2D DeltaRes_v vs phi histograms stored in histofile.root '''
    h_PhiV = mn.Get('h_DeltaResVPhi_Lyr' + str(lyr_num))
    ''' Defines histograms of projected DeltaRes_u distributions '''
    h_UMedians = TH1F(
        'h_UMedians_Lyr' +
        str(lyr_num),
        'Layer' +
        str(lyr_num) +
        ': medians of #Deltares_{u} for each overlap',
        phi_bins,
        phi_inf,
        phi_sup)
    ''' Defines histograms of projected DeltaRes_u distributions '''
    h_VMedians = TH1F(
        'h_VMedians_Lyr' +
        str(lyr_num),
        'Layer' +
        str(lyr_num) +
        ': medians of #Deltares_{v} for each overlap',
        phi_bins,
        phi_inf,
        phi_sup)
    ''' Sets the number of resampled samples for bootstrap '''
    Nrs = 100
    ''' Initializes parameters to compute the median of ROOT TH1 '''
    q_U = array('d', [0])
    p_U = array('d', [0.5])
    q_V = array('d', [0])
    p_V = array('d', [0.5])
    ''' Lists storing medians of toy MC distributions for statistical bootstrap '''
    l_U_median = []
    l_V_median = []
    ''' Lists storing estimated medians of projected DeltaRes distribution for every azimuthal overlap '''
    l_U_median_pos = []
    l_V_median_pos = []
    ''' Plots for median '''
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
        '''If not existing, creates a dedicated folder '''
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


''' Function to compute the median of the projected DeltaResU(V)
 distributions for each sensor in DeltaResU(V) vs z plots '''


def Median_plots_z(filename, lyr_num, z_bins, z_inf, z_sup):
    f = ROOT.TFile.Open(filename, 'read')
    ''' Access the TDirectory containing VXD histograms of residuals differences for overlapping hits '''
    mn = f.Get('Monitoring_VXDOverlaps')
    ''' Get 2D DeltaRes_u vs z histograms stored in histofile.root '''
    h_ZU = mn.Get('h_DeltaResUz_Lyr' + str(lyr_num))
    ''' Get 2D DeltaRes_v vs z histograms stored in histofile.root '''
    h_ZV = mn.Get('h_DeltaResVz_Lyr' + str(lyr_num))
    ''' Defines histograms of projected DeltaRes_u distributions '''
    h_UMedians = TH1F('h_UMedians_Lyr' + str(lyr_num), 'Layer' + str(lyr_num) +
                      ': medians of #Deltares_{u} for each sensor', z_bins, z_inf, z_sup)
    ''' Defines histograms of projected DeltaRes_u distributions '''
    h_VMedians = TH1F('h_VMedians_Lyr' + str(lyr_num), 'Layer' + str(lyr_num) +
                      ': medians of #Deltares_{v} for each sensor', z_bins, z_inf, z_sup)
    ''' Sets the number of resampled samples for bootstrap '''
    Nrs = 100
    ''' Initializes parameters to compute the median of ROOT TH1 '''
    q_U = array('d', [0])
    p_U = array('d', [0.5])
    q_V = array('d', [0])
    p_V = array('d', [0.5])
    ''' Lists storing medians of toy MC distributions for statistical bootstrap '''
    l_U_median = []
    l_V_median = []
    ''' Lists storing estimated medians of projected DeltaRes distribution for every sensor along z '''
    l_U_median_pos = []
    l_V_median_pos = []
    ''' Plots for median '''
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
        '''If not existing, creates a dedicated folder '''
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


''' Creates and saves Layer.Sensor plots for overlapping hits hitmaps '''


def LayerSensorPlots(filename):
    f = ROOT.TFile.Open(filename, 'read')
    ''' Accesses the TDirectory containing VXD hit-maps for overlapping hits '''
    hm = f.Get('HitMaps_VXDOverlaps')
    ''' Creates Layer.Sensor-arranged plots for hit-maps from hits in VXD overlaps '''
    l_Lyr6 = []
    l_Fit_Lyr6 = []
    l_Lyr5 = []
    l_Fit_Lyr5 = []
    l_Lyr4 = []
    l_Fit_Lyr4 = []
    l_Lyr3 = []
    l_Fit_Lyr3 = []
    l_Lyr2 = []
    l_Fit_Lyr2 = []
    l_Lyr1 = []
    l_Fit_Lyr1 = []

    for i in range(1, 6):
        c_6 = TCanvas('c_6:' + str(i), 'Layer:Sensor = 6:' + str(i), 500, 700)
        c_6.Divide(4, 4)
        l_Lyr6.append(c_6)
        c_Fit_6 = TCanvas('c_Fit_6:' + str(i), 'Layer:Sensor = 6:' + str(i), 500, 700)
        c_Fit_6.Divide(4, 4)
        l_Fit_Lyr6.append(c_Fit_6)

    for i in range(1, 5):
        c_5 = TCanvas('c_5:' + str(i), 'Layer:Sensor = 5:' + str(i), 500, 700)
        c_5.Divide(4, 3)
        l_Lyr5.append(c_5)
        c_Fit_5 = TCanvas('c_Fit_5:' + str(i), 'Layer:Sensor = 5:' + str(i), 500, 700)
        c_Fit_5.Divide(4, 3)
        l_Fit_Lyr5.append(c_Fit_5)

    for i in range(1, 4):
        c_4 = TCanvas('c_4:' + str(i), 'Layer:Sensor = 4:' + str(i), 500, 700)
        c_4.Divide(5, 2)
        l_Lyr4.append(c_4)
        c_Fit_4 = TCanvas('c_Fit_4:' + str(i), 'Layer:Sensor = 4:' + str(i), 500, 700)
        c_Fit_4.Divide(5, 2)
        l_Fit_Lyr4.append(c_Fit_4)

    for i in range(1, 3):
        c_3 = TCanvas('c_3:' + str(i), 'Layer:Sensor = 3:' + str(i), 500, 700)
        c_3.Divide(7, 1)
        l_Lyr3.append(c_3)
        c_Fit_3 = TCanvas('c_Fit_3:' + str(i), 'Layer:Sensor = 3:' + str(i), 500, 700)
        c_Fit_3.Divide(7, 1)
        l_Fit_Lyr3.append(c_Fit_3)

    for i in range(1, 3):
        c_2 = TCanvas('c_2:' + str(i), 'Layer:Sensor = 2:' + str(i), 500, 700)
        c_2.Divide(6, 2)
        l_Lyr2.append(c_2)
        c_Fit_2 = TCanvas('c_Fit_2:' + str(i), 'Layer:Sensor = 2:' + str(i), 500, 700)
        c_Fit_2.Divide(6, 2)
        l_Fit_Lyr2.append(c_Fit_2)

    for i in range(1, 3):
        c_1 = TCanvas('c_1:' + str(i), 'Layer:Sensor = 1:' + str(i), 500, 700)
        c_1.Divide(4, 2)
        l_Lyr1.append(c_1)
        c_Fit_1 = TCanvas('c_Fit_1:' + str(i), 'Layer:Sensor = 1:' + str(i), 500, 700)
        c_Fit_1.Divide(4, 2)
        l_Fit_Lyr1.append(c_Fit_1)
    '''If not existing, creates a decicated folder for HitMaps'''
    if not os.path.exists('HitMaps_plots_Overlaps'):
        os.mkdir('HitMaps_plots_Overlaps')
    for j in range(1, 7):
        if j == 1:
            for i in range(1, 3):
                for k in range(1, 9):
                    histo = hm.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr1[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = hm.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr1[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr1[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Lyr1[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.pdf')
                l_Fit_Lyr1[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr1[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.pdf')
        if j == 2:
            for i in range(1, 3):
                for k in range(1, 13):
                    histo = hm.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr2[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = hm.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr2[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr2[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Lyr2[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.pdf')
                l_Fit_Lyr2[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr2[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.pdf')
        if j == 3:
            for i in range(1, 3):
                for k in range(1, 8):
                    histo = hm.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr3[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = hm.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr3[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr3[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Lyr3[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.pdf')
                l_Fit_Lyr3[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr3[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.pdf')
        if j == 4:
            for i in range(1, 4):
                for k in range(1, 11):
                    histo = hm.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr4[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = hm.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr4[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr4[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Lyr4[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.pdf')
                l_Fit_Lyr4[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr4[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.pdf')
        if j == 5:
            for i in range(1, 5):
                for k in range(1, 13):
                    histo = hm.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr5[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = hm.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr5[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr5[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Lyr5[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.pdf')
                l_Fit_Lyr5[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr5[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.pdf')
        if j == 6:
            for i in range(1, 6):
                for k in range(1, 17):
                    histo = hm.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr6[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = hm.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr6[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr6[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Lyr6[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Layer:Sensor_' + str(j) + str(i) + '.pdf')
                l_Fit_Lyr6[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr6[i - 1].SaveAs('HitMaps_plots_Overlaps/c_Fit_Layer:Sensor_' + str(j) + str(i) + '.pdf')
    return


'''Root output of module OverlapResiduals'''
filename = 'histofile.root'

if __name__ == "__main__":
    LayerSensorPlots(filename)
    Median_plots_phi(filename, lyr_num=1, phi_bins=8, phi_inf=-3.2, phi_sup=3.2)
    Median_plots_phi(filename, lyr_num=3, phi_bins=7, phi_inf=-3.0, phi_sup=3.0)
    Median_plots_phi(filename, lyr_num=4, phi_bins=10, phi_inf=-3.0, phi_sup=3.0)
    Median_plots_phi(filename, lyr_num=5, phi_bins=13, phi_inf=-3.2, phi_sup=3.2)
    Median_plots_phi(filename, lyr_num=6, phi_bins=17, phi_inf=-3.3, phi_sup=3.3)
    Median_plots_z(filename, lyr_num=1, z_bins=2, z_inf=-3.2, z_sup=5.9)
    Median_plots_z(filename, lyr_num=3, z_bins=2, z_inf=-9.5, z_sup=15.5)
    Median_plots_z(filename, lyr_num=4, z_bins=3, z_inf=-16.5, z_sup=21.5)
    Median_plots_z(filename, lyr_num=5, z_bins=4, z_inf=-20.5, z_sup=29.5)
    Median_plots_z(filename, lyr_num=6, z_bins=5, z_inf=-25.5, z_sup=36.5)
