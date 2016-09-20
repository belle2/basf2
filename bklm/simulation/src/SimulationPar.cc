/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/simulation/SimulationPar.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>

using namespace std;

namespace Belle2 {

  namespace bklm {

    SimulationPar* SimulationPar::m_Instance = NULL;

    SimulationPar* SimulationPar::instance()
    {
      if (!m_Instance) m_Instance = new SimulationPar();
      return m_Instance;
    }

    SimulationPar::SimulationPar()
    {
      m_IsValid = false;
    }

    SimulationPar::~SimulationPar()
    {
    }

    void SimulationPar::read()
    {
      GearDir content(Gearbox::getInstance().getDetectorComponent("BKLM"), "SimulationParameters");
      if (!content) return;

      m_HitTimeMax = content.getWithUnit("/HitTimeMax");

      char name[40];
      int div = 0;
      int j = 0;
      for (div = 0; div <= c_NDIV; ++div) {
        for (j = 0; j < c_MAX_NHIT; ++j) {
          m_PhiMultiplicityCDF[div][j] = 1.0;
          m_ZMultiplicityCDF[div][j] = 1.0;
        }
      }
      double weight[c_MAX_NHIT];

      GearDir phiContent(content);
      phiContent.append("/RPCStripMultiplicity/Phi");
      int nDiv = min(phiContent.getNumberNodes("/Division"), c_NDIV + 1);
      for (div = 0; div < nDiv; ++div) {
        sprintf(name, "/Division[@id=\"%d\"]", div);
        GearDir divContent(phiContent);
        divContent.append(name);
        int nWeight = min(divContent.getNumberNodes("/Weight"), c_MAX_NHIT - 1);
        weight[0] = 0.0;
        for (j = 1; j <= nWeight; ++j) {
          sprintf(name, "/Weight[@multiplicity=\"%d\"]", j);
          weight[j] = divContent.getDouble(name) + weight[j - 1];
          weight[0] = weight[j];
        }
        for (j = 1; j < c_MAX_NHIT; ++j) {
          m_PhiMultiplicityCDF[div][j] = (j <= nWeight ? weight[j] / weight[0] : 1.0);
        }
      }
      GearDir zContent(content);
      zContent.append("/RPCStripMultiplicity/Z");
      nDiv = min(zContent.getNumberNodes("/Division"), c_NDIV + 1);
      for (div = 0; div < nDiv; ++div) {
        sprintf(name, "/Division[@id=\"%d\"]", div);
        GearDir divContent(zContent);
        divContent.append(name);
        int nWeight = min(divContent.getNumberNodes("/Weight"), c_MAX_NHIT - 1);
        weight[0] = 0.0;
        for (j = 1; j <= nWeight; ++j) {
          sprintf(name, "/Weight[@multiplicity=\"%d\"]", j);
          weight[j] = divContent.getDouble(name) + weight[j - 1];
          weight[0] = weight[j];
        }
        for (j = 1; j < c_MAX_NHIT; ++j) {
          m_ZMultiplicityCDF[div][j] = (j <= nWeight ? weight[j] / weight[0] : 1.0);
        }
      }

      m_IsValid = true;

    }

    double SimulationPar::getPhiMultiplicityCDF(double stripDiv, int mult) const
    {
      if (mult < 0) return 0.0;
      if (mult >= c_MAX_NHIT) return 1.0;
      int stripIndex = (int) fabs(stripDiv * c_NDIV / 0.5);
      if (stripIndex > c_NDIV) return 0.0;
      return m_PhiMultiplicityCDF[stripIndex][mult];
    }

    double SimulationPar::getZMultiplicityCDF(double stripDiv, int mult) const
    {
      if (mult < 0) return 0.0;
      if (mult >= c_MAX_NHIT) return 1.0;
      int stripIndex = (int) fabs(stripDiv * c_NDIV / 0.5);
      if (stripIndex > c_NDIV) return 0.0;
      return m_ZMultiplicityCDF[stripIndex][mult];
    }

  } // end of namespace bklm

} // end of namespace Belle2
