/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Markus Röhrken, Pablo Goldenzweig (KIT)                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLEOCONES_H
#define CLEOCONES_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/ContinuumSuppression/FuncPtr.h>

#include <analysis/utility/PCmsLabTransform.h>

namespace Belle2 {

  class Particle;
//class TVector3;

// ----------------------------------------------------------------------
// CleoCones
// ----------------------------------------------------------------------
  class CleoCones {
  public:
    CleoCones(std::vector<TVector3> p3_cms_all,
              std::vector<TVector3> p3_cms_roe,
              TVector3 thrustB,
              bool calc_CleoCones_with_all,
              bool calc_CleoCones_with_roe);

    ~CleoCones() {};

    std::vector<float> cleo_cone_with_all() {
      return m_cleo_cone_with_all;
    }

    std::vector<float> cleo_cone_with_roe() {
      return m_cleo_cone_with_roe;
    }

  private:
    std::vector<float> m_cleo_cone_with_all;
    std::vector<float> m_cleo_cone_with_roe;
  };

} // Belle2 namespace

#endif
