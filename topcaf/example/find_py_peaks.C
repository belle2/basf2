void find_py_peaks(TString filename, const char* outfile="", const char* trigpos=""){

  string trigstring=string(trigpos);
  cout<<trigstring<<endl;


  TFile fr(filename);


  //First find the peaks

  //TH1F *h=(TH1F*) fr.Get("ch_time_qual_py");
  TH1F *h=(TH1F*) fr.Get("ch_time_tight_py");
  
  const int npeaks=1;
  TH1F* roi[3];
  for(int ii=0;ii<3;ii++){
    //roi[ii]=(TH1F*)ch_time_proj_qual->Clone();
    roi[ii]=(TH1F*) h->Clone();
    //roi[ii]->Rebin(2);
  }
  
  const int flag_lab = (((strstr(filename, "cpr31") != NULL)) || ((strstr(filename, "cpr32") != NULL))) ;
  //0=tsukuba, 1=fuji
  if(flag_lab==1){
    if(trigstring=="mirror"){
      roi[0]->GetXaxis()->SetRangeUser(-1220,-1180);
    }
    if(trigstring=="midbar"){
      roi[0]->GetXaxis()->SetRangeUser(-1250,-1220);
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
    //Now in all cases search the whole range
    roi[0]->GetXaxis()->SetRangeUser(-500,-10);
  }
  
  TSpectrum *s0 = new TSpectrum(npeaks);
//  int nfound0 = s0->Search(roi[0],1,"",0.01);
  Double_t *xpeaks0=s0->GetPositionX();
  
  if(trigstring=="mirror"){
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+90-25,xpeaks0[0]+90+25);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]-42.5-22.5,xpeaks0[0]-42.5+22.5);
  }
  if(trigstring=="midbar"){
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+77.5-17.5,xpeaks0[0]+77.5+17.5);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]+162-20,xpeaks0[0]+162+20);
  }
  if(trigstring=="prism"){
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0]+77.5-32.5,xpeaks0[0]+77.5+32.5);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0]-37.5-22.5,xpeaks0[0]-37.5+22.5);
  }
  
  TSpectrum *s1 = new TSpectrum(npeaks);
//  int nfound1 = s1->Search(roi[1],1,"",0.01);
  Double_t *xpeaks1=s1->GetPositionX();
  TSpectrum *s2 = new TSpectrum(npeaks);
//  int nfound2 = s2->Search(roi[2],1,"",0.01);
  Double_t *xpeaks2=s2->GetPositionX();

  cout<<xpeaks0[0]<<" "<<xpeaks1[0]<<" "<<xpeaks2[0]<<" "<<endl;
  vector<float> infovect;
  infovect.push_back(xpeaks0[0]);
  infovect.push_back(xpeaks1[0]);
  infovect.push_back(xpeaks2[0]);

  //Now record the "bad" ASICs
  TH1F *h2=(TH1F*) fr.Get("asic_occupancy_all");
  Float_t a_cut=(h2->GetEntries())/64;
  cout<<a_cut<<endl;
  for(int i=1; i<65; i++){
    if(h2->GetBinContent(i)>(a_cut*1.5)){
      infovect.push_back(i-1);
      cout<<"Hot ASIC: "<<i<<" "<<h2->GetBinContent(i)<<endl;
    }
    if(h2->GetBinContent(i)<(a_cut*0.25)){
      infovect.push_back(i-1);
      cout<<"Cold ASIC: "<<i<<" "<<h2->GetBinContent(i)<<endl;
    }
  }

  ofstream out(outfile);
  for(int i=0; i<infovect.size(); i++){
    out << infovect.at(i) <<endl;
  }
  
}
