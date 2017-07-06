/*
<header>
<input>GenericB.ntup.root</input>
<output>test2_Validation_pi0_output.root</output>
<contact>Mario Merola, mario.merola@na.infn.it</contact>
</header>
*/

#include <string>
#include <fstream>
#include <map>
using namespace RooFit ;
using namespace std;


void Pi0Efficiency(TTree* tree_pi0gen, TTree* tree_pi0veryloose, TTree* tree_pi0loose, int npoints, TString txtname){


  double efficiency_pi0_loose[40], efficiency_pi0_veryloose[40];
  double efficiency_pi0_loose_unc[40], efficiency_pi0_veryloose_unc[40];

  double momentum[40];
  double momentum_unc[40];

  double momentum_step = 0.2;

  
  string pi0cutstr_true_veryloose, pi0cutstr_gen_veryloose;
  string pi0cutstr_true_loose, pi0cutstr_gen_loose;

  float efftot_loose, efftot_veryloose;
  float efftot_loose_unc, efftot_veryloose_unc;

  float pi0cut_gen_veryloose = 0.;
  float pi0cut_gen_loose = 0.;
  float pi0cut_true_loose = 0.;
  float pi0cut_true_veryloose = 0.;

  float  sum_pi0_gen_veryloose = 0.;
  float  sum_pi0_gen_loose = 0.;
  float  sum_pi0_true_loose = 0.;
  float  sum_pi0_true_veryloose = 0.;
 

  for (int j = 0; j<npoints; j++){
    
    stringstream jstr, jstrstep;
    jstr << 0. + momentum_step*j;
    jstrstep << 0. + momentum_step +  momentum_step*j;
    
    string CDCacceptance = " && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] > -0.82  && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] < 0.95 && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] > -0.82  && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] < 0.95";


    string pi0M_cut = "pi0_M > 0.09 && pi0_M < 0.165";
    string pi0M_cut_loose = "pi0_M > 0.10 && pi0_M < 0.165";

    string gammatruth_cut = "(abs(pi0_gamma0_P4[3]-pi0_gamma0_TruthP4[3])/pi0_gamma0_TruthP4[3]<0.4) && (abs(pi0_gamma1_P4[3]-pi0_gamma1_TruthP4[3])/pi0_gamma1_TruthP4[3]<0.4)";



    pi0cutstr_gen_veryloose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && " + pi0M_cut + CDCacceptance;
    pi0cutstr_true_veryloose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && pi0_mcPDG==111" + " && " + pi0M_cut + " && " + gammatruth_cut + CDCacceptance;

    pi0cutstr_gen_loose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && " + pi0M_cut_loose + CDCacceptance;
    pi0cutstr_true_loose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && pi0_mcPDG==111" + " && " + pi0M_cut_loose + " && " + gammatruth_cut + CDCacceptance;

    
    pi0cut_gen_veryloose  = tree_pi0gen->GetEntries(pi0cutstr_gen_veryloose.c_str());
    pi0cut_gen_loose      = tree_pi0gen->GetEntries(pi0cutstr_gen_loose.c_str());

    pi0cut_true_loose     = tree_pi0loose->GetEntries(pi0cutstr_true_loose.c_str());
    pi0cut_true_veryloose = tree_pi0veryloose->GetEntries(pi0cutstr_true_veryloose.c_str());

    
    momentum[j] =  0. + momentum_step/2. + momentum_step*j;

    momentum_unc[j] = momentum_step/2.;

    if (pi0cut_gen_loose!=0)
      efficiency_pi0_loose[j] = pi0cut_true_loose/pi0cut_gen_loose;
    else
      efficiency_pi0_loose[j] = 0;
    
    if (pi0cut_gen_veryloose!=0)
      efficiency_pi0_veryloose[j] = pi0cut_true_veryloose/pi0cut_gen_veryloose;
    else
      efficiency_pi0_veryloose[j] = 0;

    
    efficiency_pi0_loose_unc[j] = TMath::Sqrt((efficiency_pi0_loose[j]*(1-efficiency_pi0_loose[j]))/pi0cut_gen_loose);
    efficiency_pi0_veryloose_unc[j] = TMath::Sqrt((efficiency_pi0_veryloose[j]*(1-efficiency_pi0_veryloose[j]))/pi0cut_gen_veryloose);


    if (j>=3){
      sum_pi0_gen_veryloose  += pi0cut_gen_veryloose;
      sum_pi0_gen_loose      += pi0cut_gen_loose;
      sum_pi0_true_loose     += pi0cut_true_loose;
      sum_pi0_true_veryloose += pi0cut_true_veryloose;
    }


  }
  


  TCanvas *c3 = new TCanvas("pi0loose_efficiency","pi0loose_efficiency");
  effpi0loose_graph = new TGraphErrors(npoints,momentum,efficiency_pi0_loose,momentum_unc,efficiency_pi0_loose_unc);
  effpi0loose_graph->SetMinimum(0);
  effpi0loose_graph->SetMaximum(1);
  effpi0loose_graph->Draw("AP");
  effpi0loose_graph->GetXaxis()->SetTitle("p (GeV)");
  effpi0loose_graph->GetYaxis()->SetTitle("efficiency");
  effpi0loose_graph->SetTitle("efficiency vs momentum - pi0 loose");
  //  effpi0loose_graph->Write();
  c3->Write();

  TCanvas *c4 = new TCanvas("pi0veryloose_efficiency","pi0veryloose_efficiency");
  effpi0veryloose_graph = new TGraphErrors(npoints,momentum,efficiency_pi0_veryloose,momentum_unc,efficiency_pi0_veryloose_unc);
  effpi0veryloose_graph->SetMinimum(0);
  effpi0veryloose_graph->SetMaximum(1);
  effpi0veryloose_graph->Draw("AP");
  effpi0veryloose_graph->GetXaxis()->SetTitle("p (GeV)");
  effpi0veryloose_graph->GetYaxis()->SetTitle("efficiency");
  effpi0veryloose_graph->SetTitle("efficiency vs momentum - pi0 veryloose");
  c4->Write();



  efftot_loose = sum_pi0_true_loose/sum_pi0_gen_loose;
  efftot_veryloose = sum_pi0_true_veryloose/sum_pi0_gen_veryloose;

  efftot_loose_unc = TMath::Sqrt((efftot_loose*(1-efftot_loose))/sum_pi0_gen_loose);
  efftot_veryloose_unc = TMath::Sqrt((efftot_veryloose*(1-efftot_veryloose))/sum_pi0_gen_veryloose);


  ofstream outputtxt;
  outputtxt.open(txtname);

  cout << "Mean Efficiencies - in CDC acceptance  and momentum range [0.6,3.0] GeV  "  
       << "  efftot_loose: " << efftot_loose << "+/-" << efftot_loose_unc 
       << "  efftot_veryloose: " << efftot_veryloose << "+/-" << efftot_veryloose_unc << endl;

  outputtxt << "Mean Efficiencies - in CDC acceptance  and momentum range [0.6,3.0] GeV \n";
  outputtxt << "efftot_loose: " << efftot_loose << "+/-" << efftot_loose_unc << "\n" ;
  outputtxt << "efftot_veryloose: " << efftot_veryloose << "+/-" << efftot_veryloose_unc << "\n";


}

void Pi0Purity(TTree* tree_pi0veryloose, TTree* tree_pi0loose, int npoints, TString txtname){

  double purity_pi0_loose[40], purity_pi0_veryloose[40];
  double purity_pi0_loose_unc[40], purity_pi0_veryloose_unc[40];


  double momentum[40];
  double momentum_unc[40];

  double momentum_step = 0.2;

  string pi0cut_tot_veryloose, pi0cut_true_veryloose;
  string pi0cut_tot_loose, pi0cut_true_loose;

  float pi0cut_loose_true = 0.;
  float pi0cut_veryloose_true = 0.;
  float pi0cut_loose_tot = 0.;
  float pi0cut_veryloose_tot = 0.;

  float  sum_pi0_loose_tot       = 0.;
  float  sum_pi0_veryloose_tot   = 0.;
  float  sum_pi0_loose_true      = 0.;
  float  sum_pi0_veryloose_true  = 0.;



  for (int j = 0; j<npoints; j++){

    stringstream jstr, jstrstep;

    jstr << 0. + momentum_step*j;
    jstrstep << 0. + momentum_step +  momentum_step*j;

    string CDCacceptance = " && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] > -0.82  && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] < 0.95 && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] > -0.82  && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] < 0.95";

    string ENEcut =  " && pi0_gamma0_P4[3] < 0.8 && pi0_gamma1_P4[3] < 0.8";

    string pi0M_cut = "pi0_M > 0.09 && pi0_M < 0.165";
    string pi0M_cut_loose = "pi0_M > 0.10 && pi0_M < 0.165";


    pi0cut_tot_veryloose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + CDCacceptance + " && " + pi0M_cut;
    pi0cut_true_veryloose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && pi0_mcPDG==111" + CDCacceptance + " && " + pi0M_cut;

    pi0cut_tot_loose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + CDCacceptance + " && " + pi0M_cut_loose;
    pi0cut_true_loose  = "pi0_P > " + jstr.str() + " && pi0_P < " + jstrstep.str() + " && pi0_mcPDG==111" + CDCacceptance + " && " + pi0M_cut_loose;


    pi0cut_loose_true = tree_pi0loose->GetEntries(pi0cut_true_loose.c_str());
    pi0cut_veryloose_true = tree_pi0veryloose->GetEntries(pi0cut_true_veryloose.c_str());

    pi0cut_loose_tot = tree_pi0loose->GetEntries(pi0cut_tot_loose.c_str());
    pi0cut_veryloose_tot = tree_pi0veryloose->GetEntries(pi0cut_tot_veryloose.c_str());

    momentum[j] =  0. + momentum_step/2. + momentum_step*j;
    momentum_unc[j] = momentum_step/2.;


    if (pi0cut_loose_tot!=0){
      purity_pi0_loose[j] = pi0cut_loose_true/pi0cut_loose_tot;
    }
    else{
      purity_pi0_loose[j] = 0;
    }
    
    if (pi0cut_veryloose_tot!=0){
      purity_pi0_veryloose[j] = pi0cut_veryloose_true/pi0cut_veryloose_tot;
    }
    else {
      purity_pi0_veryloose[j] = 0;
    }
    
    purity_pi0_loose_unc[j] = TMath::Sqrt((purity_pi0_loose[j]*(1-purity_pi0_loose[j]))/pi0cut_loose_tot);
    purity_pi0_veryloose_unc[j] = TMath::Sqrt((purity_pi0_veryloose[j]*(1-purity_pi0_veryloose[j]))/pi0cut_veryloose_tot);
    


    if (j>=3){
      sum_pi0_loose_tot       += pi0cut_loose_tot;
      sum_pi0_veryloose_tot   += pi0cut_veryloose_tot;
      sum_pi0_loose_true      += pi0cut_loose_true;
      sum_pi0_veryloose_true  += pi0cut_veryloose_true;
    }


  }


  TCanvas *c1 = new TCanvas("pi0loose_purity","pi0loose_purity");
  puritypi0loose_graph = new TGraphErrors(npoints,momentum,purity_pi0_loose,momentum_unc,purity_pi0_loose_unc);
  puritypi0loose_graph->SetMinimum(0);
  puritypi0loose_graph->SetMaximum(1);
  puritypi0loose_graph->Draw("AP");
  puritypi0loose_graph->GetXaxis()->SetTitle("p (GeV)");
  puritypi0loose_graph->GetYaxis()->SetTitle("purity");
  puritypi0loose_graph->SetTitle("purity vs momentum - pi0 loose");
  c1->Write();

  TCanvas *c2 = new TCanvas("pi0veryloose_purity","pi0veryloose_purity");
  puritypi0veryloose_graph = new TGraphErrors(npoints,momentum,purity_pi0_veryloose,momentum_unc,purity_pi0_veryloose_unc);
  puritypi0veryloose_graph->SetMinimum(0);
  puritypi0veryloose_graph->SetMaximum(1);
  puritypi0veryloose_graph->Draw("AP");
  puritypi0veryloose_graph->GetXaxis()->SetTitle("p (GeV)");
  puritypi0veryloose_graph->GetYaxis()->SetTitle("purity");
  puritypi0veryloose_graph->SetTitle("purity vs momentum - pi0 veryloose");
  c2->Write(); 


  purtot_loose = sum_pi0_loose_true/sum_pi0_loose_tot;
  purtot_veryloose = sum_pi0_veryloose_true/sum_pi0_veryloose_tot;

  purtot_loose_unc = TMath::Sqrt((purtot_loose*(1-purtot_loose))/sum_pi0_loose_tot);
  purtot_veryloose_unc = TMath::Sqrt((purtot_veryloose*(1-purtot_veryloose))/sum_pi0_veryloose_tot);

  ofstream outputtxt;
  outputtxt.open(txtname,fstream::app);

  cout << "Mean Purity - in CDC acceptance  and momentum range [0.6,3.0] GeV  "
       << "  purtot_loose: " << purtot_loose << "+/-" << purtot_loose_unc
       << "  purtot_veryloose: " << purtot_veryloose << "+/-" << purtot_veryloose_unc << endl;

  outputtxt << "Mean Purity - in CDC acceptance  and momentum range [0.6,3.0] GeV \n";
  outputtxt << "purtot_loose: " << purtot_loose << "+/-" << purtot_loose_unc << "\n";
  outputtxt << "purtot_veryloose: " << purtot_veryloose << "+/-" << purtot_veryloose_unc;

  outputtxt.close();

}



void Pi0EffvsCostheta(TTree* tree_pi0gen, TTree* tree_pi0veryloose, TTree* tree_pi0loose, int npoints){

  double efficiencycth_pi0_loose[40], efficiencycth_pi0_veryloose[40];
  double efficiencycth_pi0_loose_unc[40], efficiencycth_pi0_veryloose_unc[40];


  double costheta[40];
  double costheta_unc[40];

  double costheta_step = 0.1;

  
  string pi0cutcosth_gen_veryloose, pi0cutcosth_recotrue_veryloose;
  string pi0cutcosth_gen_loose, pi0cutcosth_recotrue_loose;

  float pi0cutcth_gen_veryloose = 0.;
  float pi0cutcth_gen_loose = 0.;
  float pi0cutcth_recotrue_loose = 0.;
  float pi0cutcth_recotrue_veryloose = 0.;

  for (int j = 0; j<npoints; j++){
    
    stringstream jstrcth, jstrstepcth;
    jstrcth <<  -1 + costheta_step*j;
    jstrstepcth << -1 + costheta_step + costheta_step*j;
    
    string CDCacceptance = " && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] > -0.82  && pi0_gamma0_P4[2]/pi0_gamma0_P4[3] < 0.95 && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] > -0.82  && pi0_gamma1_P4[2]/pi0_gamma1_P4[3] < 0.95";

    string ENEcut =  " && pi0_gamma0_P4[3] < 0.8 && pi0_gamma1_P4[3] < 0.8";

    string pi0M_cut = "pi0_M > 0.09 && pi0_M < 0.165";
    string pi0M_cut_loose = "pi0_M > 0.10 && pi0_M < 0.165";

    string gammatruth_cut = "(abs(pi0_gamma0_P4[3]-pi0_gamma0_TruthP4[3])/pi0_gamma0_TruthP4[3]<0.4) && (abs(pi0_gamma1_P4[3]-pi0_gamma1_TruthP4[3])/pi0_gamma1_TruthP4[3]<0.4)";


    pi0cutcosth_gen_veryloose  = "pi0_P4[2]/pi0_P > " + jstrcth.str() + " && pi0_P4[2]/pi0_P < " + jstrstepcth.str() + " && pi0_mcPDG==111" + " && " + pi0M_cut;
    pi0cutcosth_recotrue_veryloose  = "pi0_P4[2]/pi0_P > " + jstrcth.str() + " && pi0_P4[2]/pi0_P < " + jstrstepcth.str() + " && pi0_mcPDG==111" + " && " + pi0M_cut + " && " + gammatruth_cut;

    pi0cutcosth_gen_loose  = "pi0_P4[2]/pi0_P > " + jstrcth.str() + " && pi0_P4[2]/pi0_P < " + jstrstepcth.str() + " && pi0_mcPDG==111" + " && " + pi0M_cut_loose;
    pi0cutcosth_recotrue_loose  = "pi0_P4[2]/pi0_P > " + jstrcth.str() + " && pi0_P4[2]/pi0_P < " + jstrstepcth.str() + " && pi0_mcPDG==111" + " && " + pi0M_cut_loose + " && " + gammatruth_cut;
    
    pi0cutcth_gen_veryloose    = tree_pi0gen->GetEntries(pi0cutcosth_gen_veryloose.c_str());
    pi0cutcth_gen_loose        = tree_pi0gen->GetEntries(pi0cutcosth_gen_loose.c_str());
    
    pi0cutcth_recotrue_veryloose   = tree_pi0veryloose->GetEntries(pi0cutcosth_recotrue_veryloose.c_str());
    pi0cutcth_recotrue_loose       = tree_pi0loose->GetEntries(pi0cutcosth_recotrue_loose.c_str());
    
    
    
    costheta[j] = -1 + costheta_step/2. + costheta_step*j;
    
    costheta_unc[j] = costheta_step/2.;
    
    if (pi0cutcth_gen_loose !=0)
      efficiencycth_pi0_loose[j] = pi0cutcth_recotrue_loose/pi0cutcth_gen_loose;
    else
      efficiencycth_pi0_loose[j] = 0;
    
    
    if (pi0cutcth_gen_veryloose !=0)
      efficiencycth_pi0_veryloose[j] = pi0cutcth_recotrue_veryloose/pi0cutcth_gen_veryloose;
    else
      efficiencycth_pi0_veryloose[j] = 0;
    
    
    efficiencycth_pi0_loose_unc[j] = TMath::Sqrt((efficiencycth_pi0_loose[j]*(1-efficiencycth_pi0_loose[j]))/pi0cutcth_gen_loose);
    efficiencycth_pi0_veryloose_unc[j] = TMath::Sqrt((efficiencycth_pi0_veryloose[j]*(1-efficiencycth_pi0_veryloose[j]))/pi0cutcth_gen_veryloose);
    
    
  }
  

  TCanvas *c5 = new TCanvas("pi0loose_eff_vs_costh","pi0loose_efficiency_vs_costheta");
  effpi0cthloose_graph = new TGraphErrors(npoints,costheta,efficiencycth_pi0_loose,costheta_unc,efficiencycth_pi0_loose_unc);
  effpi0cthloose_graph->SetMinimum(0);
  effpi0cthloose_graph->Draw("AP");
  effpi0cthloose_graph->GetXaxis()->SetTitle("costheta");
  effpi0cthloose_graph->GetYaxis()->SetTitle("efficiency");
  effpi0cthloose_graph->SetTitle("efficiency vs costheta - pi0 loose");
  c5->Write();


  TCanvas *c6 = new TCanvas("pi0veryloose_eff_vs_costh","pi0veryloose_efficiency_vs_costheta");
  effpi0cthveryloose_graph = new TGraphErrors(npoints,costheta,efficiencycth_pi0_veryloose,costheta_unc,efficiencycth_pi0_veryloose_unc);
  effpi0cthveryloose_graph->SetMinimum(0);
  effpi0cthveryloose_graph->Draw("AP");
  effpi0cthveryloose_graph->GetXaxis()->SetTitle("costheta");
  effpi0cthveryloose_graph->GetYaxis()->SetTitle("efficiency");
  effpi0cthveryloose_graph->SetTitle("efficiency vs costheta - pi0 veryloose");
  c6->Write();
  

}




void Pi0Resolution(TFile *input){


  std::vector<TString> pi0lists;
  
  pi0lists.push_back("pi0s_veryLoose");
  pi0lists.push_back("pi0s_Loose");

  std::map< TString , TTree* > recoTree;

  

  /* Access the Photons and pi0 M*/
  float pi0_gamma0_P4[4];
  float pi0_gamma1_P4[4];
  float pi0_M = 0.;
  float pi0_P = 0.;
  int pi0_mcErrors = 0;

  string namelist;
  TNtuple* tvalidation = new TNtuple("pi0mass","tree","mean:meanerror:width:widtherror");
  
  TCanvas *canvas = new TCanvas ("pi0resolution","pi0 resolution",1000,800);


  canvas->Divide(2,2);
  
  int i = 1;

  for( std::vector<TString>::const_iterator it_str = pi0lists.begin(); it_str != pi0lists.end(); ++it_str){

    
    TString treename = (*it_str);
    
    
    
    //Plots used in offline validation
    TH1F * h_pi0_m    = new TH1F("h_"+treename,";m(#pi^{0}) [GeV];N",35,0.1,0.16);
    h_pi0_m->GetListOfFunctions()->Add(new TNamed("Description","pi0 Mass"));
    h_pi0_m->GetListOfFunctions()->Add(new TNamed("Check","Stable S/B,non-empty (i.e. pi0 import to analysis modules is working),consistent mean."));
    
    TH1F * h_pi0_m_truth    = new TH1F("h_"+treename+"_truth",";Truth mass m(#pi^{0}) [GeV];N",35,0.1,0.16);
    h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Description","pi0 mass from photons,with mcErrors==0."));
    h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Check","Check if mean is correct."));
    
    
    recoTree[treename] = (TTree*)(input->Get(treename));

    recoTree[treename]->SetBranchAddress("pi0_M",&pi0_M);
    recoTree[treename]->SetBranchAddress("pi0_P",&pi0_P);
    recoTree[treename]->SetBranchAddress("pi0_mcErrors",&pi0_mcErrors);
    
    for(Int_t iloop=0; iloop<recoTree[treename]->GetEntries(); iloop++) {
      recoTree[treename]->GetEntry(iloop);
      if (pi0_P>-1.6){
	h_pi0_m->Fill(pi0_M);
	if( pi0_mcErrors<1 )h_pi0_m_truth->Fill(pi0_M);
      }
    }
    
    
    // Truth pi0 mass
    h_pi0_m_truth->SetLineColor(kGreen);
    h_pi0_m_truth->SetMinimum(0.);
    
    h_pi0_m->SetLineColor(kRed);
    //    h_pi0_m->Draw();
    
    RooRealVar *mass  =  new RooRealVar("mass","m(#pi^{0}) (GeV)" ,0.1,0.16);
    RooDataHist h_pi0(treename,"h_pi0",*mass,h_pi0_m);
    RooDataHist h_pi0_truth(treename+"_truth","h_pi0_truth",*mass,h_pi0_m_truth);
    
    //pi0 signal PDF is a Crystal Ball (Gaussian also listed in case we want to switch)
    RooRealVar mean("mean","mean",0.14,0.11,0.16);
    RooRealVar sig1("#sigma","sig",0.01,0.001,0.02);
    RooGaussian gau1("gau1","gau1",*mass,mean,sig1);
    
    //    RooRealVar alphacb("alphacb","alpha",1.4,0.4,1.7);
    RooRealVar alphacb("alphacb","alpha",1.4,0.6,1.6);
    RooRealVar ncb("ncb","n",8,2.,12.);
    RooCBShape sigcb("sigcb","sig",*mass,mean,sig1,alphacb,ncb);
    
    //pi0 background PDF is a 2nd order Chebyshev
    RooRealVar b1("b1","b1",-3.0021e-01,-3,2);
    RooRealVar a1("a1","a1",-3.0021e-01,-3,2);
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
    
    
    canvas->cd(i);
    gPad->SetLeftMargin(0.13);

    TString frameName;

    if (treename=="pi0s_veryLoose"){
      frameName = " fit to pi0 mass - pi0 veryloose";

    }
    else{
      frameName = " fit to pi0 mass - pi0 loose";
      
    }

    cout << "**********  FIRST FIT  ***********" << endl;
    
    /* Fit to the reco mass */
    RooFitResult* fitresult; 
    fitresult =  totalPdf.fitTo(h_pi0,RooFit::Extended(kTRUE),Minos(0), Save());
    fitresult->Write();
    RooPlot *framey = mass->frame();
    h_pi0.plotOn(framey,Binning(35),Name("Hist"));
    framey->SetMaximum(framey->GetMaximum());
    totalPdf.plotOn(framey,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
    //  totalPdf.plotOn(framey,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framey,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framey,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.paramOn(framey,Parameters(RooArgSet(sig1,mean,nsig)),Format("NELU",AutoPrecision(2)),Layout(0.5,0.89,0.89) );
    framey->getAttText()->SetTextSize(0.03);
    framey->SetMaximum(h_pi0_m->GetMaximum()*1.5);
    framey->SetTitle(frameName);
    framey->GetYaxis()->SetTitleOffset(1.4);
    framey->Draw();
    

    //    canvas->Print(plotname);

    float meancut   = mean.getVal();
    float meanerror = mean.getError();
    float width = sig1.getVal();
    float widtherror = sig1.getError();

    /* Save the numerical fit results to a validation ntuple */
    tvalidation->Fill(meancut*1000.,meanerror*1000.,width*1000.,widtherror*1000.);

    nsig.setVal(10000);
    nbkg.setVal(10000);
    mean.setVal(0.14);
    sig1.setVal(0.01);
    alphacb.setVal(1.4);
    ncb.setVal(8);
    a1.setVal(-3.0021e-01);
    b1.setVal(-3.0021e-01);


    canvas->cd(i+1);
    gPad->SetLeftMargin(0.13);

    if (treename=="pi0s_veryLoose"){
      frameName = " fit to true pi0 mass - pi0 veryloose";


    }
    else{
      frameName = " fit to true pi0 mass - pi0 loose";
      
    }

    cout << "**********  SECOND FIT  ***********" << endl;
    
    /* Fit to the true mass */
    RooFitResult* fitresult_truth =   totalPdf.fitTo(h_pi0_truth,RooFit::Extended(kTRUE),Minos(0), Save());
    fitresult_truth->Write();
    RooPlot *framez = mass->frame();
    h_pi0_truth.plotOn(framez,Binning(35),Name("Hist"));
    framez->SetMaximum(framez->GetMaximum());
    totalPdf.plotOn(framez,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
    //  totalPdf.plotOn(framez,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framez,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framez,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.paramOn(framez,Parameters(RooArgSet(sig1,mean,nsig)),Format("NELU",AutoPrecision(2)),Layout(0.5,0.89,0.89) );
    framez->getAttText()->SetTextSize(0.03);
    framez->SetMaximum(h_pi0_m_truth->GetMaximum()*1.5);
    framez->SetTitle(frameName);
    framez->GetYaxis()->SetTitleOffset(1.4);
    framez->Draw();
    //    canvas->Print(plotname);
    
    //    totalPdf.Write();
    

    meancut   = mean.getVal();
    meanerror = mean.getError();
    width = sig1.getVal();
    widtherror = sig1.getError();

    /* Save the numerical fit results to a validation ntuple */
    tvalidation->Fill(meancut*1000.,meanerror*1000.,width*1000.,widtherror*1000.);
    tvalidation->SetAlias("Description","Fit to the pi0 mass.");
    tvalidation->SetAlias("Check","Consistent numerical fit results.");
    
    //    canvas->Print(plotnamefin);

    i+=2;

    //    h_pi0_m->Write();
    //    h_pi0_m_truth->Write();

    //    h_pi0.Write();
    //    h_pi0_truth.Write();

  }

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
  gStyle->SetNdivisions(510, "XYZ");
  gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
  gStyle->SetPadTickY(1);

  gStyle->SetMarkerStyle(20);
  gStyle->SetHistMinimumZero(kTRUE);

  
  TString inputfile = "Pi0Lists_" + channelname + ".root";

  // if run locally with validate_basf2 use:
  // TString inputfile = "../GenericB.ntup.root";

  TString outputname = "Pi0Validation_" + channelname + ".root";
  
  TString outputtxtname = "Pi0Validation_" + channelname + ".txt";

  TFile * file = new TFile(inputfile,"OPEN");
  
  TTree * tree_pi0gen       = (TTree*)file->Get("pi0s_truth");
  TTree * tree_pi0veryloose = (TTree*)file->Get("pi0s_veryLoose");
  TTree * tree_pi0loose     = (TTree*)file->Get("pi0s_Loose");

  int npoints_eff = 15;
  int npoints_effvscosth = 20;
  int npoints_pur = 15;



  TFile* outputFile = new TFile(outputname,"RECREATE");
  

  Pi0Efficiency(tree_pi0gen, tree_pi0veryloose, tree_pi0loose, npoints_eff, outputtxtname);
  
  Pi0EffvsCostheta(tree_pi0gen, tree_pi0veryloose, tree_pi0loose, npoints_effvscosth);

  Pi0Purity(tree_pi0veryloose, tree_pi0loose, npoints_pur, outputtxtname);
  
  Pi0Resolution(file);

  
   
  outputFile->Close();

  
  
}
