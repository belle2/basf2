{
    //----------------------------------------
    //..Get the thetaID boundaries of each region from an output file from e1ProfilePlots.C
    TFile *fData = new TFile("output/e1ProfilePlots_exp12_4S.root");
    TH1F *lowerThetaID = (TH1F*)fData->Get("lowerThetaID");
    TH1F *upperThetaID = (TH1F*)fData->Get("upperThetaID");
    const int nThetaReg = lowerThetaID->GetNbinsX();
    const int nPhiReg = 16;
    const int nLocations = nThetaReg * nPhiReg; // location = iThetaReg*nPhiReg + iPhiReg; 
    
    double lowerEdgeThetaID[99];
    double upperEdgeThetaID[99];
    if(nThetaReg>99) {cout << "nThetaReg is greater than maximum 99 allowed: " << nThetaReg << endl; return; }
    for(int it = 0; it<nThetaReg; it++) {
        lowerEdgeThetaID[it] = lowerThetaID->GetBinContent(it+1);
        upperEdgeThetaID[it] = upperThetaID->GetBinContent(it+1);
    }

    //----------------------------------------
    //..thetaID, phiID, cellID
    int cellID = 0;
    int crysID = 0;
    int thetaID[8736], phiID[8736];
    const int nThetaID = 69;
    const short nCrystalsPerThetaID[nThetaID] = {
        48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 96, 96, 96, 96, 96, 64, 64, 64
    };

    for(int thID = 0; thID<nThetaID; thID++) {
        for(int phID = 0; phID<nCrystalsPerThetaID[thID]; phID++) {
            thetaID[crysID] = thID;
            phiID[crysID] = phID;
            crysID++;
        }
    }
    
    int firstCellIDPerThetaID[nThetaID];
    firstCellIDPerThetaID[0] = 1;
    for(int it = 1; it<nThetaID; it++) {
        firstCellIDPerThetaID[it] = firstCellIDPerThetaID[it-1] + nCrystalsPerThetaID[it-1];
    }
    
    //..Mark each thetaReg as forward (0), gap (1), barrel (2), gap (3), or backward (4)
    int eclRegion[99];
    int lastForward = 12;
    int lastBarrel = 58;
    int lastBackward = 68;
    for(int it = 0; it<nThetaReg; it++) {
        if( lowerEdgeThetaID[it]<=lastForward and upperEdgeThetaID[it]<=lastForward ) {
            eclRegion[it] = 0;
        } else if (lowerEdgeThetaID[it]<=lastForward and upperEdgeThetaID[it]<=lastBarrel) {
            eclRegion[it] = 1;
        } else if (lowerEdgeThetaID[it]<=lastBarrel and upperEdgeThetaID[it]<=lastBarrel) {
            eclRegion[it] = 2;
        } else if (lowerEdgeThetaID[it]<=lastBarrel and upperEdgeThetaID[it]<=lastBackward) {
            eclRegion[it] = 3;
        } else {
            eclRegion[it] = 4;
        }
    }

 
    //----------------------------------------
    //..Crystal locations, default geometry and six small variations
    float crysX[7][8736], crysY[7][8736], crysZ[7][8736], crysDirTheta, crysDirPhi;
    float crysTheta[7][8736], crysPhi[7][8736], crysR; // location in spherical coordinates (deg)

    //..And shift relative to nominal location
    float crysdTheta[7][8736], crysdPhi[7][8736];
    
    //..Default, three translations and three rotations
    TString fname[7] = {"defaultGeometry.txt", "shift_x_1mm.txt", "shift_y_1mm.txt", "shift_z_1mm.txt", "rotate_about_x_1mrad.txt", "rotate_about_y_1mrad.txt", "rotate_about_z_1mrad.txt"};
    for(int i = 0; i<7; i++) {
        TString input = "crystalGeometry/" + fname[i];
        std::ifstream defaultFile(input);
        std::cout << endl << "starting input file " << input << endl;
        for(int ic=1; ic<=8736; ic++) {
            crysID = ic - 1;
            defaultFile >> cellID >> crysX[i][crysID] >> crysY[i][crysID] >> crysZ[i][crysID] >> crysDirTheta >> crysDirPhi;
            
            //..Convert to spherical coordinates
            crysR = sqrt(crysX[i][crysID]*crysX[i][crysID] + crysY[i][crysID]*crysY[i][crysID] + crysZ[i][crysID]*crysZ[i][crysID]);
            crysTheta[i][crysID] = acos(crysZ[i][crysID] / crysR) * TMath::RadToDeg();
            crysPhi[i][crysID] = atan2(crysY[i][crysID],crysX[i][crysID]) * TMath::RadToDeg();
 
            //..Change in theta and phi relative to nominal
            crysdTheta[i][crysID] = crysTheta[i][crysID] - crysTheta[0][crysID];
            crysdPhi[i][crysID] = crysPhi[i][crysID] - crysPhi[0][crysID];
            
            if(ic%1000==0) {std::cout << ic << " " << cellID << " " << crysX[i][crysID] << " " << crysY[i][crysID] << " " << crysZ[i][crysID] << " " <<  crysdTheta[i][crysID] << " " << crysdPhi[i][crysID] << std::endl;}
        }
    }

    
    //----------------------------------------
    //..Define histograms of differences in each location
    TFile *fOutput = new TFile("output/readGeoFiles.root","recreate");
    TH2F *dTheta2DTemplate[6], *dPhi2DTemplate[6];
    TH1F *diff1DTemplate[6];
    TString moveType[6] = {"dx 1mm", "dy 1mm", "dz = 1mm", "rot about x 1mrad", "rot about y 1mrad", "rot about z 1mrad"};
    for(int im = 0; im<6; im++) {

        //..dPhi and dTheta binned by theta and phi region, 2D
        TString name = "dTheta2DTemplate_";
        name += im;
        TString title = "dTheta (deg) for " + moveType[im] + ";theta region;phi region";
        dTheta2DTemplate[im] = new TH2F(name, title, nThetaReg, 0, nThetaReg, nPhiReg, 0, nPhiReg);
        
        name = "dPhi2DTemplate_";
        name += im;
        title = "dPhi (deg) for " + moveType[im] + ";theta region;phi region";
        dPhi2DTemplate[im] = new TH2F(name, title, nThetaReg, 0, nThetaReg, nPhiReg, 0, nPhiReg);
        
        //..and flattened into a 1D plot. bins = 4 x locations; dTheta and dPhi first for mu- then mu+ (for ease of fitting)
        name = "diff1DTemplate_";
        name += im;
        title = "change in location (deg) for " + moveType[im] + ";1D Location";
        diff1DTemplate[im] = new TH1F(name, title, 4*nLocations, 0, 4*nLocations);
    }
    
    //..A histogram to store the eclRegion for each 1D location, and direction (theta or phi)
    TH1F *eclRegionOfLocation = new TH1F("eclRegionOfLocation", "ECL region of each location;1D Location", 4*nLocations, 0, 4*nLocations);
    TH1F *directionOfLocation = new TH1F("directionOfLocation", "direction of each location, 0=theta, 1=phi;1D Location", 4*nLocations, 0, 4*nLocations);

    //----------------------------------------
    //..Fill the four plots for each location and each movement
    int bin1D = 0;
    for(int itReg = 0; itReg<nThetaReg; itReg++) {
        int phiID0 = -1;
        for(int ipReg = 0; ipReg<nPhiReg; ipReg++){
            double dThetaAverage[6] = {};
            double dPhiAverage[6] = {};
            int nCrysInLoc = 0;
            
            //..Loop over the crystals in this location. Note there is 1 more crystal in phi than there are transitions
            for(int it = lowerEdgeThetaID[itReg]; it<= upperEdgeThetaID[itReg]; it++) {
                int phiIDperReg = nCrystalsPerThetaID[it]/nPhiReg;
                for(int ip = 0; ip<=phiIDperReg; ip++) {
                    int phiID = phiID0 + ipReg*phiIDperReg + ip;
                    if(phiID<0) {phiID += nCrystalsPerThetaID[it];}
                    int cellID = firstCellIDPerThetaID[it] + phiID;
                    
                    //..Six different templates (indexed by crysID = cellID-1). Note that im=0 is default geometry.
                    nCrysInLoc++;
                    for(int im = 0; im<6; im++) {
                        dThetaAverage[im] += crysdTheta[im+1][cellID-1];
                        dPhiAverage[im] += crysdPhi[im+1][cellID-1];
                    }
                }
            }

            //..Finished this location. Store the result in 2D and 1D histograms.
            //  We store the result twice for the 1D histogram, once for mu- and once for mu+.
            bin1D++;
            for(int im = 0; im<6; im++) {
                dThetaAverage[im] = dThetaAverage[im]/nCrysInLoc;
                dPhiAverage[im] = dPhiAverage[im]/nCrysInLoc;
                
                //..2D histogram (just for visualization)
                dTheta2DTemplate[im]->SetBinContent(itReg+1, ipReg+1, dThetaAverage[im]);
                dPhi2DTemplate[im]->SetBinContent(itReg+1, ipReg+1, dPhiAverage[im]);

                //..dTheta and dPhi for this location, mu-
                diff1DTemplate[im]->SetBinContent(bin1D, dThetaAverage[im]);
                diff1DTemplate[im]->SetBinError(bin1D, 0.);
                diff1DTemplate[im]->SetBinContent(bin1D+nLocations, dPhiAverage[im]);
                diff1DTemplate[im]->SetBinError(bin1D+nLocations, 0.);

                //..And same values offset by 2*nLocations for mu+
                diff1DTemplate[im]->SetBinContent(bin1D+2*nLocations, dThetaAverage[im]);
                diff1DTemplate[im]->SetBinError(bin1D,+2*nLocations, 0.);
                diff1DTemplate[im]->SetBinContent(bin1D+3*nLocations, dPhiAverage[im]);
                diff1DTemplate[im]->SetBinError(bin1D+3*nLocations, 0.);
            }
            
            //..Store the ECL region for these locations
            eclRegionOfLocation->SetBinContent(bin1D, eclRegion[itReg]);
            eclRegionOfLocation->SetBinError(bin1D, 0);
            eclRegionOfLocation->SetBinContent(bin1D+nLocations, eclRegion[itReg]);
            eclRegionOfLocation->SetBinError(bin1D+nLocations, 0);
            eclRegionOfLocation->SetBinContent(bin1D+2*nLocations, eclRegion[itReg]);
            eclRegionOfLocation->SetBinError(bin1D+2*nLocations, 0);
            eclRegionOfLocation->SetBinContent(bin1D+3*nLocations, eclRegion[itReg]);
            eclRegionOfLocation->SetBinError(bin1D+3*nLocations, 0);
            
            //..And whether it is dTheta (0) or dPhi (1)
            directionOfLocation->SetBinContent(bin1D, 0);
            directionOfLocation->SetBinError(bin1D, 0);
            directionOfLocation->SetBinContent(bin1D+nLocations, 1);
            directionOfLocation->SetBinError(bin1D+nLocations, 0);
            directionOfLocation->SetBinContent(bin1D+2*nLocations, 0);
            directionOfLocation->SetBinError(bin1D+2*nLocations, 0);
            directionOfLocation->SetBinContent(bin1D+3*nLocations, 1);
            directionOfLocation->SetBinError(bin1D+3*nLocations, 0);
        }
    }
    
    //----------------------------------------
    //..Finish up
    fOutput->Write();
    fOutput->Close();
}
