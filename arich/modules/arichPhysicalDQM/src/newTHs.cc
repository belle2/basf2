/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tomouyki Konno, Kindo Haruki
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichTreeMaker/newTHs.h>

#include <framework/logging/Logger.h>

#include "TROOT.h"
#include "TMath.h"
#include "TF1.h"
#include "TF2.h"
#include "TF3.h"
#include "TCanvas.h"

#include <cmath>

using namespace Belle2;

namespace Belle2 {

  TH1* newTH1(const char* name, const char* title,
              Int_t nbins, Double_t min, Double_t max,
              Int_t lcolor, Int_t /*fcolor*/)
  {
    TH1* h = new TH1D(name, title, nbins, min, max);
    h->GetYaxis()->SetTitleOffset(1);
    h->GetXaxis()->SetTitleOffset(1);
    h->GetYaxis()->SetTitleSize(0.03);
    h->GetYaxis()->SetLabelSize(0.03);
    h->GetXaxis()->SetTitleSize(0.03);
    h->GetXaxis()->SetLabelSize(0.03);
    h->SetLineWidth(2);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.6);
    h->Sumw2();
    h->SetLineColor(lcolor);
    h->SetMarkerColor(lcolor);
    //h->SetFillColor(fcolor);
    return h;
  }

  TH2* newTH2(const char* name, const char* title,
              Int_t xnbins, Double_t xmin, Double_t xmax,
              Int_t ynbins, Double_t ymin, Double_t ymax,
              Int_t lcolor, Int_t /*fcolor*/)
  {
    TH2* h = new TH2D(name, title, xnbins, xmin, xmax, ynbins, ymin, ymax);
    h->GetYaxis()->SetTitleOffset(1);
    h->GetXaxis()->SetTitleOffset(1);
    h->GetYaxis()->SetTitleSize(0.03);
    h->GetYaxis()->SetLabelSize(0.03);
    h->GetXaxis()->SetTitleSize(0.03);
    h->GetXaxis()->SetLabelSize(0.03);
    h->SetLineWidth(2);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.6);
    h->Sumw2();
    h->SetLineColor(lcolor);
    h->SetMarkerColor(lcolor);
    //h->SetFillColor(fcolor);
    return h;
  }

  TH3* newTH3(const char* name, const char* title,
              Int_t xnbins, Double_t xmin, Double_t xmax,
              Int_t ynbins, Double_t ymin, Double_t ymax,
              Int_t znbins, Double_t zmin, Double_t zmax,
              Int_t lcolor, Int_t /*fcolor*/)
  {
    TH3* h = new TH3D(name, title, xnbins, xmin, xmax, ynbins, ymin, ymax, znbins, zmin, zmax);
    h->GetZaxis()->SetTitleOffset(1);
    h->GetYaxis()->SetTitleOffset(1);
    h->GetXaxis()->SetTitleOffset(1);
    h->GetZaxis()->SetTitleSize(0.03);
    h->GetZaxis()->SetLabelSize(0.03);
    h->GetYaxis()->SetTitleSize(0.03);
    h->GetYaxis()->SetLabelSize(0.03);
    h->GetXaxis()->SetTitleSize(0.03);
    h->GetXaxis()->SetLabelSize(0.03);
    h->SetLineWidth(2);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(0.6);
    h->Sumw2();
    h->SetLineColor(lcolor);
    h->SetMarkerColor(lcolor);
    //h->SetFillColor(fcolor);
    return h;
  }

}
