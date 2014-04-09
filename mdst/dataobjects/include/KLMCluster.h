/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * KLMCluster mdst class is a 'copy' of the EKLMKLMCluster class                        *
 * adopted to fit mdst requirements                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMCluster_H
#define KLMCluster_H

// MDST classes could depend only on framework
#include <framework/datastore/RelationsObject.h>


// dependence on root is allowed
#include <TVector3.h>
#include <TMatrixFSym.h>
#include "TLorentzVector.h"
#include <Math/Point3D.h>

namespace Belle2 {

  /**
   * KLMCluster data.
   */
  class KLMCluster: public RelationsObject  {

  public:

    /**
     * Default Constructor.
     */
    KLMCluster();

    // it is very unlikely that users will create and use their own KLMCluster objects. Thus it seems worth to remove all setters and use constructor instead
    // the only exception are inline setters for connected track and ecl cluster flags (Since these flags are seeted by another module)
    /**
     * Constructor with all information
     */
    KLMCluster(ROOT::Math::XYZPointF coordinates, float time, int nLayers, int nInnermostLayer, TVector3 momentum, TMatrixFSym errormatrix);

    /**
     * Constructor with all information in floats
     */
    KLMCluster(float x, float y, float z, float time, int nLayers, int nInnermostLayer, float px, float py, float pz);


    /**
     * Destructor
     */
    ~KLMCluster();


    /**
     * Get time.
     * @return Time of decay.
     */
    inline float getTime() const
    {return m_time;}

    /**
     * Get number of layers with hits.
     * @return Number of layers.
     */
    inline int getLayers() const
    { return m_layers;}

    /**
     * Get global position of the particle hit.
     * @return Hit coordinates.
     */
    inline ROOT::Math::XYZPointF getGlobalPosition() const
    {return ROOT::Math::XYZPointF(m_globalX, m_globalY, m_globalZ);}

    /**
     * Get global position (TVector3 version).
     * @return Hit coordinates.
     */
    inline TVector3 getClusterPosition() const
    {return TVector3(m_globalX, m_globalY, m_globalZ);}

    /**
     * Get global position (TVector3 version) of the origin of K0L (always return (0,0,0) since we believe all K0Ls to originate at or near primary vertex).
     * @return K0L origin coordinates.
     */
    inline TVector3 getPosition() const
    {return TVector3(0, 0, 0);}


    /**
     * Get momentum,
     * @return 4-Momentum.
     */
    inline TLorentzVector getMomentum() const
    { return TLorentzVector(m_pX, m_pY, m_pZ, m_e);}


    /**
     * Get  ECL flag,
     * @return ECL flag.
     */
    bool getAssociatedEclClusterFlag() const
    {return m_eclClusterFound;}

    /**
     * Get KL Cluster momentum error matrix
     * @return 7x7 KL Cluster momentum error matrix
     */
    TMatrixFSym getErrorMatrix()
    {return m_momentumErrorMatrix;}


    /**
     * Get  track flag,
     * @return track flag.
     */
    bool getAssociatedTrackFlag() const
    {return m_trackFound;}



    /**
     * Set ecl connected cluster flag
     */
    inline void setAssociatedEclClusterFlag()
    {m_eclClusterFound = true;}

    /**
     * Set  connected track flag
     */
    inline void setAssociatedTrackFlag()
    {m_trackFound = true;}



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

    /** error matrix 7x7 for KL momentum */
    TMatrixFSym m_momentumErrorMatrix;

    /** True if associated track is found */
    bool m_trackFound;

    /** True if associated ECL cluster is found */
    bool m_eclClusterFound;


    /** Needed to make objects storable. */
    ClassDef(Belle2::KLMCluster, 1);

  };

}

#endif

