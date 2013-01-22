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

/* Extrenal headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * Hit coordinates.
   */
  class EKLMHitCoord {

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
     * Get global position of the particle hit.
     * @return Hit coordinates.
     */
    HepGeom::Point3D<double> getGlobalPosition();

    /**
     * Set global position of the particle hit.
     * @param[in] gpos Hit coordinates.
     */
    void setGlobalPosition(HepGeom::Point3D<double> gpos);

    /**
     * Get local position of the particle hit.
     * @return Hit coordinates.
     */
    HepGeom::Point3D<double> getLocalPosition();

    /**
     * Set local position of the particle hit.
     * @param[in] lpos Hit coordinates.
     */
    void setLocalPosition(HepGeom::Point3D<double> lpos);

    /**
     * TODO: This function is deprecated (use the next one).
     *       It must be removed after update of visualization.
     * Get global position (TVector3 version, for visualization).
     * @return Hit coordinates.
     */
    const TVector3* getPosition() const __attribute__((deprecated));

    /**
     * Get global position (TVector3 version, for visualization).
     * @return Hit coordinates.
     */
    const TVector3 getGlobalPosition() const;

  protected:

    /** Global position X coordinate. */
    double m_globalX;

    /** Global position Y coordinate. */
    double m_globalY;

    /** Global position Z coordinate. */
    double m_globalZ;

    /** Local position X coordinate. */
    double m_localX;

    /** Local position Y coordinate. */
    double m_localY;

    /** Local position Z coordinate. */
    double m_localZ;

  private:

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMHitCoord, 1);

  };

}

#endif

