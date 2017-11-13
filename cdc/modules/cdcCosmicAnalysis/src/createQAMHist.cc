#include "cdc/modules/cdcCosmicAnalysis/CDCCosmicAnalysisModule.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TTree.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TVector3.h"
#include "TGraphErrors.h"

namespace Belle2 {
  namespace CDC {

    void createQAMHist(TTree* tree)
    {

      gROOT->Reset();
      gROOT->SetBatch(1);
      gStyle->SetOptStat(1);
      gStyle->SetOptTitle(111);
      gStyle->SetStatW(0.32);
      gStyle->SetStatH(0.32);
      gStyle->SetOptFit(1);
      gStyle->SetPalette(1);

      TTree* treeTrk0 = tree;

      double ndf1, Pval1, D01, Phi01, Om1, Z01, tanLambda1, Pt1;
      double ndf2, Pval2, D02, Phi02, Om2, Z02, tanLambda2, Pt2;
      double eD01, ePhi01, eOm1, eZ01, etanL1;
      double eD02, ePhi02, eOm2, eZ02, etanL2;
      TVector3* Mom1 = 0;
      TVector3* Mom2 = 0;
      TVector3* posSeed1 = 0;
      TVector3* posSeed2 = 0;


      TH1F* h00 = new TH1F("h00", "ndf (up)", 100, 0.0, 100.0);
      TH1F* h01 = new TH1F("h01", "ndf (down)", 100, 0.0, 100.0);
      TH1F* h02 = new TH1F("h02", "pval (up)", 100, 0.0, 1.0);
      TH1F* h03 = new TH1F("h03", "pval (down)", 100, 0.0, 1.0);
      TH1F* h04 = new TH1F("h04", "d0 (up)", 100, -10.0, 10.0);
      TH1F* h05 = new TH1F("h05", "d0 (down)", 100, -10.0, 10.0);
      TH1F* h06 = new TH1F("h06", "phi0 (up)", 100, -180.0, 180.0);
      TH1F* h07 = new TH1F("h07", "phi0 (down)", 100, -180.0, 180.0);
      TH1F* h08 = new TH1F("h08", "Omega (up)", 100, -0.015, 0.015);
      TH1F* h09 = new TH1F("h09", "Omega (down)", 100, -0.015, 0.015);
      TH1F* h0a = new TH1F("h0a", "z0 (up)", 100, -20.0, 20.0);
      TH1F* h0b = new TH1F("h0b", "z0 (down)", 100, -20.0, 20.0);
      TH1F* h0c = new TH1F("h0c", "tan lambda (up)", 100, -2.0, 2.0);
      TH1F* h0d = new TH1F("h0d", "tan lambda (down)", 100, -2.0, 2.0);
      TH1F* h0e = new TH1F("h0e", "Pt (up)", 100, 0.0, 40.0);
      TH1F* h0f = new TH1F("h0f", "Pt (down)", 100, 0.0, 40.0);

      TH1F* h11 = new TH1F("h11", "d0", 100, -10.0, 10.0);
      TH1F* h12 = new TH1F("h12", "phi0", 100, -180.0, 180.0);
      TH1F* h13 = new TH1F("h13", "Omega", 100, -0.015, 0.015);
      TH1F* h14 = new TH1F("h14", "z0", 100, -20.0, 20.0);
      TH1F* h15 = new TH1F("h15", "tan lambda", 100, -2.0, 2.0);
      TH1F* h16 = new TH1F("h16", "Pt", 100, 0.0, 40.0);
      TH1F* h17 = new TH1F("h17", "ndf", 100, 0.0, 100.0);
      TH1F* h18 = new TH1F("h18", "pval", 100, 0.0, 1.0);

      TH1F* h1 = new TH1F("h1", "d0 difference", 100, -0.4, 0.4);
      TH1F* h2 = new TH1F("h2", "phi0 difference", 100, -0.5, 0.5);
      TH1F* h3 = new TH1F("h3", "Omega difference", 100, -0.00015, 0.00015);
      TH1F* h4 = new TH1F("h4", "z0 difference", 100, -2.0, 2.0);
      TH1F* h5 = new TH1F("h5", "tan lambda difference", 100, -0.05, 0.05);
      TH1F* h6 = new TH1F("h6", "Pt difference", 100, -0.15, 0.15);

      TH1F* h21 = new TH1F("h21", "d0 pull", 100, -7.5, 7.5);
      TH1F* h22 = new TH1F("h22", "phi0 pull", 100, -7.5, 7.5);
      TH1F* h23 = new TH1F("h23", "Omega pull", 100, -7.5, 7.5);
      TH1F* h24 = new TH1F("h24", "z0 pull", 100, -7.5, 7.5);
      TH1F* h25 = new TH1F("h25", "tan lambda pull", 100, -7.5, 7.5);

      Double_t Pt_min[13] = { 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8};
      Double_t Pt_max[13] = { 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0};

      TH1F* hpt0[14];
      TH1F* hd0[14];
      TH1F* hp0[14];
      TH1F* ho0[14];
      TH1F* hz0[14];
      TH1F* ht0[14];

      for (int k = 0; k < 13; ++k) {
        hpt0[k] = new TH1F(Form("hpt0_%d", k), "Pt resolution", 100, -0.04, 0.04);
        hd0[k] = new TH1F(Form("hd0_%d", k), "d0 difference", 100, -0.4, 0.4);
        hp0[k] = new TH1F(Form("hp0_%d", k), "phi0 difference", 100, -1.5, 1.5);
        ho0[k] = new TH1F(Form("ho0_%d", k), "Omega difference", 100, -0.0003, 0.0003);
        hz0[k] = new TH1F(Form("hz0_%d", k), "z0 difference", 100, -2.0, 2.0);
        ht0[k] = new TH1F(Form("hl0_%d", k), "tan lambda difference", 100, -0.05, 0.05);
      }

      Int_t ncand = 0;
      TTree* treeTrkj = treeTrk0;

      treeTrkj->SetBranchAddress("ndf1", &ndf1);
      treeTrkj->SetBranchAddress("Pval1", &Pval1);
      treeTrkj->SetBranchAddress("Pt1", &Pt1);
      treeTrkj->SetBranchAddress("D01", &D01);
      treeTrkj->SetBranchAddress("Phi01", &Phi01);
      treeTrkj->SetBranchAddress("Om1", &Om1);
      treeTrkj->SetBranchAddress("Z01", &Z01);
      treeTrkj->SetBranchAddress("tanLambda1", &tanLambda1);
      treeTrkj->SetBranchAddress("eD01", &eD01);
      treeTrkj->SetBranchAddress("ePhi01", &ePhi01);
      treeTrkj->SetBranchAddress("eOm1", &eOm1);
      treeTrkj->SetBranchAddress("eZ01", &eZ01);
      treeTrkj->SetBranchAddress("etanL1", &etanL1);
      treeTrkj->SetBranchAddress("posSeed1", &posSeed1);
      treeTrkj->SetBranchAddress("Mom1", &Mom1);

      treeTrkj->SetBranchAddress("ndf2", &ndf2);
      treeTrkj->SetBranchAddress("Pval2", &Pval2);
      treeTrkj->SetBranchAddress("Pt2", &Pt2);
      treeTrkj->SetBranchAddress("D02", &D02);
      treeTrkj->SetBranchAddress("Phi02", &Phi02);
      treeTrkj->SetBranchAddress("Om2", &Om2);
      treeTrkj->SetBranchAddress("Z02", &Z02);
      treeTrkj->SetBranchAddress("tanLambda2", &tanLambda2);
      treeTrkj->SetBranchAddress("eD02", &eD02);
      treeTrkj->SetBranchAddress("ePhi02", &ePhi02);
      treeTrkj->SetBranchAddress("eOm2", &eOm2);
      treeTrkj->SetBranchAddress("eZ02", &eZ02);
      treeTrkj->SetBranchAddress("etanL2", &etanL2);
      treeTrkj->SetBranchAddress("posSeed2", &posSeed2);
      treeTrkj->SetBranchAddress("Mom2", &Mom2);

      std::cout << " total entries = " << treeTrkj->GetEntries() << std::endl;
      for (int i = 0; (int)i < treeTrkj->GetEntries(); ++i) {
        treeTrkj->GetEntry(i);
        //    Phi01 *= 180.0/M_PI;
        //    Phi02 *= 180.0/M_PI;
        if (abs(D01) < 5. && abs(D02) < 5.0 && abs(Z01) < 10.0 && abs(Z02) < 10.)
          ncand++;
        else
          continue;

        if (posSeed1->Y() > 0) {
          h00->Fill(ndf1);
          h01->Fill(ndf2);
          h02->Fill(Pval1);
          h03->Fill(Pval2);
          h04->Fill(D01);
          h05->Fill(D02);
          h06->Fill(Phi01);
          h07->Fill(Phi02);
          h08->Fill(Om1);
          h09->Fill(Om2);
          h0a->Fill(Z01);
          h0b->Fill(Z02);
          h0c->Fill(tanLambda1);
          h0d->Fill(tanLambda2);
          h0e->Fill(Pt1);
          h0f->Fill(Pt2);

        } else {
          h00->Fill(ndf2);
          h01->Fill(ndf1);
          h02->Fill(Pval2);
          h03->Fill(Pval1);
          h04->Fill(D02);
          h05->Fill(D01);
          h06->Fill(Phi02);
          h07->Fill(Phi01);
          h08->Fill(Om2);
          h09->Fill(Om1);
          h0a->Fill(Z02);
          h0b->Fill(Z01);
          h0c->Fill(tanLambda2);
          h0d->Fill(tanLambda1);
          h0e->Fill(Pt2);
          h0f->Fill(Pt1);

        }

        h11->Fill(D01);
        h11->Fill(D02);
        h12->Fill(Phi01);
        h12->Fill(Phi02);
        h13->Fill(Om1);
        h13->Fill(Om2);
        h14->Fill(Z01);
        h14->Fill(Z02);
        h15->Fill(tanLambda1);
        h15->Fill(tanLambda2);
        h16->Fill(Pt1);
        h16->Fill(Pt2);
        h17->Fill(ndf1);
        h17->Fill(ndf2);
        h18->Fill(Pval1);
        h18->Fill(Pval2);

        if (ndf1 < 25 || ndf2 < 25) continue;

        if (posSeed1->Y() > 0) {
          h1->Fill(fabs(D01) - fabs(D02));
          h21->Fill((fabs(D01) - fabs(D02)) / sqrt(eD01 * eD01 + eD02 * eD02));
          h2->Fill(Phi01 - Phi02);
          h22->Fill((Phi01 - Phi02) / sqrt(ePhi01 * ePhi01 + ePhi02 * ePhi02));
          h3->Fill(fabs(Om1) - fabs(Om2));
          h23->Fill((fabs(Om1) - fabs(Om2)) / sqrt(eOm1 * eOm1 + eOm2 * eOm2));
          h4->Fill(Z01 - Z02);
          h24->Fill((Z01 - Z02) / sqrt(eZ01 * eZ01 + eZ02 * eZ02));
          h5->Fill(fabs(tanLambda1) - fabs(tanLambda2));
          h25->Fill((fabs(tanLambda1) - fabs(tanLambda2)) / sqrt(etanL1 * etanL1 + etanL2 * etanL2));
          h6->Fill(Pt1 - Pt2);
        } else {
          h1->Fill(fabs(D02) - fabs(D01));
          h21->Fill((fabs(D02) - fabs(D01)) / sqrt(eD01 * eD01 + eD02 * eD02));
          h2->Fill(Phi02 - Phi01);
          h22->Fill((Phi02 - Phi01) / sqrt(ePhi01 * ePhi01 + ePhi02 * ePhi02));
          h3->Fill(fabs(Om1) - fabs(Om2));
          h23->Fill((fabs(Om1) - fabs(Om2)) / sqrt(eOm1 * eOm1 + eOm2 * eOm2));
          h4->Fill(Z02 - Z01);
          h24->Fill((Z02 - Z01) / sqrt(eZ01 * eZ01 + eZ02 * eZ02));
          h5->Fill(fabs(tanLambda2) - fabs(tanLambda1));
          h25->Fill((fabs(tanLambda2) - fabs(tanLambda1)) / sqrt(etanL1 * etanL1 + etanL2 * etanL2));
          h6->Fill(Pt2 - Pt1);
        }

        if (posSeed1->Y() > 0) {
          for (int k = 0; (int)k < 13; ++k) {
            if ((Pt1 + Pt2) / 2 > Pt_min[k] && (Pt1 + Pt2) / 2 < Pt_max[k]) {
              hpt0[k]->Fill(sqrt(2) * (Pt1 - Pt2) / (Pt1 + Pt2));
              hd0[k]->Fill(fabs(D01) - fabs(D02));
              hp0[k]->Fill((Phi01 - Phi02));
              ho0[k]->Fill((fabs(Om1) - fabs(Om2)));
              hz0[k]->Fill((Z01 - Z02));
              ht0[k]->Fill((fabs(tanLambda1) - fabs(tanLambda2)));
            }
          }
        } else {
          for (int k = 0; (int)k < 13; ++k) {
            if ((Pt1 + Pt2) / 2 > Pt_min[k] && (Pt1 + Pt2) / 2 < Pt_max[k]) {
              hpt0[k]->Fill(sqrt(2) * (Pt2 - Pt1) / (Pt1 + Pt2));
              hd0[k]->Fill((fabs(D02) - fabs(D01)));
              hp0[k]->Fill((Phi02 - Phi01));
              ho0[k]->Fill((fabs(Om1) - fabs(Om2)));
              hz0[k]->Fill((Z02 - Z01));
              ht0[k]->Fill((fabs(tanLambda2) - fabs(tanLambda1)));
            }
          }
        }
      }

      Double_t x_Pt[13] {0.5, 0.7, 0.9, 1.1, 1.3, 1.5, 1.7, 1.9, 2.1, 2.3, 2.5, 2.7, 2.9};
      Double_t ex_Pt[13] {0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10};
      Double_t y0_sigma[13] {9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0};
      Double_t ey0_sigma[13] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      Double_t dy0_sigma[13] {9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0};
      Double_t edy0_sigma[13] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      Double_t py0_sigma[13] {9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0};
      Double_t epy0_sigma[13] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      Double_t oy0_sigma[13] {9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0};
      Double_t eoy0_sigma[13] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      Double_t zy0_sigma[13] {9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0, 9999.0};
      Double_t ezy0_sigma[13] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


      const int minNEntries = 100;

      for (int k = 0; (int)k < 13; ++k) {

        std::cout << "********************************** pt, " << k << std::endl;
        if (hpt0[k]->GetEntries() > minNEntries) {
          hpt0[k]->Fit("gaus");
          y0_sigma[k] = (hpt0[k]->GetFunction("gaus")->GetParameter(2));
          ey0_sigma[k] = (hpt0[k]->GetFunction("gaus")->GetParError(2));
        }

        std::cout << "********************************** d0, " << k << std::endl;
        if (hd0[k]->GetEntries() > minNEntries) {
          hd0[k]->Fit("gaus");
          dy0_sigma[k] = (hd0[k]->GetFunction("gaus")->GetParameter(2));
          edy0_sigma[k] = (hd0[k]->GetFunction("gaus")->GetParError(2));
        }

        std::cout << "********************************** phi0, " << k << std::endl;
        if (hp0[k]->GetEntries() > minNEntries) {
          hp0[k]->Fit("gaus");
          py0_sigma[k] = (hp0[k]->GetFunction("gaus")->GetParameter(2));
          epy0_sigma[k] = (hp0[k]->GetFunction("gaus")->GetParError(2));
        }

        std::cout << "********************************** omega, " << k << std::endl;
        if (ho0[k]->GetEntries() > minNEntries) {
          ho0[k]->Fit("gaus");
          oy0_sigma[k] = (ho0[k]->GetFunction("gaus")->GetParameter(2));
          eoy0_sigma[k] = (ho0[k]->GetFunction("gaus")->GetParError(2));
        }

        std::cout << "********************************** z0, " << k << std::endl;
        if (hz0[k]->GetEntries() > minNEntries) {
          hz0[k]->Fit("gaus");
          zy0_sigma[k] = (hz0[k]->GetFunction("gaus")->GetParameter(2));
          ezy0_sigma[k] = (hz0[k]->GetFunction("gaus")->GetParError(2));
        }

        std::cout << " end the loop " << k << std::endl;
      }

      gStyle->SetStatW(0.22);
      gStyle->SetStatH(0.22);

      TGraphErrors* gr = new TGraphErrors(13, x_Pt, y0_sigma, ex_Pt, ey0_sigma);
      gr->SetTitle();
      gr->SetMarkerColor(kRed);
      gr->SetMarkerStyle(24);
      gr->Draw("AP");
      gr->GetXaxis()->SetRangeUser(0.0, 3.1);
      gr->GetYaxis()->SetRangeUser(0.000, 0.013);
      gr->GetXaxis()->SetTitleOffset(1.25);
      gr->GetXaxis()->SetTitle("P_{t} (GeV/c)");
      gr->GetXaxis()->CenterTitle();
      gr->GetYaxis()->SetTitleOffset(1.5);
      gr->GetYaxis()->SetTitle("#sigma (Pt) / Pt");
      gr->GetYaxis()->CenterTitle();
      gr->Draw("AP");

      TGraphErrors* dgr = new TGraphErrors(13, x_Pt, dy0_sigma, ex_Pt, edy0_sigma);
      dgr->SetTitle();
      dgr->SetMarkerColor(kRed);
      dgr->SetMarkerStyle(24);
      dgr->Draw("AP");
      dgr->GetXaxis()->SetRangeUser(0.0, 3.1);
      dgr->GetYaxis()->SetRangeUser(0.000, 0.10);
      dgr->GetXaxis()->SetTitleOffset(1.25);
      dgr->GetXaxis()->SetTitle("P_{t} (GeV/c)");
      dgr->GetXaxis()->CenterTitle();
      dgr->GetYaxis()->SetTitleOffset(1.5);
      dgr->GetYaxis()->SetTitle("d_{0}");
      dgr->GetYaxis()->CenterTitle();
      dgr->Draw("AP");

      TGraphErrors* pgr = new TGraphErrors(13, x_Pt, py0_sigma, ex_Pt, epy0_sigma);
      pgr->SetTitle();
      pgr->SetMarkerColor(kRed);
      pgr->SetMarkerStyle(24);
      pgr->Draw("AP");
      pgr->GetXaxis()->SetRangeUser(0.0, 3.1);
      pgr->GetYaxis()->SetRangeUser(0.000, 0.50);
      pgr->GetXaxis()->SetTitleOffset(1.25);
      pgr->GetXaxis()->SetTitle("P_{t} (GeV/c)");
      pgr->GetXaxis()->CenterTitle();
      pgr->GetYaxis()->SetTitleOffset(1.5);
      pgr->GetYaxis()->SetTitle("phi_{0}");
      pgr->GetYaxis()->CenterTitle();
      pgr->Draw("AP");

      TGraphErrors* ogr = new TGraphErrors(13, x_Pt, oy0_sigma, ex_Pt, eoy0_sigma);
      ogr->SetTitle();
      ogr->SetMarkerColor(kRed);
      ogr->SetMarkerStyle(24);
      ogr->Draw("AP");
      ogr->GetXaxis()->SetRangeUser(0.0, 3.1);
      ogr->GetYaxis()->SetRangeUser(0.000, 0.000150);
      ogr->GetXaxis()->SetTitleOffset(1.25);
      ogr->GetXaxis()->SetTitle("P_{t} (GeV/c)");
      ogr->GetXaxis()->CenterTitle();
      ogr->GetYaxis()->SetTitleOffset(1.5);
      ogr->GetYaxis()->SetTitle("omega");
      ogr->GetYaxis()->CenterTitle();
      ogr->Draw("AP");

      TGraphErrors* zgr = new TGraphErrors(13, x_Pt, zy0_sigma, ex_Pt, ezy0_sigma);
      zgr->SetTitle();
      zgr->SetMarkerColor(kRed);
      zgr->SetMarkerStyle(24);
      zgr->Draw("AP");
      zgr->GetXaxis()->SetRangeUser(0.0, 3.1);
      zgr->GetYaxis()->SetRangeUser(0.000, 0.750);
      zgr->GetXaxis()->SetTitleOffset(1.25);
      zgr->GetXaxis()->SetTitle("P_{t} (GeV/c)");
      zgr->GetXaxis()->CenterTitle();
      zgr->GetYaxis()->SetTitleOffset(1.5);
      zgr->GetYaxis()->SetTitle("z_{0}");
      zgr->GetYaxis()->CenterTitle();
      zgr->Draw("AP");

      h00->Write();
      h01->Write();
      h02->Write();
      h03->Write();
      h04->Write();
      h05->Write();
      h06->Write();
      h07->Write();
      h08->Write();
      h09->Write();
      h0a->Write();
      h0b->Write();
      h0c->Write();
      h0d->Write();
      h0e->Write();
      h0f->Write();

      h11->Write();
      h12->Write();
      h13->Write();
      h14->Write();
      h15->Write();
      h16->Write();
      h17->Write();
      h18->Write();

      h1->Write();
      h2->Write();
      h3->Write();
      h4->Write();
      h5->Write();
      h6->Write();

      h21->Write();
      h22->Write();
      h23->Write();
      h24->Write();
      h25->Write();

      for (int k = 0; k < 13; ++k) {
        hpt0[k]->Write();
        hd0[k]->Write();
        hp0[k]->Write();
        ho0[k]->Write();
        hz0[k]->Write();
        ht0[k]->Write();
      }
      gr->Write();
      dgr->Write();
      pgr->Write();
      ogr->Write();
      zgr->Write();

    }
  }
}
