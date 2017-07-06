
/*
<header>
<input>../1730822030.ntup.root</input>
<output>1730822030_Validation.root</output>
<contact>Suxian Li; 13191004@buaa.edu.cn</contact>
</header>
*/

void plot_1Smumu(TFile* pfile, TTree* ptree, TFile* outputFile){

   TString pidCut1 ("Upsilon3S_chi_b12P_Upsilon_mu0_PIDmu > 0.01 && Upsilon3S_chi_b12P_Upsilon_mu1_PIDmu >0.01 ");
   
   TH1F *h_M_Y1S_mumu = new TH1F ("h_M_Y1S_mumu","M(1S2mumu))",100,9.36,9.56);
   ptree->Project("h_M_Y1S_mumu","Upsilon3S_chi_b12P_Upsilon_M",pidCut1);
   h_M_Y1S_mumu->GetXaxis()->SetTitle("M_{#mu^{+}#mu^{-}} (GeV/c^{2})");
  
   TH1F *h_M_chi_b12P_1Smumu = new TH1F("h_M_chi_b12P_1Smumu","M(chi_b1(2P)_1Smumu)",140,10.18,10.32);
   ptree->Project("h_M_chi_b12P_1Smumu","Upsilon3S_chi_b12P_M",pidCut1);
   h_M_chi_b12P_1Smumu->GetXaxis()->SetTitle("M_{#gamma#Upsilon(1S)} (Gev/c^{2})");

   outputFile->cd();
   
   h_M_Y1S_mumu->Write();
   h_M_chi_b12P_1Smumu->Write();
 
}

void plot_1See(TFile* pfile, TTree* ptree, TFile* outputFile){
   
   TString pidCut2 ("Upsilon3S_chi_b12P_Upsilon_e0_PIDe >0.01 && Upsilon3S_chi_b12P_Upsilon_e1_PIDe > 0.01");
  
   TH1F* h_M_Y1S_ee = new TH1F("h_M_Y1S_ee","M(Y1S2ee)",100,9.36,9.56);
   ptree->Project("h_M_Y1S_ee","Upsilon3S_chi_b12P_Upsilon_M",pidCut2);
   h_M_Y1S_ee->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (Gev/c^{2})");

   TH1F* h_M_chi_b12P_1See = new TH1F("h_M_chi_b12P_1See","M(chi_b1(2P)_1See)",140,10.18,10.32);
   ptree->Project("h_M_chi_b12P_1See","Upsilon3S_chi_b12P_M",pidCut2);
   h_M_chi_b12P_1See->GetXaxis()->SetTitle("M_{#gamma#Upsilon(1S)} (Gev/c^{2})");
   
   outputFile->cd();
    
   h_M_Y1S_ee->Write();
   h_M_chi_b12P_1See->Write();

}

void plot_2Smumu(TFile* pfile,TTree *ptree, TFile* outputFile){

   TString pidCut3 ("Upsilon3S_chi_b12P_Upsilon2S_mu0_PIDmu > 0.01 && Upsilon3S_chi_b12P_Upsilon2S_mu1_PIDmu >0.01");
  
   TH1F* h_M_Y2S_mumu = new TH1F("h_M_Y2S_mumu","M(Y2S2mumu)",100,9.9232,10.1232);
   ptree->Project("h_M_Y2S_mumu","Upsilon3S_chi_b12P_Upsilon2S_M",pidCut3);
   h_M_Y2S_mumu->GetXaxis()->SetTitle("M_{#mu^{+}#mu^{-}} (GeV/c^{2})");

   TH1F* h_M_chi_b12P_2Smumu = new TH1F("h_M_chi_b12P_2Smumu","M(chi_b1(2P)_2Smumu)",140,10.18,10.32);
   ptree->Project("h_M_chi_b12P_2Smumu","Upsilon3S_chi_b12P_M",pidCut3);
   h_M_chi_b12P_2Smumu->GetXaxis()->SetTitle("M_{#gamma#Upsilon(2S)} (Gev/c^{2})");

   outputFile->cd();

   h_M_Y2S_mumu->Write();
   h_M_chi_b12P_2Smumu->Write();

}

void plot_2See(TFile* pfile,TTree *ptree, TFile* outputFile){

   TString pidCut4 ("Upsilon3S_chi_b12P_Upsilon2S_e0_PIDe > 0.01 && Upsilon3S_chi_b12P_Upsilon2S_e1_PIDe > 0.01");
   
   TH1F* h_M_Y2S_ee = new TH1F("h_M_Y2S_ee","M(Y2S2ee)",100,9.9232,10.1232);
   ptree->Project("h_M_Y2S_ee","Upsilon3S_chi_b12P_Upsilon2S_M",pidCut4);
   h_M_Y2S_ee->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (Gev/c^{2})");

   TH1F* h_M_chi_b12P_2See = new TH1F("h_M_chi_b12P_2See","M(chi_b1(2P)_2See)",140,10.18,10.32);
   ptree->Project("h_M_chi_b12P_2See","Upsilon3S_chi_b12P_M",pidCut4);
   h_M_chi_b12P_2See->GetXaxis()->SetTitle("M_{#gamma#Upsilon(2S)} (Gev/c^{2})");

   outputFile->cd();

   h_M_Y2S_ee->Write();
   h_M_chi_b12P_2See->Write();

}
void test2_1730822030(){

   TString inputfile("../1730822030.ntup.root");

   TFile* sample = new TFile(inputfile);
   TTree* tree1 = (TTree*)sample->Get("ntuple_1smumu");
   TTree* tree2 = (TTree*)sample->Get("ntuple_1see");  
   TTree* tree3 = (TTree*)sample->Get("ntuple_2smumu"); 
   TTree* tree4 = (TTree*)sample->Get("ntuple_2see");
 
   TFile* outputFile = new TFile("1730822030_Validation.root","RECREATE");
   
   plot_1Smumu(sample,tree1,outputFile);
   plot_1See(sample,tree2,outputFile);
   plot_2Smumu(sample,tree3,outputFile);   
   plot_2See(sample,tree4,outputFile);
  
   outputFile->Close();



}
