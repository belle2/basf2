/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <root/TTree.h>
#include <root/TAxis.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

PerformanceEvaluationBaseClass:: PerformanceEvaluationBaseClass()
{
}

PerformanceEvaluationBaseClass::~ PerformanceEvaluationBaseClass()
{

}

TH1F*  PerformanceEvaluationBaseClass::createHistogram1D(const char* name, const char* title,
                                                         Int_t nbins, Double_t min, Double_t max,
                                                         const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}


TH2F*  PerformanceEvaluationBaseClass::createHistogram2D(const char* name, const char* title,
                                                         Int_t nbinsX, Double_t minX, Double_t maxX,
                                                         const char* titleX,
                                                         Int_t nbinsY, Double_t minY, Double_t maxY,
                                                         const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F*  PerformanceEvaluationBaseClass::createHistogram3D(const char* name, const char* title,
                                                         Int_t nbinsX, Double_t minX, Double_t maxX,
                                                         const char* titleX,
                                                         Int_t nbinsY, Double_t minY, Double_t maxY,
                                                         const char* titleY,
                                                         Int_t nbinsZ, Double_t minZ, Double_t maxZ,
                                                         const char* titleZ,
                                                         TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY, nbinsZ, minZ, maxZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F*  PerformanceEvaluationBaseClass::createHistogram3D(const char* name, const char* title,
                                                         Int_t nbinsX, Double_t* binsX,
                                                         const char* titleX,
                                                         Int_t nbinsY, Double_t* binsY,
                                                         const char* titleY,
                                                         Int_t nbinsZ, Double_t* binsZ,
                                                         const char* titleZ,
                                                         TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, binsX, nbinsY, binsY, nbinsZ, binsZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH1*  PerformanceEvaluationBaseClass::duplicateHistogram(const char* newname, const char* newtitle,
                                                         TH1* h, TList* histoList)
{

  TH1F* h1 =  dynamic_cast<TH1F*>(h);
  TH2F* h2 =  dynamic_cast<TH2F*>(h);
  TH3F* h3 =  dynamic_cast<TH3F*>(h);

  TH1* newh = 0;

  if (h1)
    newh = new TH1F(*h1);
  if (h2)
    newh = new TH2F(*h2);
  if (h3)
    newh = new TH3F(*h3);

  newh->SetName(newname);
  newh->SetTitle(newtitle);

  if (histoList)
    histoList->Add(newh);


  return newh;
}

TH1F*  PerformanceEvaluationBaseClass::createHistogramsRatio(const char* name, const char* title,
    TH1* hNum, TH1* hDen, bool isEffPlot,
    int axisRef)
{

  TH1F* h1den =  dynamic_cast<TH1F*>(hDen);
  TH1F* h1num =  dynamic_cast<TH1F*>(hNum);
  TH2F* h2den =  dynamic_cast<TH2F*>(hDen);
  TH2F* h2num =  dynamic_cast<TH2F*>(hNum);
  TH3F* h3den =  dynamic_cast<TH3F*>(hDen);
  TH3F* h3num =  dynamic_cast<TH3F*>(hNum);

  TH1* hden = 0;
  TH1* hnum = 0;

  if (h1den) {
    hden = new TH1F(*h1den);
    hnum = new TH1F(*h1num);
  }
  if (h2den) {
    hden = new TH2F(*h2den);
    hnum = new TH2F(*h2num);
  }
  if (h3den) {
    hden = new TH3F(*h3den);
    hnum = new TH3F(*h3num);
  }

  TAxis* the_axis;
  TAxis* the_other1;
  TAxis* the_other2;

  if (axisRef == 0) {
    the_axis = hden->GetXaxis();
    the_other1 = hden->GetYaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 1) {
    the_axis = hden->GetYaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 2) {
    the_axis = hden->GetZaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetYaxis();
  } else
    return NULL;


  TH1F* h;
  if (the_axis->GetXbins()->GetSize())
    h = new TH1F(name, title, the_axis->GetNbins(), (the_axis->GetXbins())->GetArray());
  else
    h = new TH1F(name, title, the_axis->GetNbins(), the_axis->GetXmin(), the_axis->GetXmax());
  h->GetXaxis()->SetTitle(the_axis->GetTitle());

  h->GetYaxis()->SetRangeUser(0.00001, 1);

  double num = 0;
  double den = 0;
  Int_t bin = 0;
  Int_t nBins = 0;


  for (int the_bin = 1; the_bin < the_axis->GetNbins() + 1; the_bin++) {

    num = 0;
    den = 0 ;

    for (int other1_bin = 1; other1_bin < the_other1->GetNbins() + 1; other1_bin++)
      for (int other2_bin = 1; other2_bin < the_other2->GetNbins() + 1; other2_bin++) {

        if (axisRef == 0) bin = hden->GetBin(the_bin, other1_bin, other2_bin);
        else if (axisRef == 1) bin = hden->GetBin(other1_bin, the_bin, other2_bin);
        else if (axisRef == 2) bin = hden->GetBin(other1_bin, other2_bin, the_bin);

        if (hden->IsBinUnderflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), UNDERFLOW");
        if (hden->IsBinOverflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), OVERFLOW");

        num += hnum->GetBinContent(bin);
        den += hden->GetBinContent(bin);

        nBins++;

      }

    double eff = 0;
    double err = 0;

    if (den > 0) {
      eff = (double)num / den;
      err = sqrt(eff * (1 - eff)) / sqrt(den);
    }

    if (isEffPlot) {
      h->SetBinContent(the_bin, eff);
      h->SetBinError(the_bin, err);
    } else {
      h->SetBinContent(the_bin, 1 - eff);
      h->SetBinError(the_bin, err);
    }

  }

  return h;

}


void  PerformanceEvaluationBaseClass::addInefficiencyPlots(TList* histoList, TH3F* h3_xPerMCParticle, TH3F* h3_MCParticle)
{

  if ((h3_xPerMCParticle == NULL) || (h3_MCParticle == NULL))
    return;

  //normalized to MCParticles
  TH1F* h_ineff_pt = createHistogramsRatio("hineffpt", "inefficiency VS pt, normalized to MCParticles", h3_xPerMCParticle,
                                           h3_MCParticle, false, 0);
  histoList->Add(h_ineff_pt);

  TH1F* h_ineff_theta = createHistogramsRatio("hinefftheta", "inefficiency VS #theta, normalized to MCParticles",
                                              h3_xPerMCParticle, h3_MCParticle, false, 1);
  histoList->Add(h_ineff_theta);

  TH1F* h_ineff_phi = createHistogramsRatio("hineffphi", "inefficiency VS #phi, normalized to MCParticles", h3_xPerMCParticle,
                                            h3_MCParticle, false, 2);
  histoList->Add(h_ineff_phi);

}

void  PerformanceEvaluationBaseClass::addEfficiencyPlots(TList* histoList, TH3F* h3_xPerMCParticle, TH3F* h3_MCParticle)
{
  if ((h3_xPerMCParticle == NULL) || (h3_MCParticle == NULL))
    return;

  //normalized to MCParticles
  TH1F* h_eff_pt = createHistogramsRatio("heffpt", "efficiency VS pt, normalized to MCParticles", h3_xPerMCParticle,
                                         h3_MCParticle, true, 0);
  histoList->Add(h_eff_pt);

  TH1F* h_eff_theta = createHistogramsRatio("hefftheta", "efficiency VS #theta, normalized to MCParticles", h3_xPerMCParticle,
                                            h3_MCParticle, true, 1);
  histoList->Add(h_eff_theta);

  TH1F* h_eff_phi = createHistogramsRatio("heffphi", "efficiency VS #phi, normalized to MCParticles", h3_xPerMCParticle,
                                          h3_MCParticle, true, 2);
  histoList->Add(h_eff_phi);

}



void  PerformanceEvaluationBaseClass::addPurityPlots(TList* histoList, TH3F* h3_MCParticlesPerX, TH3F* h3_X)
{
  if ((h3_X == NULL) || (h3_MCParticlesPerX == NULL))
    return;

//purity histograms
  TH1F* h_pur_pt = createHistogramsRatio("hpurpt", "purity VS pt", h3_MCParticlesPerX, h3_X, true, 0);
  histoList->Add(h_pur_pt);

  TH1F* h_pur_theta = createHistogramsRatio("hpurtheta", "purity VS #theta", h3_MCParticlesPerX, h3_X, true, 1);
  histoList->Add(h_pur_theta);

  TH1F* h_pur_phi = createHistogramsRatio("hpurphi", "purity VS #phi", h3_MCParticlesPerX, h3_X, true, 2);
  histoList->Add(h_pur_phi);

}


