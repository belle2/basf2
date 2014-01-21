/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * K0L mdst class is a 'copy' of the EKLMK0L class                        *
 * adopted to fit mdst requirements                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef K0L_H
#define K0L_H

// MDST classes could depend only on framework
#include <framework/datastore/RelationsObject.h>


// dependence on root is allowed
#include <TVector3.h>
#include "TLorentzVector.h"
#include <Math/Point3D.h>

namespace Belle2 {

  /**
   * K0L data.
   */
  class K0L: public RelationsObject  {

  public:

    /**
     * Default Constructor.
     */
    K0L();

    // it is very unlikely that users will create and use their own K0L objects. Thus it seems worth to remove all setters and use constructor instead

    /**
     * Constructor with all information
     */
    K0L(ROOT::Math::XYZPointF coordinates, float time, int nLayers, int nInnermostLayer, TVector3 momentum);

    /**
     * Constructor with all information in floats
     */
    K0L(float x, float y, float z, float time, int nLayers, int nInnermostLayer, float px, float py, float pz);


    /**
     * Destructor
     */
    ~K0L();


    /**
     * Get time.
     * @return Time of decay.
     */
    float getTime() const;

    /**
     * Get number of layers with hits.
     * @return Number of layers.
     */
    int getLayers() const;

    /**
     * Get global position of the particle hit.
     * @return Hit coordinates.
     */
    ROOT::Math::XYZPointF getGlobalPosition() const;

    /**
     * Get global position (TVector3 version, for visualization).
     * @return Hit coordinates.
     */
    TVector3 getPosition() const;

    /**
     * Get momentum,
     * @return 4-Momentum.
     */
    TLorentzVector getMomentum() const;


  private:

    /** Decay time. */
    float m_time;

    /** Number of layers with hits. */
    int m_layers;

    /** Number of the innermost layer with hits. */
    int m_innermostLayer;

    //the following is copied from  EKLMHitGlobalCoord.h

    /** Global position X coordinate. */
    float m_globalX;

    /** Global position Y coordinate. */
    float m_globalY;

    /** Global position Z coordinate. */
    float m_globalZ;

    // the following is taken from EKLMHitMomentum.h

    /** Energy. */
    float m_e;

    /** Momentum X component. */
    float m_pX;

    /** Momentum Y component. */
    float m_pY;

    /** Momentum Z component. */
    float m_pZ;



    /** Needed to make objects storable. */
    ClassDef(Belle2::K0L, 1);

  };

}

#endif

