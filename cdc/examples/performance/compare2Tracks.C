/************************************************************************/
/*                                                                      */
/*   This script is for checking CDC performance using cosmic muon      */
/*   Input files are from output of CDCCosmicAnalysis module            */
/*   Question send to cdc@belle2.org                                    */
/*                                                                      */
/************************************************************************/
#include "TROOT.h"
void loadStyle();
void fit(TH1D* h1);


// Apply loose or tight selection
// bool loose_selection(false);
// "compare" this result with a previous result.
// The previous result should be defined by result4Compare.
//  bool compare(true);
void compare2Tracks(bool loose_selection = false, bool compare = true)
{

  // Input file names:
  std::vector<std::string> input_filenames = {"cosmic/rootfile/twotracks*"};

  // Run analysis for positve (pos) or negative (neg) or both (all)
  TString sCharge = "all";

  // Root file contain histograms for comparing with this results.
  // it must be set properly if compare= true
  TString result4Compare = "/home/belle2/dvthanh/public/cdc_perform/result_Exp2_good_selection.root";//Exp2

  //color of input histo
  //1:black 2:red 3:green 4:blue 5:yellow 6:magenta 7:cyan
  // 5(Y)=2(R)+3(G) 6(M)=2(R)+4(B) 7(C)=3(G)+4(B)
  //std::string label4Compare = "July"; //label of input histo
  int color4Compare = 2;

  // Legend of the input histograms
  std::string label4Compare = "Exp2 (Feb-2018)";

  // Belle CDC parameters
  // Belle CDC only: = 0.28Pt oplus 0.35/beta %
  double Param_belle_CDC[2] = {0.28, 0.35};

  // mininum number of degree freedom
  int ndfmin = 25;

  //selection for track parameter resolutio as a function of Pt.
  double cut_z0min, cut_z0max, cut_d0max, cut_tanlmin, cut_tanlmax;
  if (loose_selection) {
    cut_z0min = -5;
    cut_z0max = 10;
    cut_d0max = 3;
    cut_tanlmin = -0.45;
    cut_tanlmax = 0.45;
  } else {
    cut_z0min = -2;
    cut_z0max = 4;
    cut_d0max = 3;
    cut_tanlmin = -1.4;
    cut_tanlmax = 0.65;
  }

  //starting to read data and draw
  loadStyle();

  //limits for ploting
  double max_dD0 = 0.1;
  double max_dZ0 = 1;
  double max_dPhi0 = 0.4;
  double max_dTanl = 0.02;
  double max_dPt   = 0.04;

  TH1D* hNDF1 = new TH1D("hNDF1", "Degree of Freedom;ndf;  #tracks   ", 100, 0, 100);
  TH1D* hNDF2 = new TH1D("hNDF2", "Degree of Freedom;ndf  ;#tracks  ", 100, 0, 100);
  TH1D* hPval1 = new TH1D("hPval1", "Prob(#chi^{2}); p-value  ;#tracks  ", 600, -0.1, 1.1);
  TH1D* hPval2 = new TH1D("hPval2", "Fit Probability; p-value  ;#tracks  ", 600, -0.1, 1.1);
  TH1D* hPhi01 = new TH1D("hPhi01", "#varphi_{0} distribution ;#varphi_{0} (Deg.)  ; #Tracks", 360, -180, 180);
  TH1D* hPhi02 = new TH1D("hPhi02", "#varphi_{0} distribution ;#varphi_{0} (Deg.)  ; #Tracks", 360, -180, 180);
  TH1D* htanLambda1 = new TH1D("htanLambda1", "tan#lambda distribution;  tan#lambda  ;#tracks  ", 100, -2, 2);
  TH1D* htanLambda2 = new TH1D("htanLambda2", "tan#lambda distribution;  tan#lambda  ;#tracks  ", 100, -2, 2);
  TH1D* hD01 = new TH1D("hD01", "d_{0} distribution ; d_{0} (cm)  ;#tracks", 200, -30, 30);
  TH1D* hD02 = new TH1D("hD02", "d_{0} distribution ; d_{0} (cm)  ;#tracks", 200, -30, 30);
  TH1D* hZ01 = new TH1D("hZ01", "z_{0} distribution ; z_{0} (cm)  ;#tracks", 200, -80, 150);
  TH1D* hZ02 = new TH1D("hZ02", "z_{0} distribution ; z_{0} (cm)  ;#tracks", 200, -80, 150);

  TH1D* hOmega1 = new TH1D("hOmega1", "#omega distribution;#omega  (cm^{-1});#tracks  ", 200, -0.02, 0.02);
  TH1D* hOmega2 = new TH1D("hOmega2", "#omega distribution;#omega  (cm^{-1});#tracks  ", 200, -0.02, 0.02);

  TH1D* hPt1 = new TH1D("hPt1", "P_{t} distribution;P_{t} (GeV/c)  ;#tracks  ", 200, 0, 15);
  TH1D* hPt2 = new TH1D("hPt2", "P_{t} distribution;P_{t} (GeV/c)  ;#tracks  ", 200, 0, 15);

  /**Different of track param Histogram*/
  TH1D* hdPt = new TH1D("hdPt", "#DeltaP_{t}/#surd2 ;#DeltaP_{t}/#sqrt{2}  (GeV/c)  ;Events  ", 200, -3 * max_dPt, 3 * max_dPt);
  TH1D* hdOmega = new TH1D("hdOmega", "#Delta#omega/#surd2;#Delta#omega/#sqrt{2}  (cm^{-1}) ;Events  ", 100, -0.00005, 0.00005);


  TH1D* hdD0 = new TH1D("hdD0", "#Deltad_{0}/#surd2; #Deltad_{0}/#sqrt{2} (cm)  ;Events", 75, -1 * max_dD0, max_dD0);
  TH1D* hdZ0 = new TH1D("hdZ0", "#Deltaz_{0}/#surd2; #Deltaz_{0}/#sqrt{2} (cm)  ;Events", 75, -1 * max_dZ0, max_dZ0);
  TH1D* hdPhi0 = new TH1D("hdPhi0", "#Delta#varphi_{0}/#surd2 ;#Delta#varphi_{0}/#sqrt{2} (Deg.)  ; Events ", 100, -1 * max_dPhi0,
                          max_dPhi0);
  TH1D* hdtanL = new TH1D("hdtanL", "#Deltatan#lambda/#surd2 ;#Deltatan#lambda/#sqrt{2}   ; Events ", 100, -1 * max_dTanl, max_dTanl);
  //Tanlambda dependence
  const int nbin(4);
  double binWidth = 180/nbin;
  TH2D* hdD0TanPhi0[nbin];
  TH2D* hdZ0TanPhi0[nbin];
  TH2D* hdD0TanL = new TH2D("hdD0TanL", "#Deltad_{0}/#surd2 vs. tan#lambda; tan#lambda;#Deltad_{0} [cm]", 50, -1., 1., 100,
                            -1 * max_dD0, max_dD0);
  TH2D* hdZ0TanL = new TH2D("hdZ0TanL", "#Deltaz_{0}/#surd2 vs. tan#lambda; tan#lambda;#Deltaz_{0} [cm]", 50, -1., 1., 100,
                            -1 * max_dZ0, max_dZ0);
  for (int i = 0; i < nbin; ++i) {
    hdD0TanPhi0[i] = new TH2D(Form("hdD0TanPhi0_%d", i), "#Deltad_{0}/#surd2 vs. tan#lambda; tan#lambda;#Deltad_{0} [cm]", 100, -1, 1,
                              100, -1 * max_dD0, max_dD0);
    hdZ0TanPhi0[i] = new TH2D(Form("hdZ0TanPhi0_%d", i), "#Deltaz_{0}/#surd2 vs. tan#lambda; tan#lambda;#Deltaz_{0} [cm]", 100, -1, 1,
                              100, -1 * max_dZ0, max_dZ0);
  }
  //Z0 dependence
  TH2D* hdD0Z0 = new TH2D("hdD0Z0", "#Deltad_{0}/#surd2 vs. z_{0}; z_{0};#Deltad_{0};", 100, -80, 150, 100, -1 * max_dD0, max_dD0);
  TH2D* hdZ0Z0 = new TH2D("hdZ0Z0", "#Deltaz_{0}/#surd2 vs. z_{0}; z_{0};#Deltaz_{0};", 100, -80, 150, 100, -1 * max_dZ0, max_dZ0);
  // D0 dependence
  TH2D* hdD0D0 = new TH2D("hdD0D0", "#Deltad_{0}/#surd2 vs. d_{0}; d_{0};#Deltad_{0};", 50, -50, 50, 100, -1 * max_dD0, max_dD0);
  TH2D* hdZ0D0 = new TH2D("hdZ0D0", "#Deltaz_{0}/#surd2 vs. d_{0}; d_{0};#Deltaz_{0};", 50, -50, 50, 100, -1 * max_dZ0, max_dZ0);
  // Phi0 dependence

  TH2D* hdD0Phi0 = new TH2D("hdD0Phi0", "#Deltad_{0}/#surd2 vs. #varphi_{0}; #varphi_{0};#Deltad_{0};", 60, -180, 0, 100, -0.15,
                            0.15);
  TH2D* hdZ0Phi0 = new TH2D("hdZ0Phi0", "#Deltaz_{0}/#surd2 vs. #varphi_{0}; #varphi_{0};#Deltaz_{0};", 60, -180, 0, 100, -1.5, 1.5);
  // Pt dependence
  TH2D* hdPPt = new TH2D("hdPPt", " #DeltaP vs. Pt ;P_{t} (Gev/c);#surd2(P^{up}-P^{down})/(P^{up}+P^{down})",
                         20, 0, 10, 100, -1 * max_dPt, max_dPt);
  TH2D* hdPtPt = new TH2D("hdPtPt", " #DeltaPt vs. Pt ;P_{t} (Gev/c);#surd2(P_{t}^{up}-P_{t}^{down})/(P_{t}^{up}+P_{t}^{down})",
                          20, 0, 10, 100, -1 * max_dPt, max_dPt);
  TH2D* hdPtPt_pos = new TH2D("hdPtPt_pos",
                              " #DeltaP vs. Pt (Neg. Charge);P_{t} (Gev/c);#surd2(P_{t}^{up}-P_{t}^{down})/(P_{t}^{up}+P_{t}^{down})",
                              10, 0, 10, 100, -1 * max_dPt, max_dPt);
  TH2D* hdPtPt_neg = new TH2D("hdPtPt_neg",
                              " #DeltaP vs. Pt (Neg. Charge) ;P_{t} (Gev/c);#surd2(P_{t}^{up}-P_{t}^{down})/(P_{t}^{up}+P_{t}^{down})",
                              10, 0, 10, 100, -1 * max_dPt, max_dPt);

  TH2D* hdD0Pt = new TH2D("hdD0Pt", "#Deltad_{0}/#surd2 vs. P_{t};P_{t};#Deltad_{0}", 20, 0, 10, 100, -1 * max_dD0, max_dD0);
  TH2D* hdZ0Pt = new TH2D("hdZ0Pt", "#Deltaz_{0}/#surd2 vs. P_{t};P_{t};#Deltaz_{0}", 20, 0, 10, 100, -1 * max_dZ0, max_dZ0);
  TH2D* hdPhi0Pt = new TH2D("hdPhi0Pt", "#Delta#phi_{0}/#surd2 vs. P_{t};P_{t};#Delta#phi0_{0} (deg)",
                            20, 0, 10, 100, -1 * max_dPhi0, max_dPhi0);
  TH2D* hdtanLPt = new TH2D("hdtanLPt", "#Deltatan#lambda/#surd2  vs. P_{t};P_{t};#Deltatan#lambda",
                            20, 0, 10, 100, -1 * max_dTanl, max_dTanl);
  TH2D* hdPtdPhi0 = new TH2D("hdPtdPhi0", ";P_{t};#Delta#phi_{0}", 100, -0.1, 0.1, 100, -1, 1);

  TH2D* hdPtD0 = new TH2D("hdPtD0", ";D0;#DeltaPt/Pt", 100, -15, 15, 100, -1 * max_dPt, max_dPt);
  TH2D* hdPtZ0 = new TH2D("hdPtZ0", ";Z0;#DeltaPt/Pt", 50, -15, 50, 100, -1 * max_dPt, max_dPt);

  TH2D* hEvtT0Pt = new TH2D("hEvtT0Pt", "", 20, 0, 10, 100, -40, 40);
  TH1D* hEvtT0 = new TH1D("hEvtT0", "", 100, -70, 70);
  TH3D* hdPtD0Phi0 = new TH3D("hdPtD0Phi0", "", 64, -16, 16, 36, -180, 0, 100, -0.05, 0.05);

  //Tanlambda dependence of dPt
  TH2D* hdPtTanl = new TH2D("hdPtTanl", "#DeltaPt vs. tan#lambda; tan#lambda; #DeltaP_{T}/P_{T}",
                            40, -1., 1., 100, -1 * max_dPt, max_dPt); //dpt
  //Phi0 dependence of dPt
  TH2D* hdPtPhi0 = new TH2D("hdPtPhi0", "#DeltaPt vs. #varphi_{0}; #varphi_{0}; #DeltaP_{T}/P_{T}",
                            36, -180, 0., 100, -1 * max_dPt, max_dPt); //dpt

  hNDF1->SetLineColor(kRed);
  hPval1->SetLineColor(kRed);
  hPhi01->SetLineColor(kRed);
  htanLambda1->SetLineColor(kRed);
  hD01->SetLineColor(kRed);
  hZ01->SetLineColor(kRed);
  hOmega1->SetLineColor(kRed);
  hPt1->SetLineColor(kRed);

  hdD0->GetXaxis()->SetNdivisions(8, 9);
  hdZ0->GetXaxis()->SetNdivisions(8, 9);
  hOmega1->GetXaxis()->SetNdivisions(8, 9);
  hdOmega->GetXaxis()->SetNdivisions(8, 9);
  hdtanL->GetXaxis()->SetNdivisions(8, 9);
  hdPhi0->GetXaxis()->SetNdivisions(8, 9);
  /*
  hdD0TanL->GetYaxis()->SetTitleOffset(1.4);
  hdZ0TanL->GetYaxis()->SetTitleOffset(1.4);
  hdD0TanL->GetYaxis()->SetTitleSize(0.05);
  hdZ0TanL->GetYaxis()->SetTitleSize(0.05);
  hdD0TanL->GetYaxis()->SetLabelSize(0.05);
  hdZ0TanL->GetYaxis()->SetLabelSize(0.05);*/
  TH1D* hdPtPt_sim;
  TH1D* hdD0Pt_sim;
  TH1D* hdZ0Pt_sim;
  TH1D* hdPhi0Pt_sim;
  TH1D* hdtanLPt_sim;
  TH1D*  hdPPt_sim;
  if (compare) {
    TFile ff(result4Compare);
    hdPtPt_sim = (TH1D*)ff.Get("hdPtPt_2;1");
    hdPPt_sim = (TH1D*)ff.Get("hdPPt_2;1");
    hdD0Pt_sim = (TH1D*)ff.Get("hdD0Pt_2;1");
    hdZ0Pt_sim = (TH1D*)ff.Get("hdZ0Pt_2;1");
    hdPhi0Pt_sim = (TH1D*)ff.Get("hdPhi0Pt_2;1");
    hdtanLPt_sim = (TH1D*)ff.Get("hdtanLPt_2;1");
    hdPtPt_sim->SetDirectory(0);
    hdPtPt_sim->SetLineColor(color4Compare);
    hdPtPt_sim->SetMarkerColor(color4Compare);
    hdPtPt_sim->GetFunction("f1")->SetLineColor(color4Compare);
    hdPtPt_sim->GetFunction("f1")->SetLineWidth(1);
    hdPPt_sim->SetDirectory(0);
    hdPPt_sim->SetLineColor(color4Compare);
    hdPPt_sim->SetMarkerColor(color4Compare);
    hdPPt_sim->GetFunction("f1")->SetLineColor(color4Compare);
    hdPPt_sim->GetFunction("f1")->SetLineWidth(1);

    hdD0Pt_sim->SetDirectory(0);
    hdD0Pt_sim->SetLineColor(color4Compare);
    hdZ0Pt_sim->SetDirectory(0);
    hdZ0Pt_sim->SetLineColor(color4Compare);
    hdPhi0Pt_sim->SetDirectory(0);
    hdPhi0Pt_sim->SetLineColor(color4Compare);
    hdtanLPt_sim->SetDirectory(0);
    hdtanLPt_sim->SetLineColor(color4Compare);
    ff.Close();
  }
  //Read data and fill histos

  TChain* tree = new TChain("tree");
  for (int i = 0; i < input_filenames.size(); ++i) {
    tree->Add(input_filenames[i].c_str());
    cout << "Input files: " << input_filenames[i].c_str() << endl;
  }
  if (!tree->GetBranch("Pval1")) return;
  double nEtr = tree->GetEntries();
  cout << "[INFO] Number of entries: " << nEtr << endl;

  double ndf1, Pval1, Phi01, tanLambda1, D01, Z01;
  double ndf2, Pval2, Phi02, tanLambda2, D02, Z02, evtT0;
  TVector3* posSeed1 = 0;
  TVector3* posSeed2 = 0;
  TVector3* Mom1 = 0;
  TVector3* Mom2 = 0;
  double Omega1, Omega2;
  short charge;

  tree->SetBranchAddress("evtT0", &evtT0);
  tree->SetBranchAddress("charge", &charge);
  tree->SetBranchAddress("ndf1", &ndf1);
  tree->SetBranchAddress("Pval1", &Pval1);
  tree->SetBranchAddress("Phi01", &Phi01);
  tree->SetBranchAddress("tanLambda1", &tanLambda1);
  tree->SetBranchAddress("D01", &D01);
  tree->SetBranchAddress("Z01", &Z01);
  tree->SetBranchAddress("posSeed1", &posSeed1);
  tree->SetBranchAddress("Omega1", &Omega1);
  tree->SetBranchAddress("Mom1", &Mom1);

  tree->SetBranchAddress("ndf2", &ndf2);
  tree->SetBranchAddress("Pval2", &Pval2);
  tree->SetBranchAddress("Phi02", &Phi02);
  tree->SetBranchAddress("tanLambda2", &tanLambda2);
  tree->SetBranchAddress("D02", &D02);
  tree->SetBranchAddress("Z02", &Z02);
  tree->SetBranchAddress("posSeed2", &posSeed2);
  tree->SetBranchAddress("Omega2", &Omega2);
  tree->SetBranchAddress("Mom2", &Mom2);

  Long64_t nbytes = 0;
  double sqrti = 1 / sqrt(2);
  for (int i = 0; i < nEtr; ++i) {
    nbytes += tree->GetEntry(i);
    if (Mom1->Dot(*Mom2) < 0) continue;

    if (posSeed1->Y() <  posSeed2->Y()) {
      swap(Phi01, Phi02);
      swap(Omega1, Omega2);
      swap(tanLambda1, tanLambda2);
      swap(Z01, Z02);
      swap(D01, D02);
      swap(ndf1, ndf2);
      swap(Pval1, Pval2);
      swap(Mom1, Mom2);
    }
    Phi01 *= 180 / M_PI;
    Phi02 *= 180 / M_PI;

    double dD0 = (D01 - D02) * sqrti;
    double dZ0 = (Z01 - Z02) * sqrti;
    double dPhi0 = (Phi01 - Phi02) * sqrti;
    double dtanLambda = (tanLambda1 - tanLambda2) * sqrti;
    double dOmega = (Omega1 - Omega2) * sqrti;
    double dPt = (Mom1->Perp() - Mom2->Perp()) * sqrti;
    double dP = (Mom1->Mag() - Mom2->Mag()) * sqrti;

    double D0m = (D01 + D02) * 0.5;
    double Z0m = (Z01 + Z02) * 0.5;
    double Phi0m = (Phi01 + Phi02) * 0.5;
    double tanLm = (tanLambda1 + tanLambda2) * 0.5;
    double Ptm = (Mom1->Perp() + Mom2->Perp()) * 0.5;
    double Pm = (Mom1->Mag() + Mom2->Mag()) * 0.5;

    double sigmaPt = dPt / Ptm;
    double sigmaP = dP / Pm;

    //    if(n>300000) continue;
    if (sCharge == "pos") {
      if (charge < 0) continue;
    } else if (sCharge == "neg") {
      if (charge > 0) continue;
    } else {}

    hNDF1->Fill(ndf1);   hPval1->Fill(Pval1); hPhi01->Fill(Phi01);  hD01->Fill(D01);
    hZ01->Fill(Z01);     htanLambda1->Fill(tanLambda1); hOmega1->Fill(Omega1); hPt1->Fill(Mom1->Perp());
    hNDF2->Fill(ndf2);    hPval2->Fill(Pval2); hPhi02->Fill(Phi02);  hD02->Fill(D02);
    hZ02->Fill(Z02); htanLambda2->Fill(tanLambda2); hOmega2->Fill(Omega2); hPt2->Fill(Mom2->Perp());
    hEvtT0->Fill(evtT0);
    // NDF cut
    if (ndf1 < ndfmin || ndf2 < ndfmin) continue;
    if (Phi01 > 0 || Phi02 > 0) continue;

    int index = floor(fabs(Phi01) / binWidth);
    if (index >= 0 && index < nbin) {
      hdD0TanPhi0[index]->Fill(tanLm, dD0);
      hdZ0TanPhi0[index]->Fill(tanLm, dZ0);
    }
    // z0 dependence, cut d0
    if (fabs(D0m) < 5 && fabs(tanLm) < 0.35) {
      hdD0Z0->Fill(Z0m, dD0);
      hdZ0Z0->Fill(Z0m, dZ0);
    }
    //D0 dependence, cut z0
    if (Z0m > -5 && Z0m < 10) {
      hdD0D0->Fill(D0m, dD0);
      hdZ0D0->Fill(D0m, dZ0);
    }
    if (fabs(Z0m) < 5 && Ptm > 0.5 && Ptm < 1.5)
      hdPtD0->Fill(D0m, sigmaPt);
    if (fabs(D0m) < 5 && Ptm > 0.5 && Ptm < 1.5)
      hdPtZ0->Fill(Z0m, sigmaPt);
    // cut at both d0 and z0 dependence for other dependence
    if (Z0m > -5 && Z0m < 10 &&  fabs(D0m) < 5) {
      hdD0TanL->Fill(tanLm, dD0);
      hdZ0TanL->Fill(tanLm, dZ0);
    }

    if (Z0m > -5 && Z0m < 10 &&  fabs(D0m) < 5 && fabs(tanLm) < 0.5) {
      hdD0Phi0->Fill(Phi0m, dD0);
      hdZ0Phi0->Fill(Phi0m, dZ0);
    }

    if (Ptm < 2.5) {
      hdPtD0Phi0->Fill(D0m, Phi0m, sigmaPt);
    }

    if (Z0m > cut_z0min && Z0m < cut_z0max &&  fabs(D0m) < cut_d0max && tanLm < cut_tanlmax && tanLm > cut_tanlmin) {
      //    if(Z0m>-5 && Z0m < 10 &&  fabs(D0m) <3 && fabs(tanLm) < 0.45){
      //    if(Z0m>-2 && Z0m < 4 &&  fabs(D0m) <3 && tanLm < 0.65 && tanLm>-1.4){
      hdD0->Fill(dD0);
      hdZ0->Fill(dZ0);
      hdPhi0->Fill(dPhi0);
      hdtanL->Fill(dtanLambda);
      hdPt->Fill(dPt);
      hdOmega->Fill(dOmega);

      hdPtPt->Fill(Ptm, sigmaPt);
      hdPPt->Fill(Ptm, sigmaP);
      hdD0Pt->Fill(Ptm, dD0);
      hdZ0Pt->Fill(Ptm, dZ0);
      hdPhi0Pt->Fill(Ptm, dPhi0);
      hdtanLPt->Fill(Ptm, dtanLambda);
      hEvtT0Pt->Fill(Ptm, evtT0);
      hdPtdPhi0->Fill(sigmaPt, dPhi0);
      if (charge > 0)
        hdPtPt_pos->Fill(Ptm, sigmaPt);
      if (charge < 0)
        hdPtPt_neg->Fill(Ptm, sigmaPt);
    }
    if (Z0m > -2 && Z0m < 4 &&  fabs(D0m) < 4 && Ptm < 3) {
      hdPtTanl->Fill(tanLm, sigmaPt);
      hdPtPhi0->Fill(Phi0m, sigmaPt);
    }

  }//finish reading data

  /***************************DRAWING********************/
  //parameter for drawing
  double max_d0m = 0.01;
  double max_d0s = 0.04;
  double max_z0m = 0.1;
  double max_z0s = 0.4;
  // track fit quality
  TLatex lt;  lt.SetTextSize(0.032);
  TCanvas* c1 = new TCanvas("c1", "", 900, 800);
  c1->Divide(2, 2);
  c1->cd(1);
  TDatime date;
  lt.DrawLatex(0.15, 0.8, date.AsString());
  lt.DrawLatex(0.15, 0.75, Form("Inputs: %s", input_filenames[0].c_str()));
  lt.DrawLatex(0.15, 0.7, "-------------------------------------------------------------------------------------");
  lt.DrawLatex(0.15, 0.65, "+ Page 1 and 2 are raw distributions (no cut).");
  lt.DrawLatex(0.15, 0.6, Form("+ NDF > %d is applied for pages 3-end", ndfmin));
  lt.DrawLatex(0.15, 0.55, loose_selection ? "+ Loose selection is applied: " :
               "+ Tight selection is applied for plots in pages: 3, 8, 9, 10,");
  lt.DrawLatex(0.15, 0.5, "   and #DeltaPt vs. Pt (p11): ");
  lt.DrawLatex(0.15, 0.45, Form("    #bullet |d_{0}| < %3.2f (cm)", cut_d0max));
  lt.DrawLatex(0.15, 0.4, Form("    #bullet %3.2f < z_{0} < %3.2f (cm)", cut_z0min, cut_z0max));
  lt.DrawLatex(0.15, 0.35, Form("    #bullet %3.2f < tan#lambda < %3.2f", cut_tanlmin, cut_tanlmax));
  lt.DrawLatex(0.15, 0.30, "+ Page-4, tan#lambda dependence: |d_{0}|<5  & -5< z_{0} <10 (cm)");
  lt.DrawLatex(0.15, 0.25, "+ Page-5, #varphi_{0} dependence:   |d_{0}|<5 & -5< z_{0} <10 (cm) & |tan#lambda| <0.5");
  lt.DrawLatex(0.15, 0.2, "+ More details: #it{cdc/examples/performance/compare2Tracks.C}");

  c1->cd(2); hEvtT0->Draw();
  c1->cd(3); hNDF1->Draw(); hNDF2->Draw("same");
  c1->cd(4); hPval1->Draw(); hPval2->Draw("same");
  c1->Print("cdc_performance_with_cr.pdf(", "Title:Fit_QA");

  //Second is track parameters
  delete c1; c1 = new TCanvas("c1", "", 1200, 800);
  c1->Divide(3, 2);
  c1->cd(1); hPhi01->Draw(); hPhi02->Draw("same");
  c1->cd(2); hOmega1->Draw(); hOmega2->Draw("same");
  c1->cd(3); hPt1->Draw(); hPt2->Draw("same");

  c1->cd(4); htanLambda1->Draw(); htanLambda2->Draw("same");
  c1->cd(5); hD01->Draw(); hD02->Draw("same");
  c1->cd(6); hZ01->Draw(); hZ02->Draw("same");
  c1->Print("cdc_performance_with_cr.pdf", "Title:Pars_distribution");

  gStyle->SetStatH(0.2);//change statistics box size here
  gStyle->SetStatW(0.2);
  gStyle->SetOptStat(0000);
  // Draw difference of track params
  TCanvas* c2 = new TCanvas("c2", "", 1200, 800);
  c2->Divide(3, 2);
  c2->cd(1); hdD0->Draw(); fit(hdD0);
  c2->cd(2); hdZ0->Draw(); fit(hdZ0);
  c2->cd(3); hdPhi0->Draw(); fit(hdPhi0);
  c2->cd(4); hdtanL->Draw(); fit(hdtanL);
  c2->cd(5); hdOmega->Draw(); fit(hdOmega);
  c2->cd(6); hdPt->Draw(); fit(hdPt);
  c2->Print("cdc_performance_with_cr.pdf", "Title:Pars_diff");

  //TanL dependece of D0, Z0
  TCanvas* c3 = new TCanvas("c3", "", 1200, 800);
  c3->Divide(3, 2);

  TPad* pad1 = (TPad*)c3->GetPrimitive("c3_1");
  //  pad1->cd();pad1->SetGrid();
  TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
  pad2->SetGrid();

  TPad* pad5 = (TPad*)c3->GetPrimitive("c3_5");
  pad5->SetGrid();

  c3->cd(1);
  hdD0TanL->GetXaxis()->SetNdivisions(8, 9);
  hdD0TanL->Draw("colz");
  hdD0TanL->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdD0TanL = (TH1D*)gDirectory->Get("hdD0TanL_1");
  TH1D* s_hdD0TanL = (TH1D*)gDirectory->Get("hdD0TanL_2");
  if (m_hdD0TanL && m_hdD0TanL->GetEntries() > 10) {
    c3->cd(2);
    m_hdD0TanL->SetMinimum(max_d0m * -1);
    m_hdD0TanL->SetMaximum(max_d0m);
    m_hdD0TanL->GetYaxis()->SetTitleOffset(1.4);
    s_hdD0TanL->SetMinimum(0.);
    s_hdD0TanL->SetMaximum(max_d0s);
    //      m_D0TanL->Fit("pol1","","",-1,1);
    m_hdD0TanL->GetXaxis()->SetNdivisions(8, 9);
    s_hdD0TanL->SetTitle("d_{0} resolution vs. tan#lambda ; tan#lambda   ;d_{0} resolution  (cm)  ");
    m_hdD0TanL->SetTitle("#Deltad_{0} vs. tan#lambda ; tan#lambda   ;#Deltad_{0}  (cm)  ");

    //      TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
    pad2->cd();//pad2->SetGrid();
    m_hdD0TanL->Draw();
    c3->cd(3);
    s_hdD0TanL->Draw();
  }

  c3->cd(4); hdZ0TanL->Draw("colz");
  hdZ0TanL->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdZ0TanL = (TH1D*)gDirectory->Get("hdZ0TanL_1");
  TH1D* s_hdZ0TanL = (TH1D*)gDirectory->Get("hdZ0TanL_2");
  if (m_hdZ0TanL  && m_hdZ0TanL->GetEntries() > 10) {
    m_hdZ0TanL->SetMinimum(-1 * max_z0m);
    m_hdZ0TanL->SetMaximum(max_z0m);
    m_hdZ0TanL->GetYaxis()->SetTitleOffset(1.4);
    s_hdZ0TanL->SetMinimum(0.);
    s_hdZ0TanL->SetMaximum(max_z0s);
    hdZ0TanL->GetXaxis()->SetNdivisions(8, 9);
    m_hdZ0TanL->GetXaxis()->SetNdivisions(8, 9);
    s_hdZ0TanL->SetTitle("z_{0} resolution vs. tan#lambda ; tan#lambda   ;z_{0} resolution  (cm)  ");
    m_hdZ0TanL->SetTitle("#Deltaz_{0} vs. tan#lambda ; tan#lambda   ;#Deltaz_{0}  (cm)  ");

    c3->cd(5);
    //    TPad* pad5 = (TPad*)c3->GetPrimitive("c3_5");
    pad5->cd(); //pad5->SetGrid();
    m_hdZ0TanL->Draw();
    c3->cd(6); s_hdZ0TanL->Draw();
  }
  c3->Print("cdc_performance_with_cr.pdf", "Title:tanL_dependence");

  //Phi Depdendence
  c3->Clear("D");
  c3->cd(1); hdD0Phi0->Draw("colz");
  hdD0Phi0->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdD0Phi0 = (TH1D*)gDirectory->Get("hdD0Phi0_1");
  TH1D* s_hdD0Phi0 = (TH1D*)gDirectory->Get("hdD0Phi0_2");
  if (m_hdD0Phi0) {
    m_hdD0Phi0->SetMinimum(-1 * max_d0m);
    m_hdD0Phi0->SetMaximum(max_d0m);
    s_hdD0Phi0->SetMinimum(0);
    s_hdD0Phi0->SetMaximum(max_d0s);
    s_hdD0Phi0->SetTitle("d_{0} resolution vs. #varphi_{0} ; #varphi_{0} (Deg.)   ;d_{0} resolution  (cm)  ");
    m_hdD0Phi0->SetTitle("#Deltad_{0} vs. #varphi_{0} ; #varphi_{0} (Deg.)   ;#Deltad_{0}  (cm)  ");
    c3->cd(2); m_hdD0Phi0->Draw();
    c3->cd(3); s_hdD0Phi0->Draw();
  }
  c3->cd(4); hdZ0Phi0->Draw("colz");
  hdZ0Phi0->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdZ0Phi0 = (TH1D*)gDirectory->Get("hdZ0Phi0_1");
  TH1D* s_hdZ0Phi0 = (TH1D*)gDirectory->Get("hdZ0Phi0_2");
  if (m_hdZ0Phi0) {
    m_hdZ0Phi0->SetMinimum(-1 * max_z0m);
    m_hdZ0Phi0->SetMaximum(max_z0m);
    s_hdZ0Phi0->SetMinimum(0);
    s_hdZ0Phi0->SetMaximum(max_z0s);
    s_hdZ0Phi0->SetTitle("z_{0} resolution vs. #varphi_{0} ; #varphi_{0} (Deg.)   ;z_{0} resolution  (cm)  ");
    m_hdZ0Phi0->SetTitle("#Deltaz_{0} vs. #varphi_{0} ; #varphi_{0} (Deg.)   ;#Deltaz_{0}  (cm)  ");
    c3->cd(5); m_hdZ0Phi0->Draw();
    c3->cd(6); s_hdZ0Phi0->Draw();
  }
  c3->Print("cdc_performance_with_cr.pdf", "Title:diff_Phi0");

  /***D0 dependence***/
  c3->Clear("D");
  c3->cd(1); hdD0D0->Draw("colz");
  hdD0D0->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdD0D0 = (TH1D*)gDirectory->Get("hdD0D0_1");
  TH1D* s_hdD0D0 = (TH1D*)gDirectory->Get("hdD0D0_2");
  if (m_hdD0D0) {
    m_hdD0D0->SetMinimum(-1 * max_d0m);
    m_hdD0D0->SetMaximum(max_d0m);
    s_hdD0D0->SetMinimum(0);
    s_hdD0D0->SetMaximum(max_d0s);
    s_hdD0D0->SetTitle("d_{0} resolution vs. d_{0} ; d_{0} (cm)   ;d_{0} resolution  (cm) ");
    m_hdD0D0->SetTitle("#Deltad_{0} vs. d_{0} ; d_{0} (cm)   ;#Deltad_{0}  (cm) ");
    c3->cd(2); m_hdD0D0->Draw();
    c3->cd(3); s_hdD0D0->Draw();
  }
  c3->cd(4); hdZ0D0->Draw("colz");
  hdZ0D0->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdZ0D0 = (TH1D*)gDirectory->Get("hdZ0D0_1");
  TH1D* s_hdZ0D0 = (TH1D*)gDirectory->Get("hdZ0D0_2");
  if (s_hdZ0D0) {
    m_hdZ0D0->SetMinimum(-1 * max_z0m);
    m_hdZ0D0->SetMaximum(max_z0m);
    s_hdZ0D0->SetMinimum(0);
    s_hdZ0D0->SetMaximum(max_z0s);
    s_hdZ0D0->SetTitle("z_{0} resolution vs. d_{0} ; d_{0} (cm)   ;z_{0} resolution  (cm) ");
    m_hdZ0D0->SetTitle("#Deltaz_{0} vs. d_{0} ; d_{0} (cm)   ;#Deltaz_{0}  (cm) ");
    c3->cd(5); m_hdZ0D0->Draw();
    c3->cd(6); s_hdZ0D0->Draw();
  }
  c3->Print("cdc_performance_with_cr.pdf", "Title:d0_depend");

  c3->Clear("D");
  c3->cd(1); hdD0Z0->Draw("colz");
  hdD0Z0->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdD0Z0 = (TH1D*)gDirectory->Get("hdD0Z0_1");
  TH1D* s_hdD0Z0 = (TH1D*)gDirectory->Get("hdD0Z0_2");
  if (m_hdD0Z0) {
    c3->cd(2);
    m_hdD0Z0->SetMinimum(-1 * max_d0m);    m_hdD0Z0->SetMaximum(max_d0m);
    s_hdD0Z0->SetMinimum(0);    s_hdD0Z0->SetMaximum(max_d0s);
    m_hdD0Z0->SetTitle("#Deltad_{0} vs. z_{0} ; z_{0} (cm)   ;#Deltad_{0}  (cm) ");
    s_hdD0Z0->SetTitle("d_{0} resolution vs. z_{0} ; z_{0} (cm)   ;d_{0} resolution  (cm) ");
    m_hdD0Z0->Draw();
    c3->cd(3); s_hdD0Z0->Draw();
  }

  c3->cd(4); hdZ0Z0->Draw("colz");
  hdZ0Z0->FitSlicesY(0, 0, -1, 20);
  TH1D* m_hdZ0Z0 = (TH1D*)gDirectory->Get("hdZ0Z0_1");
  TH1D* s_hdZ0Z0 = (TH1D*)gDirectory->Get("hdZ0Z0_2");
  if (m_hdZ0Z0) {
    c3->cd(5);
    m_hdZ0Z0->SetMinimum(-1 * max_z0m);    m_hdZ0Z0->SetMaximum(max_z0m);
    m_hdZ0Z0->SetTitle("#Deltaz_{0} vs. z_{0} ; z_{0} (cm)   ;#Deltaz_{0}  (cm) ");
    s_hdZ0Z0->SetMinimum(0);    s_hdZ0Z0->SetMaximum(max_z0s);
    s_hdZ0Z0->SetTitle("z_{0} resolution vs. z_{0} ; z_{0} (cm)   ;z_{0} resolution  (cm) ");
    m_hdZ0Z0->Draw();
    c3->cd(6); s_hdZ0Z0->Draw();
  }
  c3->Print("cdc_performance_with_cr.pdf", "Title:z0_depend");

  /*********dPt of Pt****************/
  //Belle results
  TF1* fbelle = new TF1("fbelle", "sqrt([0]*[0]*x*x+[1]*[1])", 0, 10);
  //  fbelle->SetParameters(0.201,0.29);// CDC+SVD
  fbelle->SetParameters(Param_belle_CDC);//CDC only
  fbelle->SetLineColor(kMagenta - 7);
  fbelle->SetLineStyle(2);
  fbelle->SetLineWidth(3);

  ///
  TF1* f1 = new TF1("f1", "sqrt([0]*[0]*x*x+[1]*[1])", 0, 10);
  f1->SetParameters(0.2, 0.2);
  f1->SetLineColor(kBlack);
  f1->SetLineWidth(1);
  TF1* f2 = (TF1*)f1->Clone("f2");
  c3->Clear("D");
  c3->cd(1);
  hdPtPt->GetXaxis()->SetNdivisions(8, 9);
  hdPtPt->Draw("colz");
  hdPtPt->FitSlicesY(0, 0, -1, 0);
  TH1D* hdPtPt_m = (TH1D*)gDirectory->Get("hdPtPt_1");
  TH1D* hdPtPt_s = (TH1D*)gDirectory->Get("hdPtPt_2");
  //  TH1D* hdPtPt_s = (TH1D*)SliceFit::doSliceFitY(hdPtPt,10)->Clone("hdPtPt_2");
  //  hdPtPt_s->SetDirectory(0);

  if (hdPtPt_m && hdPtPt_m->GetEntries() > 3 && hdPtPt_s) {
    c3->cd(2);
    gStyle->SetOptFit(0);
    pad2->cd();
    hdPtPt_m->SetTitle("#DeltaPt/Pt;P_{t} (GeV/c)  ; #DeltaP_{t}/P_{t}   ");
    hdPtPt_m->SetMinimum(-0.005);
    hdPtPt_m->SetMaximum(0.005);
    hdPtPt_m->Draw();
    hdPtPt_m->GetXaxis()->SetNdivisions(8, 9);
    c3->cd(3);

    hdPtPt_s->SetTitle("Pt Resolution; Pt (Gev/c)   ;Pt resolution (%)   ");
    hdPtPt_s->Scale(100);
    hdPtPt_s->SetMinimum(0);
    hdPtPt_s->SetMaximum(2);
    hdPtPt_s->SetMarkerStyle(8);
    hdPtPt_s->SetMarkerSize(0.6);
    hdPtPt_s->Draw();
    hdPtPt_s->Fit("f1", "MQ", "", 1.5, 10);
    gPad->Update();
    lt.DrawLatex(1.5, 0.12, Form("#sigma_{Pt}/Pt(%%) = (%3.3f#pm%3.3f)Pt #oplus (%3.3f#pm%3.3f)",
                                 fabs(f1->GetParameter(0)), f1->GetParError(0),
                                 fabs(f1->GetParameter(1)), f1->GetParError(1)));
    //      TPaveStats *s = (TPaveStats*)hdPtPt_s->GetListOfFunctions()->FindObject("stats");
    //      s->SetBorderSize(0);  s->SetFillColor(0);s->SetShadowColor(0);
    //      s->SetX1(0.5); s->SetY1(1.5);s->SetX2(6); s->SetY2(1.95);
    //      gPad->Modified();
    fbelle->DrawF1(0.6, 10, "same");
    lt.SetTextColor(kMagenta - 7);
    lt.DrawLatex(7, 1.7, "#splitline{Belle}{CDC only}");
    if (compare) {
      cout << "draw Sim Pt resolution" << endl;
      //  hdPtPt_sim->Scale(100);
      hdPtPt_sim->SetStats(0);
      hdPtPt_sim ->Draw("same");
      lt.SetTextColor(color4Compare);
      lt.DrawLatex(0.5, 1.8, label4Compare.c_str());
    }
  }

  //For Momentum reso vs. Transver mom
  c3->cd(4);
  hdPPt->GetXaxis()->SetNdivisions(8, 9);
  hdPPt->Draw("colz");
  hdPPt->FitSlicesY(0, 0, -1, 20);
  TH1D* hdPPt_m = (TH1D*)gDirectory->Get("hdPPt_1");
  TH1D* hdPPt_s = (TH1D*)gDirectory->Get("hdPPt_2");
  if (hdPPt_m && hdPPt_m->GetEntries() > 3) {
    c3->cd(5);
    hdPPt_s->SetTitle("P resolution;Pt (Gev/c)     ;P resolution  (%)  ");
    pad5->cd();
    hdPPt_m->SetTitle("#DeltaP/P vs. Pt; Pt (GeV/c)   ; #DeltaP/P (Gev/c)   ");
    hdPPt_m->SetMinimum(-0.005);
    hdPPt_m->SetMaximum(0.005);
    hdPPt_m->Draw();
    hdPPt_m->GetXaxis()->SetNdivisions(8, 9);
    c3->cd(6);
    hdPPt_s->Scale(100);
    hdPPt_s->SetMinimum(0);
    hdPPt_s->SetMaximum(2);
    hdPPt_s->Draw();
    hdPPt_s->Fit("f2", "MQ", "", 1.5, 10);
    lt.SetTextColor(kBlack);
    lt.DrawLatex(1.5, 0.12, Form("#sigma_{P}/P(%%) = (%3.3f#pm%3.3f)Pt #oplus (%3.3f#pm%3.3f)",
                                 fabs(f2->GetParameter(0)), f2->GetParError(0),
                                 fabs(f2->GetParameter(1)), f2->GetParError(1)));

    if (compare) {
      hdPPt_sim->SetStats(0);
      hdPPt_sim ->Draw("same");
    }
  }

  c3->Print("cdc_performance_with_cr.pdf", "Title:Pt_reso");
  c3->Clear("D");
  ///Pt dependence
  hdD0Pt->FitSlicesY(0, 0, -1, 5);
  TH1D* hdD0Pt_m = (TH1D*)gDirectory->Get("hdD0Pt_1");
  TH1D* hdD0Pt_s = (TH1D*)gDirectory->Get("hdD0Pt_2");
  if (hdD0Pt_s) {
    hdD0Pt_s->SetTitle("d_{0} resolution;Pt (Gev/c)   ;d_{0} resolution (cm)");
    hdD0Pt_m->SetTitle("#Deltad_{0} vs. Pt ; Pt (GeV/c)   ;#Deltad_{0}  (cm)  ");
    hdD0Pt_m->SetMinimum(-1 * max_d0m);
    hdD0Pt_m->SetMaximum(max_d0m);
    hdD0Pt_s->SetMinimum(0);
    hdD0Pt_s->SetMaximum(0.12);
    c3->cd(1); hdD0Pt->Draw("colz");
    c3->cd(2); hdD0Pt_m->Draw();
    c3->cd(3); hdD0Pt_s->Draw();
    if (compare &&  hdD0Pt_sim) {
      hdD0Pt_sim ->Draw("same");
    }
  }

  hdZ0Pt->FitSlicesY(0, 0, -1, 5);
  TH1D* hdZ0Pt_m = (TH1D*)gDirectory->Get("hdZ0Pt_1");
  TH1D* hdZ0Pt_s = (TH1D*)gDirectory->Get("hdZ0Pt_2");
  if (hdZ0Pt_s) {
    hdZ0Pt_s->SetTitle("z_{0} resolution;Pt (GeV/c)   ;z_{0} resolution (cm)   ");
    hdZ0Pt_m->SetTitle(" #Deltaz_{0} vs. Pt ;Pt (GeV/c)   ;#Deltaz_{0} (cm) ");
    hdZ0Pt_m->SetMinimum(-1 * max_z0m);
    hdZ0Pt_m->SetMaximum(max_z0m);
    hdZ0Pt_s->SetMinimum(0);
    hdZ0Pt_s->SetMaximum(max_z0s);
    c3->cd(4); hdZ0Pt->Draw("colz");
    c3->cd(5); hdZ0Pt_m->Draw();
    c3->cd(6); hdZ0Pt_s->Draw();
    if (compare) {
      hdZ0Pt_sim ->Draw("same");
    }
  }
  c3->Print("cdc_performance_with_cr.pdf", "Title:d0z0_vs._Pt");

  c3->Clear("D");
  hdPhi0Pt->FitSlicesY(0, 0, -1, 5);
  TH1D* hdPhi0Pt_m = (TH1D*)gDirectory->Get("hdPhi0Pt_1");
  TH1D* hdPhi0Pt_s = (TH1D*)gDirectory->Get("hdPhi0Pt_2");
  if (hdPhi0Pt_s) {
    hdPhi0Pt_s->SetTitle("#varphi_{0} resolution  ;Pt (GeV/c)  ;#varphi_{0} resolution (deg)  ");
    hdPhi0Pt_m->SetTitle("#Delta#varphi_{0}  vs. Pt ; Pt (GeV/c)  ; #Delta#varphi_{0}  (deg)  ");
    hdPhi0Pt_m->SetMinimum(-0.025);
    hdPhi0Pt_m->SetMaximum(0.025);
    hdPhi0Pt_s->SetMinimum(0);
    hdPhi0Pt_s->SetMaximum(0.3);
    c3->cd(1); hdPhi0Pt->Draw("colz");
    c3->cd(2); hdPhi0Pt_m->Draw();
    c3->cd(3); hdPhi0Pt_s->Draw();
    if (compare) {
      hdPhi0Pt_sim ->Draw("same");
    }
  }

  hdtanLPt->FitSlicesY(0, 0, -1, 5);
  TH1D* hdtanLPt_m = (TH1D*)gDirectory->Get("hdtanLPt_1");
  TH1D* hdtanLPt_s = (TH1D*)gDirectory->Get("hdtanLPt_2");
  if (hdtanLPt_s) {
    hdtanLPt_s->SetTitle(" tan#lambda resolution ;Pt (GeV/c)  ; tan#lambda resolution     ");
    hdtanLPt_m->SetTitle(" #Deltatan#lambda  vs. Pt ;Pt (GeV/c)  ; #Deltatan#lambda  ");
    hdtanLPt_m->SetMinimum(-0.002);
    hdtanLPt_m->SetMaximum(0.002);
    hdtanLPt_s->SetMinimum(0);
    hdtanLPt_s->SetMaximum(0.01);

    c3->cd(4); hdtanLPt->Draw("colz");
    c3->cd(5); hdtanLPt_m->Draw();
    c3->cd(6); hdtanLPt_s->Draw();
    if (compare) {
      hdtanLPt_sim ->Draw("same");
    }
  }
  c3->Print("cdc_performance_with_cr.pdf", "Title:angle_vs_Pt");
  delete c3;
  c3 = new TCanvas("c3", "", 900, 800); c3->Divide(2, 2);
  //  pad1 = (TPad*)c3->GetPrimitive("c3_1");
  TPad* pad3 = (TPad*)c3->GetPrimitive("c3_3");

  hdPtPt_neg->FitSlicesY(0, 0, -1, 20);
  TH1D* hdPtPt_neg_m = (TH1D*)gDirectory->Get("hdPtPt_neg_1");
  TH1D* hdPtPt_neg_s = (TH1D*)gDirectory->Get("hdPtPt_neg_2");
  hdPtPt_pos->FitSlicesY(0, 0, -1, 20);
  TH1D* hdPtPt_pos_m = (TH1D*)gDirectory->Get("hdPtPt_pos_1");
  TH1D* hdPtPt_pos_s = (TH1D*)gDirectory->Get("hdPtPt_pos_2");

  TF1* fpos = new TF1("fpos", "sqrt([0]*[0]*x*x+[1]*[1])", 0, 10);
  fpos->SetParameters(0.2, 0.2);
  fpos->SetLineColor(kRed);
  fpos->SetLineWidth(1);
  TF1* fneg = new TF1("fneg", "sqrt([0]*[0]*x*x+[1]*[1])", 0, 10);
  fneg->SetParameters(0.2, 0.2);
  fneg->SetLineColor(kBlue);
  fneg->SetLineWidth(1);
  gStyle->SetOptFit(0000);
  gStyle->SetOptStat(0000);
  if (hdPtPt_neg_m && hdPtPt_pos_m && hdPtPt_neg_s && hdPtPt_pos_s) {
    pad1->cd(); pad1->SetGrid();
    hdPtPt_pos_m->SetTitle("#DeltaPt/Pt;P_{t} (GeV/c)  ; #DeltaP_{t}/P_{t}   ");
    hdPtPt_pos_m->SetLineColor(kRed);
    hdPtPt_neg_m->SetLineColor(kBlue);
    hdPtPt_pos_m->SetMinimum(-0.005);
    hdPtPt_pos_m->SetMaximum(0.005);
    hdPtPt_pos_m->GetXaxis()->SetNdivisions(8, 9);
    //      s_dPtPt->SetTitle(";#sigma[]sP_{t};#sigmaP_{t}");
    hdPtPt_pos_s->SetTitle("Pt Resolution; Pt (Gev/c)   ;Pt resolution (%)   ");
    hdPtPt_pos_s->Scale(100);
    hdPtPt_pos_s->SetMinimum(0);
    hdPtPt_pos_s->SetMaximum(2);
    hdPtPt_pos_s->SetMarkerStyle(8);
    hdPtPt_pos_s->SetMarkerSize(0.6);

    hdPtPt_neg_s->Scale(100);
    hdPtPt_neg_s->SetMinimum(0);
    hdPtPt_neg_s->SetMaximum(2);
    hdPtPt_neg_s->SetMarkerStyle(8);
    hdPtPt_neg_s->SetMarkerSize(0.6);

    hdPtPt_pos_m->SetMarkerColor(kRed);
    hdPtPt_neg_m->SetMarkerColor(kBlue);
    //    TLegend* lg = new TLegend(0.2, 0.7, 0.45, 0.85);
    //    lg->AddEntry(hdPtPt_pos_m, "Pos. charge");
    //    lg->AddEntry(hdPtPt_neg_m, "Neg. charge");
    hdPtPt_pos_m->Draw();
    hdPtPt_neg_m->Draw("same");
    //    lg->Draw();

    int scale =1000;
    TF1* fp = new TF1("fp","pol1",0,10); fp->SetLineColor(kRed);
    TF1* fn = new TF1("fn","pol1",0,10) ;fn->SetLineColor(kBlue);
    hdPtPt_pos_m->Fit("fp","Q");
    hdPtPt_neg_m->Fit("fn","Q");
    lt.SetTextColor(kRed);
    lt.DrawLatex(0.5, 0.0042, Form("(+) #DeltaPt/Pt = [(%3.2f#pm%3.2f) + (%3.2f#pm%3.2f)Pt ]#times10^{-3}",
				   fp->GetParameter(0)*scale, fp->GetParError(0)*scale,
				   fp->GetParameter(1)*scale, fp->GetParError(1)*scale));

    lt.SetTextColor(kBlue);
    lt.DrawLatex(0.5, -0.0046, Form("(-) #DeltaPt/Pt = [(%3.2f#pm%3.2f) + (%3.2f#pm%3.2f)Pt ]#times10^{-3}",
				    fn->GetParameter(0)*scale, fn->GetParError(0)*scale,
				    fn->GetParameter(1)*scale, fn->GetParError(1)*scale));



    //      pad2->cd(); pad2->SetGrid(0,0);
    c3->cd(2);
    double par_pos[2];
    double par_neg[2];
    hdPtPt_pos_s->SetMarkerColor(kRed);
    hdPtPt_neg_s->SetMarkerColor(kBlue);
    hdPtPt_pos_s->SetLineColor(kRed);
    hdPtPt_neg_s->SetLineColor(kBlue);

    hdPtPt_pos_s->Draw();
    hdPtPt_neg_s->Draw("same");
    hdPtPt_pos_s->Fit("fpos", "MQ", "", 1.5, 10);
    fpos->GetParameters(par_pos);

    hdPtPt_neg_s->Fit("fneg", "MQ", "", 1.5, 10);
    fneg->GetParameters(par_neg);
    lt.SetTextColor(kRed);
    lt.DrawLatex(0.5, 1.8, Form("(+) #sigma_{Pt}/Pt(%%) = (%3.3f#pm%3.3f)Pt #oplus (%3.3f#pm%3.3f)",
                                fabs(par_pos[0]), fpos->GetParError(0),
                                fabs(par_pos[1]), fpos->GetParError(1)));

    lt.SetTextColor(kBlue);
    lt.DrawLatex(0.5, 1.6, Form("(-) #sigma_{Pt}/Pt(%%) = (%3.3f#pm%3.3f)Pt #oplus (%3.3f#pm%3.3f)",
                                fabs(par_neg[0]), fneg->GetParError(0),
                                fabs(par_neg[1]), fneg->GetParError(1)));
  }

  hdPtTanl->FitSlicesY(0, 0, -1, 15);
  TH1D* hdPtTanl_m = (TH1D*)gDirectory->Get("hdPtTanl_1");
  TH1D* hdPtTanl_s = (TH1D*)gDirectory->Get("hdPtTanl_2");
  if (hdPtTanl_m && hdPtTanl_s) {
    hdPtTanl_m->SetMinimum(-0.01);    hdPtTanl_m->SetMaximum(0.01);
    hdPtTanl_m->SetTitle("#DeltaPt vs. tan#lambda ; tan#lambda; #DeltaPt/Pt ");

    hdPtTanl_s->Scale(100);
    hdPtTanl_s->SetMinimum(0);    hdPtTanl_s->SetMaximum(1);
    hdPtTanl_s->SetTitle("Pt resolution vs. tan#lambda ; tan#lambda; Pt resolution (%) ");
    pad3->cd(); pad3->SetGrid(); hdPtTanl_m->Draw();
    c3->cd(4); hdPtTanl_s->Draw();

  }

  c3->Print("cdc_performance_with_cr.pdf)", "Title:others");

  cout << "Finish" << endl;
  TFile* result = new TFile("result.root", "recreate");
  result->cd();
  hD01->Write();
  hZ01->Write();
  htanLambda1->Write();
  hPhi01->Write();
  hPt1->Write();

  hdD0->Write();
  hdZ0->Write();
  hdPtPt->Write();
  if (hdPPt_s)
    hdPPt_s->Write();
  if (hdPtPt_s)
    hdPtPt_s->Write();
  if (hdZ0Pt_s)
    hdZ0Pt_s->Write();
  if (hdD0Pt_s)
    hdD0Pt_s->Write();
  if (hdPhi0Pt_s)
    hdPhi0Pt_s->Write();
  if (hdtanLPt_s)
    hdtanLPt_s->Write();
  //hPtReso->Write();
  hdD0D0->Write();
  hdZ0TanL->Write();
  hdD0TanL->Write();
  hdD0Phi0->Write();
  hdZ0Phi0->Write();
  hEvtT0->Write();
  hEvtT0Pt->Write();
  hdPtdPhi0->Write();
  hdPtD0->Write();
  //D0 phi0 dependence
  hdPtPhi0->Write();
  hdPtTanl->Write();
  result->Close();
}
void loadStyle()
{
  gStyle->SetTitleOffset(1.5, "y");
  gStyle->SetPadLeftMargin(0.16);
  gStyle->SetPadRightMargin(0.1);
  gStyle->SetPadTopMargin(0.14);
  gStyle->SetOptStat("nerm");
  gStyle->SetOptFit(0011);
  gStyle->SetPalette(1);
  gStyle->SetEndErrorSize(0);
  gROOT->SetBatch(1);
  gStyle->SetStatW(0.32);
  gStyle->SetTitleX(0.075);
  gStyle->SetTitleY(0.91);
  gStyle->SetTitleAlign(11);
  gStyle->SetTitleBorderSize(2);
  gStyle->SetStatH(0.1);
  gStyle->SetLabelSize(0.04, "xyz"); // size of axis value font
  gStyle->SetTitleSize(0.045, "xyz"); // size of axis title font
  gStyle->SetTitleFont(22, "xyz"); // font option
  gStyle->SetLabelFont(22, "xyz");
  gStyle->SetGridColor(15);
  TGaxis::SetMaxDigits(4);
  gROOT->ForceStyle();
}
void fit(TH1D* h1)
{
  gStyle->SetOptFit(1111);
  double p1 = h1->GetMaximum();
  double p3 = h1->GetRMS();
  Double_t par[6];
  TF1* f1 = new TF1("f1", "gaus(0)+gaus(3)", 2, 2);
  f1->SetParNames("A1", "mean1", "#sigma1", "A2", "mean2", "#sigma2");
  f1->SetLineColor(kRed);
  f1->SetParameters(p1 * 0.8, 0., p3 * 0.6, p1 * 0.4, 0, p3 * 2.);
  TF1* g1 = new TF1("g1", "gaus", -2, 2); g1->SetLineColor(kBlue);
  TF1* g2 = new TF1("g2", "gaus", -2, 2); g2->SetLineColor(kGreen);
  h1->Fit("f1", "MQ", "", -3 * p3, 3 * p3);
  f1->GetParameters(par);
  g1->SetParameters(&par[0]);
  g2->SetParameters(&par[3]);
  g1->Draw("same");
  g2->Draw("same");

}

