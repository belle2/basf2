/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//----------------------------------------------------------------------
//..Read in histogram of difference in location in data and MC for many
//  locations in the ECL, and fit using six templates of changes due to
//  small translations or rotations of the ECL geometry.
//  18 parameters, six for the barrel and six for each endcap, treating each
//  as an independent rigid body.


//----------------------------------------------------------------------
//..Job parameters
TFile *fDifferences = new TFile("output/dataMinusMC_exp12_4S_mc13b_exp10.root");
TFile *fTemplates = new TFile("output/readGeoFiles.root");

TString sample = "exp12_4S_mc13b_exp10"; // histogram names and titles, root file

//..Systematic uncertainty added to dTheta and dPhi for each ecl region
//  order is theta for reg 0, phi for reg 0, theta for reg 1, phi for reg 1, ...
const double sysUnc[5][2] = {0.013, 0.039, 0., 0., 0.019, 0.021, 0., 0., 0.032, 0.036};

//..Geometry parameters used to generate the MC sample.
//  From ecl/data/crystal_placement_xxx.dat
// # crystalID alpha theta0 phi0 R theta1 phi1
// # a crystal is rotated around the axis
// (sin(theta0)*cos(phi0),sin(theta0)*sin(phi0),cos(theta0)) by the alpha angle
// # and then the center of the crystal is moved by the vector
// R*(sin(theta1)*cos(phi1),sin(theta1)*sin(phi1),cos(theta1))
// # R is in millimetres
const double geomPar[3][6] = {
    0, 0, 0, 1965, 0, 0, // forward: 1000 0 0 0 1965 0 0
    0, 0, 0, 0, 0, 0, // barrel: 1001 0 0 0 0 0 0
    0, 0, 0, 1020, TMath::Pi(), 0}; // backward: 1000 0 0 0 -1020 0 0
const int geomID[3] = {1000, 1001, 1000};


//----------------------------------------------------------------------
//..Fit function is just the sum of six templates, each corresponding
//  to a small translation or rotation of the ECL barrel

//..ECL regions: 0 = forward, 1 = gap, 2 = barrel, 3 = gap, 4 = backward
std::vector<int> eclRegion;
int regionToFit;
double templateDiff[6][1792];
std::vector<double> dataMCDiff;
double fitTemplates (double *x, double *par) {
    int ix = x[0];
    double y = 0.;
    if(eclRegion[ix]==regionToFit) {
        for (int im = 0; im<6; im++) {y += par[im]*templateDiff[im][ix];}
    } else {
        y = dataMCDiff[ix];
    }
    return y;
}

TString regionName[3] = {"forward", "barrel", "backward"};

//----------------------------------------------------------------------
//..Main
void fitDataMinusMC () {
    
    //----------------------------------------------------------------------
    //..Read in the various required histograms
    
    //..data minus MC vs location
    TH1F *dataMinusMC1D = (TH1F*)fDifferences->Get("dataMinusMC1D");
    
    //..templates for fitting
    TH1F *diff1DTemplate[6];
    for(int im = 0; im<6; im++) {
        TString name = "diff1DTemplate_";
        name += im;
        diff1DTemplate[im] = (TH1F*)fTemplates->Get(name);
    }
    
    //..ecl region and direction of each location
    TH1F *eclRegionOfLocation = (TH1F*)fTemplates->Get("eclRegionOfLocation");
    TH1F *directionOfLocation = (TH1F*)fTemplates->Get("directionOfLocation");
    
    //----------------------------------------------------------------------
    //..Convert these into vectors
    const int nLocations = eclRegionOfLocation->GetNbinsX();
    cout << "nLocations = " << nLocations << endl;
    
    for(int i = 0; i<nLocations; i++) {
        int bin = i+1;
        for(int im = 0; im<6; im++) {
            templateDiff[im][i] = diff1DTemplate[im]->GetBinContent(bin);
        }
        dataMCDiff.push_back(dataMinusMC1D->GetBinContent(bin));
    }
    
    std::vector<int> direction;
    for(int i = 0; i<nLocations; i++) {
        int bin = i+1;
        int intValue = (int)(eclRegionOfLocation->GetBinContent(bin) + 0.001);
        eclRegion.push_back(intValue);
        intValue = (int)(directionOfLocation->GetBinContent(bin) + 0.001);
        direction.push_back(intValue);
    }
    
    //----------------------------------------------------------------------
    //..Inflate uncertainties on data-mc differences by specified amount
    for(int i = 0; i<nLocations; i++) {
        int bin = i+1;
        double origUnc = dataMinusMC1D->GetBinError(bin);
        int reg = eclRegion[i];
        int dir = direction[i];
        double extraUnc = sysUnc[reg][dir];
        double newUnc = sqrt(origUnc*origUnc + extraUnc*extraUnc);
        dataMinusMC1D->SetBinError(bin, newUnc);
    }
    dataMinusMC1D->SetMarkerStyle(20);
    dataMinusMC1D->SetMarkerSize(0.2);
    dataMinusMC1D->Draw();
    
    //---------------------------------------------------------------------
    //..Histograms to store results of fits
    TString title = "Data - MC angular differences before fit;differences (deg)" + sample;
    TH1F *differencesBefore = new TH1F("differencesBefore", title, 100, -0.3, 0.3);

    title = "Data - MC angular differences after fit " + sample + ";differences (deg)";
    TH1F *differencesAfter = new TH1F("differencesAfter", title, 100, -0.3, 0.3);
    
    //---------------------------------------------------------------------
    //..Now fit the translation/rotation templates to the observed data-mc differences.
    //  Fit each piece of the ECL separately (cannot fit 18 parameters at once).
    
    //..Define the fit function
    TF1 *fitFunction = new TF1("fitFunction", fitTemplates, 0, 1792,6);
        
    //..Vector to store the fit function values for the overlay histogram
    std::vector<double> fitValue;
    for(int i = 0; i<nLocations; i++) {fitValue.push_back(dataMCDiff[i]);}
        
    for(int iecl = 0; iecl<3; iecl++) {
        regionToFit = 2*iecl;
        fitFunction->SetParNames("dx", "dy", "dz", "rotx", "roty", "rotz");
        fitFunction->SetParameters(0., 0., 0., 0., 0., 0.);
        for(int im = 0; im<6; im++) {fitFunction->SetParLimits(im, -4., 4.);}
        
        //..Perform the fit
        cout << endl << "--------------------------------------------------" << endl;
        cout << "Starting fit for " << regionName[iecl] << endl << endl;
        
        dataMinusMC1D->Fit(fitFunction, "0", "");
        
        //..Print the parameters in a nicer form
        cout << endl;
        for(int i = 0; i<6; i++) {
            printf("%4s %7.3f +/- %7.3f\n", fitFunction->GetParName(i), fitFunction->GetParameter(i), fitFunction->GetParError(i));
        }


         //---------------------------------------------------------------------
        //..Derive parameters of crystal_placement_xxx.dat from the fit parameters
        
        //..Translation
        TVector3 dxyz(fitFunction->GetParameter(0), fitFunction->GetParameter(1), fitFunction->GetParameter(2));
        printf("%4s %7.3f %2s\n", " dR", dxyz.Mag(), "mm");
        
        TVector3 xyzOrig(geomPar[iecl][3] * sin(geomPar[iecl][4]) * cos(geomPar[iecl][5]), geomPar[iecl][3] * sin(geomPar[iecl][4]) * sin(geomPar[iecl][5]), geomPar[iecl][3] * cos(geomPar[iecl][4]));
        TVector3 xyzNew = xyzOrig + dxyz;
        
        //..Rotation
        TVector3 dRot(fitFunction->GetParameter(3)/1000., fitFunction->GetParameter(4)/1000.,fitFunction->GetParameter(5)/1000.);
        printf("%4s %7.3f %4s\n\n", " rot", 1000*dRot.Mag(), "mrad");

        TVector3 rotOrig(geomPar[iecl][0] * sin(geomPar[iecl][1]) * cos(geomPar[iecl][2]), geomPar[iecl][0] * sin(geomPar[iecl][1]) * sin(geomPar[iecl][2]), geomPar[iecl][0] * cos(geomPar[iecl][1]));
        TVector3 rotNew = rotOrig + dRot;
        
        //..Parameters
        cout << "Parameters for ecl/data/crystal_" << regionName[iecl] << ".dat" << endl;
        printf("%10s %4d %9.6f %9.6f %9.6f %12.6f %9.6f %9.6f\n", "Original:", geomID[iecl],  geomPar[iecl][0], geomPar[iecl][1], geomPar[iecl][2], geomPar[iecl][3], geomPar[iecl][4], geomPar[iecl][5]);
        printf("%10s %4d %9.6f %9.6f %9.6f %12.6f %9.6f %9.6f\n", "New:", geomID[iecl], rotNew.Mag(), rotNew.Theta(), rotNew.Phi(), xyzNew.Mag(), xyzNew.Theta(), xyzNew.Phi());
        
        
        //---------------------------------------------------------------------
        //..Store the fit function for later use, differences before and after fit,
        //  and contributions to chi square.
        int ndof = -6;
        double myChisq = 0.;
        double dthetaChisq = 0.;
        double dphiChisq = 0.;
        for(int i = 0; i<nLocations; i++) {
            int bin = i+1;
            if(eclRegion[i] == regionToFit) {
                double xcenter = dataMinusMC1D->GetBinCenter(bin);
                fitValue[i] = fitFunction->Eval(xcenter);
                
                //..Differences before and after
                double before = dataMinusMC1D->GetBinContent(bin);
                double after = before - fitValue[i];
                differencesBefore->Fill(before);
                differencesAfter->Fill(after);
                
                //..chisq
                ndof++;
                double diffUnc = dataMinusMC1D->GetBinError(bin);
                double dchi = after/diffUnc;
                myChisq += dchi*dchi;
                if( direction[i]==0 ) {
                    dthetaChisq += dchi*dchi;
                } else {
                    dphiChisq += dchi*dchi;
                }
            }
        }
        
        //..Summarize chi square
        cout << endl << "myChisq = " << myChisq << " for " << ndof << " dof" << "; dTheta share = " << dthetaChisq << "; dPhi share = " << dphiChisq << endl << endl;

    }

    //---------------------------------------------------------------------
    //..Now make an overlay plot of the fit function cobbled together from the
    //  three different fits.
    TH1F *fitOverlay = new TH1F("fitOverlay", "Fit function", nLocations, 0, nLocations);
    for(int i = 0; i<nLocations; i++) {
        int bin = i+1;
        fitOverlay->SetBinContent(bin, fitValue[i]);
        fitOverlay->SetBinError(bin, 0);
    }
    dataMinusMC1D->Draw();
    fitOverlay->SetLineColor(kBlue);
    fitOverlay->Draw("same");
    
    //---------------------------------------------------------------------
    //..Just for presentation, store the fit result in 2D histograms
    TH2F *dataMinusMC2D_neg_Phi = (TH2F*)fDifferences->Get("dataMinusMC2D_neg_Phi");
    int nx = dataMinusMC2D_neg_Phi->GetNbinsX();
    int ny = dataMinusMC2D_neg_Phi->GetNbinsY();
    
    title = "dTheta fit result " + sample + ";theta region;phi region";
    TH2F *fitResult_Theta = new TH2F("fitResult_Theta", title, nx, 0, nx, ny, 0, ny);
    title = "dPhi fit result " + sample + ";theta region;phi region";
    TH2F *fitResult_Phi = new TH2F("fitResult_Phi", title, nx, 0, nx, ny, 0, ny);
    
    //..And fill
    int iloc = 0;
    int nOffset = nx*ny;
    for(int itReg = 1; itReg<=nx; itReg++) {
        for(int ipReg = 1; ipReg<=ny; ipReg++) {
            fitResult_Theta->SetBinContent(itReg, ipReg, fitValue[iloc]);
            fitResult_Phi->SetBinContent(itReg, ipReg, fitValue[iloc+nOffset]);
            iloc++;
        }
    }
    
    //---------------------------------------------------------------------
    //..Store the difference between the observed data-mc difference and the fit.
    //  Four plots: theta and phi each for mu- and mu+
    title = "Theta: data-mc diff minus fit result " + sample + " mu-;theta region;phi region";
    TH2F *fitDifference_Theta_neg = new TH2F("fitDifference_Theta_neg", title,  nx, 0, nx, ny, 0, ny);
    title = "Phi: data-mc diff minus fit result " + sample + " mu-;theta region;phi region";
    TH2F *fitDifference_Phi_neg = new TH2F("fitDifference_Phi_neg", title,  nx, 0, nx, ny, 0, ny);
    title = "Theta: data-mc diff minus fit result " + sample + " mu+;theta region;phi region";
    TH2F *fitDifference_Theta_pos = new TH2F("fitDifference_Theta_pos", title,  nx, 0, nx, ny, 0, ny);
    title = "Phi: data-mc diff minus fit result " + sample + " mu+;theta region;phi region";
    TH2F *fitDifference_Phi_pos = new TH2F("fitDifference_Phi_pos", title,  nx, 0, nx, ny, 0, ny);
    
    //..Fill
    iloc = 0;
    for(int itReg = 1; itReg<=nx; itReg++) {
        for(int ipReg = 1; ipReg<=ny; ipReg++) {
            fitDifference_Theta_neg->SetBinContent(itReg, ipReg, dataMCDiff[iloc]- fitValue[iloc]);
            fitDifference_Phi_neg->SetBinContent(itReg, ipReg, dataMCDiff[iloc+nOffset]- fitValue[iloc+nOffset]);
            fitDifference_Theta_pos->SetBinContent(itReg, ipReg, dataMCDiff[iloc+2*nOffset]- fitValue[iloc+2*nOffset]);
            fitDifference_Phi_pos->SetBinContent(itReg, ipReg, dataMCDiff[iloc+3*nOffset]- fitValue[iloc+3*nOffset]);
            iloc++;
        }
    }
    
    //---------------------------------------------------------------------
    //..difference after fit for three ECL regions separately
    TString regName[5] = {"foward", "forgap", "barrel", "backgap", "backward"};
    TH1F *differencesAfterReg[5];
    for(int iecl = 0; iecl<5; iecl++) {
        TString name = "differencesAfterReg_";
        name += regName[iecl];
        title = "Data - MC angular differences after fit " + regName[iecl] + ", " + sample + ";differences (deg)";
        differencesAfterReg[iecl] = new TH1F(name, title, 100, -0.3, 0.3);
    }
    
    //..Fill
    iloc = 0;
    for(int itReg = 1; itReg<=nx; itReg++) {
        for(int ipReg = 1; ipReg<=ny; ipReg++) {
            fitDifference_Theta_neg->SetBinContent(itReg, ipReg, dataMCDiff[iloc]- fitValue[iloc]);
            fitDifference_Phi_neg->SetBinContent(itReg, ipReg, dataMCDiff[iloc+nOffset]- fitValue[iloc+nOffset]);
            fitDifference_Theta_pos->SetBinContent(itReg, ipReg, dataMCDiff[iloc+2*nOffset]- fitValue[iloc+2*nOffset]);
            fitDifference_Phi_pos->SetBinContent(itReg, ipReg, dataMCDiff[iloc+3*nOffset]- fitValue[iloc+3*nOffset]);
            iloc++;
        }
    }

    for(int i = 0; i<nLocations; i++) {
        double after = dataMCDiff[i] - fitValue[i];
        differencesAfterReg[eclRegion[i]]->Fill(after);
    }

    //---------------------------------------------------------------------
    //..Write out some histograms
    TString outName = "output/fitDataMinusMC_" + sample + ".root";
    TFile *fout = new TFile(outName, "recreate");
    dataMinusMC1D->Write();
    fitOverlay->Write();
    differencesBefore->Write();
    differencesAfter->Write();
    fitResult_Theta->Write();
    fitResult_Phi->Write();
    fitDifference_Theta_neg->Write();
    fitDifference_Phi_neg->Write();
    fitDifference_Theta_pos->Write();
    fitDifference_Phi_pos->Write();
    differencesAfterReg[0]->Write();
    differencesAfterReg[2]->Write();
    differencesAfterReg[4]->Write();
    fout->Close();

}
