/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITGLOBALCOORD_H
#define EKLMHITGLOBALCOORD_H

/* Extrenal headers. */
#include <CLHEP/Geometry/Point3D.h>
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
     * Get global position of the particle hit.
     * @return Hit coordinates.
     */
    HepGeom::Point3D<double> getGlobalPosition() const;

    /**
     * Set global position of the particle hit.
     * @param[in] gpos Hit coordinates.
     */
    void setGlobalPosition(HepGeom::Point3D<double> gpos);

    /**
     * Get global position (TVector3 version, for visualization).
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

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMHitGlobalCoord, 1);

  };

}

#endif

