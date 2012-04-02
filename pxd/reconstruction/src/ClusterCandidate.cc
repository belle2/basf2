/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/reconstruction/ClusterCandidate.h>

using namespace std;

namespace Belle2 {

  namespace PXD {
    void ClusterCandidate::merge(ClusterCandidate& cls)
    {
      if (&cls == this) return;

      m_charge += cls.m_charge;
      if (cls.m_seed.getCharge() > m_seed.getCharge()) {
        m_seed = cls.m_seed;
      }
      m_pixels.insert(m_pixels.end(), cls.m_pixels.begin(), cls.m_pixels.end());
      cls.m_pixels.clear();
      cls.m_charge = 0;
    }

    void ClusterCandidate::add(const Pixel& pixel)
    {
      float charge = pixel.getCharge();
      m_charge += charge;
      m_pixels.push_back(pixel);
      if (NoiseMap::getInstance().seed(pixel.getU(), pixel.getV(), charge) && m_seed.getCharge() < charge) {
        m_seed = pixel;
      }
    }

  }
} //Belle2 namespace
