/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/vectorTools/B2Vector3.h>

#include <string>
#include <vector>
#include <utility> // std::pair


namespace Belle2 {

  /** simple Hit class used for sectorMap-training. */
  class SecMapTrainerHit {
  public:
    /** Constructor of class SecMapTrainerHit*/
    SecMapTrainerHit(const std::string& fullSecID, const B2Vector3D& hit):
      m_secID(fullSecID),
      m_hitPos(hit) {}

    /** returns global hit position of current hit. */
    B2Vector3D getHitPosition() const { return m_hitPos; }

    /** returns global x-position. */
    double X() const { return m_hitPos.X(); }

    /** returns global y-position. */
    double Y() const { return m_hitPos.Y(); }

    /** returns global z-position. */
    double Z() const { return m_hitPos.Z(); }

    /** returns secID of sector containing this hit. */
    std::string getSectorID() const { return m_secID; }

  protected:
    std::string m_secID; /**< ID of sector containing this hit. */
    B2Vector3D m_hitPos; /**< global hit position. */
  };
}

