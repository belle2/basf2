/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/bklm/BKLMSimulationPar.h>

/* Belle 2 headers. */
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <cmath>

using namespace std;
using namespace Belle2;

BKLMSimulationPar::BKLMSimulationPar(const GearDir& content)
{
  read(content);
}

BKLMSimulationPar::~BKLMSimulationPar()
{
}

void BKLMSimulationPar::read(const GearDir& content)
{
  if (!content) {
    B2FATAL("The GearDir to look for BKLM simulation parameters is not valid.");
    return;
  }
  char name[40];
  double weight[c_MAX_NHIT];

  for (int div = 0; div <= c_NDIV; ++div) {
    for (int j = 0; j < c_MAX_NHIT; ++j) {
      m_PhiMultiplicityCDF[div][j] = 1.0;
      m_ZMultiplicityCDF[div][j] = 1.0;
    }
  }

  m_HitTimeMax = content.getWithUnit("/HitTimeMax");

  GearDir phiContent(content);
  phiContent.append("/RPCStripMultiplicity/Phi");
  m_NPhiDiv = phiContent.getNumberNodes("/Division");
  int nDiv = min(phiContent.getNumberNodes("/Division"), c_NDIV + 1);
  for (int div = 0; div < nDiv; ++div) {
    sprintf(name, "/Division[@id=\"%d\"]", div);
    GearDir divContent(phiContent);
    divContent.append(name);
    m_NPhiMultiplicity[div] = divContent.getNumberNodes("/Weight");
    int nWeight = min(divContent.getNumberNodes("/Weight"), c_MAX_NHIT - 1);
    weight[0] = 0.0;
    for (int j = 1; j <= nWeight; ++j) {
      sprintf(name, "/Weight[@multiplicity=\"%d\"]", j);
      m_PhiWeight[div][j] =  divContent.getDouble(name);
      weight[j] = divContent.getDouble(name) + weight[j - 1];
      weight[0] = weight[j];
    }
    for (int j = 1; j < c_MAX_NHIT; ++j) {
      m_PhiMultiplicityCDF[div][j] = (j <= nWeight ? weight[j] / weight[0] : 1.0);
    }
  }

  GearDir zContent(content);
  zContent.append("/RPCStripMultiplicity/Z");
  nDiv = min(zContent.getNumberNodes("/Division"), c_NDIV + 1);
  m_NZDiv = zContent.getNumberNodes("/Division");
  for (int div = 0; div < nDiv; ++div) {
    sprintf(name, "/Division[@id=\"%d\"]", div);
    GearDir divContent(zContent);
    divContent.append(name);
    int nWeight = min(divContent.getNumberNodes("/Weight"), c_MAX_NHIT - 1);
    m_NZMultiplicity[div] = divContent.getNumberNodes("/Weight");
    weight[0] = 0.0;
    for (int j = 1; j <= nWeight; ++j) {
      sprintf(name, "/Weight[@multiplicity=\"%d\"]", j);
      m_ZWeight[div][j] = divContent.getDouble(name);
      weight[j] = divContent.getDouble(name) + weight[j - 1];
      weight[0] = weight[j];
    }
    for (int j = 1; j < c_MAX_NHIT; ++j) {
      m_ZMultiplicityCDF[div][j] = (j <= nWeight ? weight[j] / weight[0] : 1.0);
    }
  }
}

double BKLMSimulationPar::getPhiMultiplicityCDF(double stripDiv, int mult) const
{
  if (mult < 0)
    return 0.0;
  if (mult >= c_MAX_NHIT)
    return 1.0;
  int stripIndex = (int) fabs(stripDiv * c_NDIV / 0.5);
  if (stripIndex > c_NDIV)
    return 0.0;
  return m_PhiMultiplicityCDF[stripIndex][mult];
}

double BKLMSimulationPar::getZMultiplicityCDF(double stripDiv, int mult) const
{
  if (mult < 0)
    return 0.0;
  if (mult >= c_MAX_NHIT)
    return 1.0;
  int stripIndex = (int) fabs(stripDiv * c_NDIV / 0.5);
  if (stripIndex > c_NDIV)
    return 0.0;
  return m_ZMultiplicityCDF[stripIndex][mult];
}
