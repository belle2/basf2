#define ReconW_cxx
#include "ReconW.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>
#include <TH2F.h>
#include <iostream>
void ReconW::Loop()
{
//   In a ROOT session, you can do:
//      root> .L ReconW.C
//      root> ReconW t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
  // definition of the histogram
  TH1F *HistW = new TH1F("HistW","",100,0.0, 5.);
  TH2F *HistW2 = new TH2F("HistW2","",100,0.0, 5., 20, 0., 1.0);


   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
      TLorentzVector P4K1(MCParticles_m_momentum_x[0],
			  MCParticles_m_momentum_y[0],
			  MCParticles_m_momentum_z[0],
			  MCParticles_m_energy[0]);
      TLorentzVector P4K2(MCParticles_m_momentum_x[1],
			  MCParticles_m_momentum_y[1],
			  MCParticles_m_momentum_z[1],
			  MCParticles_m_energy[1]);
      HistW->Fill((P4K1+P4K2).Mag());

      //Aproximate scattring angle
      TLorentzVector GGsys=P4K1+P4K2;    
      TLorentzVector P4K1CM = P4K1;
      P4K1CM.Boost(-GGsys.X()/GGsys.Mag(), -GGsys.Y()/GGsys.Mag(), 
           -GGsys.Z()/GGsys.Mag());
      HistW2->Fill((P4K1+P4K2).Mag(), abs(P4K1CM.CosTheta()));
   }
   HistW->Draw();
   HistW2->Draw("COLZ");
}
