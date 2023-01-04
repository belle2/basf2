/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store ECL CNN PID value
   */
  class ECLCNNPid : public RelationsObject {
  public:

    /**
     * Default constructor for ROOT
     */
    ECLCNNPid():
      m_eclCnnMuon(0.0)
    {}

    /**
     * Usable constructor
     * @param eclCnnMuon muon probability return by ECL CNN PID module
     */
    ECLCNNPid(double eclCnnMuon) { m_eclCnnMuon = eclCnnMuon; }

    /**
     * Set eclCnnMuon
     * @param eclCnnMuon muon probability return by ECL CNN PID module
     */
    void setEclCnnMuon(double eclCnnMuon) { m_eclCnnMuon = eclCnnMuon; }

    /**
     * Get eclCnnMuon
     * @return eclCnnMuon
     */
    double getEclCnnMuon() const { return m_eclCnnMuon; }

  private:

    Double32_t m_eclCnnMuon; /**< muon probability return by ECL CNN PID module */

    ClassDef(ECLCNNPid, 1); /**< ClassDef */
  };
} // end namespace Belle2