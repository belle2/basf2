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
     * Constructor with two strips
     */
    EKLMHit2d(EKLMDigit*, EKLMDigit*);

    /**
     * Destructor.
     */
    ~EKLMHit2d() {};

    /**
     * Print 2d hit information.
     */
    void Print();

    /**
     * Get X hit.
     * @return Pointer to the strip hit in X direction.
     */
    const EKLMDigit* getXStripHit() const;

    /**
     * Get Y hit.
     * @return Pointer to the strip hit in Y direction.
     */
    const EKLMDigit* getYStripHit() const;

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
     * Set Chi^2 of the crossing point.
     * @param[in] chi Chi^2.
     */
    void setChiSq(double chi);

    /**
     * Get Chi^2 of the crossing point.
     * @return Chi^2.
     */
    double getChiSq() const;

  private:

    /** Reference to the X Strip hit. */
    EKLMDigit const* m_XStrip;  //-> {ROOT streamer directive}

    /** Reference to the Y Strip hit. */
    EKLMDigit const* m_YStrip;  //-> {ROOT streamer directive}

    /** Crossing point global X coordinate */
    double m_crossPointX;

    /** Crossing point global Y coordinate */
    double m_crossPointY;

    /** Crossing point global Z coordinate */
    double m_crossPointZ;

    /** Chi^2 of the hit. */
    double m_ChiSq;

    /** Needed to make Belle2::EKLMHit2d storable. */
    ClassDef(Belle2::EKLMHit2d, 1);

  };

}

#endif

