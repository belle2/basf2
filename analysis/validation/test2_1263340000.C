/*
<header>
<input>../1263340000.ntup.root</input>
<output>1263340000_Validation.root</output>
<contact> Racha Cheaib, rcheaib@olemiss.edu, Mario Merola, mario.merola@na.infn.it, Sourav Dey, souravdey@tauex.tau.ac.il</contact>
<interval>nightly</interval>
</header>
*/
////////////////////////////////////////////////////////////
//
// test2_1163350000.C
//
// Constributor: Racha Cheaib
// March 2, 2018
//
////////////////////////////////////////////////////////////

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"

const char *contact = "rcheaib@olemiss.edu, mario.merola@na.infn.it, souravdey@tauex.tau.ac.il"; 

/* //Please do not uncomment or delete this block//SD 
void plotUpsHad( TTree* ptree, TFile *outputFile){

  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();

  const char *title = "B#rightarrow D^{*}#tau#nu (Hadronic tag)";
  
  // General Info
  TH1F* h_Mbc = new TH1F("h_had_Mbc",title,100,5.22,5.29);
  ptree->Project("h_had_Mbc", "B_Mbc");
  h_Mbc->GetXaxis()->SetTitle("M_{bc} (GeV/c^{2})");
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Description", "Beam constrained mass"));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Mbc->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_DeltaE = new TH1F("h_had_DeltaE",title,100,-0.50,0.50);
  ptree->Project("h_had_DeltaE", "B_deltaE");
  h_DeltaE->GetXaxis()->SetTitle("#Delta E (GeV)");
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Description", "Peaks at zero, longer tail to low energy"));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DeltaE->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_EExtra = new TH1F("h_had_EExtra",title,50,0,5);
  ptree->Project("h_had_EExtra", "ROE_neextra");
  h_EExtra->GetXaxis()->SetTitle("E_{extra} (GeV)");
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Description", "Extra energy in the event"));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_EExtra->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  // Continuum suppression variables
  TH1F* h_R2 = new TH1F("h_had_foxWolframR2",title,50,0,1);
  ptree->Project("h_had_foxWolframR2", "foxWolframR2");
  h_R2->GetXaxis()->SetTitle("foxWolframR2 (continuum suppression variable)");
  h_R2->GetListOfFunctions()->Add(new TNamed("Description", "The continuum suppression variable, foxWolframR2"));
  h_R2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_R2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_R2->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_cosTBTO = new TH1F("h_had_cosTBTO",title,20,0,1);
  ptree->Project("h_had_cosTBTO", "B_cosTBTO");
  h_cosTBTO->GetXaxis()->SetTitle("cos(#theta_{thrust})");
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Description", "Cosine of the angle between the B and the thrust axis of the event"));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Check", "Consistent across versions"));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_cosTBTO->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  // Missing quantities and signal side relevant variables
  TH1F* h_missM2 = new TH1F("h_had_missM2",title,40,0,40);
  ptree->Project("h_had_missM2", "m2Recoil");
  h_missM2->GetXaxis()->SetTitle("squared missing mass M_{miss}^{2} (GeV^{2}/c^{4})");
  h_missM2->GetListOfFunctions()->Add(new TNamed("Description", "The squared missing mass"));
  h_missM2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_missM2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_missM2->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_missP = new TH1F("h_had_missP",title,40,0,4);
  ptree->Project("h_had_missP", "pRecoil");
  h_missP->GetXaxis()->SetTitle("recoiling momentum (GeV/c)");
  h_missP->GetListOfFunctions()->Add(new TNamed("Description", "Recoiling  momentum"));
  h_missP->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_missP->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_missP->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert")); 
 TH1F* h_DstarMomentum = new TH1F("h_DstarMomentum",title,40,0,4);
  ptree->Project("h_DstarMomentum", "d0_d0_pCMS");
  h_DstarMomentum->GetXaxis()->SetTitle("p_{D*} (GeV/c)");
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Description", "The momentum of the D^{*}"));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DstarMomentum->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_Dmass = new TH1F("h_Dmass",title,40,1.8,1.9);
  ptree->Project("h_Dmass", "d0_d0_d0_M");
  h_Dmass->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D meson from D* decay"));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_DstarMass = new TH1F("h_DstarMass",title,40,1.94,2.04);
  ptree->Project("h_DstarMass", "d0_d0_M");
  h_DstarMass->GetXaxis()->SetTitle("m_{D*} (GeV/c^{2})");
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D* meson from B decay"));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DstarMass->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_TauMomentum = new TH1F("h_TauMomentum",title,40,0,4);
  ptree->Project("h_TauMomentum", "B_tau_p");
  h_TauMomentum->GetXaxis()->SetTitle("p_{#tau} (GeV/c)");
  h_TauMomentum->GetListOfFunctions()->Add(new TNamed("Description", "The momentum of the \tau"));
  h_TauMomentum->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_TauMomentum->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_TauMomentum->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
  TH1F* h_Q2 = new TH1F("Q2",title,40,0,4);
  ptree->Project("Q2", "Q2");
  h_Q2->GetXaxis()->SetTitle("Q^{2} (GeV/c)^{2}");
  h_Q2->GetListOfFunctions()->Add(new TNamed("Description", "Q squared"));
  h_Q2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Q2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Q2->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));
//Different D decay modes

  TH1F* h_Dmass_Mode1 = new TH1F("h_Dmass_Mode1",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode1", "d0_d0_d0_M","d_ID==1");
  h_Dmass_Mode1->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode1->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K^{-}#p^{+} from D* decay"));   
  h_Dmass_Mode1->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_Dmass_Mode1->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode1->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode2 = new TH1F("h_Dmass_Mode2",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode2", "d0_d0_d0_M","d_ID==2");
  h_Dmass_Mode2->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode2->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K^{-}#p^{+}#p^{0} from D* decay"));   
  h_Dmass_Mode2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_Dmass_Mode2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode2->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode3 = new TH1F("h_Dmass_Mode3",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode3", "d0_d0_d0_M","d_ID==3");
  h_Dmass_Mode3->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode3->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K_{s}^{0}#p^{0} from D* decay"));   
  h_Dmass_Mode3->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_Dmass_Mode3->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode3->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode4 = new TH1F("h_Dmass_Mode4",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode4", "d0_d0_d0_M","d_ID==4");
  h_Dmass_Mode4->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode4->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K^{-}#p^{+}#p^{+}#p^{-} from D* decay"));   
  h_Dmass_Mode4->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_Dmass_Mode4->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode4->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode5 = new TH1F("h_Dmass_Mode5",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode5", "d0_d0_d0_M","d_ID==5");
  h_Dmass_Mode5->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode5->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K_{s}^{0}#p^{+}#p^{-} from D* decay"));   
  h_Dmass_Mode5->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_Dmass_Mode5->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode5->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode6 = new TH1F("h_Dmass_Mode6",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode6", "d0_d0_d0_M","d_ID==6");
  h_Dmass_Mode6->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode6->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K_{s}^{0}#p^{+}#p^{-}#p^{0} from D* decay"));
  h_Dmass_Mode6->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Dmass_Mode6->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode6->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode7 = new TH1F("h_Dmass_Mode7",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode7", "d0_d0_M","dstarID==7");
  h_Dmass_Mode7->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode7->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D* to D^{0}#p^{0}"));
  h_Dmass_Mode7->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Dmass_Mode7->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode7->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_Dmass_Mode8 = new TH1F("h_Dmass_Mode8",title,70,1.5,2.2);
  ptree->Project("h_Dmass_Mode8", "d0_d0_M","dstarID==8");
  h_Dmass_Mode8->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_Dmass_Mode8->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to D* to D^{0}#gamma"));
  h_Dmass_Mode8->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_Dmass_Mode8->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_Dmass_Mode8->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  outputFile->cd();

  h_Mbc->Write();
  h_DeltaE->Write();
  h_EExtra->Write();
  
  h_R2->Write();
  h_cosTBTO->Write();
  h_missM2->Write();
  h_missP->Write();
  h_DstarMomentum->Write();
  h_Dmass->Write();
  h_TauMomentum->Write();
  h_Q2->Write();
  h_Dmass_Mode1->Write();
  h_Dmass_Mode2->Write();
  h_Dmass_Mode3->Write();
  h_Dmass_Mode4->Write();
  h_Dmass_Mode5->Write();
  h_Dmass_Mode6->Write();
  h_Dmass_Mode7->Write();
  h_Dmass_Mode8->Write();
  h_DstarMass->Write();

}
*/

void plotDzero(TTree* ptree, TFile *outputFile){

  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();

  const char *title = "D reconstruction forB#rightarrow D^{*}#tau#nu (Hadronic tag)";

  TH1F* h_DmassNoCut_Mode1 = new TH1F("h_DmassNoCut_Mode1",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode1", "InvM","dmID==1");
  h_DmassNoCut_Mode1->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode1->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K^{-}#p^{+} from D* decay"));   
  h_DmassNoCut_Mode1->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_DmassNoCut_Mode1->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode1->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_DmassNoCut_Mode2 = new TH1F("h_DmassNoCut_Mode2",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode2", "InvM","dmID==2");
  h_DmassNoCut_Mode2->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode2->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K^{-}#p^{+}#p^{0} from D* decay"));   
  h_DmassNoCut_Mode2->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_DmassNoCut_Mode2->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode2->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_DmassNoCut_Mode3 = new TH1F("h_DmassNoCut_Mode3",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode3", "InvM","dmID==3");
  h_DmassNoCut_Mode3->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode3->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K_{s}^{0}#p^{0} from D* decay"));   
  h_DmassNoCut_Mode3->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_DmassNoCut_Mode3->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode3->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_DmassNoCut_Mode4 = new TH1F("h_DmassNoCut_Mode4",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode4", "InvM","dmID==4");
  h_DmassNoCut_Mode4->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode4->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K^{-}#p^{+}#p^{+}#p^{-} from D* decay"));   
  h_DmassNoCut_Mode4->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_DmassNoCut_Mode4->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode4->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_DmassNoCut_Mode5 = new TH1F("h_DmassNoCut_Mode5",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode5", "InvM","dmID==5");
  h_DmassNoCut_Mode5->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode5->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K_{s}^{0}#p^{+}#p^{-} from D* decay"));   
  h_DmassNoCut_Mode5->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));  
  h_DmassNoCut_Mode5->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode5->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_DmassNoCut_Mode6 = new TH1F("h_DmassNoCut_Mode6",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode6", "InvM","dmID==6");
  h_DmassNoCut_Mode6->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode6->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D to K_{s}^{0}#p^{+}#p^{-}#p^{0} from D* decay"));
  h_DmassNoCut_Mode6->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DmassNoCut_Mode6->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode6->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  outputFile->cd();
  h_DmassNoCut_Mode1->Write();
  h_DmassNoCut_Mode2->Write();
  h_DmassNoCut_Mode3->Write();
  h_DmassNoCut_Mode4->Write();
  h_DmassNoCut_Mode5->Write();
  h_DmassNoCut_Mode6->Write();
}

void plotDSTsig(TTree* ptree, TFile *outputFile){

  gStyle->SetOptStat(0);
  gStyle->SetHistMinimumZero();

  const char *title = "D reconstruction forB#rightarrow D^{*}#tau#nu (sig)";

  TH1F* h_DmassNoCut_Mode7 = new TH1F("h_DmassNoCut_Mode7",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode7", "InvM","dmID==7");
  h_DmassNoCut_Mode7->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode7->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D* to D^{0}#p^{0} "));
  h_DmassNoCut_Mode7->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DmassNoCut_Mode7->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode7->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  TH1F* h_DmassNoCut_Mode8 = new TH1F("h_DmassNoCut_Mode8",title,70,1,3);
  ptree->Project("h_DmassNoCut_Mode8", "InvM","dmID==8");
  h_DmassNoCut_Mode8->GetXaxis()->SetTitle("m_{D0} (GeV/c^{2})");
  h_DmassNoCut_Mode8->GetListOfFunctions()->Add(new TNamed("Description", "invariant mass of D* to D^{0}#gamma "));
  h_DmassNoCut_Mode8->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape"));
  h_DmassNoCut_Mode8->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  h_DmassNoCut_Mode8->GetListOfFunctions()->Add(new TNamed("MetaOptions", "expert"));

  outputFile->cd();
  h_DmassNoCut_Mode7->Write();
  h_DmassNoCut_Mode8->Write();
}


void test2_1263340000(){

  TString inputfile("../1263340000_test.ntup.root");

  TFile* sample = new TFile(inputfile);
  //  TTree* treeUpsHad = (TTree*)sample->Get("Y4S"); //Please do not uncomment or delete//SD 
  TTree * treeD0 =(TTree*)sample->Get("D0all");
  TTree * treeDSTsig=(TTree*)sample->Get("DSTsig");

  TFile* outputFile = new TFile("1263340000_Validation.root","RECREATE");
  
  TString Cuts= "";
  //  plotUpsHad(treeUpsHad, outputFile );
  plotDzero(treeD0,outputFile);
  plotDSTsig(treeDSTsig,outputFile);
  outputFile->Close();

}


