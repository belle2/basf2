/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTDATA_H
#define EKLMALIGNMENTDATA_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM Alignment data.
   */
  class EKLMAlignmentData : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMAlignmentData();

    /**
     * Constructor.
     * @param[in] dx     dx.
     * @param[in] dy     dy.
     * @param[in] dalpha dalpha.
     */
    EKLMAlignmentData(float dx, float dy, float dalpha);

    /**
     * Destructor.
     */
    ~EKLMAlignmentData();

    /**
     * Set dx.
     */
    void setDx(float dx);

    /**
     * Get dx.
     */
    float getDx() const;

    /**
     * Set dy.
     */
    void setDy(float dy);

    /**
     * Get dy.
     */
    float getDy() const;

    /**
     * Set dalpha.
     */
    void setDalpha(float dalpha);

    /**
     * Get dalpha.
     */
    float getDalpha() const;

  private:

    /** Change of the segment x coordinate (along the strips). */
    float m_Dx;

    /** Change of the segment y coordinate (perpendicular to the strips). */
    float m_Dy;

    /** Additional rotation of the segment. */
    float m_Dalpha;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMAlignmentData, 1);

  };

}

#endif

