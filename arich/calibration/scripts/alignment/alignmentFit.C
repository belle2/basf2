TGraphErrors *g[36];
Double_t offset[6][36];
Double_t offset_err[6][36];
Double_t amp[6][36];
Double_t phase[6][36];
Double_t amp_err[6][36];
Double_t phase_err[6][36];
Double_t data[36][13];
Double_t data_err[36][13];
Double_t malt[6]={ 5.0, 5.0, 5.0, 1.0, 1.0, 0.5};

static void FCN(int &npar, Double_t *gin, Double_t &f, Double_t *par, int iflag){

  f = 0;
  for (Int_t i = 0; i < 36; i++){  // loop over graphs
    for (Int_t j = 0; j < 13; j++){  // loop over points
      Double_t dataval = data[i][j];
      Double_t err = data_err[i][j];
      Double_t fitval = 0;
      Double_t offsetsum = 0;
      Double_t parsum = 0;
      for (Int_t k = 0; k < 6; k++){  // loop over six assumptions
	offsetsum +=offset[k][i]*par[k]/malt[k];
	parsum +=par[k]/malt[k];
        fitval += par[k]/malt[k]*amp[k][i]*cos((-3.0+j*0.5)+phase[k][i]);
      }
      fitval +=par[6]*offsetsum/parsum; //Add weighted mean of offset 
      f += (dataval-fitval)*(dataval-fitval)/err/err;  //caluculation of chi2
    }
  }
}

void alignmentFit(){
  //TFile*file=new TFile("simbha/Graph/g36_Bhabha_Prod5_MC.root");
  TFile*file=new TFile("Prod5data/g36_Prod5Bhabha.root");
  //TFile*file=new TFile("Prod5data/g36_Prod5Bhabha_fix.root");

  FILE*fxo=fopen("simbha/Offset/Offset_Bhabha_x5mm.txt","r");
  FILE*fyo=fopen("simbha/Offset/Offset_Bhabha_y5mm.txt","r");
  FILE*fzo=fopen("simbha/Offset/Offset_Bhabha_z5mm.txt","r");
  FILE*fao=fopen("simbha/Offset/Offset_Bhabha_a1deg.txt","r");
  FILE*fbo=fopen("simbha/Offset/Offset_Bhabha_b1deg.txt","r");
  FILE*fgo=fopen("simbha/Offset/Offset_Bhabha_g05deg.txt","r");

  FILE*fxa=fopen("simbha/Amplitude/Amp_Bhabha_x5mm.txt","r");
  FILE*fya=fopen("simbha/Amplitude/Amp_Bhabha_y5mm.txt","r");
  FILE*fza=fopen("simbha/Amplitude/Amp_Bhabha_z5mm.txt","r");
  FILE*faa=fopen("simbha/Amplitude/Amp_Bhabha_a1deg.txt","r");
  FILE*fba=fopen("simbha/Amplitude/Amp_Bhabha_b1deg.txt","r");
  FILE*fga=fopen("simbha/Amplitude/Amp_Bhabha_g05deg.txt","r");

  FILE*fxp=fopen("simbha/Phase/Phase_Bhabha_x5mm.txt","r");
  FILE*fyp=fopen("simbha/Phase/Phase_Bhabha_y5mm.txt","r");
  FILE*fzp=fopen("simbha/Phase/Phase_Bhabha_z5mm.txt","r");
  FILE*fap=fopen("simbha/Phase/Phase_Bhabha_a1deg.txt","r");
  FILE*fbp=fopen("simbha/Phase/Phase_Bhabha_b1deg.txt","r");
  FILE*fgp=fopen("simbha/Phase/Phase_Bhabha_g05deg.txt","r");

  Int_t l=0;
  while(fscanf(fxo, "%lf,%lf,", &offset[0][l], &offset_err[0][l])!=EOF){l++;}
  l=0;      
  while(fscanf(fyo, "%lf,%lf,", &offset[1][l], &offset_err[1][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fzo, "%lf,%lf,", &offset[2][l], &offset_err[2][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fao, "%lf,%lf,", &offset[3][l], &offset_err[3][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fbo, "%lf,%lf,", &offset[4][l], &offset_err[4][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fgo, "%lf,%lf,", &offset[5][l], &offset_err[5][l])!=EOF){l++;}
  
  l=0;    
  while(fscanf(fxa, "%lf,%lf,", &amp[0][l], &amp_err[0][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fya, "%lf,%lf,", &amp[1][l], &amp_err[1][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fza, "%lf,%lf,", &amp[2][l], &amp_err[2][l])!=EOF){l++;}
  l=0;    
  while(fscanf(faa, "%lf,%lf,", &amp[3][l], &amp_err[3][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fba, "%lf,%lf,", &amp[4][l], &amp_err[4][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fga, "%lf,%lf,", &amp[5][l], &amp_err[5][l])!=EOF){l++;}
  
  l=0;    
  while(fscanf(fxp, "%lf,%lf,", &phase[0][l], &phase_err[0][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fyp, "%lf,%lf,", &phase[1][l], &phase_err[1][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fzp, "%lf,%lf,", &phase[2][l], &phase_err[2][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fap, "%lf,%lf,", &phase[3][l], &phase_err[3][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fbp, "%lf,%lf,", &phase[4][l], &phase_err[4][l])!=EOF){l++;}
  l=0;    
  while(fscanf(fgp, "%lf,%lf,", &phase[5][l], &phase_err[5][l])!=EOF){l++;}
  
  Double_t dummy;
  
  for (Int_t i = 0; i < 36; i++){   // loop over graphs
    g[i]=(TGraphErrors*)file->Get(Form("Graph;%d",i+1));
    
    for (Int_t j = 0; j < 13; j++){  // loop over points
      g[i]->GetPoint(j, dummy, data[i][j]);  // store value to the array
      data_err[i][j] = g[i]->GetErrorY(j);     // store error to the array
    }
  }
  fclose(fxo);
  fclose(fyo);
  fclose(fzo);
  fclose(fao);
  fclose(fbo);
  fclose(fgo);
  
  fclose(fxa);
  fclose(fya);
  fclose(fza);
  fclose(faa);
  fclose(fba);
  fclose(fga);
    
  fclose(fxp);
  fclose(fyp);
  fclose(fzp);
  fclose(fap);
  fclose(fbp);
  fclose(fgp);
    
  int ierflg = 0;
  TMinuit minuit(7);
  minuit.SetFCN(FCN);
  minuit.mnparm(0, "trans_x", -0.82, 0.01, -2.0, 2.0, ierflg); // Unit: mm
  minuit.mnparm(1, "trans_y", 0.35, 0.01, -2.0, 2.0, ierflg); // Unit: mm
  minuit.mnparm(2, "trans_z", 1.79, 0.01, -2.0, 2.0, ierflg); // Unit: mm
  minuit.mnparm(3, "rot_x", 0.053, 0.001, -1.0, 1.0, ierflg); // Unit: deg
  minuit.mnparm(4, "rot_y", 0.103, 0.001, -1.0, 1.0, ierflg); // Unit: deg
  minuit.mnparm(5, "rot_z", -0.013, 0.001, -1.0, 1.0, ierflg); // Unit: deg */
  /*minuit.mnparm(0, "trans_x", 0.00001, 0.0001, -2.0, 2.0, ierflg); // Unit: mm
  minuit.mnparm(1, "trans_y", 0.00001, 0.0001, -2.0, 2.0, ierflg); // Unit: mm
  minuit.mnparm(2, "trans_z", 0.00001, 0.0001, -2.0, 2.0, ierflg); // Unit: mm
  minuit.mnparm(3, "rot_x", 0.00001, 0.00001, -1.0, 1.0, ierflg); // Unit: deg
  minuit.mnparm(4, "rot_y", 0.00001, 0.00001, -1.0, 1.0, ierflg); // Unit: deg
  minuit.mnparm(5, "rot_z", 0.00001, 0.00001, -1.0, 1.0, ierflg); // Unit: deg */
  minuit.mnparm(6, "offset", 1.0, 0.01, 0.8, 1.2, ierflg); 
  //minuit.FixParameter(6);
  minuit.Migrad();
  minuit.mnhess();
  minuit.mnmnos();

  double val[7], err[7];
  for (Int_t k = 0; k < 7; k++){
    minuit.GetParameter(k, val[k], err[k]);
  }
  
  ofstream fout("fitresult.txt");  
  double chi2_ndf;
  double ln0,edm,errdef;                                                                                         
  int nvpar, nparx,icstat; 
  for(Int_t k=0;k<7;k++){                                                                                              
    fout<<val[k]<<","<<err[k]<<","<<endl;                                                                                        
  }                                                                                                                     
  minuit.mnstat(ln0,edm,errdef,nvpar,nparx,icstat);
  chi2_ndf=ln0/(36*13-7);
  fout<<"chi2/ndf= "<<chi2_ndf<<endl;
  

  // Draw plots and fit curve
  // cppcheck-suppress unreadVariable
  TF1 *f1 = new TF1("f1", "[0]*cos(x+[1])", -M_PI, M_PI); //parameter 0~5

  TCanvas*c1=new TCanvas("c1");
  c1->Divide(6,6);

  for (Int_t i = 0; i < 36; i++){ // loop over graphs
    c1->cd(i+1);
    Double_t offsetsum=0;
    Double_t valsum=0;
    TF1 *func = new TF1("func", "f1+f1+f1+f1+f1+f1+[12]", -M_PI, M_PI);
    Int_t m=0;
    for (Int_t k = 0; k < 6; k++){  // loop over assumptions translation*3+rotation*3
      offsetsum +=val[k]*offset[k][i]/malt[k];
      valsum +=val[k]/malt[k];
      func->SetParameter(m++, val[k]/malt[k]*amp[k][i]);
      func->SetParameter(m++, phase[k][i]);
    }
    func->SetParameter(12, val[6]*offsetsum/valsum);
#if 1
    // Write data points and the fit curve
    g[i]->Draw("AP");
    func->Draw("SAME P");
#else  
    // Write fit curve(black) and each compoments(tx: red, ty: green, tz: blue, 
    //                                            rx: yellow, ry: magenta, rz: cyan)
    func->SetLineColor(1);
    func->Draw();
    for (Int_t k = 0; k < 6; k++){  // loop over assumptions translation*3+rotation*3
      TF1 *f2 = new TF1("f2", "[0]+[1]*cos(x+[2])", -M_PI, M_PI); //parameter 0~5
      f2->SetParameter(0, offset[k][i]);
      f2->SetParameter(1, val[k]/malt[k]*amp[k][i]);
      f2->SetParameter(2, phase[k][i]);
      f2->SetParLimits(2,-TMath::Pi(),TMath::Pi());
      f2->SetLineColor(k+2);
      f2->Draw("SAME P");
    }
#endif
  }
}
