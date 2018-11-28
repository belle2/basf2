/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <TObject.h>
#include <TMath.h>
#include <TH2F.h>
#include <TAxis.h>

#include <framework/gearbox/Unit.h>
#include <analysis/dbobjects/PIDPriorsTable.h>
#include <analysis/dbobjects/PIDPriors.h>


using namespace Belle2;


void PIDPriors::setPriors(int PDGCode, TH2F* priorHistogram)
{
  auto index = parsePDGForPriors(PDGCode);
  if (index < 0) {
    B2ERROR("The PDG code " << PDGCode << " does not belong to a stable particle. Priors not set");
    return ;
  }

  double xStart = priorHistogram->GetXaxis()->GetBinLowEdge(priorHistogram->GetXaxis()->GetFirst());
  double xStop = priorHistogram->GetXaxis()->GetBinUpEdge(priorHistogram->GetXaxis()->GetLast());
  double xWidth = priorHistogram->GetXaxis()->GetBinWidth(1);

  double yStart = priorHistogram->GetYaxis()->GetBinLowEdge(priorHistogram->GetYaxis()->GetFirst());
  double yStop = priorHistogram->GetYaxis()->GetBinUpEdge(priorHistogram->GetYaxis()->GetLast());
  double yWidth = priorHistogram->GetYaxis()->GetBinWidth(1);

  std::vector<float> xEdges;
  std::vector<float> yEdges;

  while (xStart < xStop + xWidth * 0.5) {
    xEdges.push_back(xStart);
    xStart += xWidth;
  }

  while (yStart < yStop + yWidth * 0.5) {
    yEdges.push_back(yStart);
    yStart += yWidth;
  }

  std::vector<std::vector<float>>  prior(yEdges.size() - 1, std::vector<float>(xEdges.size() - 1));
  std::vector<std::vector<float>>  error(yEdges.size() - 1, std::vector<float>(xEdges.size() - 1));

  for (int ix = 0; ix < (int)xEdges.size() - 1; ix++) {
    for (int iy = 0; iy < (int)yEdges.size() - 1; iy++) {
      prior[iy][ix] = priorHistogram->GetBinContent(ix + 1, iy + 1);
      error[iy][ix] = priorHistogram->GetBinError(ix + 1, iy + 1);
    }
  }

  m_priors[index].setBinEdges(xEdges, yEdges);
  m_priors[index].setPriorsTable(prior);
  m_priors[index].setErrorsTable(error);

  return;
};



void PIDPriors::setPriors(int PDGCode, TH2F* counts, TH2F* normalization)
{
  TH2F* priorHistogram = (TH2F*)counts->Clone();

  priorHistogram->Sumw2();
  priorHistogram->Divide(normalization);

  setPriors(PDGCode, priorHistogram);
};



