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
    EKLMAlignmentData(double dx, double dy, double dalpha);

    /**
     * Destructor.
     */
    ~EKLMAlignmentData();

    /**
     * Set dx.
     */
    void setDx(double dx);

    /**
     * Get dx.
     */
    double getDx();

    /**
     * Set dy.
     */
    void setDy(double dy);

    /**
     * Get dy.
     */
    double getDy();

    /**
     * Set dalpha.
     */
    void setDalpha(double dalpha);

    /**
     * Get dalpha.
     */
    double getDalpha();

  private:

    /** Change of the segment x coordinate (along the strips). */
    double m_dx;

    /** Change of the segment y coordinate (perpendicular to the strips). */
    double m_dy;

    /** Additional rotation of the segment. */
    double m_dalpha;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMAlignmentData, 1);

  };

}

#endif

