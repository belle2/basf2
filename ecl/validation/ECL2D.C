/* ECL2D.C 
 * ROOT macro for ECL validation plots 
 * Author: Benjamin Oberhof 
 * 2014
*/

/*
<header>
<input>ECL2DOutput.root</input>
<output>ECL2D.root</output>
<contact>ecl2ml@bpost.kek.jp</contact>
</header>
*/

void ECL2D()
{

  TFile* inputFile = TFile::Open("../ECL2DOutput.root");
  TTree* tree = (TTree*) inputFile->Get("m_tree");
  TFile* output = TFile::Open("ECL2D.root", "recreate");

  //gStyle->SetOptStat(0);

  TH2F* BDyz = new TH2F("BDyz", "Cluster position", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz1 = new TH2F("BDyz1", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz2 = new TH2F("BDyz2", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz3 = new TH2F("BDyz3", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz4 = new TH2F("BDyz4", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz5 = new TH2F("BDyz5", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  TH2F* BDyz6 = new TH2F("BDyz6", "", 200, -1.5, 2.5, 100, -1.5, 1.5);
  
  BDyz->GetXaxis()->SetTitle("x (m)");
  BDyz->GetYaxis()->SetTitle("y (m)");
  BDyz->GetListOfFunctions()->Add(new TNamed("Description","Cluster position in the ecl (upper part is phi<0, lower part phi>0)")); 
  BDyz->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 

  tree->Draw("(TMath::Tan(eclClusterTheta))*(1.9616):1.9616>>BDyz","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)&&(eclClusterPhi<0)");
  tree->Draw("1.250:(1.250*(TMath::Cos(eclClusterTheta))/((TMath::Sin(eclClusterTheta))))>>BDyz2","(eclClusterTheta>32.20*(3.1415/180))&&eclClusterTheta<(180-51.28)*(3.1415/180)&&(eclClusterPhi<0)");
  tree->Draw("(-TMath::Tan(eclClusterTheta))*(1.0216):-1.0216>>BDyz3","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)&&eclClusterPhi<(0)");
  tree->Draw("(-(TMath::Tan(eclClusterTheta))*(1.9616)):1.9616>>BDyz4","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)&&(eclClusterPhi>0)");
  tree->Draw("(-1.250):(1.250*(TMath::Cos(eclClusterTheta))/((TMath::Sin(eclClusterTheta))))>>BDyz5","(eclClusterTheta>32.20*(3.1415/180))&&eclClusterTheta<(180-51.28)*(3.1415/180)&&(eclClusterPhi>0)");
  tree->Draw("((TMath::Tan(eclClusterTheta))*(1.0216)):-1.0216>>BDyz6","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)&&(eclClusterPhi>0)");

  BDyz->Add(BDyz2, 1);
  BDyz->Add(BDyz3, 1);
  BDyz->Add(BDyz4, 1);
  BDyz->Add(BDyz5, 1);
  BDyz->Add(BDyz6, 1);
  BDyz->Draw("colz");
  BDyz->Write();

  TH2F* BDyz7 = new TH2F("BDyz7", "Cluster position FWD", 200, -2, 2, 200, -1.5, 1.5);
  TH2F* BDyz8 = new TH2F("BDyz8", "Cluster position BWD", 200, -2, 2, 200, -1.5, 1.5);

  BDyz7->GetXaxis()->SetTitle("x (m)");
  BDyz7->GetYaxis()->SetTitle("y (m)");
  BDyz7->GetListOfFunctions()->Add(new TNamed("Description","Cluster position in the FWD ecl")); 
  BDyz7->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz7->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("(2.402*TMath::Cos(eclClusterPhi)*TMath::Sin(eclClusterTheta)):(2.402*TMath::Sin(eclClusterPhi)*TMath::Sin(eclClusterTheta))>>BDyz7","TMath::Abs(eclClusterTheta)<31.36*(3.1415/180)");
  BDyz7->Draw("colz");
  BDyz7->Write();

  BDyz8->GetXaxis()->SetTitle("x (m)");
  BDyz8->GetYaxis()->SetTitle("y (m)");
  BDyz8->GetListOfFunctions()->Add(new TNamed("Description","Cluster position in the BWD ecl")); 
  BDyz8->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz8->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("(1.669*TMath::Cos(eclClusterPhi)*TMath::Sin(eclClusterTheta)):(1.669*TMath::Sin(eclClusterPhi)*TMath::Sin(eclClusterTheta))>>BDyz8","TMath::Abs(eclClusterTheta)>(180-48.5)*(3.1415/180)");
  BDyz8->Draw("colz");
  BDyz8->Write();

  TH2F* BDyz9 = new TH2F("BDyz9", "Cluster position barrel", 200, -2, 2, 200, -1.5, 1.5);
  BDyz9->GetXaxis()->SetTitle("x (m)");
  BDyz9->GetYaxis()->SetTitle("y (m)");
  BDyz9->GetListOfFunctions()->Add(new TNamed("Description","Cluster position in barrel ecl")); 
  BDyz9->GetListOfFunctions()->Add(new TNamed("Check","Distibution should not vary much"));
  BDyz9->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("(1.250*TMath::Cos(eclClusterPhi)):(1.250*TMath::Sin(eclClusterPhi))>>BDyz9","(32.20*(3.1415/180))<eclClusterTheta<(180-51.28)*(3.1415/180)");
  BDyz9->Draw("colz");
  BDyz9->Write();
  
  output->Close();
  delete output;
}
