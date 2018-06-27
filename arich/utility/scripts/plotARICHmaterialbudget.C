/**
 * This script plots the map of ARICH material budget  
 *
 * run as: "root -l ../../examples/ARICHMaterialScan.root plotARICHmaterialbudget.C"
 *
 * Author: Leonid Burmistrov, 27.05.2018
 **/

Int_t plotARICHmaterialbudget(){

  _file0->cd("Planar");
  TH2D *h2_1 = (TH2D*)_file0->Get("Planar/All_Materials_x0");
  TH2D *h2_2 = (TH2D*)_file0->Get("Planar/All_Materials_lambda");
  //TH2D *h2_1 = (TH2D*)_file0->Get("Planar/ARICH_cablesEffectiveMaterial_x0");
  //TH2D *h2_2 = (TH2D*)_file0->Get("Planar/ARICH_cablesEffectiveMaterial_lambda");

  h2_1->SetTitle("(Radiation length)/x_{0}");
  h2_2->SetTitle("(Nuclear interaction length)/#lambda_{nuc.}");

  gStyle->SetPalette(1);
  gStyle->SetFrameBorderMode(0);
  gROOT->ForceStyle();
  gStyle->SetStatColor(kWhite);
  gStyle->SetOptStat(kFALSE);

  TCanvas *c1 = new TCanvas("c1",_file0->GetName(),10,10,1000,1000);
  c1->SetRightMargin(0.1);
  c1->SetLeftMargin(0.09);
  c1->SetTopMargin(0.12);
  c1->SetBottomMargin(0.09);
  h2_1->Draw("ZCOLOR");
  //h2_1->Draw("ZCOLOR TEXT");
  h2_1->GetXaxis()->SetTitle("x, cm");
  h2_1->GetYaxis()->SetTitle("y, cm");
  h2_1->GetZaxis()->SetRangeUser(0.0,1.0);
  //h2_1->GetZaxis()->SetRangeUser(0.0,3.0);
  h2_1->GetXaxis()->CenterTitle();
  h2_1->GetYaxis()->CenterTitle();

  TCanvas *c2 = new TCanvas("c2",_file0->GetName(),20,20,1000,1000);
  c2->SetRightMargin(0.1);
  c2->SetLeftMargin(0.09);
  c2->SetTopMargin(0.12);
  c2->SetBottomMargin(0.09);
  h2_2->Draw("ZCOLOR");
  //h2_2->Draw("ZCOLOR TEXT");
  h2_2->GetXaxis()->SetTitle("x, cm");
  h2_2->GetYaxis()->SetTitle("y, cm");
  h2_2->GetZaxis()->SetRangeUser(0.0,1.0);
  //h2_2->GetZaxis()->SetRangeUser(0.0,3.0);
  h2_2->GetXaxis()->CenterTitle();
  h2_2->GetYaxis()->CenterTitle();

  return 0;

}
