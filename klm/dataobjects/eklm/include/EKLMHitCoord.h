/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <klm/dataobjects/eklm/EKLMHitGlobalCoord.h>

/* ROOT headers. */
#include <TVector3.h>

namespace Belle2 {

  /**
   * Hit coordinates.
   */
  class EKLMHitCoord : public EKLMHitGlobalCoord {

  public:

    /**
     * Constructor.
     */
    EKLMHitCoord();

    /**
     * Destructor.
     */
    ~EKLMHitCoord();

    /**
     * Set hit local position.
     * @param[in] x Hit x coordinate.
     * @param[in] y Hit y coordinate.
     * @param[in] z Hit z coordinate.
     */
    void setLocalPosition(float x, float y, float z);

    /**
     * Get hit local position x coordinate.
     * @return Hit x coordinate.
     */
    float getLocalPositionX() const
    {
      return m_localX;
    }

    /**
     * Get hit local position y coordinate.
     * @return Hit y coordinate.
     */
    float getLocalPositionY() const
    {
      return m_localY;
    }

    /**
     * Get hit local position z coordinate.
     * @return Hit z coordinate.
     */
    float getLocalPositionZ() const
    {
      return m_localZ;
    }

    /**
     * Get ihit local position.
     * @return Hit coordinates.
     */
    TVector3 getLocalPosition() const
    {
      return TVector3(m_localX, m_localY, m_localZ);
    }

  protected:

    /** Local position X coordinate. */
    float m_localX;

    /** Local position Y coordinate. */
    float m_localY;

    /** Local position Z coordinate. */
    float m_localZ;

  private:

    /** Class version. */
    ClassDef(Belle2::EKLMHitCoord, 1);

  };

}
