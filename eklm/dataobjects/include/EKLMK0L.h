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
#include <framework/datastore/RelationsObject.h>


/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitGlobalCoord.h>
#include <eklm/dataobjects/EKLMHitMomentum.h>

namespace Belle2 {

  /**
   * K0L data.
   */
  class EKLMK0L : public RelationsObject, public EKLMHitGlobalCoord,
    public EKLMHitMomentum {

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

    /**
     * Get number of the innermost level with hits
     * @return Innermost hitted layer
     */
    inline int getInnermostLayer()
    {return m_innermostLayer;};


    /**
     * Get number of the innermost level with hits
     * @param[in] l Innermost hitted layer
     */
    inline void setInnermostLayer(int l)
    {m_innermostLayer = l;};

  private:


    /** Decay time. */
    float m_time;

    /** Number of layers with hits. */
    int m_layers;

    /** Innermost hitted layer. */
    int m_innermostLayer;

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMK0L, 2);

  };

}

#endif

