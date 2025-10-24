/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <utility>
#include <framework/datastore/RelationsObject.h>
#include "Math/Vector3D.h"

namespace Belle2 {

  /** Extra info for analysis of the 3DFinder of the CDC trigger */
  class CDCTrigger3DFinderInfo : public RelationsObject {
  public:
    // Default constructor
    CDCTrigger3DFinderInfo() = default;
    /** Constructor with arguments */
    CDCTrigger3DFinderInfo(std::vector<ROOT::Math::XYZVector> houghSpace,
                           std::vector<ROOT::Math::XYZVector> readoutCluster)
      : m_houghSpace(std::move(houghSpace)), m_readoutCluster(std::move(readoutCluster)) {}
    /** Return the Hough space */
    std::vector<ROOT::Math::XYZVector> getHoughSpace() const { return m_houghSpace; }
    /** Return the cluster readout */
    std::vector<ROOT::Math::XYZVector> getClusterReadout() const { return m_readoutCluster; }

  private:
    /** List of the complete Hough space weights. */
    std::vector<ROOT::Math::XYZVector> m_houghSpace = {};
    /** Includes the peak weight, cluster weights, center of gravity and indices. */
    std::vector<ROOT::Math::XYZVector> m_readoutCluster = {};
    /** Needed to make the ROOT object storable */
    ClassDef(CDCTrigger3DFinderInfo, 2);
  };
}
