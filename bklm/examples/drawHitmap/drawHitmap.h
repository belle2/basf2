/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __DRAWHITMAP_H
#define __DRAWHITMAP_H

TH1F* createTH1(const char* name, const char* title, Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                const char* titleY, bool drawStat)
{
  TH1F* hist = new TH1F(name, title, nbinsX, minX, maxX);
  hist->GetXaxis()->SetTitle(titleX);
  hist->GetYaxis()->SetTitle(titleY);
  TGaxis::SetMaxDigits(3);
  hist->SetLineWidth(1);
  hist->SetLineColor(kRed);
  if (!drawStat)
    hist->SetStats(0);
  return hist;
}

TH2F* createTH2(const char* name, const char* title, Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX, Int_t nbinsY,
                Double_t minY, Double_t maxY, const char* titleY, bool drawStat)
{
  TH2F* hist = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);
  hist->GetXaxis()->SetTitle(titleX);
  hist->GetYaxis()->SetTitle(titleY);
  TGaxis::SetMaxDigits(3);
  if (!drawStat)
    hist->SetStats(0);
  return hist;
}

#endif
