/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/**
 * This is an example script for producing ARICH performance plots
 * As an input it takes file produced by ARICHNtuple module
 * Histograms of various distributions of interest are produced,
 * including PID efficiency plots and reconstructed Cherenkov angle
 * distributions (see below).
 *
 * run as: "root -l ntuplefile.root plotEfficiency.C"
 * saves ARICHPerformance.root
 * Author: Luka Santelj, 28.10.2016
 **/

void plotEfficiency(){

  TTree* ch = (TTree*)_file0->Get("arich");

  // momentum range for "vs. momentum plots"
  double pstart =0.3; // minimum momentum
  double pstop = 4;   // maximal momentum
  int pbin = 50;      // number of momentum bins

  // momentum range for momentum integrated plots (number of Cherenkov photons and Cherenkov angle distribution)
  double pmin=3.0;
  double pmax=4.0;

  // likelihood distributions for efficiency calculation
  TH2 *h_pi = new TH2F("h_pi","h_pi",pbin,pstart,pstop,50000,-200,200);
  TH2 *h_K = new TH2F("h_K","h_K",pbin,pstart,pstop,50000,-200,200);
  ch->Draw("(logL.pi-logL.K):mcHit.p>>h_pi","abs(mcHit.PDG)==211 && primary==1");
  ch->Draw("(logL.pi-logL.K):mcHit.p>>h_K","abs(mcHit.PDG)==321 && primary==1");

  // 2D histogram of K efficiency vs. pi fake rate vs. momentum (to je faca)
  TH2 *hh = new TH2F("hh","K id. efficiency;Momentum [GeV];#pi missid. prob.",pbin,pstart,pstop,100,0.005,0.10);

  // 1D histograms of K efficiency vs. momentum for 3 different pion fake rates
  TH1F* h_eff1 = new TH1F("h_eff1","Kaon id. efficiency at 2% pion fake rate;momentum [GeV]",pbin,pstart,pstop);
  TH1F* h_eff2 = new TH1F("h_eff2","Kaon id. efficiency at 4% pion fake rate;momentum [GeV]",pbin,pstart,pstop);
  TH1F* h_eff3 = new TH1F("h_eff3","Kaon id. efficiency at 2%,4%,6% pion fake rate;momentum [GeV]",pbin,pstart,pstop);

  // main loop to calculate K efficiency
  for (int j=1; j<pbin+1;j++){

    TH1F* h1 = (TH1F*)h_pi->ProjectionY("_py",j,j);
    TH1F* h2 = (TH1F*)h_K->ProjectionY("1_py",j,j);

    int startbin = h1->FindFirstBinAbove(0);
    int nentrk = h2->Integral(0,startbin-1);
    int nall = h1->GetEntries();
    int nallk = h2->GetEntries();

    int nentr = 0;
    double fake  = 0.005;
    double fakee = 0.;
    int i=0;

    while (fake < 0.10){
      while (fakee < fake){
	nentr+=h1->GetBinContent(startbin);
	nentrk+=h2->GetBinContent(startbin);
	startbin++;
 	fakee = double(nentr)/double(nall);
      }

      // efficiency at fake fake
      double eff = double(nentrk)/double(nallk);
      if(nallk>0){
	if(fabs(fake-0.02)<0.00001) h_eff1->SetBinContent(j,eff);
	if(fabs(fake-0.04)<0.00001) h_eff2->SetBinContent(j,eff);
	if(fabs(fake-0.06)<0.00001) h_eff3->SetBinContent(j,eff);
      }
      // use to convert "efficiency+fake" to separation in sigmas!
      // double sep = sqrt(2)*(TMath::ErfInverse(2.*(1-fake)-1) + TMath::ErfInverse(2.*eff-1));

      hh->SetBinContent(j,i,eff);
      fake+=0.001;
      i++;
    }
  }
  // momentum cut for some histograms
  TString momCut = TString::Format("mcHit.p>%f && mcHit.p<%f",pmin,pmax);

  // likelihood difference distributions for pions and kaons
  TH1F* h_lkhPi = new TH1F("h_lkhPi","Pion/kaon likelihood difference; L_{#pi}-L_{K}",500,-100,100);
  TH1F* h_lkhK  = new TH1F("h_lkhK","Pion/kaon likelihood difference; L_{#pi}-L_{K}",500,-100,100);
  ch->Draw("(logL.pi-logL.K)>>h_lkhPi","abs(mcHit.PDG)==211 && primary==1 && " + momCut);
  ch->Draw("(logL.pi-logL.K)>>h_lkhK","abs(mcHit.PDG)==321 && primary==1 && " + momCut);

  // reconstructed tracks position resolution on aerogel plane
  TH2F* h_trk = new TH2F("h_trk","Track position resolution;momentum [GeV];|x_{MC}-x_{rec}|",pbin,pstart,pstop,50,0,1); //200
  ch->Draw("sqrt((mcHit.x-recHit.x)**2 + (mcHit.y-recHit.y)**2):mcHit.p>>h_trk","abs(mcHit.PDG)==211 || abs(mcHit.PDG)==321 && primary==1");

  // tracks momentum distribution
  TH1F* h_momK = new TH1F("h_momK","Kaon/pion track momentum distribution;momentum [GeV]", pbin,pstart,pstop);
  TH1F* h_momPi = new TH1F("h_momPi","Kaon/pion track momentum distribution;momentum [GeV]", pbin,pstart,pstop);
  ch->Draw("mcHit.p>>h_momPi","abs(mcHit.PDG)==211 && primary==1");
  ch->Draw("mcHit.p>>h_momK","abs(mcHit.PDG)==321 && primary==1");

  // tracks azimuthal angle distribution
  TH1F* h_thetaK = new TH1F("h_thetaK","Kaon/pion track theta distribution;#theta", 200,0.25,0.6);
  TH1F* h_thetaPi = new TH1F("h_thetaPi","Kaon/pion track theta distribution;#theta", 200,0.25,0.6);
  ch->Draw("mcHit.theta>>h_thetaPi","abs(mcHit.PDG)==211 && primary==1");
  ch->Draw("mcHit.theta>>h_thetaK","abs(mcHit.PDG)==321 && primary==1");

  // number of detected photons in kaon/pion ring
  TH1F* h_nphotK = new TH1F("h_nphotK","Number of detected photons in kaon/pion ring;# of photons/ring", 30,-0.5,29.5);
  TH1F* h_nphotPi = new TH1F("h_nphotPi","Number of detected photons in kaon/pion ring;# of photons/ring", 30,-0.5,29.5);
  ch->Draw("detPhot.pi>>h_nphotPi","abs(mcHit.PDG)==211 && primary==1 && " + momCut);
  ch->Draw("detPhot.K>>h_nphotK","abs(mcHit.PDG)==321 && primary==1 && " + momCut);

  //--------------------------------------
  // Plot Cherenkov angle distribution
  //--------------------------------------
  // !!!NOTE!!!: option "storePhotons" of ARICHReconstruction module must be set to 1 in your steering script,
  //             otherwise reconstructed photons are not stored!
  //
  // For one photon hit the Cherenkov angle is reconstructed multiple times, using different hypotheses for aerogel layer of
  // emission point and hypotheses of possible reflections from mirror plates.
  // In ntuple information of reconstructed angles is stored in a vector of ARICHPhoton objects
  // (check arich/dataobjects/include/ARICHPhoton.h for members list)
  // To plot cherenkov angle distribution the following members are relevant:
  // photons.m_thetaCer  -  reconstructed Cherenkov theta angle
  // photons.m_phiCer  -  reconstructed Cherenkov phi angle
  // photons.m_mirror  -  mirror plate hypothesis (0 - no reflection, 1 - , 2 - two plates closest to the track)

  // lets plot Cherenkov theta angle for pi,K tracks, assuming photons were emitted from the 1st aerogel layer
  // and not reflected from mirrors

  TH1F* h_chK = new TH1F("h_chK","Cherenkov angle distribution for kaons/pions;#theta_{ch}", 200,0,0.5);
  TH1F* h_chPi = new TH1F("h_chPi","Cherenkov angle distribution for kaons/pions;#theta_{ch}", 200,0,0.5);
  ch->Draw("photons.m_thetaCer>>h_chPi","abs(mcHit.PDG)==211 && photons.m_mirror==0 && primary==1  && " + momCut);
  ch->Draw("photons.m_thetaCer>>h_chK","abs(mcHit.PDG)==321  && photons.m_mirror==0 && primary==1 && " + momCut);
  //--------------------------------------

  TLegend* leg = new TLegend(0.1,0.75,0.48,0.9);
  leg->AddEntry(h_momK,"kaon","l");
  leg->AddEntry(h_momPi,"pion","l");

  TLegend* leg1 = new TLegend(0.1,0.75,0.48,0.9);
  leg1->AddEntry(h_eff3,"6% #pi fake rate","l");
  leg1->AddEntry(h_eff2,"4% #pi fake rate","l");
  leg1->AddEntry(h_eff1,"2% #pi fake rate","l");


  // Draw histograms on canvas and save
  TCanvas* c1 = new TCanvas("c1");
  c1->Update();
  c1->Divide(4,2);
  c1->cd(1);
  hh->Draw("colz");
  c1->cd(2);
  h_eff3->Draw();
  h_eff2->SetLineColor(kRed);
  h_eff2->Draw("same");
  h_eff1->SetLineColor(kGreen);
  h_eff1->Draw("same");
  leg1->Draw();
  c1->cd(3);
  h_trk->Draw("colz");
  c1->cd(4);
  h_momPi->Draw();
  h_momK->SetLineColor(kRed);
  h_momK->Draw("sames");
  leg->Draw();
  c1->cd(5);
  h_thetaPi->Draw();
  h_thetaK->SetLineColor(kRed);
  h_thetaK->Draw("sames");
  c1->cd(6);
  h_nphotPi->Draw();
  h_nphotK->SetLineColor(kRed);
  h_nphotK->Draw("sames");
  c1->cd(7);
  h_chPi->Draw();
  h_chK->SetLineColor(kRed);
  h_chK->Draw("sames");
  c1->cd(8);
  h_lkhPi->Draw();
  h_lkhK->SetLineColor(kRed);
  h_lkhK->Draw("sames");

  c1->SaveAs("ARICHPerformance.root");
}
