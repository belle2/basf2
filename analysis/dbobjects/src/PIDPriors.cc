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

void PIDPriors::setPriors(const Const::ChargedStable& particle, TH2F* priorHistogram)
{
  auto index = particle.getIndex();

  std::vector<float> xEdges;
  std::vector<float> yEdges;

  auto& xaxis = *(priorHistogram->GetXaxis());
  for (int i = xaxis.GetFirst(); i <= xaxis.GetLast(); ++i) {
    xEdges.push_back(xaxis.GetBinLowEdge(i));
  }
  xEdges.push_back(xaxis.GetBinUpEdge(xaxis.GetLast()));

  auto& yaxis = *(priorHistogram->GetYaxis());
  for (int i = yaxis.GetFirst(); i <= yaxis.GetLast(); ++i) {
    yEdges.push_back(yaxis.GetBinLowEdge(i));
  }
  yEdges.push_back(yaxis.GetBinUpEdge(yaxis.GetLast()));


  std::vector<float>  prior;
  std::vector<float>  error;

  for (int ix = 0; ix < (int)xEdges.size() - 1; ix++) {
    for (int iy = 0; iy < (int)yEdges.size() - 1; iy++) {
      prior.push_back(priorHistogram->GetBinContent(ix + 1, iy + 1));
      error.push_back(priorHistogram->GetBinError(ix + 1, iy + 1));
    }
  }

  m_priors[index].setBinEdges(xEdges, yEdges);
  m_priors[index].setPriorsTable(prior);
  m_priors[index].setErrorsTable(error);

  return;
};



void PIDPriors::setPriors(const Const::ChargedStable& particle, TH2F* counts, TH2F* normalization)
{
  TH2F* priorHistogram = (TH2F*)counts->Clone();

  priorHistogram->Sumw2();
  priorHistogram->Divide(normalization);

  setPriors(particle, priorHistogram);
};



