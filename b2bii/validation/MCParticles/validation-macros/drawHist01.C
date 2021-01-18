/*
<header>
<noexecute>Used as library.</noexecute>
</header>
*/

#include <iostream>
#include "histio.C"
#include "TCanvas.h"
#include "TText.h"
#include "TChain.h"
#include "TLegend.h"
#include "TH1.h"
#include "TCut.h"
#include "TLine.h"
#include "TStyle.h"

  using std::cout ;
  using std::endl ;

  TCanvas* c3 ;
  TPad *pad1;
  TPad *pad2;

  Int_t drawset(const char* hbasename, const char* xtitle, const char* ytitle,
               const char* plottitle,
               Bool_t savePlots=kFALSE, Bool_t interactive=kTRUE) ;

  void getBinContentDifference(TH1F* hfirst, TH1F* hsecond, TH1F* hdiff);

//===============================================================================

  void draw01(Int_t plotIndex, Bool_t savePlots=kFALSE, Bool_t interactive=kTRUE ) {

       cout << "argument savePlots=" << savePlots << " argument interactive=" << interactive
            << endl ;

       gStyle->SetOptTitle(0) ;
       gStyle->SetOptStat(0) ;

       c3 = (TCanvas*) gDirectory->FindObject("c3") ;
       if (c3==0) {
          cout << "Creating canvas." << endl ;
          c3 = new TCanvas("c3","c3",750,600) ;
       }

       pad1 = new TPad("pad1", "pad1", 0.01, 0.21, 0.99, 0.99) ;
       pad2 = new TPad("pad2", "pad2", 0.01, 0.01, 0.99, 0.20) ;

       pad1->Draw() ;
       pad2->Draw() ;

       gDirectory->Delete("h*") ;
       loadHist("scratch/cc-test.root") ;

       Int_t index(0) ;

       Bool_t sp(savePlots) ;
       Bool_t iv(interactive) ;

       Int_t retVal(0) ;

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("idhep_mpdg01", "idhep-m_pdg", "Events / ", "idhep-m_pdg",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("idhep_mpdg02", "idhep-m_pdg", "Events / ", "idhep-m_pdg",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("idhep_mpdg03", "idhep-m_pdg", "Events / ", "idhep-m_pdg",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("idhep_mpdg04", "idhep-m_pdg", "Events / ", "idhep-m_pdg",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("idhep_mpdg05", "idhep-m_pdg", "Events / ", "idhep-m_pdg",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("idhep_mpdg06", "idhep-m_pdg", "Events / ", "idhep-m_pdg",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("Px", "Px (GeV/c)", "Events / ", "Px",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("Py", "Py (GeV/c)", "Events / ", "Py",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("Pz", "Pz (GeV/c)", "Events / ", "Pz",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("Energy", "Energy (GeV)", "Events / ", "Energy",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("Mass", "Mass (GeV/c^2)", "Events / ", "Mass",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("ProdVertexX", "Production Vertex (X) (mm)","Events / ", "Production Vertex (X)",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("ProdVertexY", "Production Vertex (Y) (mm)","Events / ", "Production Vertex (Y)",sp,iv) ;
          if (retVal!=0) return ;
       }

       if (plotIndex==++index || plotIndex<0) { cout << " Drawing index " << index << endl ;
          retVal = drawset("ProdVertexZ", "Production Vertex (Z) (mm)","Events / ", "Production Vertex (Z)",sp,iv) ;
          if (retVal!=0) return ;
       }


  }
//===============================================================================

  Int_t drawset(const char* hbasename, const char* xtitle, const char* ytitle,
               const char* plottitle, Bool_t savePlots, Bool_t interactive) {

       char histname[500] ;
       char histname1[500] ;

       sprintf(histname,"h%s_basf",hbasename) ;
       TH1F *hbasf = (TH1F*)gDirectory->FindObject(histname) ;
       if ( hbasf==0 ) {cout << "*** can't find " << histname << endl ; return 1 ; }

       sprintf(histname1,"h%s_basf2",hbasename) ;
       TH1F* hbasf2 = (TH1F*) gDirectory->FindObject(histname1) ;
       if ( hbasf2==0 ) {cout << "*** can't find " << histname1 << endl ; return 1 ; }

       pad1->cd() ;
       pad1->Clear() ;

       // to put grid lines on X axis and Y axis
       pad1->SetGridx();
       pad1->SetGridy();
       // To set the log scale on the Y axis
//       pad1->SetLogy() ;

       hbasf->SetLineColor(2) ;
       hbasf->Draw() ;
       hbasf->GetXaxis()->SetTitle(xtitle) ;
       hbasf->GetYaxis()->SetTitle(ytitle) ;

       hbasf2->SetLineStyle(2) ;
       hbasf2->SetLineWidth(2) ;
       hbasf2->Draw("same") ;

       TLegend* legend = new TLegend(0.80,0.80,0.90,0.90) ;
       legend->AddEntry(hbasf,"BASF") ;
       legend->AddEntry(hbasf2,"BASF2") ;
       legend->Draw() ;

       pad2->cd() ;
       pad2->Clear() ;

       // to put grid lines on X axis
       pad2->SetGridy() ;

       int nbins = hbasf->GetSize()-2;  //2: underflow + overflow

       TH1F *hdiff = new TH1F("hdiff","hdiff", nbins, 0, nbins);

       // to avoid "Warning in <TROOT::Append>: Replacing existing TH1: hdiff· (Potential memory leak)"
       hdiff->SetDirectory(0) ;
       TH1::AddDirectory(kFALSE) ;

       getBinContentDifference(hbasf, hbasf2, hdiff) ;
       hdiff->Draw() ;

       c3->Update() ;

       char savename[300] ;
       sprintf( savename, "scratch/plots-cc-test/%s.eps", hbasename ) ;
       if (savePlots) c3->SaveAs(savename) ;
       sprintf( savename, "scratch/plots-cc-test/%s.gif", hbasename ) ;
       if (savePlots) c3->SaveAs(savename) ;

       if (interactive) {
          char answ ;
          cout << "Continue? " ;
          answ = getchar() ;
          if (answ=='q' || answ=='Q' || answ=='n' || answ=='N') return 1 ;
       }

       return 0 ;

  } // drawset

//----------------------------------------------------------------------------------

  void getBinContentDifference(TH1F* hfirst, TH1F* hsecond, TH1F* hdiff) {

       int nbin = hfirst->GetSize()-2;  //2: underflow + overflow
       cout << "nbins " << nbin << endl ;

       int i = 0;
       for (i=0; i<nbin; i++) {
           double nfirst = hfirst->GetBinContent(i) ;
           double nsecond = hsecond->GetBinContent(i) ;

           double diff = nfirst -nsecond ;

           hdiff->SetBinContent(i,diff);
       }

       hdiff->GetYaxis()->SetNdivisions(405) ;
       hdiff->GetXaxis()->SetRangeUser(0.0, i) ;
       hdiff->GetYaxis()->CenterTitle(true) ;
       hdiff->GetYaxis()->SetTitle("") ;

       hdiff->SetMarkerStyle(20) ;
       hdiff->SetMarkerSize(1.0) ;
//       hdiff->GetXaxis()->SetTitle("nbin") ;
//       hdiff->GetXaxis()->CenterTitle(true);
       hdiff->GetXaxis()->SetLabelSize(0.118) ;
//       hdiff->GetXaxis()->SetTitleSize(0.055) ;
       hdiff->GetYaxis()->SetTitle("bin content diff") ;
       hdiff->GetYaxis()->CenterTitle(true);
       hdiff->GetYaxis()->SetTitleFont(62) ;
       hdiff->GetYaxis()->SetTitleSize(0.055) ;
       hdiff->GetYaxis()->SetLabelSize(0.118) ;

       hdiff->SetLineColor(6) ;

       } //getBinContentDifference
