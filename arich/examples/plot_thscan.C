/*
 * Plots histograms from ARICHRateCal module
 * run as: "root -l histogram_file.root plot_thscan.C"
 * By: Tomoyuki Konno
 */

TCanvas* newTCanvas(const char* cname,
                    Int_t width = 680, Int_t height = 240);

void myinit();

void plot_thscan(){
  myinit();

  const std::string www = "/var/www/html/hvala/plots";
  system(("mkdir -p "+www).c_str());
  std::ofstream fout((www+"/index.html").c_str());
  fout << "<html><head>" << std::endl
       << "<title>" << "Threshold scan" << "</title>" << std::endl
       << "</head><body>" << std::endl
       << _file0->GetName() << "<br/>" << std::endl;
  for (int i = 0; i< 100; i++) {
    TH1F* h = (TH1F*)_file0->Get(Form("h_rate2D_%d", i));
    if (h!=NULL && h->GetEntries() > 0) {
      TCanvas* c = NULL;
      std::string cname = Form("c_rate2D_%d",i);
      c = newTCanvas(cname.c_str(), 720, 560);
      //c->SetGrid();
      //c->SetGridx();
      //c->SetGridy();
      h->Draw("colz");
      Double_t max = 500;//h->GetMaximum();
      Double_t min = -500;//h->GetMinimum();
      TLine* line=new TLine(144, min,144,max);
      line->Draw();
      line=new TLine(144*2, min,144*2,max);
      line->Draw();
      line=new TLine(144*3, min,144*3,max);
      line->Draw();
      line=new TLine(144*4, min,144*4,max);
      line->Draw();
      line=new TLine(144*5, min,144*5,max);
      line->Draw();

      c->Print((cname+".png").c_str());
      //c->Print((www+"/"+cname+".png").c_str());
      fout << "<a href=\"./" << cname << ".png\" >" << std::endl
	   << "<img src=\"./" << cname << ".png\" alt=\"" << cname<< "\"/>" << std::endl
	   << "</a>" << std::endl;
    }
  }
  fout << "</body></html>" << std::endl;
  std::cout << "open http://disk.arich.kek.jp/hvala/plots/" << std::endl;
}

TCanvas* newTCanvas(const char* cname, Int_t width, Int_t height)
{
  TCanvas* c = new TCanvas(cname, cname, width, height);
  c->SetTickx(1);
  c->SetTicky(1);
  c->SetLeftMargin(0.11);
  c->SetRightMargin(0.1);
  c->SetBottomMargin(0.13);
  return c;
}

void myinit()
{
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetOptStat("");
  gStyle->SetLabelFont(62,"");
  gStyle->SetLabelSize(0.047,"");
  gStyle->SetLabelFont(62,"xyz");
  gStyle->SetLabelSize(0.042,"xyz");
  gStyle->SetTitleFont(62,"xyz");
  gStyle->SetTitleFontSize(0.045);
  gStyle->SetTitleFont(62);
  gStyle->SetTitleSize(0.044,"xyz");
  gStyle->SetTitleOffset(1.0,"x");
  gStyle->SetTitleOffset(1.0,"y");
  gStyle->SetLabelOffset(0.0001,"x");
  gStyle->SetStripDecimals(kFALSE);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetStatW(0.35);
  gStyle->SetStatH(0.25);
  gStyle->SetPadTickX(kTRUE);
  gStyle->SetPadTickY(kTRUE);
  gStyle->SetPalette(1);
  gStyle->SetNumberContours(99);
}
