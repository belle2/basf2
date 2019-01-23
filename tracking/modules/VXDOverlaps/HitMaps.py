import ROOT
from ROOT import TCanvas, TH2F


# Takes the output of module OverlapResiduals as input, and plots hit-maps for overlapping hits.
def Store_LayerSensor_plots(filename='VXDOverlappingHits.root'):

    f = ROOT.TFile.Open(filename, 'read')
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
        l_Lyr5.append(c_6)
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

    for j in range(1, 7):
        if j == 1:
            for i in range(1, 3):
                for k in range(1, 9):
                    histo = f.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr1[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = f.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr1[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr1[i - 1].SaveAs('c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr1[i - 1].SaveAs('c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
        if j == 2:
            for i in range(1, 3):
                for k in range(1, 13):
                    histo = f.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr2[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = f.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr2[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr2[i - 1].SaveAs('c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr2[i - 1].SaveAs('c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
        if j == 3:
            for i in range(1, 3):
                for k in range(1, 8):
                    histo = f.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr3[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = f.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr3[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr3[i - 1].SaveAs('c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr3[i - 1].SaveAs('c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
        if j == 4:
            for i in range(1, 4):
                for k in range(1, 11):
                    histo = f.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr4[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = f.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr4[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr4[i - 1].SaveAs('c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr4[i - 1].SaveAs('c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
        if j == 5:
            for i in range(1, 5):
                for k in range(1, 13):
                    histo = f.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr5[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = f.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr5[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr5[i - 1].SaveAs('c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr5[i - 1].SaveAs('c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
        if j == 6:
            for i in range(1, 6):
                for k in range(1, 17):
                    histo = f.Get('h_' + str(j) + str(k) + str(i))
                    l_Lyr6[i - 1].cd(k)
                    histo.Draw('COLZ')
                    histo_fit = f.Get('h_Fit_' + str(j) + str(k) + str(i))
                    l_Fit_Lyr6[i - 1].cd(k)
                    histo_fit.Draw('COLZ')
                l_Lyr6[i - 1].SaveAs('c_Layer:Sensor_' + str(j) + str(i) + '.root')
                l_Fit_Lyr6[i - 1].SaveAs('c_Fit_Layer:Sensor_' + str(j) + str(i) + '.root')
