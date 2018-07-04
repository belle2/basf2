#include "defs_gdl.C"
void
timingForEachBit(const char *fname){

//TFile f1("dqm.e3r529.root");
//TFile f1("dqm.e3r686.root");
//  TFile f1("dqm.e3.r78x.root");
//TFile f1("dqm.hadron.e3r78x.root");
//TFile f1("dqm.trg01.0111.root");
// TFile f1("dqm.trg01.0112.root");
//const char *target = "dqm.e3r1355.bhabha";
 //onst char *target = "dqm.e3r1333.hadron";
  TString sname(fname);
//const char *target = "dqm.e3r1333.bhabha";
//const char *target = "dqm.e3r1355.hadron";
//TFile f1("dqm.e3r1355.root");
//TFile f1("dqm.e3r532.root");
 //onst char *target = "dqm.e3r1333.hadron";
	TString target=sname.Replace(sname.Index(".root"), 5, "");
 cerr << "target(" << target.Data() << ")" << endl;
  TFile f1(Form("%s.root", target.Data()));
gDirectory->ls();
 
  TDirectoryFile *_TRG = (TDirectoryFile *)gROOT->FindObject("TRG");
  //_TRG->cd();
//TRG->cd();

 TFile f0(Form("itd.%s.root", target.Data()), "recreate");
 //_TRG->cd();

  /*
  TCanvas* canvas = new TCanvas("canvas");
  hGDL_gdlL1TocomL1->Draw();
  canvas->Print("trggdlDQM.pdf(","gdlL1TocomL1");
  hGDL_rvcoutTogdlL1->Draw();
  canvas->Print("trggdlDQM.pdf","rvcoutTogdlL1");
  hGDL_eclTogdlL1->Draw();
  canvas->Print("trggdlDQM.pdf","eclTogdlL1");
  hGDL_ecl8mToGDL->Draw();
  canvas->Print("trggdlDQM.pdf","ecl8mToGDL");
  hGDL_eclToGDL->Draw();
  canvas->Print("trggdlDQM.pdf","eclToGDL");
  hGDL_ns_cdcTocomL1->Draw();
  canvas->Print("trggdlDQM.pdf","cdcTocomL1");
  hGDL_ns_cdcTogdlL1->Draw();
  canvas->Print("trggdlDQM.pdf","cdcTogdlL1");
  hGDL_ns_topToecl->Draw();
  canvas->Print("trggdlDQM.pdf","topToecl");
  hGDL_ns_topTocdc->Draw();
  canvas->Print("trggdlDQM.pdf","topTocdc");
  hGDL_cdcToecl->Draw();
  canvas->Print("trggdlDQM.pdf","cdcToecl");
  hGDL_ns_cdcToecl->Draw();
  canvas->Print("trggdlDQM.pdf","cdcToecl");
  hGDL_inp->Draw();
  canvas->Print("trggdlDQM.pdf","input bits");
  hGDL_itd->Draw();
  canvas->Print("trggdlDQM.pdf","ITD bits");
  hGDL_ftd->Draw();
  canvas->Print("trggdlDQM.pdf","FTD bits");
  hGDL_psn->Draw();
  canvas->Print("trggdlDQM.pdf","PSNM bits");
  hGDL_timtype->Draw();
  canvas->Print("trggdlDQM.pdf)","TimType bits");
  */

 TH1I *hpsnfff = new TH1I("hpsnfff", "hpsnfff", 32, 0, 32);
 TH1I *hfffdouble = new TH1I("hfffdouble", "hfffdouble", 32, 0, 32);
 TH1I *hfffnodouble = new TH1I("hfffnodouble", "hfffnodouble", 32, 0, 32);
 TH1I *hc4orhie = new TH1I("hc4orhie", "hc4orhie", 32, 0, 32);

 TH1I *htdsrcp_r = new TH1I("htdsrcp_r", "timsrcP in tmdl", 32, 0, 32);
 htdsrcp_r->SetLineColor(kRed);
 TH1I *htdsrcp_t = new TH1I("htdsrcp_t", "timsrcP in tmdl", 32, 0, 32);
 htdsrcp_t->SetLineColor(kGreen);
 TH1I *htdtopp_r = new TH1I("htdtopp_r", "topP in tmdl", 32, 0, 32);
 htdtopp_r->SetLineColor(kRed);
 TH1I *htdtopp_t = new TH1I("htdtopp_t", "topP in tmdl", 32, 0, 32);
 htdtopp_t->SetLineColor(kGreen);
 TH1I *htdeclp_r = new TH1I("htdeclp_r", "eclP in tmdl", 32, 0, 32);
 htdeclp_r->SetLineColor(kRed);
 TH1I *htdeclp_t = new TH1I("htdeclp_t", "eclP in tmdl", 32, 0, 32);
 htdeclp_t->SetLineColor(kGreen);
 TH1I *htdcdcp_r = new TH1I("htdcdcp_r", "cdcP in tmdl", 32, 0, 32);
 htdcdcp_r->SetLineColor(kRed);
 TH1I *htdcdcp_t = new TH1I("htdcdcp_t", "cdcP in tmdl", 32, 0, 32);
 htdcdcp_t->SetLineColor(kGreen);

  const int nclks=32;
  TH1I *h_i[96], *h_p[96], *h_f[96], *h_r[96], *h_t[96];
  for(int i=0; i<96; i++){
    h_i[i] = new TH1I(Form("_hi%02d", i), Form("itd %i %s", i, CINAME[i]), nclks+1, -1, nclks);
    h_i[i]->GetXaxis()->SetTitle("clk32ns");
    h_p[i] = new TH1I(Form("_hp%02d", i), Form("psn %i %s", i, CONAME[i]), nclks+1, -1, nclks);
    h_p[i]->GetXaxis()->SetTitle("clk32ns");
    h_f[i] = new TH1I(Form("_hf%02d", i), Form("ftd %i %s", i, CONAME[i]), nclks+1, -1, nclks);
    h_f[i]->GetXaxis()->SetTitle("clk32ns");
    h_r[i] = new TH1I(Form("_hu%02d", i), Form("%s rising - trailing", CINAME[i]), nclks, 0, nclks);
    h_r[i]->GetXaxis()->SetTitle("clk32ns");
    h_t[i] = new TH1I(Form("_hd%02d", i), Form("%s trailing", CINAME[i]), nclks, 0, nclks);
    h_t[i]->GetXaxis()->SetTitle("clk32ns");
  }
  TH1I *h_effi = new TH1I("h_effi", "hie, c4, hie||c4, fff, fff&&(hie||c4)", 10, 0, 10);
  h_effi->GetXaxis()->SetBinLabel(1, "hie");
  h_effi->GetXaxis()->SetBinLabel(2, "c4");
  h_effi->GetXaxis()->SetBinLabel(3, "hie||c4");
  h_effi->GetXaxis()->SetBinLabel(4, "fff");
  h_effi->GetXaxis()->SetBinLabel(5, "fff&&(hie||c4)");
  // TIter next(gDirectory->GetListOfKeys());    

  TIter next(_TRG->GetListOfKeys());    

  int ind=0;
  TKey *key = (TKey*)next();
  while(key){
    TObject *obj = key->ReadObj(); 
    if(obj->InheritsFrom("TH2I")){ 
      TH2I* hobj = (TH2I *)obj;

      // tdsrcp, tdtopp, tdeclp, tdcdcp
      if(TString(obj->GetName()).Contains("hgdl")){
	// tdsrcp
	bool rising_done=false;
	for(int clk=0; clk<nclks; clk++){
	  if(hobj->GetBinContent(clk+1, 1 + e_tdsrcp) > 0){
	    if(! rising_done){
	      htdsrcp_r->Fill(clk+0.5);
	      rising_done=true;
	    }
	  }else if(rising_done){ // == 0
	    htdsrcp_t->Fill(clk+0.5);
	    rising_done=false;
	  }
	}
	// tdtopp
	rising_done=false;
	for(int clk=0; clk<nclks; clk++){
	  if(hobj->GetBinContent(clk+1, 1 + e_tdtopp) > 0){
	    if(! rising_done){
	      htdtopp_r->Fill(clk+0.5);
	      rising_done=true;
	    }
	  }else if(rising_done){ // == 0
	    htdtopp_t->Fill(clk+0.5);
	    rising_done=false;
	  }
	}
	// tdeclp
	rising_done=false;
	for(int clk=0; clk<nclks; clk++){
	  if(hobj->GetBinContent(clk+1, 1 + e_tdeclp) > 0){
	    if(! rising_done){
	      htdeclp_r->Fill(clk+0.5);
	      rising_done=true;
	    }
	  }else if(rising_done){ // == 0
	    htdeclp_t->Fill(clk+0.5);
	    rising_done=false;
	  }
	}
	// tdcdcp
	rising_done=false;
	for(int clk=0; clk<nclks; clk++){
	  if(hobj->GetBinContent(clk+1, 1 + e_tdcdcp) > 0){
	    if(! rising_done){
	      htdcdcp_r->Fill(clk+0.5);
	      rising_done=true;
	    }
	  }else if(rising_done){ // == 0
	    htdcdcp_t->Fill(clk+0.5);
	    rising_done=false;
	  }
	}

      } // hgdl

      if(TString(obj->GetName()).Contains("hitd")){
	for(int i=0; i<87; i++){
	  hobj->GetYaxis()->SetBinLabel(i+1, CINAME[i]);
	}
	hobj->Write();
      }

      for(int bit=0; bit<89; bit++){

	bool rising_done=false;
	bool falling_done=false;
	
	// Clock
	for(int clk=0; clk<nclks; clk++){ // j=0 is for all events
	  if(hobj->GetBinContent(clk+1, bit+1) == 0){
	    if(TString(obj->GetName()).Contains("hitd")){
	      if(rising_done && ! falling_done){
		h_t[bit]->Fill(clk+0.5);
		falling_done=true;
	      }
	    }
	  }
	  if(hobj->GetBinContent(clk+1, bit+1)>0){
	    if(TString(obj->GetName()).Contains("hitd")){
	      h_i[bit]->Fill(clk+0.5);
	      if(! rising_done){
		h_r[bit]->Fill(clk+0.5);
		rising_done=true;
	      }else if(rising_done && ! falling_done && clk == nclks-1){
		h_t[bit]->Fill(clk+0.5);
	      }
	    }
	    if(TString(obj->GetName()).Contains("hftd")){
	      h_f[bit]->Fill(clk+0.5);
	    }
	    if(TString(obj->GetName()).Contains("hpsn")){
	      h_p[bit]->Fill(clk+0.5);
	    }
	  }
	} // End of Clock

	// To count number of all events
	if(TString(obj->GetName()).Contains("itd")){
	  h_i[bit]->Fill(-0.5);
	}
	if(TString(obj->GetName()).Contains("ftd")){
	  h_f[bit]->Fill(-0.5);
	}
	if(TString(obj->GetName()).Contains("psn")){
	  h_p[bit]->Fill(-0.5);
	}

	// fff 2nd peak study
	if(TString(obj->GetName()).Contains("psn")){
	  if(bit == 2){
	    if(hobj->GetBinContent(21, bit+1) == 1 ||
	       hobj->GetBinContent(22, bit+1) == 1 ||
	       hobj->GetBinContent(23, bit+1) == 1 ||
	       hobj->GetBinContent(24, bit+1) == 1 ||
	       hobj->GetBinContent(25, bit+1) == 1 ||
	       hobj->GetBinContent(26, bit+1) == 1){
	      for(int clk=0; clk<nclks; clk++){
		if(hobj->GetBinContent(clk+1, bit+1) == 1){
		  hpsnfff->Fill(clk+0.5);
		}
	      }
	    }
	    int a=0;
	    for(int clk=0; clk<nclks; clk++){
	      a += hobj->GetBinContent(clk+1, bit+1);
	    }
	    if(a > 1){
	      for(int clk=0; clk<nclks; clk++){
		if(hobj->GetBinContent(clk+1, bit+1) == 1){
		  hfffdouble->Fill(clk+0.5);
		}
	      }
	    }else if(a == 1){
	      for(int clk=0; clk<nclks; clk++){
		if(hobj->GetBinContent(clk+1, bit+1) == 1){
		  hfffnodouble->Fill(clk+0.5);
		}
	      }
	    }
	  }
	}
      } // for(int bit=0; bit<96; bit++)

      if(TString(obj->GetName()).Contains("hpsn")){
	bool bl_hie=false;
	bool bl_c4=false;
	bool bl_fff=false;
	bool bl_hiec4=false;
	bool bl_fffhiec4=false;
	for(int clk=0; clk<nclks; clk++){
	  if(hobj->GetBinContent(clk+1, b_fff+1) == 1){
	    bl_fff=true;
	  }
	  if(hobj->GetBinContent(clk+1, b_hie+1) == 1){
	    bl_hie=true;
	  }
	  if(hobj->GetBinContent(clk+1, b_c4+1) == 1){
	    bl_c4=true;
	  }
	  if(hobj->GetBinContent(clk+1, b_hie+1) == 1 || 
	     hobj->GetBinContent(clk+1, b_c4+1) == 1){
	    hc4orhie->Fill(clk+0.5);
	  }
	}
	if(bl_hie)      h_effi->Fill(0.5);
	if(bl_c4)       h_effi->Fill(1.5);
	if(bl_hie || bl_c4)             h_effi->Fill(2.5);
	if(bl_fff)                      h_effi->Fill(3.5);
	if(bl_fff && (bl_hie || bl_c4)) h_effi->Fill(4.5);
      }
      
    } // if(obj->InheritsFrom("TH2I")) 
    key = (TKey*)next();
  }

  TPostScript *ps=new TPostScript(Form("%s.ps", target.Data()));

  // gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1");
  for(int i=0; i<88; i++){
    /*
    ps->NewPage();
    h_i[i]->GetXaxis()->SetRange(2, 33);
    h_i[i]->Draw();
    c1->Update();
    */
    ps->NewPage();
    h_r[i]->SetLineColor(kRed);
    h_r[i]->Draw();
    h_t[i]->SetLineColor(kGreen);
    h_t[i]->Draw("same");
    c1->Update();
  }
  ps->NewPage(); htdsrcp_r->Draw(); htdsrcp_t->Draw("same"); c1->Update();
  ps->NewPage(); htdtopp_r->Draw(); htdtopp_t->Draw("same"); c1->Update();
  ps->NewPage(); htdeclp_r->Draw(); htdeclp_t->Draw("same"); c1->Update();
  ps->NewPage(); htdcdcp_r->Draw(); htdcdcp_t->Draw("same"); c1->Update();
  for(int i=0; i<96; i++){
    ps->NewPage();
    h_p[i]->GetXaxis()->SetRange(h_p[i]->GetXaxis()->FindBin(0.5), 
				 h_p[i]->GetXaxis()->FindBin(31.5));
    h_p[i]->Draw();
    c1->Update();
    /*
    ps->NewPage();
    h_f[i]->GetXaxis()->SetRange(2, 33);
    h_f[i]->Draw();
    c1->Update();
    */
  }
  ps->NewPage(); hpsnfff->Draw(); c1->Update();
  ps->NewPage(); hfffdouble->Draw(); c1->Update();
  ps->NewPage(); hfffnodouble->Draw(); c1->Update();
  ps->NewPage(); hc4orhie->Draw(); c1->Update();
  ps->Close();

  c1->Clear();
  c1->Divide(1,4);
  c1->cd(1); h_r[4]->Draw(); h_t[4]->Draw("same");
  c1->cd(2); h_r[5]->Draw(); h_t[5]->Draw("same");
  c1->cd(3); h_r[6]->Draw(); h_t[6]->Draw("same");
  c1->cd(4); h_r[7]->Draw(); h_t[7]->Draw("same");
  c1->Print(Form("t2.%s.eps", target.Data()));
  c1->Print(Form("t2.%s.png", target.Data()));
  cout << "hie(" << h_effi->GetBinContent(1)
       << "), c4(" << h_effi->GetBinContent(2)
       << "), hie||c4(" << h_effi->GetBinContent(3)
       << "), fff(" << h_effi->GetBinContent(4)
       << "), (hie||c4)&&fff(" << h_effi->GetBinContent(5)
       << "), (hie||c4) m fff(" << h_effi->GetBinContent(5)/h_effi->GetBinContent(4)
       << "), fff m (hie||c4) m fff(" << h_effi->GetBinContent(5)/h_effi->GetBinContent(3)
       << ")" << endl;

  f0.Close();
}
