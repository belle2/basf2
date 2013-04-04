// This macro evaluates cluster size in sensor set by the "condition" variable below
// run by starting ROOT and typing ".x ClusterSize.cpp"


{


    gROOT->Reset();

    TFile *file = new TFile("TBSimulationOutput.root");
    TTree *tree = (TTree*) file.Get("tree");
    
    double pxd_height = 0.7;
    double svd_height = 3.0;

    std::string sensors[6];
    sensors[0]="8480";
    sensors[1]="8512";
    sensors[2]="8544";
    sensors[3]="8576";
    sensors[4]="8608";
    sensors[5]="8640";
    
    TCanvas *Cpsa = new TCanvas("Primary/Secondary/All particles", "Cpsa", 1200, 600);
    Cpsa->Divide(6, 2);
    
    for(int i=0;i<6;i++){
    gStyle->SetHistLineColor(kBlack);



    Cpsa->cd(i+1);

    //Ha = new TH1F("Ha", "All", 200, -4, 4);
    //Hp->GetXaxis()->SetTitle("Residual [\\mum]");
    //Hp->GetYaxis()->SetTitle("Entries");    
    if (i<2){
    Hp = new TH1F("Hp", "Primary", 200, -pxd_height, pxd_height);
    Hs = new TH1F("Hs", "Secondary", 200, -pxd_height, pxd_height);
    }else{
    Hp = new TH1F("Hp", "Primary", 200, -svd_height, svd_height);
    Hs = new TH1F("Hs", "Secondary", 200, -svd_height, svd_height);        
    }

    std::string cond1 = std::string("VXDHits.m_fromPrimary&&VXDHits.m_sensorID==" + std::string(sensors[i]));
    tree->Draw("VXDHits.m_v>>Hp",cond1.c_str());    
    
    gStyle->SetHistFillColor(kRed);
    Hp->UseCurrentStyle();
    Hp->SetStats(kFALSE);
    
  

    std::string cond2 = std::string("!VXDHits.m_fromPrimary&&VXDHits.m_sensorID==" + std::string(sensors[i]));
    tree->Draw("VXDHits.m_v>>Hs",cond2.c_str());    
    
    gStyle->SetHistFillColor(kYellow);
    Hs->UseCurrentStyle();
    Hs->SetStats(kFALSE);  
       
    Hp->Draw("");
    gPad->Update();      
    Hs->Draw("same");
  
    leg = new TLegend(0.0, 0.9, 1.0, 1.0);
    leg->SetTextSize(0.05);    
    //leg->AddEntry("Ha", "All", "f");
    char legCap[20];
    sprintf(legCap, "Primary %d", Hp->GetEntries());
    leg->AddEntry("Hp", legCap, "f");
    double np = Hp->GetEntries();
    double ns = Hs->GetEntries();
    
    double percents = ns/np*100;
    //sprintf(legCap, "Secondary %d = %4.2f %%", Hs->GetEntries(), percents);
    sprintf(legCap, "Secondary %d", Hs->GetEntries());
    
    
    leg->AddEntry("Hs", legCap, "f");
    leg->Draw();
    
    Cpsa->cd(i+1+6);
    if (i<2){
        Ha = new TH1F("Ha", "All", 200, -pxd_height, pxd_height);
    }else{
        Ha = new TH1F("Ha", "All", 200, -svd_height, svd_height);
    }
    std::string cond3 = std::string("VXDHits.m_sensorID==" + std::string(sensors[i]));
    tree->Draw("VXDHits.m_v>>Ha",cond3.c_str());    
    
    gStyle->SetHistFillColor(kWhite);
    Ha->UseCurrentStyle();
    //Ha->SetStats(kFALSE);  
     
    Ha->Draw("");    
    
    Cpsa->cd(i+1);
}
}
