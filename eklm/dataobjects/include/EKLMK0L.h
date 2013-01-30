/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMK0L_H
#define EKLMK0L_H

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   * K0L data.
   */
  class EKLMK0L : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMK0L();

    /**
     * Destructor.
     */
    ~EKLMK0L();

    /**
     * Set hit position.
     * @param[in] pos Hit coordinates.
     */
    void setHitPosition(HepGeom::Point3D<double> pos);

    /**
     * Get hit position.
     * @return Hit coordinates.
     */
    HepGeom::Point3D<double> getHitPosition() const;

    /**
     * Get hit position, TVector3 version.
     * @return Hit coordinates.
     */
    TVector3 getPosition() const;

    /**
     * Set decay time.
     * @param[in] time Time.
     */
    void setTime(float time);

    /**
     * Get time.
     * @return Time of decay.
     */
    float getTime();

    /**
     * Set number of layers with hits.
     * @param[in] l Number of layers.
     */
    void setLayers(int l);

    /**
     * Get number of layers with hits.
     * @return Number of layers.
     */
    int getLayers();

  private:

    /** Hit X coordinate. */
    float m_hitX;

    /** Hit Y coordinate. */
    float m_hitY;

    /** Hit Z coordinate. */
    float m_hitZ;

    /** Decay time. */
    float m_time;

    /** Number of layers with hits. */
    int m_layers;

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMK0L, 1);

  };

}

#endif

