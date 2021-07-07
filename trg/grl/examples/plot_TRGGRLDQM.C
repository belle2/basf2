/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include<TH1F.h>
#include<TH1I.h>
#include<TCanvas.h>
#include<TLegend.h>

void plot_TRGGRLDQM() {

  gStyle->SetPalette(1);

  TH1I* h_N_track = (TH1I*)_file0->Get("TRGGRL/h_N_track");
  TH1F* h_phi_i = (TH1F*)_file0->Get("TRGGRL/h_phi_i");
  TH1F* h_phi_CDC = (TH1F*)_file0->Get("TRGGRL/h_phi_CDC");
  TH1F* h_sector_CDC = (TH1F*)_file0->Get("TRGGRL/h_sector_CDC");
  TH1F* h_sector_KLM = (TH1F*)_file0->Get("TRGGRL/h_sector_KLM");
  TH1F* h_slot_CDC = (TH1F*)_file0->Get("TRGGRL/h_slot_CDC");
  TH1F* h_slot_TOP = (TH1F*)_file0->Get("TRGGRL/h_slot_TOP");

  TH1F* h_phi_ECL = (TH1F*)_file0->Get("TRGGRL/h_phi_ECL");
  TH1F* h_theta_ECL = (TH1F*)_file0->Get("TRGGRL/h_theta_ECL");
  TH1F* h_E_ECL = (TH1F*)_file0->Get("TRGGRL/h_E_ECL");

  TH1F* h_CDCL1 = (TH1F*)_file0->Get("TRGGRL/h_CDCL1");
  TH1F* h_CDC3DL1 = (TH1F*)_file0->Get("TRGGRL/h_CDC3DL1");
  TH1F* h_CDCNNL1 = (TH1F*)_file0->Get("TRGGRL/h_CDCNNL1");
  TH1F* h_TOPL1 = (TH1F*)_file0->Get("TRGGRL/h_TOPL1");
  TH1F* h_KLML1 = (TH1F*)_file0->Get("TRGGRL/h_KLML1");
  TH1F* h_ECLL1 = (TH1F*)_file0->Get("TRGGRL/h_ECLL1");
  TH1F* h_ECLL1_2nd = (TH1F*)_file0->Get("TRGGRL/h_ECLL1_2nd");
  TH1F* h_TSFL1 = (TH1F*)_file0->Get("TRGGRL/h_TSFL1");
  TH1F* h_B2LL1 = (TH1F*)_file0->Get("TRGGRL/h_B2LL1");

  TCanvas* zone = new TCanvas("zone","zone", 640,640);

  h_N_track->SetLineWidth(3);
  h_N_track->SetLineColor(kBlack);
  h_N_track->Draw();
  zone->SaveAs("N_track.eps");

  h_phi_i->SetLineWidth(3);
  h_phi_i->SetLineColor(kBlack);
  h_phi_i->Draw();
  zone->SaveAs("phi_i.eps");

  h_phi_CDC->SetLineWidth(3);
  h_phi_CDC->SetLineColor(kBlack);
  h_phi_CDC->Draw();
  zone->SaveAs("phi_CDC.eps");

  h_sector_CDC->SetLineWidth(3);
  h_sector_CDC->SetLineColor(kBlack);
  h_sector_CDC->Draw();
  zone->SaveAs("sector_CDC.eps");

  h_sector_KLM->SetLineWidth(3);
  h_sector_KLM->SetLineColor(kBlack);
  h_sector_KLM->Draw();
  zone->SaveAs("sector_KLM.eps");

  h_slot_CDC->SetLineWidth(3);
  h_slot_CDC->SetLineColor(kBlack);
  h_slot_CDC->Draw();
  zone->SaveAs("slot_CDC.eps");

  h_slot_TOP->SetLineWidth(3);
  h_slot_TOP->SetLineColor(kBlack);
  h_slot_TOP->Draw();
  zone->SaveAs("slot_TOP.eps");

  h_phi_ECL->SetLineWidth(3);
  h_phi_ECL->SetLineColor(kBlack);
  h_phi_ECL->Draw();
  zone->SaveAs("phi_ECL.eps");

  h_theta_ECL->SetLineWidth(3);
  h_theta_ECL->SetLineColor(kBlack);
  h_theta_ECL->Draw();
  zone->SaveAs("theta_ECL.eps");

  h_E_ECL->SetLineWidth(3);
  h_E_ECL->SetLineColor(kBlack);
  h_E_ECL->Draw();
  zone->SaveAs("E_ECL.eps");

//-------------------------------------
    TH1 *frame = zone->DrawFrame(-2500,0,0,0.12);
    int b; double y;
    b = h_CDCL1->GetMaximumBin(); y = h_CDCL1->GetBinContent(b);
    h_CDCL1->Scale(0.1/y);
    b = h_CDC3DL1->GetMaximumBin(); y = h_CDC3DL1->GetBinContent(b);
    h_CDC3DL1->Scale(0.1/y);
    b = h_CDCNNL1->GetMaximumBin(); y = h_CDCNNL1->GetBinContent(b);
    h_CDCNNL1->Scale(0.1/y);
    b = h_TOPL1->GetMaximumBin(); y = h_TOPL1->GetBinContent(b);
    h_TOPL1->Scale(0.1/y);
    b = h_ECLL1->GetMaximumBin(); y = h_ECLL1->GetBinContent(b);
    h_ECLL1->Scale(0.1/y);
    b = h_KLML1->GetMaximumBin(); y = h_KLML1->GetBinContent(b);
    h_KLML1->Scale(0.1/y);
    b = h_ECLL1_2nd->GetMaximumBin(); y = h_ECLL1_2nd->GetBinContent(b);
    h_ECLL1_2nd->Scale(0.1/y);  
    b = h_TSFL1->GetMaximumBin(); y = h_TSFL1->GetBinContent(b);
    h_TSFL1->Scale(0.1/y);
    b = h_B2LL1->GetMaximumBin(); y = h_B2LL1->GetBinContent(b);
    h_B2LL1->Scale(0.1/y);

    h_KLML1->SetLineWidth(3);
    h_KLML1->SetLineColor(kMagenta);
    h_KLML1->Draw("HISTsame");
    h_CDCL1->SetLineWidth(3);
    h_CDCL1->SetLineColor(kBlack);
    h_CDCL1->Draw("HISTsame");
    h_TSFL1->SetLineWidth(3);
    h_TSFL1->SetLineColor(kBlack);
    h_TSFL1->SetLineStyle(kDashed);
    h_TSFL1->Draw("HISTsame");
    h_CDC3DL1->SetLineWidth(3);
    h_CDC3DL1->SetLineColor(kRed);
    h_CDC3DL1->Draw("HISTsame");
    h_CDCNNL1->SetLineWidth(3);
    h_CDCNNL1->SetLineColor(kRed);
    h_CDCNNL1->SetLineStyle(kDashed);
    h_CDCNNL1->Draw("HISTsame");
    h_ECLL1->SetLineWidth(3);
    h_ECLL1->SetLineColor(kBlue);
    h_ECLL1->Draw("HISTsame");
//    h_ECLL1_2nd->SetLineWidth(3);
//    h_ECLL1_2nd->SetLineColor(kCyan+1);
//    h_ECLL1_2nd->Draw("HISTsame");
    h_TOPL1->SetLineWidth(3);
    h_TOPL1->SetLineColor(kGreen+3);
    h_TOPL1->Draw("HISTsame");

    h_B2LL1->SetLineWidth(3);
    h_B2LL1->SetFillColor(kBlack);
    h_B2LL1->SetLineColor(kBlack);
    h_B2LL1->Draw("HISTsame");

    frame->GetXaxis()->SetTitle("Sub-trigger -> L1 jitter in GRL [ns]");

   leg = new TLegend(0.1,0.6,0.3,0.9);
   leg->AddEntry(h_CDCL1,"CDC 2D","l");
   leg->AddEntry(h_TSFL1,"CDC ST","l");
   leg->AddEntry(h_CDC3DL1,"CDC 3D","l");
   leg->AddEntry(h_CDCNNL1,"CDC NN","l");
   leg->AddEntry(h_ECLL1,"ECL","l");
//   leg->AddEntry(h_ECLL1_2nd,"ECL 2nd","l");
   leg->AddEntry(h_KLML1,"KLM","l");
   leg->AddEntry(h_TOPL1,"TOP","l");
   leg->SetTextSize(0.035);
   leg->Draw();

   leg1 = new TLegend(0.5,0.85,0.9,0.9);
   leg1->AddEntry(h_B2LL1,"B2L window","F");
   leg1->SetTextSize(0.03);
   leg1->Draw();

   zone->SaveAs("./plot/GRL_latency.eps");

}
