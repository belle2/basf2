////////////////////////////////////////////////////////////
// Validation_PID.cc
// Check the PID efficiency
// Look at single particles, and use truth matching
// Note this is only PID efficiency, not tracking efficiency
// 
// Phillip Urquijo
// May 3, 2013
//
gSystem->Load("libRooFit.so");
using namespace RooFit ;
void test2_Validation_PID(int region=0, bool runOffline=False){//0=all,1=forward,2=barrel,3=backward,4=endcap

  if(runOffline){
    SetBelle2Style();
    gROOT->LoadMacro("Belle2Labels.C");
  }

  TCanvas *c_pidvalidation = new TCanvas ("PIDvalidation","PIDvalidation",1600,800);
  c_pidvalidation->Print("pid.pdf[");			      
  TPaveText *belleName = new TPaveText(0.6,0.9,0.9,0.95,"BRNDC");
  belleName->SetFillColor(0);
  belleName->SetTextColor(kGray);
  belleName->SetTextAlign(12);
  belleName->SetBorderSize(0);
  belleName->AddText("Belle II Validation");
   
  /*  Take the pituple prepared by the NtupleMaker */
  TChain * recoTree = new TChain("pituple");
  recoTree->AddFile("../GenericB.ntup.root");
  
  enum hypo {pion=0, kaon=1, electron=2, muon=3, proton=4, ntypes=5};
  const int pid[] = {211,321,11,13,2212};
  const float pdgmasses[] = {0.13957,0.49367,0.000511,0.105658,0.93827};
  const char *names[] = { "pi", "k", "e", "mu", "p"};

  const int nthbins=20;
  const float thlow=-1.;
  const float thhigh=1.;

  const int npbins=18;
  const float plow=0.;
  const float phigh=3.6;

  const int npidbins=21;
  const float pidlow=0.;
  const float pidhigh=1.05;


  for(int i=0;i<2;i++){ //Hypo counter

    //Lab momentum
    TH1F * h_Ppi          = new TH1F(Form("P_%s_pi",string(names[i]).c_str())           ,Form(";p(%s) GeV;N",string(names[i]).c_str())            ,npbins,plow,phigh);
    TH1F * h_Ppi_PID      = new TH1F(Form("P_%s_pi_PID",string(names[i]).c_str())       ,Form(";PIDcut p(%s) GeV;N",string(names[i]).c_str())     ,npbins,plow,phigh);
    //Lab cos(theta)
    TH1F * h_cosThpi      = new TH1F(Form("CosTheta_%s_pi",string(names[i]).c_str())    ,Form(";cos#theta(%s) ;N",string(names[i]).c_str())       ,nthbins,thlow,thhigh);			  
    TH1F * h_cosThpi_PID  = new TH1F(Form("CosTheta_%s_pi_PID",string(names[i]).c_str()),Form(";PIDcut cos#theta(%s) ;N",string(names[i]).c_str()),nthbins,thlow,thhigh);			  
    TH1F * h_PIDpi        = new TH1F(Form("P_%s_PIDpi",string(names[i]).c_str())        ,Form(";PID#pi(%s);N",string(names[i]).c_str())           ,npidbins,pidlow,pidhigh);

    TH1F * h_Pk           = new TH1F(Form("P_%s_k",string(names[i]).c_str())            ,Form(";p(%s) GeV;N",string(names[i]).c_str())            ,npbins,plow,phigh);
    TH1F * h_Pk_PID       = new TH1F(Form("P_%s_k_PID",string(names[i]).c_str())        ,Form(";PIDcut p(%s) GeV;N",string(names[i]).c_str())     ,npbins,plow,phigh);
    TH1F * h_cosThk       = new TH1F(Form("CosTheta_%s_k",string(names[i]).c_str())     ,Form(";cos#theta(%s) ;N",string(names[i]).c_str())       ,nthbins,thlow,thhigh);			  
    TH1F * h_cosThk_PID   = new TH1F(Form("CosTheta_%s_k_PID",string(names[i]).c_str()) ,Form(";PIDcut cos#theta(%s) ;N",string(names[i]).c_str()),nthbins,thlow,thhigh);			  
    TH1F * h_PIDk         = new TH1F(Form("P_%s_PIDk",string(names[i]).c_str())         ,Form(";PIDk(%s);N",string(names[i]).c_str())             ,npidbins,pidlow,pidhigh);

    TH1F * h_Pe           = new TH1F(Form("P_%s_e",string(names[i]).c_str())            ,Form(";p(%s) GeV;N",string(names[i]).c_str())            ,npbins,plow,phigh);
    TH1F * h_Pe_PID       = new TH1F(Form("P_%s_e_PID",string(names[i]).c_str())        ,Form(";PIDcut p(%s) GeV;N",string(names[i]).c_str())     ,npbins,plow,phigh);
    TH1F * h_cosThe       = new TH1F(Form("CosTheta_%s_e",string(names[i]).c_str())     ,Form(";cos#theta(%s) ;N",string(names[i]).c_str())       ,nthbins,thlow,thhigh);			  
    TH1F * h_cosThe_PID   = new TH1F(Form("CosTheta_%s_e_PID",string(names[i]).c_str()) ,Form(";PIDcut cos#theta(%s) ;N",string(names[i]).c_str()),nthbins,thlow,thhigh);			  
    TH1F * h_PIDe         = new TH1F(Form("P_%s_PIDe",string(names[i]).c_str())         ,Form(";PIDe(%s);N",string(names[i]).c_str())             ,npidbins,pidlow,pidhigh);

    TH1F * h_Pmu          = new TH1F(Form("P_%s_mu",string(names[i]).c_str())           ,Form(";p(%s) GeV;N",string(names[i]).c_str())            ,npbins,plow,phigh);
    TH1F * h_Pmu_PID      = new TH1F(Form("P_%s_mu_PID",string(names[i]).c_str())       ,Form(";PIDcut p(%s) GeV;N",string(names[i]).c_str())     ,npbins,plow,phigh);
    TH1F * h_cosThmu      = new TH1F(Form("CosTheta_%s_mu",string(names[i]).c_str())    ,Form(";cos#theta(%s) ;N",string(names[i]).c_str())       ,nthbins,thlow,thhigh);			  
    TH1F * h_cosThmu_PID  = new TH1F(Form("CosTheta_%s_mu_PID",string(names[i]).c_str()),Form(";PIDcut cos#theta(%s) ;N",string(names[i]).c_str()),nthbins,thlow,thhigh);			  
    TH1F * h_PIDmu        = new TH1F(Form("P_%s_PIDmu",string(names[i]).c_str())         ,Form(";PIDmu(%s);N",string(names[i]).c_str())            ,npidbins,pidlow,pidhigh);

    TH1F * h_Pp           = new TH1F(Form("P_%s_p",string(names[i]).c_str())            ,Form(";p(%s) GeV;N",string(names[i]).c_str())            ,npbins,plow,phigh);
    TH1F * h_Pp_PID       = new TH1F(Form("P_%s_p_PID",string(names[i]).c_str())        ,Form(";PIDcut p(%s) GeV;N",string(names[i]).c_str())     ,npbins,plow,phigh);
    TH1F * h_cosThp       = new TH1F(Form("CosTheta_%s_p",string(names[i]).c_str())     ,Form(";cos#theta(%s) ;N",string(names[i]).c_str())       ,nthbins,thlow,thhigh);			  
    TH1F * h_cosThp_PID   = new TH1F(Form("CosTheta_%s_p_PID",string(names[i]).c_str()) ,Form(";PIDcut cos#theta(%s) ;N",string(names[i]).c_str()),nthbins,thlow,thhigh);			  
    TH1F * h_PIDp         = new TH1F(Form("P_%s_PIDp",string(names[i]).c_str())         ,Form(";PIDp(%s);N",string(names[i]).c_str())             ,npidbins,pidlow,pidhigh);
    
    TH2F * h_P_Efficiency = new TH2F("P_Efficiency",";p GeV;Efficiency",10,0,3.6,10,0,1.1);
    TH2F * h_cosTh_Efficiency = new TH2F("cosTh_Efficiency",";cos#theta;Efficiency",10,-1,1,10,0,1.1);
    
    float fpi_TruthP;  
    float fpi_P4[4];  
    float fpi_PIDpi;  
    float fpi_PIDk;  
    float fpi_PIDe;  
    float fpi_PIDmu;  
    float fpi_PIDp;  
    int ipi_TruthID;  
    recoTree->SetBranchAddress("pi_TruthP",  &fpi_TruthP);  
    recoTree->SetBranchAddress("pi_TruthID", &ipi_TruthID);  
    recoTree->SetBranchAddress("pi_P4",      &fpi_P4);  
    recoTree->SetBranchAddress("pi_PIDpi",   &fpi_PIDpi);  
    recoTree->SetBranchAddress("pi_PIDk",    &fpi_PIDk);  
    recoTree->SetBranchAddress("pi_PIDe",    &fpi_PIDe);  
    recoTree->SetBranchAddress("pi_PIDmu",   &fpi_PIDmu);  
    recoTree->SetBranchAddress("pi_PIDp",    &fpi_PIDp);  
    
    for(Int_t iloop=0;iloop<recoTree->GetEntries();iloop++) {
      recoTree->GetEntry(iloop);
      TLorentzVector lv_pi_temp(fpi_P4);  
      //Recalculate the particle 4 vector for different hypotheses
      TLorentzVector lv_pi(lv_pi_temp.Px(),lv_pi_temp.Py(),lv_pi_temp.Pz(),sqrt(lv_pi_temp.E()*lv_pi_temp.E() - pdgmasses[0]*pdgmasses[0] + pdgmasses[i]*pdgmasses[i]));
      int pdgid=abs(ipi_TruthID);

      if(region==1 && (lv_pi.Theta()<0.21 || lv_pi.Theta()>0.58) ) continue; //forward
      if(region==2 && (lv_pi.Theta()<0.58 || lv_pi.Theta()>2.23) ) continue; //barrel
      if(region==3 && (lv_pi.Theta()<2.23 || lv_pi.Theta()>2.71) ) continue; //backward
      if(region==4 && (lv_pi.Theta()<0.21 || lv_pi.Theta()>2.71 || (lv_pi.Theta()>0.58 && lv_pi.Theta()<2.23) ) ) continue; //backward
      if(region==5 && (lv_pi.Theta()<0.58 || lv_pi.Theta()>2.23) && lv_pi.Rho()>0.5&&lv_pi.Rho()<1.5 ) continue; //special case for presentations 

      //PID cut
      bool passPID = false;
      if(i==pion     && fpi_PIDpi>0.5)    passPID=true; 
      if(i==kaon     && fpi_PIDk>0.5)     passPID=true; 
      if(i==electron && fpi_PIDe>0.5)     passPID=true; 
      if(i==muon     && fpi_PIDmu>0.5)    passPID=true; 
      if(i==proton   && fpi_PIDp>0.5)     passPID=true; 

      float PID = -2;
      if(i==pion     ) PID=fpi_PIDpi; 
      if(i==kaon     ) PID=fpi_PIDk; 
      if(i==electron ) PID=fpi_PIDe; 
      if(i==muon     ) PID=fpi_PIDmu; 
      if(i==proton   ) PID=fpi_PIDp; 

      
      if(pdgid==pid[pion]){
	h_PIDpi->Fill(PID);
	h_Ppi->Fill(lv_pi.Rho());
	h_cosThpi->Fill(lv_pi.CosTheta());
      }
      if(pdgid==pid[kaon]){
	h_PIDk->Fill(PID);
	h_Pk->Fill(lv_pi.Rho());
	h_cosThk->Fill(lv_pi.CosTheta());
      }
      if(pdgid==pid[electron]){
	h_PIDe->Fill(PID);
	h_Pe->Fill(lv_pi.Rho());
	h_cosThe->Fill(lv_pi.CosTheta());
      }
      if(pdgid==pid[muon]){
	h_PIDmu->Fill(PID);
	h_Pmu->Fill(lv_pi.Rho());
	h_cosThmu->Fill(lv_pi.CosTheta());
      }
      if(pdgid==pid[proton]){
	h_PIDp->Fill(PID);
	h_Pp->Fill(lv_pi.Rho());
	h_cosThp->Fill(lv_pi.CosTheta());
      }
      
      if(passPID){

	if(pdgid==pid[pion]){
	  h_Ppi_PID->Fill(lv_pi.Rho());
	  h_cosThpi_PID->Fill(lv_pi.CosTheta());
	}
	if(pdgid==pid[kaon]){
	  h_Pk_PID->Fill(lv_pi.Rho());
	  h_cosThk_PID->Fill(lv_pi.CosTheta());
	}
	if(pdgid==pid[electron]){
	  h_Pe_PID->Fill(lv_pi.Rho());
	  h_cosThe_PID->Fill(lv_pi.CosTheta());
	}
	if(pdgid==pid[muon]){
	  h_Pmu_PID->Fill(lv_pi.Rho());
	  h_cosThmu_PID->Fill(lv_pi.CosTheta());
	}
	
	if(pdgid==pid[proton]){
	  h_Pp_PID->Fill(lv_pi.Rho());
	  h_cosThp_PID->Fill(lv_pi.CosTheta());
	}
      }
      
    }

    TCanvas *tc = new TCanvas (Form("tc%s",string(names[i]).c_str()),"tcReco");
    
    tc->cd();
    h_Ppi->SetLineColor(kRed);
    h_Ppi->Draw();
    h_Pk->SetLineColor(kBlue);
    h_Pk->Draw("same");
    h_Pe->SetLineColor(kGreen);
    h_Pe->Draw("same");
    h_Pmu->SetLineColor(kOrange);
    h_Pmu->Draw("same");
    h_Pp->SetLineColor(kMagenta);
    h_Pp->Draw("same");
    if(runOffline)BELLE2Label(0.5,0.9,"Belle II Validation");
    tc->Print("pid.pdf",Form("Title: P noPID, %s hypothesis",string(names[i]).c_str()));
    
    tc->cd();
    h_cosThpi->SetLineColor(kRed);
    h_cosThpi->Draw();
    h_cosThk->SetLineColor(kBlue);
    h_cosThk->Draw("same");
    h_cosThe->SetLineColor(kGreen);
    h_cosThe->Draw("same");
    h_cosThmu->SetLineColor(kOrange);
    h_cosThmu->Draw("same");
    h_cosThp->SetLineColor(kMagenta);
    h_cosThp->Draw("same");
    if(runOffline)BELLE2Label(0.5,0.9,"Belle II Validation");
    tc->Print("pid.pdf",Form("Title: cos#theta noPID, %s hypothesis",string(names[i]).c_str()));

    tc->cd();
    h_Ppi_PID->SetLineColor(kRed);
    h_Ppi_PID->Draw();
    h_Pk_PID->SetLineColor(kBlue);
    h_Pk_PID->Draw("same");
    h_Pe_PID->SetLineColor(kGreen);
    h_Pe_PID->Draw("same");
    h_Pmu_PID->SetLineColor(kOrange);
    h_Pmu_PID->Draw("same");
    h_Pp_PID->SetLineColor(kMagenta);
    h_Pp_PID->Draw("same");
    if(runOffline)BELLE2Label(0.5,0.9,"Belle II Validation");
    tc->Print("pid.pdf",Form("Title: P PID, %s hypothesis",string(names[i]).c_str()));
    
    tc->cd(4);
    h_cosThpi_PID->SetLineColor(kRed);
    h_cosThpi_PID->Draw();
    h_cosThk_PID->SetLineColor(kBlue);
    h_cosThk_PID->Draw("same");
    h_cosThe_PID->SetLineColor(kGreen);
    h_cosThe_PID->Draw("same");
    h_cosThmu_PID->SetLineColor(kOrange);
    h_cosThmu_PID->Draw("same");
    h_cosThp_PID->SetLineColor(kMagenta);
    h_cosThp_PID->Draw("same");
    if(runOffline)BELLE2Label(0.5,0.9,"Belle II Validation");
    tc->Print("pid.pdf",Form("Title: cos#theta PID, %s hypothesis",string(names[i]).c_str()));


    h_PIDpi->SetLineColor(kRed);
    h_PIDpi->Draw();
    h_PIDk->SetLineColor(kBlue);
    h_PIDk->Draw("same");
    h_PIDe->SetLineColor(kGreen);
    h_PIDe->Draw("same");
    h_PIDmu->SetLineColor(kOrange);
    h_PIDmu->Draw("same");
    h_PIDp->SetLineColor(kMagenta);
    h_PIDp->Draw("same");
    tc->Print("pid.pdf",Form("Title: PID, %s hypothesis",string(names[i]).c_str()));

    if(runOffline){ 
      tc->SetLogy(1);
      h_PIDpi->SetFillColor(kYellow);
      h_PIDpi->SetLineColor(kYellow);
      h_PIDpi->SetMinimum(10);
      h_PIDpi->SetMaximum(1000000);
      h_PIDpi->Draw();
      h_PIDk->SetLineColor(kBlack);
      h_PIDk->SetFillColor(kBlack);
      h_PIDk->SetFillStyle(3004);
      h_PIDk->Draw("same");
      tc->Print("pid.pdf",Form("Title: PID, %s hypothesis",string(names[i]).c_str()));
      tc->Clear();
      tc->SetLogy(0);
    }
   
    TFile* output = new TFile(Form("PIDValidation%sID.root",string(names[i]).c_str()), "recreate");
    
    //TGraph for efficiencies
    TGraphAsymmErrors *Eff_P_pi = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_P_k  = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_P_e  = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_P_mu = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_P_p  = new TGraphAsymmErrors();

    //Histogram efficiencies - needed for the validation page
    TH1F * h_Eff_Ppi  = new TH1F(Form("P_Eff_%s_pi",string(names[i]).c_str())  ,Form("Eff pi;p(%s) GeV;PIDpi Hypothesis Efficiency",string(names[i]).c_str()) ,npbins,plow,phigh);
    h_Eff_Ppi->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched pi tracks in bins of lab momentum. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_Ppi->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuous efficiency."));

    TH1F * h_Eff_Pk   = new TH1F(Form("P_Eff_%s_k",string(names[i]).c_str())   ,Form("Eff k ;p(%s) GeV;PIDk  Hypothesis Efficiency",string(names[i]).c_str()) ,npbins,plow,phigh);
    h_Eff_Pk->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched K tracks in bins of lab momentum. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_Pk->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    TH1F * h_Eff_Pe   = new TH1F(Form("P_Eff_%s_e",string(names[i]).c_str())   ,Form("Eff e ;p(%s) GeV;PIDe  Hypothesis Efficiency",string(names[i]).c_str()) ,npbins,plow,phigh);
    h_Eff_Pe->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched e tracks in bins of lab momentum. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_Pe->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    TH1F * h_Eff_Pmu  = new TH1F(Form("P_Eff_%s_mu",string(names[i]).c_str())  ,Form("Eff mu;p(%s) GeV;PIDmu Hypothesis Efficiency",string(names[i]).c_str()) ,npbins,plow,phigh);
    h_Eff_Pmu->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched mu tracks in bins of lab momentum. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_Pmu->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    TH1F * h_Eff_Pp   = new TH1F(Form("P_Eff_%s_p",string(names[i]).c_str())   ,Form("Eff pr;p(%s) GeV;PIDpr Hypothesis Efficiency",string(names[i]).c_str()) ,npbins,plow,phigh);
    h_Eff_Pp->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched proton tracks in bins of lab momentum. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_Pp->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    Eff_P_pi ->SetMarkerColor(kRed);     Eff_P_pi ->SetLineColor(kRed);
    Eff_P_k  ->SetMarkerColor(kBlue);    Eff_P_k ->SetLineColor(kBlue);
    Eff_P_e  ->SetMarkerColor(kGreen);   Eff_P_e ->SetLineColor(kGreen);
    Eff_P_mu ->SetMarkerColor(kOrange);  Eff_P_mu ->SetLineColor(kOrange);
    Eff_P_p  ->SetMarkerColor(kMagenta); Eff_P_p ->SetLineColor(kMagenta);

    Eff_P_pi -> Divide(h_Ppi_PID,h_Ppi,"cl=0.683 b(1,1) mode");
    Eff_P_k  -> Divide(h_Pk_PID ,h_Pk ,"cl=0.683 b(1,1) mode");
    Eff_P_e  -> Divide(h_Pe_PID ,h_Pe ,"cl=0.683 b(1,1) mode");
    Eff_P_mu -> Divide(h_Pmu_PID,h_Pmu,"cl=0.683 b(1,1) mode");
    Eff_P_p  -> Divide(h_Pp_PID ,h_Pp ,"cl=0.683 b(1,1) mode");

    h_Eff_Ppi -> Divide(h_Ppi_PID,h_Ppi);
    h_Eff_Pk  -> Divide(h_Pk_PID,h_Pk);
    h_Eff_Pe  -> Divide(h_Pe_PID,h_Pe);
    h_Eff_Pmu -> Divide(h_Pmu_PID,h_Pmu);
    h_Eff_Pp  -> Divide(h_Pp_PID,h_Pp);

    h_P_Efficiency->Draw();
    Eff_P_pi ->Draw("p");
    Eff_P_k  ->Draw("p");
    Eff_P_e  ->Draw("p");
    Eff_P_mu ->Draw("p");
    Eff_P_p  ->Draw("p");
    tc->Print("pid.pdf",Form("Title: Efficiency, %s hypothesis",string(names[i]).c_str()));

    if(runOffline){
      tc->cd()->SetGridy(1);
      tc->cd()->SetGridx(1);
      h_P_Efficiency->Draw();
      Eff_P_pi ->Draw("p");
      Eff_P_k  ->Draw("p");
      if(region==4)    BELLE2Label(0.5,0.9,"Endcaps");
      if(region==2)    BELLE2Label(0.5,0.9,"Barrel");
      tc->Print("pid.pdf",Form("Title: Efficiency, %s hypothesis",string(names[i]).c_str()));
    }


    TGraphAsymmErrors *Eff_cosTh_pi = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_cosTh_k  = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_cosTh_e  = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_cosTh_mu = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff_cosTh_p  = new TGraphAsymmErrors();
    Eff_cosTh_pi ->SetMarkerColor(kRed);     Eff_cosTh_pi ->SetLineColor(kRed);
    Eff_cosTh_k  ->SetMarkerColor(kBlue);    Eff_cosTh_k ->SetLineColor(kBlue);
    Eff_cosTh_e  ->SetMarkerColor(kGreen);   Eff_cosTh_e ->SetLineColor(kGreen);
    Eff_cosTh_mu ->SetMarkerColor(kOrange);  Eff_cosTh_mu ->SetLineColor(kOrange);
    Eff_cosTh_p  ->SetMarkerColor(kMagenta); Eff_cosTh_p ->SetLineColor(kMagenta);

    //Histogram efficiencies - needed for the validation page
    TH1F * h_Eff_cosThpi  = new TH1F(Form("cosTh_Eff_%s_pi",string(names[i]).c_str())           ,Form(";cos#theta(%s);PIDpi Efficiency",string(names[i]).c_str())            ,nthbins,thlow,thhigh);
    h_Eff_cosThpi->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched #pi tracks in bins of cos theta_lab. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_cosThpi->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuous efficiency."));

    TH1F * h_Eff_cosThk   = new TH1F(Form("cosTh_Eff_%s_k",string(names[i]).c_str())            ,Form(";cos#theta(%s);PIDk Efficiency",string(names[i]).c_str())            ,nthbins,thlow,thhigh);
    h_Eff_cosThk->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched K tracks in bins of cos theta_lab. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_cosThk->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    TH1F * h_Eff_cosThe   = new TH1F(Form("cosTh_Eff_%s_e",string(names[i]).c_str())            ,Form(";cos#theta(%s);PIDe Efficiency",string(names[i]).c_str())            ,nthbins,thlow,thhigh);
    h_Eff_cosThe->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched e tracks in bins of cos theta_lab. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_cosThe->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    TH1F * h_Eff_cosThmu  = new TH1F(Form("cosTh_Eff_%s_mu",string(names[i]).c_str())           ,Form(";cos#theta(%s);PIDmu Efficiency",string(names[i]).c_str())            ,nthbins,thlow,thhigh);
    h_Eff_cosThmu->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched #mu tracks in bins of cos theta_lab. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_cosThmu->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    TH1F * h_Eff_cosThp   = new TH1F(Form("cosTh_Eff_%s_p",string(names[i]).c_str())            ,Form(";cos#theta(%s);PIDpr Efficiency",string(names[i]).c_str())            ,nthbins,thlow,thhigh);
    h_Eff_cosThp->GetListOfFunctions()->Add(new TNamed("Description", Form("PID-%s>0.5 efficiency of truth-matched proton tracks in bins of cos theta_lab. A Generic BBbar sample is used.",string(names[i]).c_str())));
    h_Eff_cosThp->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuou efficiency."));

    Eff_cosTh_pi -> Divide(h_cosThpi_PID,h_cosThpi,"cl=0.683 b(1,1) mode");
    Eff_cosTh_k  -> Divide(h_cosThk_PID ,h_cosThk ,"cl=0.683 b(1,1) mode");
    Eff_cosTh_e  -> Divide(h_cosThe_PID ,h_cosThe ,"cl=0.683 b(1,1) mode");
    Eff_cosTh_mu -> Divide(h_cosThmu_PID,h_cosThmu,"cl=0.683 b(1,1) mode");
    Eff_cosTh_p  -> Divide(h_cosThp_PID ,h_cosThp ,"cl=0.683 b(1,1) mode");

    h_cosTh_Efficiency->Draw();
    Eff_cosTh_pi ->Draw("p");
    Eff_cosTh_k  ->Draw("p");
    Eff_cosTh_e  ->Draw("p");
    Eff_cosTh_mu ->Draw("p");
    Eff_cosTh_p  ->Draw("p");

    h_Eff_cosThpi -> Divide(h_cosThpi_PID,h_cosThpi);
    h_Eff_cosThk  -> Divide(h_cosThk_PID,h_cosThk);
    h_Eff_cosThe  -> Divide(h_cosThe_PID,h_cosThe);
    h_Eff_cosThmu -> Divide(h_cosThmu_PID,h_cosThmu);
    h_Eff_cosThp  -> Divide(h_cosThp_PID,h_cosThp);
    tc->Print("pid.pdf",Form("Title: Efficiency, %s hypothesis",string(names[i]).c_str()));

    if(runOffline){
      h_cosTh_Efficiency->Draw();
      Eff_cosTh_pi ->Draw("p");
      Eff_cosTh_k  ->Draw("p");
      if(region==4)    BELLE2Label(0.5,0.9,"Endcaps");
      if(region==2)    BELLE2Label(0.5,0.9,"Barrel");
      h_Eff_cosThpi -> Divide(h_cosThpi_PID,h_cosThpi);
      h_Eff_cosThk  -> Divide(h_cosThk_PID,h_cosThk);
      h_Eff_cosThe  -> Divide(h_cosThe_PID,h_cosThe);
      h_Eff_cosThmu -> Divide(h_cosThmu_PID,h_cosThmu);
      h_Eff_cosThp  -> Divide(h_cosThp_PID,h_cosThp);
      tc->Print("pid.pdf",Form("Title: Efficiency, %s hypothesis",string(names[i]).c_str()));
    }


    output->Write();
    
  }
  
  
    c_pidvalidation->Print("pid.pdf]");


}
