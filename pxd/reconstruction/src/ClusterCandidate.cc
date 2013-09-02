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

    ClusterCandidate* ClusterCandidate::merge(ClusterCandidate& cls)
    {
      //If this cluster has been merged, delegate to the correct cluster
      if (m_merged) return m_merged->merge(cls);
      ClusterCandidate* pcls = &cls;
      //No need to merge with ourselves
      if (pcls == this) return this;
      //If the other cluster has been merged, find the parent
      while (pcls->m_merged != 0) {
        ClusterCandidate* tmp = pcls->m_merged;
        if (tmp == this) return this;
        //Update all merged pointers to point to this cluster
        pcls->m_merged = this;
        pcls = tmp;
      }
      //Sum charge, set seed and copy pixels
      m_charge += pcls->m_charge;
      if (pcls->m_seed.getCharge() > m_seed.getCharge()) {
        m_seed = pcls->m_seed;
      }
      //copy only the smaller vector
      if (pcls->m_pixels.size() > m_pixels.size()) std::swap(m_pixels, pcls->m_pixels);
      m_pixels.insert(m_pixels.end(), pcls->m_pixels.begin(), pcls->m_pixels.end());
      //If the allocated memory is too large, shrink it down to default
      if (pcls->m_pixels.capacity() > c_maxCapacity) {
        std::vector<Pixel> tmp(c_defaultCapacity);
        std::swap(pcls->m_pixels, tmp);
      }
      //Clear the pixels and charge in the merged cluster
      pcls->m_pixels.clear();
      pcls->m_charge = 0;
      //And set the merge pointer
      pcls->m_merged = this;
      return this;
    }

    void ClusterCandidate::add(const Pixel& pixel)
    {
      //Delegate to the correct cluster
      if (m_merged) {
        m_merged->add(pixel);
        return;
      }
      //check seed charge
      float charge = pixel.getCharge();
      m_charge += charge;
      if (m_seed.getCharge() < charge) {
        m_seed = pixel;
      }
      //add pixel
      m_pixels.push_back(pixel);
    }
  }
} //Belle2 namespace
