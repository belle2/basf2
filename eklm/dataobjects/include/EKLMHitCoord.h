/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITCOORD_H
#define EKLMHITCOORD_H

/* External headers. */
#include <TObject.h>
#include <TVector3.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitGlobalCoord.h>

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
    virtual ~EKLMHitCoord();

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
    float getLocalPositionX() const;

    /**
     * Get hit local position y coordinate.
     * @return Hit y coordinate.
     */
    float getLocalPositionY() const;

    /**
     * Get hit local position z coordinate.
     * @return Hit z coordinate.
     */
    float getLocalPositionZ() const;

    /**
     * Get ihit local position.
     * @return Hit coordinates.
     */
    TVector3 getLocalPosition() const;

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

#endif

