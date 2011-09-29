/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/NoiseMap.h>

using namespace std;

namespace Belle2 {
  namespace PXD {
    NoiseMap& NoiseMap::getInstance()
    {
      static auto_ptr<NoiseMap> instance(new NoiseMap());
      return *instance;
    }
  }
} //Belle2 namespace
