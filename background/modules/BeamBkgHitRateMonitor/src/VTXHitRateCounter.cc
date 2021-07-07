/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Benjamin Schwenker                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/VTXHitRateCounter.h>
#include <string>

// framework aux
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Background {

    void VTXHitRateCounter::initialize(TTree* tree)
    {

      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // register collection(s) as optional, your detector might be excluded in DAQ
      m_clusters.isOptional();

      string leaflist =
        "averageRate/F:averageRateCluster/F:numEvents/I:valid/O";

      // set branch address
      tree->Branch("vtx", &m_rates, leaflist.c_str());
    }

    void VTXHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void VTXHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if ((not m_digits.isValid()) or (not m_clusters.isValid())) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      // accumulate hits
      rates.averageRate += m_digits.getEntries();
      rates.averageRateCluster += m_clusters.getEntries();

      // set flag to true to indicate the rates are valid
      rates.valid = true;
    }

    void VTXHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      if (m_rates.numEvents == 0) return;

      // Average number of VTXDigits per 1Hz
      m_rates.averageRate /= m_rates.numEvents;

      // Average number of VTXClusters per 1Hz
      m_rates.averageRateCluster /= m_rates.numEvents;
    }



  } // Background namespace
} // Belle2 namespace

