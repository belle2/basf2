//-----------------------------------------------------
//..Use muon pairs from muPairAlignment.py to fill TProfiles of energy
//  in the most energetic crystal (e1) vs location throughout the ECL, measured in
//  spherical coordinates theta and phi as determined by the tracking
//  system. Phi is extrapolated to the middle of the ECL crystals.

// Usage:
// root -q -b 'e1ProfilePlots.C("input_ntuples.root", "sample_name", "output_root_file.root")'

void e1ProfilePlots (TString input, TString sample, TString output) {
    
    //----------------------------------------------------------------
    //..Constants
    double zForward = 208.; // (cm) extrapolate tracks to this distance in z to get phi
    double zBackward = -114.; // (cm) extrapolate tracks to this distance in z to get phi
    double rBarrel = 137; // (cm) extrapolate tracks to this distance in R
    double muonIDCut = 0.9; // at least one track must satisfy this muon ID requirement
    
    //----------------------------------------------------------------
    //..Theta divisions. Boundaries are the centers of the specified thetaIDs. All angles in degrees.
    const int nThetaReg = 28;
    const int lowerEdgeThetaID[nThetaReg] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 55, 58, 59, 60, 61, 62, 63, 64};
    const int upperEdgeThetaID[nThetaReg] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 58, 59, 60, 61, 62, 63, 64, 65};
    const int nThetaID = 69;
    const double thetaIDCenter[nThetaID] = {13.18, 14.70, 16.25, 17.79, 19.26, 20.79, 22.29, 23.77, 25.24, 26.65, 28.06, 29.48, 30.87, 32.92, 34.27, 35.66, 37.10, 38.59, 40.14, 41.73, 43.37, 45.07, 46.83, 48.63, 50.49, 52.39, 54.37, 56.38, 58.44, 60.55, 62.73, 64.94, 67.19, 69.48, 71.83, 74.20, 76.61, 79.04, 81.52, 84.00, 86.50, 88.87, 91.13, 93.50, 96.00, 98.48, 100.96, 103.39, 105.80, 108.17, 110.52, 112.81, 115.06, 117.27, 119.45, 121.56, 123.62, 125.63, 127.59, 131.37, 133.55, 135.79, 138.06, 140.45, 142.92, 145.52, 148.15, 150.71, 153.36};
    double lowerEdgeTheta[nThetaReg], upperEdgeTheta[nThetaReg];
    
    //..Adjust the upper theta boundary to get an even number of histogram bins
    double forwardWidth = 0.01;
    double barrelWidth = 0.02;
    double backwardWidth = 0.02;
    double thetaBinWidth[nThetaReg];
    for(int it = 0; it<nThetaReg; it++) {
        thetaBinWidth[it] = barrelWidth;
        if(upperEdgeThetaID[it]<=13) {thetaBinWidth[it] = forwardWidth;}
        if(lowerEdgeThetaID[it]>=58) {thetaBinWidth[it] = backwardWidth;}
    }
        
    int thetaNBins[nThetaReg];
    for(int it = 0; it<nThetaReg; it++) {
        int thetaID = lowerEdgeThetaID[it];
        lowerEdgeTheta[it] = thetaIDCenter[thetaID];
        double dTheta = thetaIDCenter[upperEdgeThetaID[it]] - lowerEdgeTheta[it];
        thetaNBins[it] = (int)(dTheta/thetaBinWidth[it]);
        upperEdgeTheta[it] = lowerEdgeTheta[it] + thetaNBins[it]*thetaBinWidth[it];
    }

    //----------------------------------------------------------------
    //..16 divisions in phi, starting at -1/2 crystal
    const int nPhiReg = 16;
    const float phiRegWidth = 360./nPhiReg;
    const int nCrysPerID[nThetaID] ={48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 96, 96, 96, 96, 96, 64, 64, 64};
    double phiStart[nThetaReg];
    for(int it = 0; it<nThetaReg; it++) {
        int thetaID = lowerEdgeThetaID[it];
        phiStart[it] = -360./nCrysPerID[thetaID]/2.;
    }
    
    forwardWidth = 0.05;
    barrelWidth = 0.02;
    backwardWidth = 0.05;
    double phiBinWidth[nThetaReg];
    for(int it = 0; it<nThetaReg; it++) {
        phiBinWidth[it] = barrelWidth;
        if(upperEdgeThetaID[it]<=13) {phiBinWidth[it] = forwardWidth;}
        if(lowerEdgeThetaID[it]>=58) {phiBinWidth[it] = backwardWidth;}
    }
    
    int phiNBins[nThetaReg];
    for(int it = 0; it<nThetaReg; it++) {
        phiNBins[it] = (int)(phiRegWidth/phiBinWidth[it]);
    }
        
    //----------------------------------------------------------------
    //..TTree contents
    TChain *allNtuples = new TChain("ntuple");
    cout << "Input files = " << input << endl;
    allNtuples->Add(input);
    Long64_t nentries = allNtuples->GetEntries();
    cout << "nentries = " << nentries << endl;
    
    //..The variables in the ntuple
    double charge, muThetaLab, muPhiLab, bestMuonID, worstMuonID;
    double e1Uncorr, muptLab;
    allNtuples->SetBranchAddress("muThetaLab",&muThetaLab);
    allNtuples->SetBranchAddress("muPhiLab",&muPhiLab);
    allNtuples->SetBranchAddress("bestMuonID",&bestMuonID);
    allNtuples->SetBranchAddress("worstMuonID",&worstMuonID);
    allNtuples->SetBranchAddress("e1Uncorr",&e1Uncorr);
    allNtuples->SetBranchAddress("charge",&charge);
    allNtuples->SetBranchAddress("muptLab",&muptLab);

    
    //----------------------------------------------------------------
    //..Define histograms
    TString name, title;
    TFile *fout = new TFile(output,"recreate");
    TString cName[2] = { "negative", "positive"};

    //..e1 vs position, basic plots upon which the alignment is based.
    TProfile *e1VsTheta[nThetaReg][nPhiReg][2];
    TProfile *e1VsPhi[nThetaReg][nPhiReg][2];
    for(int it = 0; it<nThetaReg; it++) {
        TString thetaBin, phiBin;
        thetaBin.Form("%5.3f", thetaBinWidth[it]);
        phiBin.Form("%5.3f", phiBinWidth[it]);
        for(int ip = 0; ip<nPhiReg; ip++) {
            double phiLo = phiStart[it] + ip*phiRegWidth;
            double phiHi = phiLo + phiRegWidth;
            for(int ic = 0; ic<2; ic++) {
                
                //..Location of theta transition between thetaIDs in the endcap
                name = "e1VsTheta_";
                name += it;
                name += "_";
                name += ip;
                name += "_";
                name += ic;
                title = "e1 vs theta, thetaReg = ";
                title += it;
                title += ", phiReg = ";
                title += ip;
                title += ", " + cName[ic] + ", " + sample + ";theta (deg);e1 per " + thetaBin + " deg (GeV)";
                e1VsTheta[it][ip][ic] = new TProfile(name, title, thetaNBins[it], lowerEdgeTheta[it], upperEdgeTheta[it], -0.01, 1.);
                
                //..Location of the mechanical structure every 1/16th of the endcap in phi
                name = "e1VsPhi_";
                name += it;
                name += "_";
                name += ip;
                name += "_";
                name += ic;
                title = "e1 vs extrapolated phi, thetaReg = ";
                title += it;
                title += ", phiReg = ";
                title += ip;
                title += ", " + cName[ic] + ", " + sample + ";extrapolated phi (deg);e1 per " + phiBin + " deg (GeV)";
                e1VsPhi[it][ip][ic] = new TProfile(name, title, phiNBins[it], phiLo, phiHi, -0.01, 1.);
            }
        }
    }
                
    //..Check out muon ID as well
    TH1F *muonID = new TH1F("muonID", "best muon ID, tracks to be used in profile plots;best muon ID", 101, 0, 1.01);
    TH2F *muonIDvsTheta = new TH2F("muonIDvsTheta", "bestMuonID vs thetaReg;theta region;muonID", nThetaReg, 0, nThetaReg, 102, -0.01, 1.01);
    
    //** temp phi offset
    TH2F *offsetvsTheta = new TH2F("offsetvsTheta", "phiOffset vs thetaReg;theta region;phiOffset (deg)", nThetaReg, 0, nThetaReg, 100, 0, 10.);
    
    //..And store corresponding thetaIDs
    TH1F *lowerThetaID = new TH1F("lowerThetaID", "lower thetaID edge of each theta region;theta region", nThetaReg, 0, nThetaReg);
    TH1F *upperThetaID = new TH1F("upperThetaID", "upper thetaID edge of each theta region;theta region", nThetaReg, 0, nThetaReg);
    for(int it = 0; it<nThetaReg; it++) {
        lowerThetaID->SetBinContent(it+1, lowerEdgeThetaID[it]);
        lowerThetaID->SetBinError(it+1, 0.);
        upperThetaID->SetBinContent(it+1, upperEdgeThetaID[it]);
        upperThetaID->SetBinError(it+1, 0.);
    }

    
    
    //----------------------------------------------------------------
    //..Loop over all entries
    for (Long64_t i=0;i<nentries;i++) {
        allNtuples->GetEntry(i);
        if(i%100000==0) {cout << "event " << i << endl;}
        
        //..Must be in theta range of interest
        int iThetaReg = -1;
        for(int it = 0; it<nThetaReg; it++) {
            if(muThetaLab>=lowerEdgeTheta[it] and muThetaLab<upperEdgeTheta[it]) {
                iThetaReg = it;
                break;
            }
        }
        if(iThetaReg==-1) {continue;}
                
        //..Make sure this is a muon pair
        muonID->Fill(bestMuonID);
        muonIDvsTheta->Fill(iThetaReg+0.001, bestMuonID);
        if(bestMuonID<muonIDCut) {continue;}

        //----------------------------------------------------------------
        //..offset between phi cluster and phi track
        // radius of curvature = pt / (0.3 B)
        // sin(offset) = radius of ecl / (2 radius of curvature)
        //  = r_ecl / (444.444 pt) for B = 1.5T and r_ecl in cm
        // For endcaps, use r_ecl = z_ecl*tan(theta)
        double rECL = rBarrel;
        if(upperEdgeThetaID[iThetaReg]<=13) {rECL = zForward*tan(muThetaLab*TMath::DegToRad());}
        if(lowerEdgeThetaID[iThetaReg]>=58) {rECL = zBackward*tan(muThetaLab*TMath::DegToRad());}
        float phiOffset = 57.29578*asin(rECL/ (444.444 * muptLab));
        offsetvsTheta->Fill(iThetaReg+0.001, phiOffset);
        
        //..Extrapolated phi, between 0 and 360 deg, relative to phiStart for this theta region
        float phiExtrapolated = 360. - phiStart[iThetaReg] + muPhiLab - charge*phiOffset;
        if(phiExtrapolated>360.) {phiExtrapolated -= 360.;}
        
        //..Use this to get the phi region
        int iPhiReg = (int) (phiExtrapolated/phiRegWidth);
        
        //..And now add back phiStart to get the actual extrapolated phi
        phiExtrapolated += phiStart[iThetaReg];

        //..charge bin
        int iCharge = 0;
        if(charge>0) {iCharge = 1;}
        
        //..Energy in most energetic crystal
        double e1 = 0;
        if( !std::isnan(e1Uncorr) ) {e1 = e1Uncorr;}
        
        //..Fill the histograms
        e1VsTheta[iThetaReg][iPhiReg][iCharge]->Fill(muThetaLab, e1);
        e1VsPhi[iThetaReg][iPhiReg][iCharge]->Fill(phiExtrapolated, e1);
    }
    
    //----------------------------------------------------------------
    //..Write out
    fout->Write();
    fout->Close();
}
