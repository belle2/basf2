/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// run as root processDstar.C 
// set correctly the path to the input files!

void processDstar(){
  
  TChain* tree = new TChain("dsttree");
  
  // input files (for example output of "arich/examples/reconstruct_Dstar.py")
  tree->Add("Dstar2D0Pi-PID_exp8_r0-100.root");
  
  // output file name (.pdf and .root are appended)
  TString outf = "arich_pid_dstar";

  // candidate selection criteria
  TString selection = "abs(M - D0_M - 0.14543) < 0.0015 && D0_pi_nCDCHits > 0 && D0_K_nCDCHits> 0 && abs(D0_K_z0)<5 && abs(D0_K_d0)<2 &&  abs(D0_pi_z0)<5 &&  abs(D0_pi_d0)<2";

  // additional selection for pi/K track
  TString cutspi = " && D0_pi_cosTheta>0.85 && D0_pi_p > 0.5";
  TString cutsk = " && D0_K_cosTheta>0.85 && D0_K_p > 0.5";

  gStyle->SetOptStat(0);
  
  TCanvas *cc = new TCanvas("cc","cc",800,400);
  cc->Divide(2);
  cc->cd(1);

  // number of bins for M_D0 histograms 
  int mbins = 20;
  double scale =  double(mbins) / 0.09;

  // make histograms for M_D0 distributions
  TH1F* h_md0_pi = new TH1F("h_md0_pi","Pion in ARICH (p > 0.7 GeV);M [D^{0}] (GeV/c^{2});Entries",mbins,1.81,1.9);
  TH1F* h_md0_k = new TH1F("h_md0_k","Kaon in ARICH (p > 0.7 GeV);M [D^{0}] (GeV/c^{2});Entries",mbins,1.81,1.9);
  TH1F* h_md0_all = new TH1F("h_md0_all","Any in ARICH (p > 0.7 GeV);M [D^{0}] (GeV/c^{2});Entries",mbins,1.81,1.9);

  // fill histograms
  tree->Draw("D0_M>>h_md0_pi",  selection + cutspi,"E1");
  tree->Draw("D0_M>>h_md0_k",  selection + cutsk,"E1");
  
  // add both (pi in arich and K in arich) histograms for the signal shape fit 
  h_md0_all->Add(h_md0_pi);
  h_md0_all->Add(h_md0_k);

  // set plotting options
  double maxall = h_md0_pi->GetMaximum() > h_md0_k->GetMaximum() ? h_md0_pi->GetMaximum()+200 : h_md0_k->GetMaximum()+200;  
  h_md0_pi->SetMaximum(maxall);
  h_md0_k->SetMaximum(maxall);
  h_md0_pi->SetMinimum(0);
  h_md0_k->SetMinimum(0);
  h_md0_pi->SetMarkerStyle(20);
  h_md0_pi->SetMarkerSize(0.62);
  h_md0_k->SetMarkerStyle(20);
  h_md0_k->SetMarkerSize(0.62);
  h_md0_pi->SetLineColor(1);
  h_md0_k->SetLineColor(1);
  h_md0_pi->SetMarkerColor(1);
  h_md0_k->SetMarkerColor(1);
    
  // make fit functions (double gaussian for signal peak + flat backgroud)
  TF1* fitf = new TF1("fitf", "[0]*(exp(-0.5*((x-[1])/[2])**2) + [3]*exp(-0.5*((x-[1])/[4])**2)) + [5]",1.81,1.9);
  TF1* sigf = new TF1("sigf", "[0]*(exp(-0.5*((x-[1])/[2])**2) + [3]*exp(-0.5*((x-[1])/[4])**2))",1.81,1.9);
  TF1* bkgf = new TF1("bkgf", "pol0",1.81,1.9);
  fitf->SetParameters(1000,1.867,0.004368,0.2,0.010,100);
  fitf->SetLineColor(4);
  sigf->SetLineColor(8);
  sigf->SetLineWidth(1);
  bkgf->SetLineColor(46);
  bkgf->SetLineWidth(1);

  // fit M_D0 distribution of all D0 candidates that have one of the tracks in arich
  h_md0_all->Fit(fitf,"RL");
  // fix the signal shape parameters for next fits! 
  fitf->FixParameter(1,fitf->GetParameter(1));
  fitf->FixParameter(2,fitf->GetParameter(2));
  fitf->FixParameter(3,fitf->GetParameter(3));
  fitf->FixParameter(4,fitf->GetParameter(4));

  // fit M_D0 distribution of D0 candidates with pi in arich -> determine total number of pions in arich
  h_md0_pi->Fit(fitf,"RL");
  
  sigf->SetParameters(fitf->GetParameter(0),fitf->GetParameter(1),fitf->GetParameter(2),fitf->GetParameter(3),fitf->GetParameter(4));
  bkgf->SetParameter(0,fitf->GetParameter(5));
  
  //double sigmaPi = sigf->GetParameter(2);
  // total number of pions in arich and its error
  double ntotPi = sigf->Integral(1.81,1.9) *  scale; 
  double ntotPiErr = fitf->GetParError(0)/sigf->GetParameter(0) * ntotPi;

  // plot everything (data distribution + fit)
  cc->cd(1);
  h_md0_pi->Draw("E1");
  sigf->SetFillColor(8);
  sigf->SetFillStyle(3004);
  sigf->DrawCopy("fcsame");
  bkgf->SetFillColor(46);
  bkgf->SetFillStyle(3004);
  bkgf->DrawCopy("fcsame");
  TText text0(1.815,maxall-30./250.*maxall,TString::Format("total pi: %.2f +/- %.2f", ntotPi,ntotPiErr));
  text0.Draw();

  cc->cd(2);

  // same thing but for D0 candidates with kaon track in arich
  h_md0_k->Fit(fitf,"RL"); 
  sigf->SetParameters(fitf->GetParameter(0),fitf->GetParameter(1),fitf->GetParameter(2),fitf->GetParameter(3),fitf->GetParameter(4));
  bkgf->SetParameter(0,fitf->GetParameter(5));
  
  //double sigmaK = sigf->GetParameter(2);
  // total number of kaons in arich and its error
  double ntotK = sigf->Integral(1.81,1.9) * scale;
  double ntotKErr = fitf->GetParError(0)/sigf->GetParameter(0)*ntotK;

  h_md0_k->Draw("E1");
  sigf->Draw("fcsame"); bkgf->Draw("fcsame");
  TText textt(1.815,maxall-30./250.*maxall,TString::Format("total K: %.2f +/- %.2f", ntotK,ntotKErr));
  textt.Draw();  
  
  cc->Print(outf + ".pdf(","pdf");

  // clone histograms to add them to next plots with likelihood cut applied
  TH1F* h_md0_allpi = (TH1F*)h_md0_pi->Clone("h_md0_allpi");
  TH1F* h_md0_allk = (TH1F*)h_md0_k->Clone("h_md0_allk");
  h_md0_allpi->SetMarkerColor(16); h_md0_allpi->SetLineColor(16);
  h_md0_allk->SetMarkerColor(16); h_md0_allk->SetLineColor(16);
  h_md0_allpi->GetFunction("fitf")->SetLineColor(38);
  h_md0_allk->GetFunction("fitf")->SetLineColor(38);	   

  // graphs for eff vs. fake curve, pion eff. vs lkh cut, and kaon eff. vs lkh cut
  TGraphErrors* efffake = new TGraphErrors();
  TGraphErrors* pion_eff = new TGraphErrors();
  TGraphErrors* kaon_eff = new TGraphErrors();

  int i=1;

  // likelihood step 
  double lkhstep=0.1;
  
  double lkh = lkhstep;

  // redo D0 fit for each step of PID likelihood cut
  while (lkh<0.99){
  
    fitf->SetParameter(0,100.);

    // likehlihood cuts 
    TString lkhcutpi = TString::Format("D0_pi_pidPairProbabilityExpert_321_211_ARICH > %f && ", lkh);
    TString lkhcutK = TString::Format("D0_K_pidPairProbabilityExpert_321_211_ARICH > %f && ", lkh);
    
    tree->Draw("D0_M>>h_md0_pi",lkhcutpi + selection + cutspi,"E1");
    tree->Draw("D0_M>>h_md0_k",lkhcutK + selection + cutsk,"E1");

    // do the fitting for pions and calculate efficiency 
    h_md0_pi->Fit(fitf,"RL");
    sigf->SetParameters(fitf->GetParameter(0),fitf->GetParameter(1),fitf->GetParameter(2),fitf->GetParameter(3),fitf->GetParameter(4));
    bkgf->SetParameter(0,fitf->GetParameter(5));
    double nPi = sigf->Integral(1.81,1.9) *  scale;
    double nPiErr = fitf->GetParError(0)/sigf->GetParameter(0)*nPi;    
    double effPi = nPi / ntotPi;
    std::cout << "pieff: " <<  effPi << std::endl;
    
    cc->cd(1);         
    h_md0_allpi->Draw("E1");  
    h_md0_pi->Draw("E1same");
    sigf->DrawCopy("fcsame"); bkgf->DrawCopy("fcsame");
    
    TText textL1(1.815,maxall-30./250.*maxall,TString::Format("R [K/pi] > %.2f", lkh));
    TText text1(1.815,maxall-50./250.*maxall,TString::Format("npi: %.2f +/- %.2f", nPi,nPiErr));
    TText textE1(1.815,maxall-70./250.*maxall,TString::Format("pi eff: %.3f", nPi/ntotPi));
    text1.Draw();
    textL1.Draw();
    textE1.Draw();

    // same thing for kaon, fitting and calculate efficiency
    cc->cd(2);
    h_md0_k->Fit(fitf,"RL");        
    sigf->SetParameters(fitf->GetParameter(0),fitf->GetParameter(1),fitf->GetParameter(2),fitf->GetParameter(3),fitf->GetParameter(4));
    bkgf->SetParameter(0,fitf->GetParameter(5));
    double nK = sigf->Integral(1.81,1.9) *  scale;
    double nKErr = fitf->GetParError(0)/sigf->GetParameter(0)*nK;        
    double effK = nK / ntotK;    
    
    h_md0_allk->Draw("E1");
    h_md0_k->Draw("E1same");
    sigf->Draw("fcsame"); bkgf->Draw("fcsame");
    
    TText textL2(1.815, maxall-30./250.*maxall,TString::Format("R [K/pi] > %.2f", lkh));
    TText text2(1.815,  maxall-50./250.*maxall,TString::Format("nK: %.2f +/- %.2f", nK, nKErr));
    TText textE2(1.815, maxall-70./250.*maxall,TString::Format("K fake: %.3f", nK/ntotK));
    text2.Draw();
    textL2.Draw();
    textE2.Draw();
        
    cc->Print(outf + ".pdf","pdf");    
    
    // set point of the graphs
    efffake->SetPoint(i-1,nPi/ntotPi, nK/ntotK);
    efffake->SetPointError(i-1,1/ntotPi*sqrt(ntotPiErr*ntotPiErr*effPi*effPi + nPiErr*nPiErr - 2*pow(effPi,3./2.)*nPiErr*ntotPiErr), 1/ntotK*sqrt(ntotKErr*ntotKErr*effK*effK + nKErr*nKErr - 2*pow(effK,3./2.)*nKErr*ntotKErr));
    
    pion_eff->SetPoint(i-1,lkh,effPi);
    pion_eff->SetPointError(i-1,0,1/ntotPi*sqrt(ntotPiErr*ntotPiErr*effPi*effPi + nPiErr*nPiErr - 2*pow(effPi,3./2.)*nPiErr*ntotPiErr));
    
    kaon_eff->SetPoint(i-1,lkh,effK);
    kaon_eff->SetPointError(i-1,0,1/ntotK*sqrt(ntotKErr*ntotKErr*effK*effK + nKErr*nKErr - 2*pow(effK,3./2.)*nKErr*ntotKErr));
    
    lkh += lkhstep;
    i++;    
  }

  // plot final graphs and store pdf and root file 
  cc->cd(1);
  efffake->Draw("ap");
  gPad->SetGridx();
  gPad->SetGridy();
  efffake->GetXaxis()->SetTitle("#pi fake rate");
  efffake->GetYaxis()->SetTitle("K efficiency");
  efffake->GetYaxis()->SetTitleOffset(1.47);
  efffake->GetXaxis()->SetRangeUser(0.0,0.3);
  pion_eff->GetYaxis()->SetRangeUser(0.0,1.0);
  kaon_eff->GetYaxis()->SetRangeUser(0.0,1.0);
  efffake->SetMarkerStyle(8);
  efffake->SetMarkerColor(2);
  efffake->SetLineColor(12);
  kaon_eff->SetMarkerStyle(8);
  kaon_eff->SetMarkerColor(2);
  kaon_eff->SetLineColor(12);
  pion_eff->SetMarkerStyle(8);
  pion_eff->SetMarkerColor(4);
  pion_eff->SetLineColor(12);
  TMultiGraph* mg = new TMultiGraph();
  kaon_eff->SetTitle("K efficiency");
  pion_eff->SetTitle("#pi fake rate");
  mg->Add(kaon_eff);
  mg->Add(pion_eff);
  cc->cd(2);  
  mg->Draw("ap");
  gPad->SetGridx();
  gPad->SetGridy();
  mg->GetYaxis()->SetRangeUser(0.0,1.0);
  mg->GetXaxis()->SetTitle("R[K/pi]");
  mg->GetYaxis()->SetTitle("#pi fake rate / K efficiency");
  gPad->BuildLegend(0.2,0.3,0.5,0.5);

  cc->Print(outf + ".pdf)","pdf"); 
  
  TFile* f = new TFile(outf+".root","RECREATE");
  efffake->SetName("feff");
  kaon_eff->SetName("keff");
  pion_eff->SetName("pieff");
  efffake->Write();
  pion_eff->Write();
  kaon_eff->Write();
  f->Close();
      
 
}
