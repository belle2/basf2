/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHIT2D_H
#define EKLMHIT2D_H

/* External headers. */
#include <CLHEP/Vector/ThreeVector.h>
#include <CLHEP/Geometry/Point3D.h>
#include <TObject.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <framework/datastore/DataStore.h>

namespace Belle2 {

  /**
   * Class for 2d hits handling.
   */
  class EKLMHit2d : public EKLMHitBase  {

  public:

    /**
     * Constructor.
     */
    EKLMHit2d();

    /**
     * Constructor with two strips.
     * @param[in] s1 Strip from plane 1.
     * @param[in] s2 Strip from plane 2.
     */
    EKLMHit2d(EKLMDigit* s1, EKLMDigit* s2);

    /**
     * Destructor.
     */
    ~EKLMHit2d();

    /**
     * Print 2d hit information.
     */
    void Print();

    /**
     * Get EKLMDigit.
     * @param[in[ plane plane number.
     * @return Pointer to the hit.
     */
    EKLMDigit* getDigit(int plane) const;

    /**
     * Set coordinates of the crossing point.
     * @param[in] point Coordinates.
     */
    void setCrossPoint(HepGeom::Point3D<double> point);

    /**
     * Get crossing point.
     * @return coordinates of the crossing point.
     */
    HepGeom::Point3D<double> getCrossPoint() const;

    /**
     * Get strip number (from EKLMDigit).
     * @param[in] plane  Number if plane.
     */
    int getStrip(int plane);

    /**
     * Set Chi^2 of the crossing point.
     * @param[in] chi Chi^2.
     */
    void setChiSq(float chi);

    /**
     * Get Chi^2 of the crossing point.
     * @return Chi^2.
     */
    float getChiSq() const;

  private:

    /** EKLMdigts used to create this hit. */
    EKLMDigit* m_digit[2];  //! {ROOT streamer directive}

    /** Crossing point global X coordinate */
    float m_crossPointX;

    /** Crossing point global Y coordinate */
    float m_crossPointY;

    /** Crossing point global Z coordinate */
    float m_crossPointZ;

    /** Chi^2 of the hit. */
    float m_ChiSq;

    /** Needed to make Belle2::EKLMHit2d storable. */
    ClassDef(Belle2::EKLMHit2d, 1);

  };

}

#endif

