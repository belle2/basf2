void plot_package(TString filename, const char* outfile="", const char* trigpos=""){

  //Opens ROOT "filename"
  //Makes plots based on trigpos and lab
  //Saves as PS file "outfile"

  string trigstring=string(trigpos);
  //cout<<trigstring<<endl;

  TFile fr(filename);

  //First find the peaks

  //TH1F *h=(TH1F*) fr.Get("ch_time_qual_py");
  TH1F *h=(TH1F*) fr.Get("ch_time_optics_py");
  
  const int npeaks=1;
  TH1F* roi[3];
  for(int ii=0;ii<3;ii++){
    roi[ii]=(TH1F*) h->Clone();
  }

  const int flag_lab = (((strstr(filename, "cpr31") != NULL)) || ((strstr(filename, "cpr32") != NULL))) ;
  //0=tsukuba, 1=fuji
  if(flag_lab==1){
    if(trigstring=="mirror"){
      roi[0]->GetXaxis()->SetRangeUser(-1220,-1180);
    }
    if(trigstring=="midbar"){
      roi[0]->GetXaxis()->SetRangeUser(-1250,-1210);
    }
    if(trigstring=="prism"){
      roi[0]->GetXaxis()->SetRangeUser(-1240,-1170);
    }
  }
  else{
    if(trigstring=="mirror"){
      roi[0]->GetXaxis()->SetRangeUser(-500,-100);
    }
    if(trigstring=="midbar"){
      roi[0]->GetXaxis()->SetRangeUser(-475,-445);
    }
    if(trigstring=="prism"){
      roi[0]->GetXaxis()->SetRangeUser(-500,-100);
    }
    if(trigstring=="laser"){
      roi[0]->GetXaxis()->SetRangeUser(300,500);
    }
    //For installed modules, use full negative range
    if(strstr(filename, "slot") != NULL){
      roi[0]->GetXaxis()->SetRangeUser(-500,-5);
    }
  }
  
  TSpectrum *s0 = new TSpectrum(npeaks);
  int nfound0 = s0->Search(roi[0],1,"",0.01);
  Double_t *xpeaks0=s0->GetPositionX();


  Float_t sigma1,sigma2;
  sigma1=9;
  sigma2=13;

  if(trigstring=="mirror"){
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+90-25,xpeaks0[0]+90+25);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]-42.5-22.5,xpeaks0[0]-42.5+22.5);
    sigma1=12;
    sigma2=16;
  }
  if(trigstring=="midbar"){
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+77.5-17.5,xpeaks0[0]+77.5+17.5);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]+162-20,xpeaks0[0]+162+20);
  }
  if(trigstring=="prism"){
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+77.5-32.5,xpeaks0[0]+77.5+32.5);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]-37.5-22.5,xpeaks0[0]-37.5+22.5);
  }
  if(trigstring=="laser"){
    //currently incorrect values; do not matter
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+100-20,xpeaks0[0]+100+20);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]-42.5-22.5,xpeaks0[0]-42.5+22.5);
    sigma1=3;
    sigma2=3;
  }
  
  TSpectrum *s1 = new TSpectrum(npeaks);
  int nfound1 = s1->Search(roi[1],1,"",0.01);
  Double_t *xpeaks1=s1->GetPositionX();
  TSpectrum *s2 = new TSpectrum(npeaks);
  int nfound2 = s2->Search(roi[2],1,"",0.01);
  Double_t *xpeaks2=s2->GetPositionX();

  cout<<xpeaks0[0]<<" "<<xpeaks1[0]<<" "<<xpeaks2[0]<<" "<<endl;

  TH1F *h_scrod=(TH1F*) fr.Get("scrods_tight");
  TCanvas *c_scrod = new TCanvas("c_scrod","c_scrod",800,800);
  gStyle->SetOptStat(0);
  c_scrod->SetLogy();
  c_scrod->cd(1);
  h_scrod->Draw();
  c_scrod->Print(Form("%s.ps(",outfile));


  TH2F *h_hitmap0=(TH2F*) fr.Get("pmt_xy0");
  TH2F *h_hitmap1=(TH2F*) fr.Get("pmt_xy_tight0");
  TH2F *h_hitmap2=(TH2F*) fr.Get("pmt_xy_tight1");
  TH2F *h_hitmap3=(TH2F*) fr.Get("pmt_xy_tight2");
  TCanvas *c_hitmap = new TCanvas("chitmap","chitmap",800,800);  
  c_hitmap->Divide(1,4);
  //c_hitmap->SetLogy();
  c_hitmap->SetLogz();
  c_hitmap->cd(1);
  h_hitmap0->Draw();
  c_hitmap->cd(2);
  h_hitmap1->Draw();
  c_hitmap->cd(3);
  h_hitmap2->Draw();
  c_hitmap->cd(4);
  h_hitmap3->Draw();
  c_hitmap->Print(Form("%s.ps",outfile));

  TH1F *py2=(TH1F*) fr.Get("ch_time_optics_py");
  TCanvas *c_py2= new TCanvas("c_py2","c_py2",800,800);
  c_py2->Divide(1,2);
  gStyle->SetOptStat(0);
  c_py2->cd(1);
  if(flag_lab==1){
    py2->GetXaxis()->SetRangeUser(-1300,-1050);
  }
  else{
    py2->GetXaxis()->SetRangeUser(-500,-250);
    if(trigstring=="laser"){
      py2->GetXaxis()->SetRangeUser(300,500);      
      if(strstr(filename, "slot") != NULL){
	py2->GetXaxis()->SetRangeUser(xpeaks0[0]-20,xpeaks0[0]+150);
	TPad* c_py2_1=(TPad*)(c_py2->GetPrimitive("c_py2_1"));
	c_py2_1->SetLogy();
      }
    }
  }
  TF1 *fn0=new TF1("fn0","gaus",xpeaks0[0]-sigma1,xpeaks0[0]+sigma1);
  fn0->SetParLimits(0,10,py2->GetMaximum()*1.2);
  fn0->SetParLimits(1,xpeaks0[0]-sigma1,xpeaks0[0]+sigma1);
  fn0->SetParLimits(2,4,20);
  if(trigstring=="laser"){
    fn0->SetParLimits(2,0.5,5);
  }
  py2->Fit(fn0,"RB");
  TF1 *fn1=new TF1("fn1","gaus(0)+pol2(3)",xpeaks1[0]-sigma2*2,xpeaks1[0]+sigma2*2);
  fn1->SetParLimits(0,10,py2->GetMaximum()*1.2);
  fn1->SetParLimits(1,xpeaks1[0]-sigma1,xpeaks1[0]+sigma1);
  fn1->SetParLimits(2,4,30);
  if(trigstring=="laser"){
    fn1->SetParLimits(2,0.5,5);
  }
  py2->Fit(fn1,"RB");
  TPaveText *py1t;
  if(strstr(filename, "slot") != NULL){
    py1t=new TPaveText(0.8,0.7,1.0,0.9,"NDC");
  }
  else{
    py1t=new TPaveText(0.6,0.5,1.0,0.9,"NDC");
  }
  char ct1[25];
  sprintf(ct1,"1: x=%f +/- %f",fn0->GetParameter(1),fn0->GetParError(1));
  py1t->AddText(ct1);
  sprintf(ct1,"1: #s=%f +/- %f",fn0->GetParameter(2),fn0->GetParError(2));
  py1t->AddText(ct1);
  sprintf(ct1,"2: x=%f +/- %f",fn1->GetParameter(1),fn1->GetParError(1));
  py1t->AddText(ct1);
  sprintf(ct1,"2: #s=%f +/- %f",fn1->GetParameter(2),fn1->GetParError(2));
  py1t->AddText(ct1);
  sprintf(ct1,"diff: %f ",fn1->GetParameter(1)-fn0->GetParameter(1));
  py1t->AddText(ct1);
  sprintf(ct1,"ratio: %f ",fn1->GetParameter(0)*fn1->GetParameter(2)/fn0->GetParameter(0)/fn0->GetParameter(2));
  py1t->AddText(ct1);
  py2->Draw();
  fn0->Draw("SAME");
  fn1->Draw("SAME");
  py1t->Draw("SAME");

  TH1F *h_nhit0=(TH1F*) fr.Get("nhit0");
  TH1F *h_nhit1=(TH1F*) fr.Get("nhit_tight0");
  TPad *c_subpad=(TPad*)c_py2->cd(2);
  c_subpad->Divide(2);
  TPaveText *ptnhit[10];
  char ct[20];
  c_subpad->cd(1);
  ptnhit[0]=new TPaveText(0.5,0.8,0.8,0.9,"NDC");
  sprintf(ct,"Mean no0: %f", h_nhit0->GetMean(1)*h_nhit0->GetEntries()/(h_nhit0->GetEntries()-h_nhit0->GetBinContent(1)));
  ptnhit[0]->AddText(ct);
  if(h_nhit0->GetMaximumBin()==1){
    h_nhit0->GetXaxis()->SetRangeUser(1,100);
  }
  h_nhit0->Draw();
  ptnhit[0]->Draw("same");
  c_subpad->cd(2);
  ptnhit[1]=new TPaveText(0.5,0.8,0.8,0.9,"NDC");
  sprintf(ct,"Mean no0: %f", h_nhit1->GetMean(1)*h_nhit1->GetEntries()/(h_nhit1->GetEntries()-h_nhit1->GetBinContent(1)));
  ptnhit[1]->AddText(ct);
  if(h_nhit1->GetMaximumBin()==1){
    h_nhit1->GetXaxis()->SetRangeUser(1,100);
  }
  h_nhit1->Draw();
  ptnhit[1]->Draw("same");
  c_py2->Print(Form("%s.ps",outfile));

  TH1F *asict[64];
  for(int i=0;i<64;i++){
    asict[i]=(TH1F*) fr.Get(Form("asic_time_optics%i",i));
  }
  TCanvas *cbs[4];
  TPad *cbs_i[64];
  for(int ii=0;ii<4;ii++){
    cbs[ii]=new TCanvas(Form("cbs%i",ii),Form("cbs%i",ii),800,800);
    cbs[ii]->Divide(4,4);
    for(int iii=0;iii<16;iii++){
      cbs[ii]->cd(iii+1);
      asict[ii*16+iii]->Rebin(2);
      if(flag_lab==1){
        asict[ii*16+iii]->GetXaxis()->SetRangeUser(-1300,-1050);
      }
      else{
	asict[ii*16+iii]->GetXaxis()->SetRangeUser(-500,-250);
	if(trigstring=="laser"){
	  asict[ii*16+iii]->GetXaxis()->SetRangeUser(300,500);
	  if(strstr(filename, "slot") != NULL){
	    asict[ii*16+iii]->GetXaxis()->SetRangeUser(xpeaks0[0]-20,xpeaks0[0]+150);
	    cbs_i[ii*16+iii]=(TPad*)(cbs[ii]->GetPrimitive(Form("cbs%i_%i",ii,iii+1)));
	    cbs_i[ii*16+iii]->SetLogy();
	  }
	}
      }
      asict[ii*16+iii]->Draw();
    }
    cbs[ii]->Print(Form("%s.ps",outfile));
  }

  TH2F *ctt=(TH2F*) fr.Get("tdcbin_time_tight");
  TCanvas *cctt= new TCanvas("cctt","cctt",800,800);
  gStyle->SetOptStat(0);
  cctt->SetLogz();
  cctt->cd(1);
  if(flag_lab==1){
    ctt->GetXaxis()->SetRangeUser(200,600);
    ctt->GetYaxis()->SetRangeUser(-1280,-1200);
  }
  else{
    ctt->GetXaxis()->SetRangeUser(0,400);
    ctt->GetYaxis()->SetRangeUser(-500,-420);
    if(trigstring=="laser"){
      ctt->GetXaxis()->SetRangeUser(500,1000);
      ctt->GetYaxis()->SetRangeUser(300,500);
      if(strstr(filename, "slot") != NULL){
	ctt->GetXaxis()->SetRangeUser(0,500);
        ctt->GetYaxis()->SetRangeUser(xpeaks0[0]-20,xpeaks0[0]+150);
      }
    }
  }
  ctt->Draw("COLZ");
  cctt->Print(Form("%s.ps",outfile)); 

  TH2F *cht=(TH2F*) fr.Get("ch_time_tight");
  TCanvas *ccht= new TCanvas("ccht","ccht",800,800);
  gStyle->SetOptStat(0);
  ccht->SetLogz();
  if(flag_lab==1){
    cht->GetYaxis()->SetRangeUser(-1350,-1050);
  }
  else{
    cht->GetYaxis()->SetRangeUser(-550,-250);
    if(trigstring=="laser"){
      cht->GetYaxis()->SetRangeUser(250,550);
      if(strstr(filename, "slot") != NULL){
        cht->GetYaxis()->SetRangeUser(xpeaks0[0]-20,xpeaks0[0]+150);
      }
    }
  }
  ccht->cd(1);
  cht->Draw("COLZ");
  ccht->Print(Form("%s.ps)",outfile));




}
