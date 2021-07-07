/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TVector3.h>

namespace Belle2 {

  /**
   * Hit coordinates.
   */
  class EKLMHitGlobalCoord {

  public:

    /**
     * Constructor.
     */
    EKLMHitGlobalCoord();

    /**
     * Destructor.
     */
    virtual ~EKLMHitGlobalCoord();

    /**
     * Set hit global position.
     * @param[in] x Hit x coordinate.
     * @param[in] y Hit y coordinate.
     * @param[in] z Hit z coordinate.
     */
    void setPosition(float x, float y, float z);

    /**
     * Set hit global position.
     * @param[in] pos Position.
     */
    void setPosition(const TVector3& pos);

    /**
     * Get hit global position x coordinate.
     * @return Hit x coordinate.
     */
    float getPositionX() const
    {
      return m_globalX;
    }

    /**
     * Get hit global position y coordinate.
     * @return Hit y coordinate.
     */
    float getPositionY() const
    {
      return m_globalY;
    }

    /**
     * Get hit global position z coordinate.
     * @return Hit z coordinate.
     */
    float getPositionZ() const
    {
      return m_globalZ;
    }

    /**
     * Get hit global position.
     * @return Hit coordinates.
     */
    TVector3 getPosition() const
    {
      return TVector3(m_globalX, m_globalY, m_globalZ);
    }

  protected:

    /** Global position X coordinate. */
    float m_globalX;

    /** Global position Y coordinate. */
    float m_globalY;

    /** Global position Z coordinate. */
    float m_globalZ;

  private:

    /** Class version. */
    ClassDef(Belle2::EKLMHitGlobalCoord, 1);

  };

}
