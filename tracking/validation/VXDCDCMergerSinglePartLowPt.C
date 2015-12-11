/* VXDCDCMergerSinglePart.C 
 * ROOT macro for ECL validation plots 
 * Author: Benjamin Oberhof 
 * 2014
*/

/*
<header>
<input>VXDCDCMergerSinglePartLowPt.root</input>
<output>VXDCDCMergerSinglePartLowPtHist.root</output>
<contact>tracking@belle2.kek.jp</contact>
</header>
*/

void VXDCDCMergerSinglePartLowPt(){

  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));

  TFile* inputFile = TFile::Open("../VXDCDCMergerSinglePartLowPt.root");
  TTree* tree = (TTree*) inputFile->Get("Stats");

  // write output file for the validation histograms
  TFile* output = TFile::Open("VXDCDCMergerSinglePartLowPtHist.root", "recreate");

  //TCanvas *c1 = new TCanvas("c1","prova",10,10,700,500);
  //c1->SetFillColor(0);

  //  TH1F *h1 = new TH1F("h1","Merged Tracks VXDTF+Trasan",10,0,10);
  // h1->GetXaxis()->SetTitle("# of merged tracks");
  // h1->GetListOfFunctions()->Add(new TNamed("Description","Number of merged tracks for single 1GeV charged muon")); 
  // h1->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 1"));
  // h1->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //tree->Draw("eclClusterEnergy>>hClusterE","eclClusterEnergy>0");
  //tree->Draw("MergedTracks>>h1");
  //h1->Write();
  //c1->SaveAs("plots/MergedTracks_5GeV_muons_1000_Theta.jpg");

  // TH1F *h2 = new TH1F("h2","True Matched Tracks  VXDTF+Trasan",10,0,10);
  // tree->Draw("TrueMatchedTracks>>h2");
  //h2->Write();
  //c1->SaveAs("plots/TrueMatchedTracks_5GeV_muons_1000_Theta.jpg");

  // TH1F *h3 = new TH1F("h3","CDCTracks",10,0,10);
  // tree->Draw("CDCTracks>>h3");
  //h3->Write();
  //c1->SaveAs("plots/CDCTracks_5GeV_muons_1000_Theta.jpg");

  // TH1F *h4 = new TH1F("h4","VXDTracks",10,0,10);
  // tree->Draw("VXDTracks>>h4");
  //h4->Write();
  //c1->SaveAs("plots/VXDTracks_5GeV_muons_1000_Theta.jpg");

  //11122015 
  /*
  TH1F *h5 = new TH1F("h5","Chi2",100,0,100);
  gStyle->SetOptStat("e");
  tree->Draw("Chi2>>h5","GoodTag==1&&TruthTag==1");
  h5->GetListOfFunctions()->Add(new TNamed("Description","6D Chi2")); 
  h5->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 0"));
  h5->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //c1->SaveAs("plots/Chi2_5GeV_muons_1000_Theta.jpg");
  h5->Write();

  TH1F *h6 = new TH1F("h6","Position Residual",100,0,0.5);
  gStyle->SetOptStat("e");
  tree->Draw("PosRes>>h6","GoodTag==1&&TruthTag==1");
  h6->GetListOfFunctions()->Add(new TNamed("Description","Position Residual")); 
  h6->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 0"));
  h6->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  //c1->SaveAs("plots/PosRes_5GeV_muons_1000_Theta.jpg");
  h6->Write();
  */

  //TH1F *h7 = new TH1F("h7","Dx",100,-0.05,0.05);
  //tree->Draw("Dx>>h7","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Dx_5GeV_muons_1000_Theta.jpg");

  //TH1F *h8 = new TH1F("h8","Dy",100,-0.05,0.05);
  //tree->Draw("Dy>>h8","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Dy_5GeV_muons_1000_Theta.jpg");

  //TH1F *h9 = new TH1F("h9","Dz",100,-0.5,0.5);
  //tree->Draw("Dz>>h9","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Dz_5GeV_muons_1000_Theta.jpg");

  //11122015 
  /*
  TH1F *h10 = new TH1F("h10","Momentum Resisual",100,0,0.5);
  gStyle->SetOptStat("e");
  tree->Draw("MomRes>>h10","GoodTag==1&&TruthTag==1");
  h10->GetListOfFunctions()->Add(new TNamed("Description","Momentum Residual")); 
  h10->GetListOfFunctions()->Add(new TNamed("Check","Should be peaked at 0"));
  h10->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  h10->Write();
  */

  //TH1F *h11 = new TH1F("h11","DMomx",100,-2,2);
  //tree->Draw("DMomx>>h11","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/DMomx_5GeV_muons_1000_Theta.jpg");

  //TH1F *h12 = new TH1F("h12","DMomy",100,-2,2);
  //tree->Draw("DMomy>>h12","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/DMomy_5GeV_muons_1000_Theta.jpg");

  //TH1F *h13 = new TH1F("h13","DMomz",100,-1,1);
  //tree->Draw("DMomz>>h13","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/DMomz_5GeV_muons_1000_Theta.jpg");

  //TH1F *h14 = new TH1F("h14","P",100,4.8,5.2);
  //tree->Draw("P>>h14","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/P_5GeV_muons_1000_Theta.jpg");

  //TH1F *h15 = new TH1F("h15","Px",100,-6,6);
  //tree->Draw("Px>>h15","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Px_5GeV_muons_1000_Theta.jpg");

  //TH1F *h16 = new TH1F("h16","Py",100,-6,6);
  //tree->Draw("Py>>h16","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Py_5GeV_muons_1000_Theta.jpg");

  //TH1F *h17 = new TH1F("h17","Pz",100,-3,3);
  //tree->Draw("Pz>>h17","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/Pz_5GeV_muons_1000_Theta.jpg");


  //TH1F *h18 = new TH1F("h18","VXDP",100,2,8);
  //tree->Draw("VXDP>>h18","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDP_5GeV_muons_1000_Theta.jpg");
  //h18->Write();

  //TH1F *h19 = new TH1F("h19","VXDPx",100,-8,8);
  //tree->Draw("VXDPx>>h19","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDPx_5GeV_muons_1000_Theta.jpg");

  //TH1F *h20 = new TH1F("h20","VXDPy",100,-8,8);
  //tree->Draw("VXDPy>>h20","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDPy_5GeV_muons_1000_Theta.jpg");

  //TH1F *h21 = new TH1F("h21","VXDP",100,-4,4);
  //tree->Draw("VXDPz>>h21","GoodTag==1&&TruthTag==1");
  //c1->SaveAs("plots/VXDPz_5GeV_muons_1000_Theta.jpg");

 /////////////SCANS//////////////

  TH1F *h22 = new TH1F("h22","Eff vs Pt VXDTF+Trasan",100,0.,1.);
  h22->SetMaximum(1.1);
  tree->Draw("TMath::Sqrt((Px*Px+Py*Py))>>h22","TruthTag==1");
  h22->Sumw2();
  TH1F *eff_ptsp = new TH1F("eff_ptsp","Eff vs Pt VXDTF+Trasan Single Muon",100,0.,1.);
  tree->Draw("TMath::Sqrt((Px*Px+Py*Py))>>eff_ptsp","TruthTag==1&&GoodTag==1");
  eff_ptsp->SetMaximum(1.1);
  eff_ptsp->Sumw2();
  gPad->RedrawAxis();
  //eff_thetasp->Divide(h22);
  //eff_thetasp->GetYaxis()->SetRange(0,2);
  //eff_thetasp->Draw("E");
  TH1F *eff_pt = new TH1F("eff_pt", "Eff vs Pt VXDTF+Trasan Single Muon",100,0.,1.);
  //TLegend *leg1= new TLegend(0.6,0.1,0.9,0.3,"Eff vs. Pt");
  eff_ptsp->GetXaxis()->SetTitle("Pt (GeV)");
  eff_ptsp->GetYaxis()->SetTitle("Efficiency");
  eff_ptsp->GetListOfFunctions()->Add(new TNamed("Description","Efficiency vs Pt VXDTF+Trasan")); 
  eff_ptsp->GetListOfFunctions()->Add(new TNamed("Check","Should be -> 1 above 0.3 GeV"));
  eff_ptsp->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  eff_pt=eff_ptsp;
  eff_pt->Divide(eff_ptsp, h22, 1.0, 1.0, "B");
  //TPaveStats *st = (TPaveStats*)h->FindObject("stats");
  //st->SetX1NDC(0.7); //new x start position
  //st->SetX2NDC(0.1); //new x end position
  gStyle->SetOptStat("e");
  eff_pt->GetYaxis()->SetRangeUser(0,1.1);
  gPad->RedrawAxis();
  eff_pt->Draw("E");
  //leg1->Draw();
    //c1->SaveAs("plots/EffvsPt_5GeV_muons_1000_Theta.jpg");
  eff_pt->Write();
  //delete eff_pt;

  TH1F *h24 = new TH1F("h24","Eff vs Theta VXDTF+Trasan",50, -1, 1.);
  h24->SetMaximum(1.1);
  tree->Draw("(Pz/(P))>>h24","TruthTag==1");
  h24->Sumw2();
  TH1F *eff_thetasp = new TH1F("eff_thetasp","Eff vs Theta VXDTF+Trasan Single Muon",50, -1, 1.);
  eff_thetasp->SetMaximum(1.1);
  tree->Draw("(Pz/(P))>>eff_thetasp","TruthTag==1&&GoodTag==1");
  eff_thetasp->Sumw2();
  gPad->RedrawAxis();
  //eff_thetasp->Divide(h24);
  //eff_thetasp->GetYaxis()->SetRange(0,2);
  //eff_thetasp->Draw("E");
  TH1F *eff_theta = new TH1F("eff_theta", "Eff vs Theta VXDTF+Trasan Single Muon", 50, -1, 1.);
  //TLegend *leg2= new TLegend(0.6,0.1,0.9,0.3,"Eff vs. theta");
  eff_thetasp->GetXaxis()->SetTitle("Theta (rad)");
  eff_thetasp->GetYaxis()->SetTitle("Efficiency");
  eff_thetasp->GetListOfFunctions()->Add(new TNamed("Description","Efficiency vs theta for [0, 5] GeV Muon")); 
  eff_thetasp->GetListOfFunctions()->Add(new TNamed("Check","Should be (ideally) flat"));
  eff_thetasp->GetListOfFunctions()->Add(new TNamed("Contact","tracking@belle2.kek.jp")); 
  eff_theta=eff_thetasp;
  eff_theta->Divide(eff_thetasp, h24, 1.0, 1.0, "B");
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
  h2->Draw();*/
  /*  h3->SetLineColor(3);
  h3->SetLineWidth(3);
  h3->Draw("same");
  h4->SetLineColor(4);
  h4->SetLineWidth(3);
  h4->Draw("same");*/
  /*h1->SetLineColor(2);
  h1->SetLineWidth(3);
  h1->Draw("same");*/
  //h1->Write();
  //c1->SaveAs("plots/TrueMergedTrk_5GeV_muons_1000_Theta.jpg");
  /*
  h3->SetLineColor(4);
  h3->SetLineWidth(3);
  //  h3->GetYaxis()->SetRange(-1.,1.);
  //h3->Draw();
  h4->SetLineColor(3);
  h4->SetLineWidth(3);
  h4->Draw();
  h3->Draw("same");*/
  //h3->Write();
  //c1->SaveAs("plots/VXDCDCTrk_5GeV_muons_1000_Theta.jpg");

}
