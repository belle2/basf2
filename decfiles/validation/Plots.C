TCanvas *canvas= new TCanvas("ValidationCanvas","ValidationCanvas");
void B2VV();
void B2Vlnu();

void Plots(){

  canvas->Print("Validation.pdf[");
  //B2VV();
  B2Vlnu();
  canvas->Print("Validation.pdf]");

}

void B2VV(){

  TString infile (Form("../Validate_B2VV.root"));
  TChain* truthTree = new TChain("TruthTree");
  truthTree->AddFile(infile);
  TH1F* h_helicityAngleV1;
  TH1F* h_helicityAngleV2;
  TH1F* h_chiPlanarAngle;

  h_helicityAngleV1 = new TH1F("helicityAngleV1", ";cos#theta_{helicity}(V1);Arb. Norm.",50,-1.2,1.2);
  h_helicityAngleV2 = new TH1F("helicityAngleV2", ";cos#theta_{helicity}(V2);Arb. Norm.",50,-1.2,1.2);
  h_chiPlanarAngle  = new TH1F("chiPlanarAngle", ";#chi(Planar Angle);Arb. Norm.",50,-3.2,3.2);

  Float_t B_hel1;
  Float_t B_hel1;
  Float_t B_chiPlanar;

  truthTree->SetBranchAddress("B_hel1",&B_hel1);
  truthTree->SetBranchAddress("B_hel2",&B_hel2);
  truthTree->SetBranchAddress("B_chiPlanar",&B_chiPlanar);

  for(Int_t iloop=0; iloop<truthTree->GetEntries(); iloop++) {
    truthTree->GetEntry(iloop);
    h_helicityAngleV1->Fill(B_hel1);
    h_helicityAngleV2->Fill(B_hel2);
    h_chiPlanarAngle ->Fill(B_chiPlanar);
  }


  h_helicityAngleV1->Draw();
  canvas->Print("Validation.pdf","VV Hel Angle 1");
  h_helicityAngleV2->Draw();
  canvas->Print("Validation.pdf","VV Hel Angle 2");
  h_chiPlanarAngle->Draw();
  canvas->Print("Validation.pdf","VV Planar Angle");

}

void B2Vlnu(){

  TString infile (Form("Validate_B2Vlnu.root"));
  TChain* truthTree = new TChain("TruthTree");
  truthTree->AddFile(infile);
  TH1F* h_helicityAngleV1;
  TH1F* h_helicityAngleV2;
  TH1F* h_chiPlanarAngle;
  TH1F* h_q2;

  h_helicityAngleV1 = new TH1F("helicityAngleV1", ";cos#theta_{helicity}(V1);Arb. Norm.",50,-1.2,1.2);
  h_helicityAngleV2 = new TH1F("helicityAngleV2", ";cos#theta_{helicity}(V2);Arb. Norm.",50,-1.2,1.2);
  h_chiPlanarAngle  = new TH1F("chiPlanarAngle", ";#chi(Planar Angle);Arb. Norm.",50,-3.2,3.2);
  h_q2  = new TH1F("q2", ";q^{2};Arb. Norm.",50,-1,12);

  Float_t B_hel1;
  Float_t B_lephel;
  Float_t B_chiPlanar;
  Float_t B_q2;

  truthTree->SetBranchAddress("B_hel1",&B_hel1);
  truthTree->SetBranchAddress("B_lephel",&B_lephel);
  truthTree->SetBranchAddress("B_chiPlanar",&B_chiPlanar);
  truthTree->SetBranchAddress("B_q2",&B_q2);

  for(Int_t iloop=0; iloop<truthTree->GetEntries(); iloop++) {
    truthTree->GetEntry(iloop);
    h_helicityAngleV1->Fill(B_hel1);
    h_helicityAngleV2->Fill(B_lephel);
    h_chiPlanarAngle ->Fill(B_chiPlanar);
    h_q2 ->Fill(B_q2);
  }


  h_helicityAngleV1->Draw();
  canvas->Print("Validation.pdf","Vlnu Hel Angle 1");
  h_helicityAngleV2->Draw();
  canvas->Print("Validation.pdf","Vlnu Hel Angle 2");
  h_chiPlanarAngle->Draw();
  canvas->Print("Validation.pdf","Vlnu Planar Angle");
  h_q2->Draw();
  canvas->Print("Validation.pdf","Vlnu q2");

}
