/*
<header>
<input>GenericB.ntup.root</input>
<output>test2_Validation_pi0_output.root</output>
<contact>Mario Merola, mario.merola@na.infn.it</contact>
<interval>release</interval>
</header>
*/

#include <string>
#include <fstream>
#include <map>
using namespace RooFit ;
using namespace std;


void Pi0Efficiency(TTree* tree_pi0gen, TTree* tree_pi0reco,  int npoints, TString txtname, TString plotname){


  double efficiency_pi0reco[40];
  double efficiency_pi0reco_unc[40];

  double momentum[40];
  double momentum_unc[40];

  double momentum_step = 0.2;

  string pi0cutstr_reco, pi0cutstr_gen;

  float efftot_reco;
  float efftot_reco_unc;

  float  sum_pi0_gen = 0.;
  float  sum_pi0_reco = 0.;
 

  for (int j = 0; j<npoints; j++){
    
    float pi0cut_gen;
    float pi0cut_reco;
    
    stringstream jstr, jstrstep;
    jstr << 0. + momentum_step*j;
    jstrstep << 0. + momentum_step +  momentum_step*j;
    
    string CDCacceptance = "  pi0_gamma0_P4[2]/pi0_gamma0_P4[3] > -0.866023  && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] < 0.956305 && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] > -0.866023  && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] < 0.956305";


    string gammatruth_cut = "((pi0_gamma0_P4[3] > 0.60 * pi0_gamma0_TruthP4[3]) && (pi0_gamma1_P4[3] > 0.60 * pi0_gamma1_TruthP4[3]))";



    pi0cutstr_gen  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && " + CDCacceptance;
    pi0cutstr_reco = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && pi0_mcPDG==111" + " && " + gammatruth_cut + " && " + CDCacceptance;

    
    pi0cut_gen      = tree_pi0gen->GetEntries(pi0cutstr_gen.c_str());
    pi0cut_reco     = tree_pi0reco->GetEntries(pi0cutstr_reco.c_str());

    
    momentum[j] =  0. + momentum_step/2. + momentum_step*j;

    momentum_unc[j] = momentum_step/2.;

    if (pi0cut_gen!=0)
      efficiency_pi0reco[j] = pi0cut_reco/pi0cut_gen;
    else
      efficiency_pi0reco[j] = 0;
    
    efficiency_pi0reco_unc[j] = TMath::Sqrt((efficiency_pi0reco[j]*(1-efficiency_pi0reco[j]))/pi0cut_gen);


    if (j>=0){
      sum_pi0_gen      += pi0cut_gen;
      sum_pi0_reco     += pi0cut_reco;
    }


  }
  


  TCanvas *c3 = new TCanvas(plotname, plotname);
  effpi0_graph = new TGraphErrors(npoints,momentum,efficiency_pi0reco,momentum_unc,efficiency_pi0reco_unc);
  effpi0_graph->SetMinimum(0);
  effpi0_graph->SetMaximum(1);
  effpi0_graph->Draw("AP");
  effpi0_graph->GetXaxis()->SetTitle("p (GeV)");
  effpi0_graph->GetYaxis()->SetTitle("efficiency");
  effpi0_graph->SetTitle("efficiency vs momentum - pi0 ");
  c3->Write();

  efftot_reco = sum_pi0_reco/sum_pi0_gen;

  efftot_reco_unc = TMath::Sqrt((efftot_reco*(1-efftot_reco))/sum_pi0_gen);


  ofstream outputtxt;
  outputtxt.open(txtname,fstream::app);

  cout << "Efficiency for list " << tree_pi0reco->GetName() << " - in CDC acceptance and momentum range [0.,3.0] GeV: " 
       << " " << efftot_reco << "+/-" << efftot_reco_unc << endl;

  outputtxt << "Efficiency for list " << tree_pi0reco->GetName() << " - in CDC acceptance and momentum range [0.,3.0] GeV: ";
  outputtxt << " " << efftot_reco << "+/-" << efftot_reco_unc << "\n" ;


}

void Pi0Purity(TTree* tree_pi0, int npoints, TString txtname, TString plotname){

  double purity_pi0[40];
  double purity_pi0_unc[40];


  double momentum[40];
  double momentum_unc[40];

  double momentum_step = 0.2;

  string pi0cutstr_reco, pi0cutstr_true;

  float  sum_pi0_reco       = 0.;
  float  sum_pi0_true      = 0.;



  for (int j = 0; j<npoints; j++){

    float pi0cut_true;
    float pi0cut_reco;
    
    stringstream jstr, jstrstep;

    jstr << 0. + momentum_step*j;
    jstrstep << 0. + momentum_step +  momentum_step*j;

    string CDCacceptance = "  pi0_gamma0_P4[2]/pi0_gamma0_P4[3] > -0.866023  && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] < 0.956305 && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] > -0.866023  && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] < 0.956305";




    pi0cutstr_reco = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && " + CDCacceptance; 
    pi0cutstr_true = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && pi0_mcPDG==111 " + " && " + CDCacceptance;


    pi0cut_true = tree_pi0->GetEntries(pi0cutstr_true.c_str());
    pi0cut_reco = tree_pi0->GetEntries(pi0cutstr_reco.c_str());


    momentum[j] =  0. + momentum_step/2. + momentum_step*j;
    momentum_unc[j] = momentum_step/2.;


    if (pi0cut_reco!=0){
      purity_pi0[j] = pi0cut_true/pi0cut_reco;
    }
    else{
      purity_pi0[j] = 0;
    }
    
    purity_pi0_unc[j] = TMath::Sqrt((purity_pi0[j]*(1-purity_pi0[j]))/pi0cut_reco);
 

    if (j>=0){
      sum_pi0_reco      += pi0cut_reco;
      sum_pi0_true      += pi0cut_true;
    }


  }


  TCanvas *c1 = new TCanvas(plotname,plotname);
  puritypi0_graph = new TGraphErrors(npoints,momentum,purity_pi0,momentum_unc,purity_pi0_unc);
  puritypi0_graph->SetMinimum(0);
  puritypi0_graph->SetMaximum(1);
  puritypi0_graph->Draw("AP");
  puritypi0_graph->GetXaxis()->SetTitle("p (GeV)");
  puritypi0_graph->GetYaxis()->SetTitle("purity");
  puritypi0_graph->SetTitle("purity vs momentum - pi0 ");
  c1->Write();

  purtot = sum_pi0_true/sum_pi0_reco;
  purtot_unc = TMath::Sqrt((purtot*(1-purtot))/sum_pi0_reco);

  ofstream outputtxt;
  outputtxt.open(txtname,fstream::app);

  cout << "Purity for list " << tree_pi0->GetName() << " - in CDC acceptance and momentum range [0.,3.0] GeV: "
       << " " << purtot << "+/-" << purtot_unc << endl;

  outputtxt << "Purity for list " << tree_pi0->GetName() << " - in CDC acceptance  and momentum range [0.,3.0] GeV: ";
  outputtxt << " " << purtot << "+/-" << purtot_unc << "\n";

  outputtxt.close();

}



void Pi0EffvsCostheta(TTree* tree_pi0gen,  TTree* tree_pi0reco, int npoints, TString plotname){

  double efficiencycth_pi0[40];
  double efficiencycth_pi0_unc[40];


  double costheta[40];
  double costheta_unc[40];

  double costheta_step = 0.1;

  
  string pi0cutcosth_gen, pi0cutcosth_reco;


  for (int j = 0; j<npoints; j++){
    
    float pi0cutcth_gen;
    float pi0cutcth_reco;
    
    stringstream jstrcth, jstrstepcth;
    jstrcth <<  -1 + costheta_step*j;
    jstrstepcth << -1 + costheta_step + costheta_step*j;
    

    string gammatruth_cut = "((pi0_gamma0_P4[3] > 0.60 * pi0_gamma0_TruthP4[3]) && (pi0_gamma1_P4[3] > 0.60 * pi0_gamma1_TruthP4[3]))";


    pi0cutcosth_gen  = "pi0_P4[2]/pi0_P > " + jstrcth.str() + " && pi0_P4[2]/pi0_P < " + jstrstepcth.str(); 
    pi0cutcosth_reco  = "pi0_P4[2]/pi0_P > " + jstrcth.str() + " && pi0_P4[2]/pi0_P < " + jstrstepcth.str() + " && pi0_mcPDG==111" + " && " + gammatruth_cut;
    
    pi0cutcth_gen        = tree_pi0gen->GetEntries(pi0cutcosth_gen.c_str());
    
    pi0cutcth_reco       = tree_pi0reco->GetEntries(pi0cutcosth_reco.c_str());
    
    
    
    costheta[j] = -1 + costheta_step/2. + costheta_step*j;
    
    costheta_unc[j] = costheta_step/2.;
    
    if (pi0cutcth_gen !=0)
      efficiencycth_pi0[j] = pi0cutcth_reco/pi0cutcth_gen;
    else
      efficiencycth_pi0[j] = 0;
    
    
    efficiencycth_pi0_unc[j] = TMath::Sqrt((efficiencycth_pi0[j]*(1-efficiencycth_pi0[j]))/pi0cutcth_gen);
    
    
  }
  

  TCanvas *c5 = new TCanvas(plotname, plotname);
  effpi0cth_graph = new TGraphErrors(npoints,costheta,efficiencycth_pi0,costheta_unc,efficiencycth_pi0_unc);
  effpi0cth_graph->SetMinimum(0);
  effpi0cth_graph->Draw("AP");
  effpi0cth_graph->GetXaxis()->SetTitle("costheta");
  effpi0cth_graph->GetYaxis()->SetTitle("efficiency");
  effpi0cth_graph->SetTitle("efficiency vs costheta - pi0");
  c5->Write();


}




void Pi0Resolution(TTree* tree_pi0, TString treename, double fitmin, double fitmax){

  

  /* Access the Photons and pi0 M*/
  float pi0_M = 0.;
  float pi0_P = 0.;
  int pi0_mcErrors = 0;

  TNtuple* tvalidation = new TNtuple("pi0mass","tree","mean:meanerror:width:widtherror");
  
  TCanvas *canvas = new TCanvas (treename,treename);
  
  canvas->Divide(2,1);
  
  //Plots used in offline validation
  TH1F * h_pi0_m    = new TH1F("h_"+treename,";m(#pi^{0}) [GeV];N",50,0.11,0.16);
  h_pi0_m->GetListOfFunctions()->Add(new TNamed("Description","pi0 Mass"));
  h_pi0_m->GetListOfFunctions()->Add(new TNamed("Check","Stable S/B,non-empty (i.e. pi0 import to analysis modules is working),consistent mean."));
  
  TH1F * h_pi0_m_truth    = new TH1F("h_"+treename+"_truth",";Truth mass m(#pi^{0}) [GeV];N",50,0.11,0.16);
  h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Description","pi0 mass from photons,with mcErrors==0."));
  h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Check","Check if mean is correct."));
  
  h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Contact","Mario Merola (mario.merola@na.infn.it)"));  

  tree_pi0->SetBranchAddress("pi0_M",&pi0_M);
  tree_pi0->SetBranchAddress("pi0_P",&pi0_P);
  tree_pi0->SetBranchAddress("pi0_mcErrors",&pi0_mcErrors);
  
  for(Int_t iloop=0; iloop<tree_pi0->GetEntries(); iloop++) {
    tree_pi0->GetEntry(iloop);
    h_pi0_m->Fill(pi0_M);
    if( pi0_mcErrors<1 )h_pi0_m_truth->Fill(pi0_M);
  }
  
  
  // Truth pi0 mass
  h_pi0_m_truth->SetLineColor(kGreen);
  h_pi0_m_truth->SetMinimum(0.);
  
  h_pi0_m->SetLineColor(kRed);
  
  RooRealVar *mass  =  new RooRealVar("mass","m(#pi^{0}) (GeV)" ,0.11,0.16);
  RooDataHist h_pi0(treename,"h_pi0",*mass,h_pi0_m);
  RooDataHist h_pi0_truth(treename+"_truth","h_pi0_truth",*mass,h_pi0_m_truth);
  
  //pi0 signal PDF is a Crystal Ball (Gaussian also listed in case we want to switch)
  RooRealVar mean("mean","mean",0.135,0.12,0.145);
  RooRealVar sig1("#sigma","sig",0.01,0.002,0.018);
  RooGaussian gau1("gau1","gau1",*mass,mean,sig1);
  
  //    RooRealVar alphacb("alphacb","alpha",1.4,0.4,1.7);
  RooRealVar alphacb("alphacb","alpha",1.1,0.8,1.6);
  RooRealVar ncb("ncb","ncb",11,9.,15.);
  RooCBShape sigcb("sigcb","sig",*mass,mean,sig1,alphacb,ncb);
  
  //pi0 background PDF is a 2nd order Chebyshev
  RooRealVar b1("b1","b1",-3.0021e-01,-1.,-0.01);
  RooRealVar a1("a1","a1",-3.0021e-01,-1.,-0.01);
  RooArgList bList(a1,b1);
  RooChebychev bkg("bkg","pol 2",*mass,bList);
  
  RooArgList shapes;
  //shapes.add(gau1);
  shapes.add(sigcb);
  shapes.add(bkg);
  
  // Perform the extended fit to get a rough idea of yields.
  RooRealVar nsig("Nsig","Nsig",10000,0,10000000);
  RooRealVar nbkg("Nbkg","Nbkg",10000,0,10000000);
  RooArgList yields;
  yields.add(nsig);
  yields.add(nbkg);
  
  RooAddPdf totalPdf("totalpdf","",shapes,yields);
  
  
  
  mass->setRange("sigreg", fitmin,fitmax);
  
  canvas->cd(1);
  gPad->SetLeftMargin(0.18);
  gPad->SetRightMargin(0.05);
  
  TString frameName;
  
  frameName = " fit to pi0 mass - " + treename;
  
  cout << "**********  FIRST FIT  ***********" << endl;
  
  /* Fit to the reco mass */
  RooFitResult* fitresult; 
  fitresult =  totalPdf.fitTo(h_pi0,RooFit::Extended(kTRUE),Minos(0), Range("sigreg"),Save());
  fitresult->Write();
  RooPlot *framey = mass->frame();
  h_pi0.plotOn(framey,Binning(50),Name("Hist"));
  framey->SetMaximum(framey->GetMaximum());
  totalPdf.plotOn(framey,Name("curve"));
  totalPdf.plotOn(framey,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),ProjectionRange("sigreg"));
  totalPdf.plotOn(framey,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),ProjectionRange("sigreg"));
  totalPdf.paramOn(framey,Parameters(RooArgSet(sig1,mean,nsig)),Format("NELU",AutoPrecision(2)),Layout(0.5,0.92,0.85) );
  framey->getAttText()->SetTextSize(0.03);
  framey->SetMaximum(h_pi0_m->GetMaximum()*1.6);
  framey->SetTitle(frameName);
  framey->GetYaxis()->SetTitleOffset(1.94);
  framey->Draw();
  
  
  
  float meancut   = mean.getVal();
  float meanerror = mean.getError();
  float width = sig1.getVal();
  float widtherror = sig1.getError();
  
  /* Save the numerical fit results to a validation ntuple */
  tvalidation->Fill(meancut*1000.,meanerror*1000.,width*1000.,widtherror*1000.);
  
  canvas->cd(2);
  gPad->SetLeftMargin(0.18);
  gPad->SetRightMargin(0.05);
  
  frameName = " fit to true pi0 mass " + treename;
  
  
  
  cout << "**********  SECOND FIT  ***********" << endl;
  
  /* Fit to the true mass */
  RooFitResult* fitresult_truth =   totalPdf.fitTo(h_pi0_truth,RooFit::Extended(kTRUE),Minos(0),Range("sigreg"), Save());
  fitresult_truth->Write();
  RooPlot *framez = mass->frame();
  h_pi0_truth.plotOn(framez,Binning(50),Name("Hist"));
  framez->SetMaximum(framez->GetMaximum());
  totalPdf.plotOn(framez,Name("curve"));
  totalPdf.plotOn(framez,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),ProjectionRange("sigreg"));
  totalPdf.plotOn(framez,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),ProjectionRange("sigreg"));
  totalPdf.paramOn(framez,Parameters(RooArgSet(sig1,mean,nsig)),Format("NELU",AutoPrecision(2)),Layout(0.5,0.92,0.85) );
  framez->getAttText()->SetTextSize(0.03);
  framez->SetMaximum(h_pi0_m_truth->GetMaximum()*1.6);
  framez->SetTitle(frameName);
  framez->GetYaxis()->SetTitleOffset(1.94);
  framez->Draw();
  
  
  meancut   = mean.getVal();
  meanerror = mean.getError();
  width = sig1.getVal();
  widtherror = sig1.getError();
  
  /* Save the numerical fit results to a validation ntuple */
  tvalidation->Fill(meancut*1000.,meanerror*1000.,width*1000.,widtherror*1000.);
  tvalidation->SetAlias("Description","Fit to the pi0 mass.");
  tvalidation->SetAlias("Check","Consistent numerical fit results.");
  
  
  tvalidation->Write();
  canvas->Write();
  
}



void test2_Pi0Validation(TString channelname) {
  
  
  
  gStyle->SetPadBottomMargin(0.13);
  gStyle->SetPadTopMargin(0.12);
  gStyle->SetPadLeftMargin(0.12);
  gStyle->SetPadRightMargin(0.03);
  
  gStyle->SetTitleSize(0.05);
  gStyle->SetTitleX(0.28); // Set the position of the title box
  gStyle->SetTitleY(1.3); // Set the position of the title box
  gStyle->SetTitleXSize(0.05); // Set the position of the title box
  gStyle->SetTitleYSize(0.05); // Set the position of the title box
  
  
  // For the axis labels:
  gStyle->SetFrameLineColor(1);
  gStyle->SetFrameLineStyle(1);
  gStyle->SetFrameLineWidth(1);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameBorderSize(1);
  
  gStyle->SetCanvasBorderMode(0);
  
  //  gStyle->SetOptStat(0);
  gStyle->SetPadColor(0);
  gStyle->SetCanvasColor(0);
  //  gStyle->SetStatColor(0);
  gStyle->SetLegendBorderSize(0);
  
  gStyle->SetLabelColor(1, "XYZ");
  gStyle->SetLabelFont(42, "XYZ");
  gStyle->SetLabelOffset(0.008, "XYZ");
  gStyle->SetLabelSize(0.05, "XYZ");
  
  gStyle->SetAxisColor(1, "XYZ");
  gStyle->SetStripDecimals(kTRUE);
  gStyle->SetTickLength(0.03, "XYZ");
  gStyle->SetNdivisions(508, "XYZ");
  gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
  gStyle->SetPadTickY(1);
  
  gStyle->SetMarkerStyle(20);
  gStyle->SetHistMinimumZero(kTRUE);
  
  
  //  TString inputfile = "Pi0Lists_" + channelname + ".root";
  TString inputfile = "GenericB_PhaseII.root";
  
  // if run locally with validate_basf2 use:
  // TString inputfile = "../GenericB.ntup.root";
  
  TString outputname = "Pi0Validation_" + channelname + ".root";
  
  TString outputtxtname = "Pi0Validation_" + channelname + ".txt";
  
  TFile * file = new TFile(inputfile,"OPEN");
  
  TTree * tree_pi0gen       = (TTree*)file->Get("pi0s_truth");
  TTree * tree_pi0eff20     = (TTree*)file->Get("pi0s_eff20");
  TTree * tree_pi0eff30     = (TTree*)file->Get("pi0s_eff30");
  TTree * tree_pi0eff40     = (TTree*)file->Get("pi0s_eff40");
  TTree * tree_pi0eff50     = (TTree*)file->Get("pi0s_eff50");
  TTree * tree_pi0eff60     = (TTree*)file->Get("pi0s_eff60");
  
  int npoints_eff = 15;
  int npoints_effvscosth = 20;
  int npoints_pur = 15;



  TFile* outputFile = new TFile(outputname,"RECREATE");
  
  
  Pi0Efficiency(tree_pi0gen, tree_pi0eff20, npoints_eff, outputtxtname,"pi0eff_eff20");
  Pi0Efficiency(tree_pi0gen, tree_pi0eff30, npoints_eff, outputtxtname,"pi0eff_eff30");
  Pi0Efficiency(tree_pi0gen, tree_pi0eff40, npoints_eff, outputtxtname,"pi0eff_eff40");
  Pi0Efficiency(tree_pi0gen, tree_pi0eff50, npoints_eff, outputtxtname,"pi0eff_eff50");
  Pi0Efficiency(tree_pi0gen, tree_pi0eff60, npoints_eff, outputtxtname,"pi0eff_eff60");
  

  Pi0EffvsCostheta(tree_pi0gen, tree_pi0eff20, npoints_effvscosth, "pi0effvscosth_eff20");
  Pi0EffvsCostheta(tree_pi0gen, tree_pi0eff30, npoints_effvscosth, "pi0effvscosth_eff30");
  Pi0EffvsCostheta(tree_pi0gen, tree_pi0eff40, npoints_effvscosth, "pi0effvscosth_eff40");
  Pi0EffvsCostheta(tree_pi0gen, tree_pi0eff50, npoints_effvscosth, "pi0effvscosth_eff50");
  Pi0EffvsCostheta(tree_pi0gen, tree_pi0eff60, npoints_effvscosth, "pi0effvscosth_eff60");


  Pi0Purity(tree_pi0eff20, npoints_pur, outputtxtname, "pi0pur_eff20");
  Pi0Purity(tree_pi0eff30, npoints_pur, outputtxtname, "pi0pur_eff30");
  Pi0Purity(tree_pi0eff40, npoints_pur, outputtxtname, "pi0pur_eff40");
  Pi0Purity(tree_pi0eff50, npoints_pur, outputtxtname, "pi0pur_eff50");
  Pi0Purity(tree_pi0eff60, npoints_pur, outputtxtname, "pi0pur_eff60");
  

  Pi0Resolution(tree_pi0eff50,"pi0massfit_eff50",0.11,0.16);
  Pi0Resolution(tree_pi0eff60,"pi0massfit_eff60",0.11,0.16);
  
   
  outputFile->Close();
  
}
