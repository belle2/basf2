void getResult(){
  ifstream ifs("runlist");
  string id;
  int n=0;
  double pval = 0;
  double ndf = 0;
  TString filename;
  while(getline(ifs,id)){
    n +=1;
    filename = "rootfile/output_"+id;
    TFile f(filename);
    TH1D* hPval = (TH1D*)f.Get("trackfit/hPval");
    TH1D* hNDF = (TH1D*)f.Get("trackfit/hNDF");
    if(hPval){
      pval += hPval->GetMean();
      ndf += hNDF->GetMean();
    }
    f.Close();
  }
  ofstream output("result.out", std::ios::app);
  output << ndf/n <<"  "<<pval/n <<endl;
  output.close();
}
