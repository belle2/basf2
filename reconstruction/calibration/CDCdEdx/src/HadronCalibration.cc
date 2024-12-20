/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCdEdx/HadronCalibration.h>


using namespace Belle2;

HadronCalibration::HadronCalibration() {}


void HadronCalibration::fitBGCurve(std::vector< std::string > particles, std::string filename, std::string paramfile,
                                   std::string suffix)
{

  // read in a file that contains fit results for bg bins
  double bgmin1 = 0.25, bgmax1 = 5.1; //using untill 0 --> 4.5
  double bgmin2 = 3.9, bgmax2 = 15.0; //using till 4.5 --> 10
  double bgmin3 = 7.5, bgmax3 = 5000; //using above 10 --> 6000 *use this range only

  const int npart = 5;
  TFile* infile = new TFile(filename.data());

  CDCDedxMeanPred gpar;
  gpar.setParameters(paramfile.data());
  CDCDedxWidgetCurve* gc = new CDCDedxWidgetCurve();

  // multigraphs to hold the curve and residual results
  TGraphErrors part_dedxvsbg[npart];
  TMultiGraph* gr_dedxvsbg = new TMultiGraph(Form("gr_dedxvsbg_%s", suffix.data()), ";#beta#gamma;dE/dx");
  TGraphErrors part_dedxvsp[npart];
  TMultiGraph* gr_dedxvsp = new TMultiGraph(Form("gr_dedxvsp_%s", suffix.data()), ";momentum(GeV/c);dE/dx");

  // --------------------------------------------------
  // FILL BG CURVE VALUES
  // --------------------------------------------------
  TLegend tleg(0.4, 0.50, 0.65, 0.85);
  tleg.SetBorderSize(0);

  TLegend tlegPart(0.80, 0.80, 0.98, 0.98);
  tlegPart.SetBorderSize(0);

  for (int i = 0; i < int(particles.size()); ++i) {

    std::string particle = particles[i];

    double mass = m_prep.getParticleMass(particle);
    if (mass == 0.0) B2FATAL("Mass of particle " << particle.data() << " is zero");

    if (!infile->GetListOfKeys()->Contains(particle.data())) continue;

    TTree* hadron = (TTree*) infile->Get(particle.data());
    B2INFO("HadronCalibration: reading " << particle.data() << " in file " << filename.data());

    double dedx, dedxerr, bg;      // dE/dx without electron saturation correction

    hadron->SetBranchAddress("dedx", &dedx);
    hadron->SetBranchAddress("dedxerr", &dedxerr);
    hadron->SetBranchAddress("bg_avg", &bg);

    for (int j = 0; j < hadron->GetEntries(); ++j) {

      hadron->GetEvent(j);

      part_dedxvsbg[i].SetPoint(j, bg, dedx);
      part_dedxvsbg[i].SetPointError(j, 0, dedxerr);

      part_dedxvsp[i].SetPoint(j, bg * mass, dedx);
      part_dedxvsp[i].SetPointError(j, 0, dedxerr);
    }

    part_dedxvsbg[i].SetName(particle.data());
    if (i == 4) setGraphStyle(part_dedxvsbg[i], i + 2);
    else setGraphStyle(part_dedxvsbg[i], i + 1);
    gr_dedxvsbg->Add(&part_dedxvsbg[i]);

    if (i == 4) setGraphStyle(part_dedxvsp[i], i + 2);
    else setGraphStyle(part_dedxvsp[i], i + 1);
    gr_dedxvsp->Add(&part_dedxvsp[i]);

    tleg.AddEntry(&part_dedxvsbg[i], particles[i].data(), "p");
    tlegPart.AddEntry(&part_dedxvsbg[i], particles[i].data(), "p");

  }

  TMultiGraph* grcopy1 = (TMultiGraph*)gr_dedxvsbg->Clone(Form("datapoints_%s", suffix.data()));

  //Region 1
  TF1* fdedx1 = new TF1("fdedx1", gc, bgmin1, bgmax1, 8, "WidgetCurve");
  TF1* fdedx1Copy = new TF1("fdedx1Copy", gc, bgmin1, bgmax1, 8, "WidgetCurve");
  fdedx1->SetParameter(0, 1);
  fdedx1Copy->SetParameter(0, 1);
  for (int i = 1; i < 8; ++i) {
    fdedx1->SetParameter(i, gpar.getCurvePars(i - 1));
    fdedx1Copy->SetParameter(i, gpar.getCurvePars(i - 1));
  }

  //Region 2
  TF1* fdedx2 = new TF1("fdedx2", gc, bgmin2, bgmax2, 5, "WidgetCurve");
  TF1* fdedx2Copy = new TF1("fdedx2Copy", gc, bgmin2, bgmax2, 5, "WidgetCurve");
  fdedx2->SetParameter(0, 2);
  fdedx2Copy->SetParameter(0, 2);
  for (int i = 1; i < 5; ++i) {
    fdedx2->SetParameter(i, gpar.getCurvePars(6 + i));
    fdedx2Copy->SetParameter(i, gpar.getCurvePars(6 + i));
  }

  //Region 3
  TF1* fdedx3 = new TF1("fdedx3", gc, bgmin3, bgmax3, 5, "WidgetCurve");
  TF1* fdedx3Copy = new TF1("fdedx3Copy", gc, bgmin3, bgmax3, 5, "WidgetCurve");
  fdedx3->SetParameter(0, 3);
  fdedx3Copy->SetParameter(0, 3);
  for (int i = 1; i < 5; ++i) {
    fdedx3->SetParameter(i, gpar.getCurvePars(10 + i));
    fdedx3Copy->SetParameter(i, gpar.getCurvePars(10 + i));
  }

  // --------------------------------------------------
  // FIT BG CURVE
  // --------------------------------------------------

  // //Fitting part1 1/beta^2 region and new constants
  // //----------------------------------------------

  int stat1 = gr_dedxvsbg->Fit("fdedx1", "", "", bgmin1, bgmax1);
  for (int i = 0; i < 50; ++i) {
    if (stat1 == 0) {
      B2INFO("\n\tPART-1 FIT STATUS is OK: irr # " << i);
      break;
    }
    stat1 = gr_dedxvsbg->Fit("fdedx1", "", "", bgmin1, bgmax1);
  }

  // if the fit was successful, write out the updated parameters
  if (stat1 == 0) {
    B2INFO("\t--> HadronCalibration: ATTENTIONS: PART-1 FIT OK..updating parameters");
    for (int i = 1; i < 8; ++i) {
      B2INFO("\t" << i << ") Old = " << gpar.getCurvePars(i - 1) << " --> New = " << fdedx1->GetParameter(i));
      gpar.setCurvePars(i - 1, fdedx1->GetParameter(i));
    }
  } else B2INFO("\t--> HadronCalibration: WARNING: PART-1 FIT FAILED...");


  //Fitting part2 min ionisation region and new constants
  //----------------------------------------------
  int stat2 = gr_dedxvsbg->Fit("fdedx2", "FQR", "", bgmin2, bgmax2);
  for (int i = 0; i < 50; ++i) {
    if (stat2 == 0) {
      B2INFO("\n\tPART-2 FIT STATUS is OK: irr # " << i);
      break;
    }
    stat2 = gr_dedxvsbg->Fit("fdedx2", "FQR", "", bgmin2, bgmax2);
  }
  if (stat2 == 0) {
    B2INFO("\t--> HadronCalibration: ATTENTIONS: PART-2 FIT OK..updating parameters");
    for (int i = 1; i < 5; ++i) {
      B2INFO("\t" << i << ") Old = " << gpar.getCurvePars(6 + i) << " --> New = " << fdedx2->GetParameter(i));
      gpar.setCurvePars(6 + i, fdedx2->GetParameter(i));
    }
  } else B2INFO("\t--> HadronCalibration: WARNING: PART-2 FIT FAILED...");


  // //Fitting part3 relativistic region and new constants
  // //----------------------------------------------
  int stat3 = gr_dedxvsbg->Fit("fdedx3", "FQR", "", bgmin3, bgmax3);
  for (int i = 0; i < 50; ++i) {
    if (stat3 == 0) {
      B2INFO("\n\tPART-3 FIT STATUS is OK: irr # " << i);
      break;
    }
    stat3 = gr_dedxvsbg->Fit("fdedx3", "FQR", "", bgmin3, bgmax3);
  }

  if (stat3 == 0) {
    B2INFO("\t--> HadronCalibration: ATTENTIONS: PART-3 FIT OK..updating parameters");
    for (int i = 1; i < 5; ++i) {
      B2INFO("\t" << i << ") Old = " << gpar.getCurvePars(10 + i) << " --> New = " << fdedx3->GetParameter(i));
      gpar.setCurvePars(10 + i, fdedx3->GetParameter(i));
    }
  } else B2INFO("\t--> HadronCalibration: WARNING: PART-3 FIT FAILED...");

  // //Plot without fitting (old fits + data points)

  TLine bgline1(4.5, 0.50, 4.5, 1.20);
  bgline1.SetLineStyle(kDashed);
  bgline1.SetLineColor(kGray);
  TLine bgline2(10.0, 0.50, 10.0, 1.20);
  bgline2.SetLineStyle(kDashed);
  bgline2.SetLineColor(kGray);
  TLine dedxline1(0.75, 1.0, 1000, 1.0);
  dedxline1.SetLineStyle(kDashed);
  dedxline1.SetLineColor(kGray);

  setFitterStyle(fdedx1, 2, 1);
  setFitterStyle(fdedx2, 5, 1);
  setFitterStyle(fdedx3, 8, 1);

  setFitterStyle(fdedx1Copy, 13, 6);
  setFitterStyle(fdedx2Copy, 4, 6);
  setFitterStyle(fdedx3Copy, 1, 6);

  TCanvas bgcurvecan(Form("bgcurvecan_%s", suffix.data()), "bg curve and fitting", 1400, 800);
  bgcurvecan.Divide(3, 2);
  for (int i = 0; i < 6; i++) {
    TMultiGraph* grcopy = (TMultiGraph*)grcopy1->Clone(Form("datapoints_%s_%d", suffix.data(), i));

    bgcurvecan.cd(i + 1);
    gPad->cd();
    if (i == 0 || i == 3) gPad->SetLogy();
    gPad->SetLogx();
    grcopy->GetListOfGraphs()->SetDrawOption("AXIS");
    grcopy->Draw("A*");
    gPad->Modified(); gPad->Update();

    if (i == 0 || i == 3) {
      grcopy->GetXaxis()->SetLimits(0.10, 14500);
      grcopy->SetMinimum(0.50);
      grcopy->SetMaximum(50.0);
    } else if (i == 1 || i == 4) {
      grcopy->GetXaxis()->SetLimits(0.75, 100);
      grcopy->SetMinimum(0.020);
      grcopy->SetMaximum(3.0);
    } else if (i == 2 || i == 5) {
      grcopy->GetXaxis()->SetLimits(0.75, 50);
      grcopy->SetMinimum(0.50);
      grcopy->SetMaximum(1.20);
    }
    gPad->Modified(); gPad->Update();
    TPaveText* ptold = new TPaveText(.35, .40, .75, .50, "blNDC");
    ptold->SetBorderSize(0);
    ptold->SetFillStyle(0);
    if (i < 3) {
      fdedx1Copy->Draw("same");
      fdedx2Copy->Draw("same");
      fdedx3Copy->Draw("same");
      ptold->AddText("Old parameters");
    } else {
      fdedx1->Draw("same");
      fdedx2->Draw("same");
      fdedx3->Draw("same");
      ptold->AddText("New parameters");

    }

    bgline1.Draw("same");
    bgline2.Draw("same");
    dedxline1.Draw("same");

    if (i == 0) {
      tleg.AddEntry(fdedx1Copy, "Pub-Fit: 1", "f1");
      tleg.AddEntry(fdedx2Copy, "Pub-Fit: 2", "f1");
      tleg.AddEntry(fdedx3Copy, "Pub-Fit: 3", "f1");
      tleg.Draw("same");
    }
    if (i == 0 || i == 3) ptold->Draw("same");
  }

  bgcurvecan.SaveAs(Form("plots/HadronCal/BGfits/bgcurve_vsfits_%s.pdf", suffix.data()));

  TCanvas bgcurveraw(Form("bgcurveraw_%s", suffix.data()), "bg curvs", 600, 600);
  gPad->SetLogy();
  gPad->SetLogx();
  grcopy1->GetListOfGraphs()->SetDrawOption("AXIS");
  grcopy1->Draw("A*");
  gPad->Modified(); gPad->Update();
  tlegPart.Draw("same");
  fdedx1->Draw("same");
  fdedx2->Draw("same");
  fdedx3->Draw("same");
  bgcurveraw.SaveAs(Form("plots/HadronCal/BGfits/bgcurve_raw_%s.root", suffix.data()));
  bgcurveraw.SaveAs(Form("plots/HadronCal/BGfits/bgcurve_raw_%s.pdf", suffix.data()));

  bgcurveraw.cd();
  gPad->SetLogy();
  gPad->SetLogx();
  gr_dedxvsp->GetListOfGraphs()->SetDrawOption("AXIS");
  gr_dedxvsp->Draw("A*");
  gPad->Modified(); gPad->Update();
  tlegPart.Draw("same");
  bgcurveraw.SaveAs(Form("plots/HadronCal/BGfits/dedx_vs_mom_raw_%s.pdf", suffix.data()));


  double func1a = fdedx1->Eval(4.5);
  double func2a = fdedx2->Eval(4.5);
  double func2b = fdedx2->Eval(10);
  double func3a = fdedx3->Eval(10);
  double diffval1 = 100 * abs(func1a - func2a) / func2a;
  double diffval2 = 100 * abs(func2b - func3a) / func3a;
  B2INFO("\t\n FIT Constraint for 1/beta^2 region (bg = 4.5): func1 --> " << func1a << ", func2 --> " << func2a <<
         ", diff in % = " << diffval1);
  B2INFO("\t\n FIT Constraint for 1/beta^2 region (bg = 10.): func1 --> " << func2b << ", func2 --> " << func3a <<
         ", diff in % = " << diffval2);


  // --------------------------------------------------
  // GET RESIDUALS AND CHIS
  // --------------------------------------------------
  TMultiGraph* fit_bgratio = new TMultiGraph(Form("fit_bgratio_%s", suffix.data()), ";#beta#gamma;ratio");
  TGraph part_bgfit_ratio[npart];

  TMultiGraph* fit_residual = new TMultiGraph(Form("fit_residual_%s", suffix.data()), ";#beta#gamma;residual");
  TGraph part_bgfit_residual[npart];

  double A = 4.5, B = 10;
  int respoint = 1;
  double rmin = 1.0, rmax = 1.0;

  for (int i = 0; i < npart; ++i) {

    for (int j = 0; j < part_dedxvsbg[i].GetN(); ++j) {

      double x, y, fit;
      part_dedxvsbg[i].GetPoint(j, x, y);

      if (y == 0) continue;

      if (x < A)
        fit = fdedx1->Eval(x);
      else if (x < B)
        fit = fdedx2->Eval(x);
      else
        fit = fdedx3->Eval(x);

      // the curve is just 1 for electrons...
      if (npart == 4) fit = 1.0;
      if (x < 2000) {
        part_bgfit_ratio[i].SetPoint(respoint++, x, fit / y);
        part_bgfit_residual[i].SetPoint(respoint++, x, fit - y);
      }

      if (fit / y < rmin) rmin = fit / y;
      else if (fit / y > rmax) rmax = fit / y;

    }

    part_bgfit_ratio[i].SetMarkerSize(0.50);
    part_bgfit_ratio[i].SetMarkerStyle(4);
    part_bgfit_ratio[i].SetMarkerColor(i + 1);
    if (i == 4) part_bgfit_ratio[i].SetMarkerColor(i + 2);
    if (i <= 3)fit_bgratio->Add(&part_bgfit_ratio[i]);

    part_bgfit_residual[i].SetMarkerSize(0.50);
    part_bgfit_residual[i].SetMarkerStyle(4);
    part_bgfit_residual[i].SetMarkerColor(i + 1);
    if (i == 4)part_bgfit_residual[i].SetMarkerColor(i + 2);
    if (i <= 3)fit_residual->Add(&part_bgfit_residual[i]);
  }

  fit_bgratio->SetMinimum(rmin * 0.97);
  fit_bgratio->SetMaximum(rmax * 1.03);

  TCanvas* bgfitratiocan = new TCanvas(Form("bgfitratiocan_%s", suffix.data()), "dE/dx fit residual", 450, 350);
  bgfitratiocan->cd()->SetLogx();
  bgfitratiocan->cd()->SetGridy();
  fit_bgratio->Draw("AP");
  tlegPart.Draw("same");
  bgfitratiocan->SaveAs(Form("plots/HadronCal/BGfits/bgfit_ratios_%s.pdf", suffix.data()));
  delete bgfitratiocan;

  fit_residual->SetMinimum(-0.12);
  fit_residual->SetMaximum(+0.12);

  TCanvas* bgrescan = new TCanvas(Form("bgrescan_%s", suffix.data()), "dE/dx fit residual", 450, 350);
  bgrescan->cd()->SetLogx();
  bgrescan->cd()->SetGridy();
  fit_residual->Draw("AP");
  tlegPart.Draw("same");
  bgrescan->SaveAs(Form("plots/HadronCal/BGfits/bgfit_residual_%s.pdf", suffix.data()));
  delete bgrescan;

  // write out the (possibly) updated parameters to file
  gpar.printParameters("parameters.bgcurve.fit"); //Creating new file with new parameters

  delete gr_dedxvsbg;
  delete gr_dedxvsp;

  delete fdedx1;
  delete fdedx2;
  delete fdedx3;
  delete fdedx1Copy;
  delete fdedx2Copy;
  delete fdedx3Copy;

  infile->Close();
}

void HadronCalibration::plotBGMonitoring(std::vector< std::string > particles, std::string filename, std::string suffix)
{
  //1. chi-mean vs bg
  std::string title = "chi-fit-means of different particles;#beta#gamma;#chi(#mu)";
  std::string sname = Form("gr_chimean_vs_bg_%s", suffix.data());
  plotMonitoring(particles, filename, sname, title, "bg", "chi");

  //2. Sigma vs bg
  title = "chi-fit-width of different particles;#beta#gamma;#chi(#sigma)";
  sname = Form("gr_chiwidth_vs_bg_%s", suffix.data());
  plotMonitoring(particles, filename, sname, title, "bg", "sigma");

  //3. chi mean vs p
  title = "chi-fit-means of different particles: latest curve+sigma pars;p(GeV/c);#chi(#mu)";
  sname = Form("gr_chimean_vs_mom_%s", suffix.data());
  plotMonitoring(particles, filename, sname, title, "mom", "chi");

  //4. Sigma vs p
  title = "chi-fit-width of different particles: w/ latest curve+sigma pars;p(GeV/c);#chi(#sigma)";
  sname = Form("gr_chiwidth_vs_mom_%s", suffix.data());
  plotMonitoring(particles, filename, sname, title, "mom", "sigma");

  std::string svar = "ler";
  for (int ir = 0; ir < 2; ir++) {
    if (ir == 1) svar = "her";

    //5. chi mean vs injection time
    title = Form("#chi mean vs injection time, %s;injection time;#chi_{#mu}", svar.data());
    sname = Form("gr_chimean_vs_inj_%s_%s", suffix.data(), svar.data());
    plotMonitoring(particles, filename, sname, title, svar, "chi");

    //5. chi sigma vs injection time
    title = Form("#chi sigma vs injection time, %s;injection time;#chi_{#sigma}", svar.data());
    sname = Form("gr_chiwidth_vs_inj_%s_%s", suffix.data(), svar.data());
    plotMonitoring(particles, filename, sname, title, svar, "sigma");
  }
}

void HadronCalibration::plotMonitoring(std::vector< std::string > particles, std::string filename, std::string sname,
                                       std::string title,
                                       std::string sxvar, std::string syvar)
{

  const int npart = int(particles.size());

  // read in a file that contains fit results for bg bins
  TFile* infile = new TFile(filename.data());

  // multigraphs to hold the curve and residual results
  TGraphErrors grchim[npart];
  TMultiGraph* gr_var = new TMultiGraph(Form("%s", sname.data()), "");

  TLegend tlegPart(0.75, 0.65, 0.90, 0.90);
  tlegPart.SetBorderSize(0);

  for (int i = 0; i < npart; ++i) {
    std::string particle = particles[i];
    double mass = m_prep.getParticleMass(particle);
    if (mass == 0.0) B2FATAL("Mass of particle " << particle.data() << " is zero");

    if (!infile->GetListOfKeys()->Contains(particle.data())) continue;
    TTree* hadron;
    if (sxvar == "bg"  || sxvar == "mom") hadron = (TTree*)infile->Get(particle.data());
    else hadron = (TTree*)infile->Get(Form("%s_%s", particle.data(), sxvar.data()));

    B2INFO("HadronCalibration: reading " << particle.data() << " in file " << filename.data());

    double chimean, chisigma, avg, chimean_err, chisigmaerr;

    hadron->SetBranchAddress("chimean", &chimean);
    hadron->SetBranchAddress("chimean_err", &chimean_err);
    hadron->SetBranchAddress("chisigma", &chisigma);
    hadron->SetBranchAddress("chisigma_err", &chisigmaerr);
    if (sxvar == "bg"  || sxvar == "mom") hadron->SetBranchAddress("bg_avg", &avg);
    else hadron->SetBranchAddress("inj_avg", &avg);

    for (int j = 0; j < hadron->GetEntries(); ++j) {

      hadron->GetEvent(j);
      double var, varerr, xvar;
      if (syvar == "chi") {var = chimean;  varerr = chimean_err;}
      else  {var = chisigma;  varerr = chisigmaerr;}
      if (sxvar == "mom") xvar = avg * mass;
      else xvar = avg;

      grchim[i].SetPoint(j, xvar, var);
      grchim[i].SetPointError(j, 0, varerr);
    }

    tlegPart.AddEntry(&grchim[i], particles[i].data(), "p");

    setGraphStyle(grchim[i], i + 1);
    if (i == 4) grchim[i].SetMarkerColor(i + 2);
    gr_var->Add(&grchim[i]);

  }

  TCanvas ctemp(Form("%s", sname.data()), Form("%s", sname.data()), 450, 350);
  gPad->SetGridy();
  if (sxvar == "bg" || sxvar == "mom")  gPad->SetLogx();

  if (syvar == "chi") { gr_var->SetMinimum(-1.0); gr_var->SetMaximum(+1.0);}
  else { gr_var->SetMinimum(0.6); gr_var->SetMaximum(1.4); }

  gr_var->SetTitle(Form("%s", title.data()));
  gr_var->Draw("AP");
  tlegPart.Draw("same");
  ctemp.SaveAs(Form("plots/HadronCal/Monitoring/%s.pdf", sname.data()));
  if (sxvar == "bg" || sxvar == "mom") {
    gPad->SetLogx();
    if (sxvar == "bg") gr_var->GetXaxis()->SetLimits(0.1, 20);
    else gr_var->GetXaxis()->SetLimits(0.1, 1.0);
    gr_var->Draw("AP");
    tlegPart.Draw("same");
    ctemp.SaveAs(Form("plots/HadronCal/Monitoring/%s_zoomed.pdf", sname.data()));
  }
  delete gr_var;
}

void HadronCalibration::fitSigmavsIonz(std::vector< std::string > particles, std::string filename, std::string paramfile,
                                       std::string suffix)
{

  // read in a file that contains fit results for bg bins
  const int npart = int(particles.size());

  CDCDedxSigmaPred gpar;
  gpar.setParameters(paramfile.data());

  TFile* infile = new TFile(filename.data());

  TGraphErrors part_resovsdedx[npart];
  TMultiGraph* gr_resovsdedx = new TMultiGraph("gr_resovsdedx", ";dedx;#sigma(ionz)");

  TLegend tlegPart(0.72, 0.15, 0.88, 0.40);

  for (int i = 0; i < int(particles.size()); ++i) {

    std::string particle = particles[i];

    double mass = m_prep.getParticleMass(particle);
    if (mass == 0.0) B2FATAL("Mass of particle " << particle.data() << " is zero");

    if (particle == "electron") continue;

    if (!infile->GetListOfKeys()->Contains(particle.data())) continue;
    TTree* hadron = (TTree*)infile->Get(particle.data());
    B2INFO("\tHadronCalibration: reading " << particle << " in file " << filename.data());

    double dedx, ionzres;

    hadron->SetBranchAddress("dedx", &dedx);
    hadron->SetBranchAddress("ionzres", &ionzres);

    part_resovsdedx[i].SetName(particle.data());

    for (int j = 0; j < hadron->GetEntries(); ++j) {
      hadron->GetEvent(j);
      part_resovsdedx[i].SetPoint(j, dedx, ionzres);
    }

    if (i == 4) setGraphStyle(part_resovsdedx[i], i + 2);
    else setGraphStyle(part_resovsdedx[i], i + 1);
    gr_resovsdedx->Add(&part_resovsdedx[i]);
    tlegPart.AddEntry(&part_resovsdedx[i], particles[i].data(), "p");
  }

  gStyle->SetOptStat(0);
  gStyle->SetStatY(0.9);     // Set y-position (fraction of pad size)
  gStyle->SetStatX(0.4);     // Set x-position (fraction of pad size)
  gStyle->SetStatW(0.15);    // Set width of stat-box (fraction of pad size)
  gStyle->SetStatH(0.15);    // Set height of stat-box (fraction of pad size)

  TF1* sigvsdedx = new TF1("sigvsdedx", "[0]+[1]*x", 0.50, 15.0);
  sigvsdedx->SetParameter(0, gpar.getDedxPars(0));
  sigvsdedx->SetParameter(1, gpar.getDedxPars(1));

  TF1* sigvsdedxCopy = (TF1*)sigvsdedx->Clone("sigvsdedxcopy");
  setFitterStyle(sigvsdedxCopy, 13, 6);

  TCanvas sigcan("sigcan", " Reso(ionz) vs dE/dx", 820, 750);
  sigcan.cd();
  gr_resovsdedx->Draw("APE");
  sigvsdedxCopy->Draw("same");
  tlegPart.Draw("same");

  int status = gr_resovsdedx->Fit("sigvsdedx", "MF", "", 0.50, 7.0);
  for (int i = 0; i < 10; ++i) {
    if (status == 0) break;
    status = gr_resovsdedx->Fit("sigvsdedx", "", "", 0.50, 7.0);
  }
  gr_resovsdedx->SetTitle(Form("%s (slope = %0.03f, const = %0.03f)", gr_resovsdedx->GetTitle(), sigvsdedx->GetParameter(1),
                               sigvsdedx->GetParameter(0)));
  gPad->Modified(); gPad->Update();
  sigcan.SaveAs(Form("plots/HadronCal/Resofits/sigma_vsionz_%s.pdf", suffix.data()));

  gr_resovsdedx->GetXaxis()->SetLimits(0.2, 2.00);
  gr_resovsdedx->GetHistogram()->SetMaximum(0.50);
  gr_resovsdedx->GetHistogram()->SetMinimum(0.00);
  gr_resovsdedx->Draw("APE");
  sigvsdedxCopy->Draw("same");
  tlegPart.Draw("same");
  sigcan.SaveAs(Form("plots/HadronCal/Resofits/sigma_vsionz_zoomed_%s.pdf", suffix.data()));
  gStyle->SetOptStat(11);

  // if the fit was successful, save the updated parameters
  if (status == 0) {
    B2INFO("\tHadronCalibration: SigmavsdEdx FITs Ok. updating parameters");
    for (int i = 0; i < 2; ++i) {
      B2INFO("\t" << i << ") Old = " << gpar.getDedxPars(i) << " --> New = " << sigvsdedx->GetParameter(i));
      gpar.setDedxPars(i, sigvsdedx->GetParameter(i));
    }
  } else B2INFO("\tHadronCalibration: WARNING: SigmavsdEdx FIT FAILED... \n \tHadronCalibration: Skipping parameters");

  // write out the (possibly) updated parameters to file
  gpar.printParameters("parameters.ionz.fit"); //Creating new file with new parameters
  infile->Close();

  delete gr_resovsdedx;
  delete sigvsdedx;
}

void HadronCalibration::fitSigmaVsNHit(std::vector< std::string > particles, std::string filename, std::string paramsigma,
                                       std::string suffix)
{

  const double lowernhit = 7, uppernhit = 39;

  CDCDedxSigmaPred sgpar;
  sgpar.setParameters(paramsigma.data());

  CDCDedxWidgetSigma* gs = new CDCDedxWidgetSigma();

  TF1* fsigma = new TF1("fsigma", gs, lowernhit, uppernhit, 6, "CDCDedxWidgetSigma");
  fsigma->SetParameter(0, 2);
  for (int i = 1; i < 6; ++i) fsigma->SetParameter(i, sgpar.getNHitPars(i - 1));

  TF1* fsigmacopy = (TF1*)fsigma->Clone("fsigmacopy");
  setFitterStyle(fsigmacopy, 13, 6);

  TFile* infile = new TFile(filename.data());

  for (int ip = 0; ip < int(particles.size()); ++ip) {

    std::string particle = particles[ip];
    TGraphErrors gr_dedxvsbg;

    if (!infile->GetListOfKeys()->Contains(Form("%s_nhit", particle.data()))) continue;
    TTree* hadron = (TTree*)infile->Get(Form("%s_nhit", particle.data()));
    B2INFO("\tHadronCalibration: reading " << particle << " in file " << filename.data());

    double avg, sigma, sigmaerr;

    hadron->SetBranchAddress("avg", &avg);
    hadron->SetBranchAddress("chisigma", &sigma);
    hadron->SetBranchAddress("chisigma_err", &sigmaerr);

    for (int j = 0; j < hadron->GetEntries(); ++j) {
      hadron->GetEvent(j);
      gr_dedxvsbg.SetPoint(j, avg, sigma);
      gr_dedxvsbg.SetPointError(j, 0, sigmaerr);
    }

    // --------------------------------------------------
    // FIT SIGMA VS NHIT CURVE
    // --------------------------------------------------
    gStyle->SetOptFit(0);
    TCanvas sigvsnhitcan(Form("sigvsnhitcan_%s", suffix.data()), "#sigma vs. nHit", 600, 600);

    gr_dedxvsbg.SetMarkerStyle(8);
    gr_dedxvsbg.SetMarkerSize(0.3);
    gr_dedxvsbg.SetMaximum(2.2);
    gr_dedxvsbg.SetMinimum(0.0);
    gr_dedxvsbg.SetTitle(Form("width of (dedx-pred)/(#sigma_{Cos * Ion * InjReso}) vs lNHitsUsed, %s ;lNHitsUsed; #sigma",
                              particle.data()));
    gr_dedxvsbg.Draw("AP");
    fsigmacopy->Draw("same");

    TLegend tleg(0.4, 0.70, 0.65, 0.85);
    tleg.SetBorderSize(0);
    tleg.AddEntry(&gr_dedxvsbg, "Data points", "P");
    tleg.AddEntry(fsigmacopy, "Public Fit", "f1");

    if (particle == "muon") {

      // if the fit succeeds, write out the new parameters
      int status = gr_dedxvsbg.Fit("fsigma", "FR", "", lowernhit, uppernhit);

      for (int i = 0; i < 20; ++i) {
        if (status == 0) break;
        status = gr_dedxvsbg.Fit("fsigma", "FR", "", lowernhit, uppernhit);
      }

      if (status == 0) {
        B2INFO("\tHadronCalibration::fitSigmaVsNHit --> FIT OK..upadting parameters");
        for (int j = 1; j < 6; ++j) {
          B2INFO("\t" << j << ") Old = " << sgpar.getNHitPars(j - 1) << " --> New = " << fsigma->GetParameter(j));
          sgpar.setNHitPars(j - 1, fsigma->GetParameter(j));
        }
      } else B2INFO("\tHadronCalibration::fitSigmaVsNHit --> WARNING: FIT FAILED..: status = " << status);

      // write out the (possibly) updated parameters to file
      sgpar.printParameters("parameters.sigmanhit.fit");

      tleg.AddEntry(fsigma, "New Fit", "f1");
    }

    else {
      sgpar.setParameters("parameters.sigmanhit.fit");
      for (int i = 1; i < 6; ++i) fsigma->SetParameter(i, sgpar.getNHitPars(i - 1));
      fsigma->SetRange(7, 39);
      fsigma->Draw("same");
      tleg.AddEntry(fsigma, "New (param. from muon fit)", "f1");
    }

    tleg.Draw("same");

    sigvsnhitcan.SaveAs(Form("plots/HadronCal/Resofits/sigma_vsnhits_%s_%s.pdf", suffix.data(), particle.data()));
  }
}

void HadronCalibration::fitSigmaVsCos(std::vector< std::string > particles, std::string filename, std::string paramsigma,
                                      std::string suffix)
{
  double lowercos = -0.84, uppercos = 0.96;

  CDCDedxSigmaPred gpar;
  gpar.setParameters(paramsigma.data());

  CDCDedxWidgetSigma* gs = new CDCDedxWidgetSigma();

  TF1* total = new TF1("total", gs, lowercos, uppercos, 11, "CDCDedxWidgetSigma");
  for (int i = 0; i < 10; ++i) total->SetParameter(i + 1, gpar.getCosPars(i));
  total->FixParameter(0, 3);
  total->FixParameter(2, 0.0);

  TF1* fsigmacopy = (TF1*)total->Clone("fsigmacopy");
  setFitterStyle(fsigmacopy, 13, 6);

  TFile* infile = new TFile(filename.data());

  for (int ip = 0; ip < int(particles.size()); ++ip) {

    std::string particle = particles[ip];
    TGraphErrors gr_dedx;

    if (!infile->GetListOfKeys()->Contains(Form("%s_costh", particle.data()))) continue;
    TTree* hadron = (TTree*)infile->Get(Form("%s_costh", particle.data()));
    B2INFO("\tHadronCalibration: reading " << particle << " in file " << filename.data());

    double avg, sigma, sigmaerr;

    hadron->SetBranchAddress("avg", &avg);
    hadron->SetBranchAddress("chisigma", &sigma);
    hadron->SetBranchAddress("chisigma_err", &sigmaerr);

    for (int j = 0; j < hadron->GetEntries(); ++j) {
      hadron->GetEvent(j);
      gr_dedx.SetPoint(j, avg, sigma);
      gr_dedx.SetPointError(j, 0, sigmaerr);
    }

    // --------------------------------------------------
    // FIT SIGMA VS COS CURVE
    // --------------------------------------------------
    gStyle->SetOptFit(0);

    TCanvas sigvscos("sigvscos", "#sigma vs. cos(#theta)", 400, 400);

    gr_dedx.SetMaximum(1.8);
    gr_dedx.SetMinimum(0.4);
    gr_dedx.SetTitle(Form("(dedx-pred)/(#sigma_{Nhit * Ion * InjReso}) vs cos(#theta), %s;cos(#theta);#sigma", particle.data()));
    gr_dedx.Draw("AP");

    fsigmacopy->Draw("same");

    TLegend tleg(0.75, 0.75, 0.89, 0.89);
    tleg.SetBorderSize(0);
    tleg.AddEntry(&gr_dedx, "Data points", "P");
    tleg.AddEntry(fsigmacopy, "Public Fit", "f1");

    if (particle == "muon") {

      int status = gr_dedx.Fit("total", "FR", "", lowercos, uppercos);

      for (int i = 0; i < 20; ++i) {
        if (status == 0) break;
        status = gr_dedx.Fit("total", "FR", "", lowercos, uppercos);
      }
      if (status == 0) {
        B2INFO("\tHadronCalibration::fitSigmaVsCos --> FIT OK..upadting parameters");
        for (int j = 1; j < 11; ++j) {
          B2INFO("\t" << j - 1 << ") Old = " << gpar.getCosPars(j - 1) << " --> New = " << total->GetParameter(j));
          gpar.setCosPars(j - 1, total->GetParameter(j));
        }
      } else B2INFO("\tHadronCalibration::fitSigmaVsCos --> WARNING: FIT FAILED..: status = " << status);

      gpar.printParameters("parameters.sigmacos.fit");
      tleg.AddEntry(total, "New Fit", "f1");

    } else {
      gpar.setParameters("parameters.sigmacos.fit");
      for (int i = 1; i < 11; ++i) total->SetParameter(i, gpar.getCosPars(i - 1));
      total->Draw("same");
      tleg.AddEntry(total, "New (param. from muon fit)", "f1");

    }

    tleg.Draw("same");
    sigvscos.SaveAs(Form("plots/HadronCal/Resofits/sigma_vscos_%s_%s.pdf", suffix.data(), particle.data()));
  }

}
