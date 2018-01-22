/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Kindo Haruki                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BELLE2_NEWTHS_h
#define BELLE2_NEWTHS_h

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TVector2.h>
#include <TText.h>
#include <TFile.h>

#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  TH1* newTH1(const char* name, const char* title,
              Int_t nbins, Double_t min, Double_t max,
              Int_t lcolor, Int_t /*fcolor*/);

  TH2* newTH2(const char* name, const char* title,
              Int_t xnbins, Double_t xmin, Double_t xmax,
              Int_t ynbins, Double_t ymin, Double_t ymax,
              Int_t lcolor, Int_t /*fcolor*/);

  TH3* newTH3(const char* name, const char* title,
              Int_t xnbins, Double_t xmin, Double_t xmax,
              Int_t ynbins, Double_t ymin, Double_t ymax,
              Int_t znbins, Double_t zmin, Double_t zmax,
              Int_t lcolor, Int_t /*fcolor*/);
}

#endif
