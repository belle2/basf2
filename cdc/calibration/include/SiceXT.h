namespace Belle2 {
  namespace CDC {
    class SliceXT {
      static SliceY(TH2D& h2)
      for (int i = 1; i < h2->GetNbinsX(); ++i)
      {
        //    h2->FitSlicesY(g1,i,i,0,"MQ");

        //    if(htmp){
        double p0 = h0->GetBinContent(i);
        double p1 = hm->GetBinContent(i);
        double p2 = hs->GetBinContent(i);
        //    g1->GetParameters(par);
        TF1* g2 = new TF1("g2", "gaus", p1 - fabs(p2), p1 + 1.2 * fabs(p2));
        //    TF1* g2 = new TF1("g2","gaus",-1*par[1]+fabs(par[2]),par[1]+fabs(par[2]));
        g2->SetParameters(p0, p1, p2);
        //    g2->SetParLimits(1,p1-fabs(p2)*0.5,p1+fabs(p2)*0.5);
        h2->FitSlicesY(g2, i, i, 5, "QR");
        htmp = (TH1D*)gDirectory->Get("h55_1_0_2_1");
        g2->GetParameters(par2);
        double er = g2->GetParError(1);
        hlast->SetBinContent(i, par2[1]);
        hlast->SetBinError(i, er);
        g2->ReleaseParameter(1);
        cout << i << "   " << p1 << "   " << par2[1] << endl;
        //  else{cout<<"failure fit"<<endl;}
      }


    };
  }
}
