/* VXDCDCMergerSinglePartTruthFinder.C 
 * ROOT macro for ECL validation plots 
 * Author: Benjamin Oberhof 
 * 2014
*/

/*
<header>
<input>VXDCDCMergerEvtGen.root</input>
<output>VXDCDCMergerEvtGenHist.root</output>
<contact>tracking@belle2.kek.jp</contact>
</header>
*/

void VXDCDCMergerEvtGen(){

  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));

  TFile* inputFile = TFile::Open("../VXDCDCMergerEvtGen.root");
  TTree* tree = (TTree*) inputFile->Get("Stats");

  // write output file for the validation histograms
  TFile* output = TFile::Open("VXDCDCMergerEvtGenHist.root", "recreate");

  //TCanvas *c1 = new TCanvas("c1","prova",10,10,700,500);
  //c1->SetFillColor(0);

  TH1F *EG1 = new TH1F("EG1","Merged Tracks EvtGen",10,0,10);
  EG1->GetXaxis()->SetTitle("# of merged tracks");
  EG1->GetListOfFunctions()->Add(new TNamed("Description","Number of merged tracks for EvtGen events")); 
  EG1->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 1"));
  EG1->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //tree->Draw("eclClusterEnergy>>hClusterE","eclClusterEnergy>0");
  //tree->Draw("MergedTracks>>EG1");
  //EG1->Write();
  //c1->SaveAs("plots/MergedTracks_5GeV_muons_1000_Theta.jpg");

  TH1F *EG2 = new TH1F("EG2","True Matched Tracks Truth Track Finder",10,0,10);
  tree->Draw("TrueMatchedTracks>>EG2");
  //EG2->Write();
  //c1->SaveAs("plots/TrueMatchedTracks_5GeV_muons_1000_Theta.jpg");

  TH1F *EG3 = new TH1F("EG3","CDCTracks",10,0,10);
  tree->Draw("CDCTracks>>EG3");
  //EG3->Write();
  //c1->SaveAs("plots/CDCTracks_5GeV_muons_1000_Theta.jpg");

  TH1F *EG4 = new TH1F("EG4","VXDTracks",10,0,10);
  tree->Draw("VXDTracks>>EG4");
  //EG4->Write();
  //c1->SaveAs("plots/VXDTracks_5GeV_muons_1000_Theta.jpg");

  TH1F *EG5 = new TH1F("EG5","Chi2",100,0,100);
  tree->Draw("Chi2>>EG5","GoodTag==1&&TruthTag==1");
  EG1->GetListOfFunctions()->Add(new TNamed("Description","6D Chi2")); 
  EG1->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 0"));
  EG1->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //c1->SaveAs("plots/Chi2_5GeV_muons_1000_Theta.jpg");
  //h5->Write();

  TH1F *EG6 = new TH1F("EG6","Position Residual",100,0,0.5);
  EG1->GetListOfFunctions()->Add(new TNamed("Description","Position Residual")); 
  EG1->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 0"));
  EG1->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //c1->SaveAs("plots/PosRes_5GeV_muons_1000_Theta.jpg");
  //tree->Draw("PosRes>>EG6","GoodTag==1&&TruthTag==1");
  //EG6->Write();

  //TH1F *EG7 = new TH1F("EG7","Dx",100,-0.05,0.05);
  //tree->Draw("Dx>>EG7","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Dx_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG8 = new TH1F("EG8","Dy",100,-0.05,0.05);
  //tree->Draw("Dy>>EG8","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Dy_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG9 = new TH1F("EG9","Dz",100,-0.5,0.5);
  //tree->Draw("Dz>>EG9","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Dz_5GeV_muons_1000_Theta.jpg");


  TH1F *EG10 = new TH1F("EG10","Momentum Resisual",100,0,0.5);
  EG1->GetListOfFunctions()->Add(new TNamed("Description","Momentum Residual")); 
  EG1->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 0"));
  EG1->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //tree->Draw("MomRes>>EG10","GoodTag==1&&TruthTag==1");
  //EG10->Write();

  //TH1F *EG11 = new TH1F("EG11","DMomx",100,-2,2);
  //tree->Draw("DMomx>>EG11","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/DMomx_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG12 = new TH1F("EG12","DMomy",100,-2,2);
  //tree->Draw("DMomy>>EG12","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/DMomy_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG13 = new TH1F("EG13","DMomz",100,-1,1);
  //tree->Draw("DMomz>>EG13","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/DMomz_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG14 = new TH1F("EG14","P",100,4.8,5.2);
  //tree->Draw("P>>EG14","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/P_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG15 = new TH1F("EG15","Px",100,-6,6);
  //tree->Draw("Px>>EG15","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Px_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG16 = new TH1F("EG16","Py",100,-6,6);
  //tree->Draw("Py>>EG16","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Py_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG17 = new TH1F("EG17","Pz",100,-3,3);
  //tree->Draw("Pz>>EG17","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Pz_5GeV_muons_1000_Theta.jpg");


  //TH1F *EG18 = new TH1F("EG18","VXDP",100,2,8);
  //tree->Draw("VXDP>>EG18","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDP_5GeV_muons_1000_Theta.jpg");
  //EG18->Write();

  //TH1F *EG19 = new TH1F("EG19","VXDPx",100,-8,8);
  //tree->Draw("VXDPx>>EG19","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDPx_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG20 = new TH1F("EG20","VXDPy",100,-8,8);
  //tree->Draw("VXDPy>>EG20","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDPy_5GeV_muons_1000_Theta.jpg");

  //TH1F *EG21 = new TH1F("EG21","VXDP",100,-4,4);
  //tree->Draw("VXDPz>>EG21","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDPz_5GeV_muons_1000_Theta.jpg");

 /////////////SCANS//////////////

  TH1F *h22 = new TH1F("h22","Eff vs Theta EvtGen",100,0.,3.);
  h22->SetMaximum(1.1);
  tree->Draw("TMath::Sqrt((Px*Px+Py*Py))>>h22","TruthTag==1");
  h22->Sumw2();
  TH1F *eff_ptEG = new TH1F("eff_ptEG","Eff vs Pt EvtGen VXDTF+Trasan",100,0.,3.);
  tree->Draw("TMath::Sqrt((Px*Px+Py*Py))>>eff_ptEG","TruthTag==1&&GoodTag==1");
  eff_ptEG->SetMaximum(1.1);
  eff_ptEG->Sumw2();
  gPad->RedrawAxis();
  //eff_ptEG->Divide(h22);
  //eff_ptEG->GetYaxis()->SetRange(0,2);
  //eff_ptEG->Draw("E");
  TH1F *eff_pt = new TH1F("eff_pt", "Eff vs Pt EvtGen",100,0.,3.);
  //TLegend *leg1= new TLegend(0.6,0.1,0.9,0.3,"Eff vs. Pt");
  eff_ptEG->GetListOfFunctions()->Add(new TNamed("Description","Efficiency vs Pt")); 
  eff_ptEG->GetListOfFunctions()->Add(new TNamed("Check","Should be -> 1 above 0.3 GeV"));
  eff_ptEG->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  eff_ptEG->GetXaxis()->SetTitle("Pt (GeV)");
  eff_ptEG->GetYaxis()->SetTitle("Efficiency");
  eff_pt=eff_ptEG;
  eff_pt->Divide(eff_ptEG, h22, 1.0, 1.0, "B");
  //TPaveStats *st = (TPaveStats*)h->FindObject("stats");
  //st->SetX1NDC(0.7); //new x start position
  //st->SetX2NDC(0.1); //new x end position
  gStyle->SetOptStat("e");
  eff_pt->GetYaxis()->SetRangeUser(0,1.1);
  gPad->RedrawAxis();
  eff_pt->Draw("E");
  //leg1->Draw();
    //c1->SaveAs("plots/EffvsPt_5GeV_muons_1000_Pt.jpg");
  eff_pt->Write();
  //delete eff_pt;

  TH1F *h24 = new TH1F("h24","Eff vs Theta Truth Track Finder",50, -1, 1.);
  h24->SetMaximum(1.1);
  tree->Draw("(Pz/(P))>>h24","TruthTag==1");
  h24->Sumw2();
  TH1F *eff_thetaEG = new TH1F("eff_thetaEG","Eff vs Theta EvtGen VXDTF+Trasan",50, -1, 1.);
  eff_thetaEG->SetMaximum(1.1);
  tree->Draw("(Pz/(P))>>eff_thetaEG","TruthTag==1&&GoodTag==1");
  eff_thetaEG->Sumw2();
  gPad->RedrawAxis();
  //eff_thetaEG->Divide(h24);
  //eff_thetaEG->GetYaxis()->SetRange(0,2);
  //eff_thetaEG->Draw("E");
  TH1F *eff_theta = new TH1F("eff_theta", "Eff vs Theta EvtGen", 50, -1, 1.);
  //TLegend *leg2= new TLegend(0.6,0.1,0.9,0.3,"Eff vs. theta");
  eff_thetaEG->GetXaxis()->SetTitle("Polar Angle (rad)");
  eff_thetaEG->GetYaxis()->SetTitle("Efficiency");
  eff_thetaEG->GetListOfFunctions()->Add(new TNamed("Description","Efficiency vs thetaEG")); 
  eff_thetaEG->GetListOfFunctions()->Add(new TNamed("Check","Should be (ideally) flat"));
  eff_thetaEG->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  eff_theta=eff_thetaEG;
  eff_theta->Divide(eff_thetaEG, h24, 1.0, 1.0, "B");
  //TPaveStats *st = (TPaveStats*)h->FindObject("stats");
  //st->SetX1NDC(0.7); //new x start position
  //st->SetX2NDC(0.1); //new x end position
  //gPad->SetOptStat(1)
  //leg2->Draw();
  gStyle->SetOptStat("e");
  eff_theta->GetYaxis()->SetRangeUser(0,1.1);
  gPad->RedrawAxis();
  eff_theta->Draw("E");
    //c1->SaveAs("plots/EffvsTheta_5GeV_muons_1000_Theta.jpg");
  eff_theta->Write();
  //delete eff_theta;
  /*
  h2->SetLineWidth(3);
  h2->SetLineColor(1);
  h2->GetYaxis()->SetRange(0,2);
  h2->Draw();
  h3->SetLineColor(3);
  h3->SetLineWidth(3);
  h3->Draw("same");
  h4->SetLineColor(4);
  h4->SetLineWidth(3);
  h4->Draw("same");*/
  //h1->SetLineColor(2);
  //h1->SetLineWidth(3);
  //h1->Draw("same");
  //h1->Write();
  //c1->SaveAs("plots/TrueMergedTrk_5GeV_muons_1000_Theta.jpg");

  //h3->SetLineColor(4);
  //h3->SetLineWidth(3);
  //  h3->GetYaxis()->SetRange(-1.,1.);
  //h3->Draw();
  //h4->SetLineColor(3);
  //h4->SetLineWidth(3);
  //h4->Draw();
  //h3->Draw("same");
  //h3->Write();
  //c1->SaveAs("plots/VXDCDCTrk_5GeV_muons_1000_Theta.jpg");

  output->Close();
}
