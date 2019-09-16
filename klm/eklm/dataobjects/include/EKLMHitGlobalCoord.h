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

/* External headers. */
#include <TObject.h>
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
    float getPositionX() const;

    /**
     * Get hit global position y coordinate.
     * @return Hit y coordinate.
     */
    float getPositionY() const;

    /**
     * Get hit global position z coordinate.
     * @return Hit z coordinate.
     */
    float getPositionZ() const;

    /**
     * Get hit global position.
     * @return Hit coordinates.
     */
    TVector3 getPosition() const;

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
