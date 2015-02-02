TCanvas *canvas= new TCanvas("ValidationCanvas","ValidationCanvas");
void B2VV();
void B2Vlnu();

void Plots(){

  canvas->Print("Validation.pdf[");
  B2VV();
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

  Float_t B_rho0_pi0B_rho0_pi1_hel;
  Float_t B_rho_pi0B_rho_pi_hel;
  Float_t B_rho0_pi0_chiPlanar;

  truthTree->SetBranchAddress("B_rho0_pi0B_rho0_pi1_hel",&B_rho0_pi0B_rho0_pi1_hel);
  truthTree->SetBranchAddress("B_rho_pi0B_rho_pi_hel",&B_rho_pi0B_rho_pi_hel);
  truthTree->SetBranchAddress("B_rho0_pi0_chiPlanar",&B_rho0_pi0_chiPlanar);

  for(Int_t iloop=0; iloop<truthTree->GetEntries(); iloop++) {
    truthTree->GetEntry(iloop);
    h_helicityAngleV1->Fill(B_rho0_pi0B_rho0_pi1_hel);
    h_helicityAngleV2->Fill(B_rho_pi0B_rho_pi_hel);
    h_chiPlanarAngle ->Fill(B_rho0_pi0_chiPlanar);
  }


  h_helicityAngleV1->Draw();
  canvas->Print("Validation.pdf","VV Hel Angle 1");
  h_helicityAngleV2->Draw();
  canvas->Print("Validation.pdf","VV Hel Angle 2");
  h_chiPlanarAngle->Draw();
  canvas->Print("Validation.pdf","VV Planar Angle");

}

void B2Vlnu(){

  TString infile (Form("../Validate_B2Vlnu.root"));
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

  Float_t B_DST0_D0B_DST0_pi0_hel;
  Float_t B_eB_nu_e_hel;
  Float_t B_DST0_D0_chiPlanar;
  Float_t B_e_q2;

  truthTree->SetBranchAddress("B_DST0_D0B_DST0_pi0_hel",&B_DST0_D0B_DST0_pi0_hel);
  truthTree->SetBranchAddress("B_eB_nu_e_hel",&B_eB_nu_e_hel);
  truthTree->SetBranchAddress("B_DST0_D0_chiPlanar",&B_DST0_D0_chiPlanar);
  truthTree->SetBranchAddress("B_e_q2",&B_e_q2);

  for(Int_t iloop=0; iloop<truthTree->GetEntries(); iloop++) {
    truthTree->GetEntry(iloop);
    h_helicityAngleV1->Fill(B_DST0_D0B_DST0_pi0_hel);
    h_helicityAngleV2->Fill(B_eB_nu_e_hel);
    h_chiPlanarAngle ->Fill(B_DST0_D0_chiPlanar);
    h_q2 ->Fill(B_e_q2);
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
