/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//..For each location, offset MC to find best fit to data distribution.
//  Print out the resulting fit (data overlaid with shifted MC) and
//  record resulting difference in 2D and 1D plots as a function of location

{
    //----------------------------------------------------
    //..Job parameters
    TString dataInput = "output/e1ProfilePlots_exp12_4S.root";
    TString mcInput = "output/e1ProfilePlots_mc13b_exp10_recl.root";
    TString plotOutputDir = "plots/dataMinusMC/exp12_4S_mc13b_exp10_recl/"; // directory for pdf
    TString plotLabel = "exp12_4S_mc13b_exp10_recl"; // part of the name of the pdf and root files
    TString rootOutput = "output/dataMinusMC_" + plotLabel + ".root";
    
    const int nSearchMax = 10; // Shift mc by up to this many bins to look for best fit
    
    //----------------------------------------------------
    //..Set up
    //  Read in the data and MC transition plots
    TFile *fData = new TFile(dataInput);
    TFile *fMC = new TFile(mcInput);
    
    //..Get the thetaID boundaries of each region from an output file from e1ProfilePlots.C
    TH1F *lowerThetaID = (TH1F*)fData->Get("lowerThetaID");
    TH1F *upperThetaID = (TH1F*)fData->Get("upperThetaID");
    const int nThetaReg = lowerThetaID->GetNbinsX();
    const int nPhiReg = 16;
    const int nLocations = nThetaReg * nPhiReg; // location = iThetaReg*nPhiReg + iPhiReg;
    
    //..pdf files of fits
    gStyle->SetOptStat(0);
    TCanvas *myC = new TCanvas("myC","shift MC e1 profile plots");
    
    //----------------------------------------------------
    //..2D and 1D histograms of data minus MC differences in location.
    //  delta Theta and delta Phi for mu- and mu+, then folded into a single 1D hist.
    
    //..1D, and histogram of differences
    TString title = "data MC location difference " + plotLabel + ";1D location";
    TH1F *dataMinusMC1D = new TH1F("dataMinusMC1D", title, 4*nLocations, 0, 4*nLocations);
    
    //..2D
    TString charge[2] = {"neg", "pos"};
    TString coordinate[2] = {"Theta", "Phi"};
    TH2F *dataMinusMC2D[2][2];
    
    for(int icharge = 0; icharge<2; icharge++) {
        for(int icoord = 0; icoord<2; icoord++) {
            TString name = "dataMinusMC2D_" + charge[icharge] + "_" + coordinate[icoord];
            title = "data MC location difference " + charge[icharge] + " " + coordinate[icoord];
            title += " " + plotLabel + ";theta region;phi region";
            dataMinusMC2D[icharge][icoord] = new TH2F(name, title, nThetaReg, 0, nThetaReg, nPhiReg, 0, nPhiReg);
        }
    }
    
    //..Difference between mu- and mu+ determination of dTheta and dPhi
    TH2F *negPosDifference2D[2];
    TH1F *negPosDifference[2];
    for(int icoord = 0; icoord<2; icoord++) {
        TString name = "negPosDifference2D_" + coordinate[icoord];
        TString title = "Difference in mu- and mu+ determination of " + coordinate[icoord];
        title += " " + plotLabel + ";theta region;phi region";
        negPosDifference2D[icoord] = new TH2F(name, title, nThetaReg, 0, nThetaReg, nPhiReg, 0, nPhiReg);
        
        name = "negPosDifference_" + coordinate[icoord];
        title = "Difference in mu- and mu+ determination of " + coordinate[icoord];
        title += ";difference (deg)";
        negPosDifference[icoord] = new TH1F(name, title, 100, -0.1, 0.1);
    }
    
    //----------------------------------------------------
    //..Now loop over each of the nLocations histograms in order
    int bin1D = 0; // this is a TH1F bin, so it runs from 1 to 4*nLocations
    for(int icharge = 0; icharge<2; icharge++) {
        for(int icoord = 0; icoord<2; icoord++) {
            for(int itReg = 0; itReg<nThetaReg; itReg++) {
                for(int ipReg = 0; ipReg<nPhiReg; ipReg++) {
                    
                    //..Read in the data and mc histograms
                    TString name = "e1Vs" + coordinate[icoord] + "_";
                    name += itReg;
                    name += "_";
                    name += ipReg;
                    name += "_";
                    name += icharge;
                    TProfile *dataPlot = (TProfile*)fData->Get(name);
                    TProfile *mcPlot = (TProfile*)fMC->Get(name);
                    
                    //----------------------------------------------------
                    //..The data fit range includes all bins except the first and last nSearchMax
                    //  Find the fit range
                    const int nBins = dataPlot->GetNbinsX();
                    int firstDataBin = nSearchMax+1;
                    int lastDataBin = nBins - nSearchMax -1;
                    double fitRangeLow = dataPlot->GetBinLowEdge(firstDataBin);
                    double fitRangeHigh = dataPlot->GetBinLowEdge(lastDataBin+1);
                    cout << "fit range = bins " << firstDataBin << " to " << lastDataBin << ", " << fitRangeLow << " to " << fitRangeHigh << " degrees " << endl;
                    double degPerBin = (dataPlot->GetBinLowEdge(lastDataBin+1) - dataPlot->GetBinLowEdge(firstDataBin))/(1 + lastDataBin - firstDataBin);
                    double degOfFreedom  = lastDataBin - firstDataBin;
                    
                    //----------------------------------------------------
                    //..Store the data and MC in vectors for easier use.
                    std::vector<double> datae1;
                    std::vector<double> dataUnc;
                    std::vector<double> mce1;
                    std::vector<double> mcUnc;
                    for(int ibin = 1; ibin<=nBins; ibin++) {
                        datae1.push_back(dataPlot->GetBinContent(ibin));
                        dataUnc.push_back(dataPlot->GetBinError(ibin));
                        
                        mce1.push_back(mcPlot->GetBinContent(ibin));
                        mcUnc.push_back(mcPlot->GetBinError(ibin));
                    }
                    
                    //----------------------------------------------------
                    //..Calculate and record chi square for each mc offset
                    std::vector<double> chiVsOffset;
                    std::vector<int> offsetInBins;
                    for(int ishift = -nSearchMax; ishift<=nSearchMax; ishift++) {
                        double chisq = 0.;
                        for(int ibin = firstDataBin; ibin<=lastDataBin; ibin++) {
                            int iMC = ibin + ishift;
                            double sigma2 = dataUnc[ibin]*dataUnc[ibin] + mcUnc[iMC]*mcUnc[iMC];
                            double diff2 = (datae1[ibin]-mce1[iMC])*(datae1[ibin]-mce1[iMC]);
                            double dchi = 0.;
                            if(sigma2>0.) {dchi = diff2 / sigma2;}
                            chisq += dchi;
                        }
                        chiVsOffset.push_back(chisq);
                        offsetInBins.push_back(ishift);
                    }
                    
                    
                    //----------------------------------------------------
                    //..Find and record minimum and corresponding shift
                    double minChiSq = 99999.;
                    int nCases = chiVsOffset.size();
                    int minOffset = -1;
                    for(int i = 0; i<nCases; i++) {
                        if(chiVsOffset[i]<minChiSq) {
                            minChiSq = chiVsOffset[i];
                            minOffset = i;
                        }
                    }
                    
                    //----------------------------------------------------
                    //..Now scale chisquare to 1 per DOF, and record scaled chi in histogram
                    TH1F *chisqPlot = new TH1F("chisqPlot", "Chi square vs offset;offset (bins)", nCases, offsetInBins[0]-0.5, offsetInBins[nCases-1]+0.5);
                    double scaleFactor = minChiSq / degOfFreedom;
                    for(int i = 0; i<nCases; i++) {
                        chisqPlot->SetBinContent(i+1, chiVsOffset[i] / scaleFactor);
                        chisqPlot->SetBinError(i+1, 10.);
                    }
                    
                    
                    //----------------------------------------------------
                    //..Fit chi sq vs offset with a quadratic to get better estimate of minimum
                    double lowX = offsetInBins[minOffset] - 2.5; // fit 5 bins
                    double highX = offsetInBins[minOffset] + 2.5;
                    
                    //..Check that we are not too close to the edge before fitting
                    if(lowX<offsetInBins[0]-0.5 or highX>offsetInBins[nCases-1]+0.5 ) {
                        cout << "*** failed fit to chisq distribution. lowX, highX = " << lowX << " " << highX << endl;
                        break;
                    }
                    chisqPlot->Fit("pol2", "", "", lowX, highX);
                    TF1 *func = (TF1*)chisqPlot->GetFunction("pol2");
                    
                    //..Find the minimum of the quadratic, plus corresponding offset in bins
                    double minFitChiSq = func->GetMinimum(lowX, highX);
                    double offsetBins = func->GetMinimumX(lowX, highX);
                    
                    //..Look for scaled chisq to increase by 1 to estimate uncertainty in bins
                    double targetChiSq = minFitChiSq + 1.;
                    double targetX = func->GetX(targetChiSq, offsetBins, highX);
                    double uncertaintyBins = targetX - offsetBins;
                    
                    //..And convert to degrees
                    double dataMinusMC = -offsetBins * degPerBin;
                    double uncertainty = uncertaintyBins*degPerBin;
                    
                    
                    //----------------------------------------------------
                    //..Record data-mc difference versus location
                    bin1D++;
                    dataMinusMC2D[icharge][icoord]->SetBinContent(itReg+1, ipReg+1, dataMinusMC);
                    dataMinusMC1D->SetBinContent(bin1D, dataMinusMC);
                    dataMinusMC1D->SetBinError(bin1D, uncertainty);
                    
                    
                    //----------------------------------------------------
                    //..Create shifted mc plot and overlay on data to visual fit quality
                    TString mcPlotName = mcPlot->GetName();
                    TString newname = mcPlotName + "_shifted";
                    TString title = mcPlot->GetTitle();
                    double xMin = mcPlot->GetXaxis()->GetXmin();
                    double xMax = mcPlot->GetXaxis()->GetXmax();
                    TH1F *mcShifted = new TH1F(newname, title, nBins, xMin, xMax);
                    
                    //..Figure out how many bins to shift the mc plot, and in what direction
                    double absShiftBins = abs(dataMinusMC)/degPerBin;
                    int isign = 1;
                    if(dataMinusMC>0) {isign = -1;}
                    int ishift = (int)absShiftBins;
                    double alphaBin = 1 + ishift - absShiftBins;
                    
                    //..And fill with the shifted values
                    for(int ibin = 1; ibin<=nBins; ibin++) {
                        int i1 = ibin + isign*ishift;
                        int i2 = i1 + isign;
                        if(i1>=1 and i2>=1 and i1<=nBins and i2<=nBins){
                            double y1 = mcPlot->GetBinContent(i1);
                            double y2 = mcPlot->GetBinContent(i2);
                            double y1Unc = mcPlot->GetBinError(i1);
                            double y2Unc = mcPlot->GetBinError(i2);
                            mcShifted->SetBinContent(ibin, alphaBin*y1 + (1-alphaBin)*y2);
                            mcShifted->SetBinError(ibin, alphaBin*y1Unc + (1-alphaBin)*y2Unc);
                        }
                    }
                    
                    //----------------------------------------------------
                    //..Plot data overlaid with shifted and unshifted MC
                    double ymin = 0.;
                    double ymax = 0.23;
                    dataPlot->GetYaxis()->SetRangeUser(ymin,ymax);
                    dataPlot->Draw();
                    
                    //..overlay the original mc (without error bars)
                    mcPlot->SetLineColor(kWhite);
                    mcPlot->SetMarkerColor(kGreen);
                    mcPlot->SetMarkerStyle(20);
                    mcPlot->SetMarkerSize(0.3);
                    mcPlot->Draw("same");
                    
                    //..Shifted MC
                    mcShifted->SetLineColor(kRed);
                    mcShifted->SetMarkerColor(kRed);
                    mcShifted->SetMarkerStyle(20);
                    mcShifted->SetMarkerSize(0.3);
                    mcShifted->Draw("same");
                    
                    //..Write the data-MC difference on the plot
                    TString fitResult, fitUncertainty, label;
                    fitResult.Form("%0.4f", dataMinusMC);
                    fitUncertainty.Form("%0.4f", uncertainty);
                    label = "data-MC = " + fitResult + " +/- " + fitUncertainty + " deg";
                    TText *tlabel = new TText(0.9*xMin+0.1*xMax, 0.9*ymin+0.1*ymax, label);
                    tlabel->SetTextSize(0.03);
                    tlabel->Draw();
                    
                    //..Print out plot with overlay
                    TString out = plotOutputDir + name + "_" + plotLabel + ".pdf";
                    myC->Print(out);
                }
            }
        }
    }
    
    //----------------------------------------------------
    //..Now record difference between mu- and mu+ determination of location differences
    for(int icoord = 0; icoord<2; icoord++) {
        for(int itReg = 0; itReg<nThetaReg; itReg++) {
            for(int ipReg = 0; ipReg<nPhiReg; ipReg++) {
                double negDiff =                     dataMinusMC2D[0][icoord]->GetBinContent(itReg+1, ipReg+1);
                double posDiff =                     dataMinusMC2D[1][icoord]->GetBinContent(itReg+1, ipReg+1);
                negPosDifference2D[icoord]->SetBinContent(itReg+1, ipReg+1, posDiff-negDiff);
                negPosDifference[icoord]->Fill(posDiff-negDiff);
            }
        }
    }
    
    
    
    //----------------------------------------------------
    //..Write out summary histograms
    TFile *fout = new TFile(rootOutput,"recreate");
    fout->cd();
    for(int icoord = 0; icoord<2; icoord++) {
        for(int icharge = 0; icharge<2; icharge++) {
            dataMinusMC2D[icharge][icoord]->Write();
        }
        
        negPosDifference2D[icoord]->Write();
        negPosDifference[icoord]->Write();
    }
    dataMinusMC1D->Write();
    fout->Close();
}
