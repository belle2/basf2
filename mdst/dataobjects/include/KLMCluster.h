/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMCLUSTER_H
#define KLMCLUSTER_H

/* External headers. */
#include <Math/Point3D.h>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>
#include <TVector3.h>

/* Belle2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * KLM cluster data.
   */
  class KLMCluster: public RelationsObject {

  public:

    /**
     * Constructor.
     */
    KLMCluster();

    /**
     * Constructor.
     * @param[in] x               Vertex X coordinate.
     * @param[in] y               Vertex Y coordinate.
     * @param[in] z               Vertex Z coordinate.
     * @param[in] time            Decay (interaction) time.
     * @param[in] nLayers         Number of layers with signal.
     * @param[in] nInnermostLayer First layer.
     * @param[in] px              Momentum X component.
     * @param[in] py              Momentum Y component.
     * @param[in] pz              Momentum Z component.
     */
    KLMCluster(float x, float y, float z, float time, int nLayers,
               int nInnermostLayer, float px, float py, float pz);

    /**
     * Destructor.
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
     * Get number of the innermost layer with hits.
     * @return Number of the innermost layer with hits.
     */
    inline int getInnermostLayer() const
    { return m_innermostLayer;}

    /**
     * Get global position (TVector3 version).
     * @return Hit coordinates.
     */
    inline TVector3 getClusterPosition() const
    {return TVector3(m_globalX, m_globalY, m_globalZ);}

    /**
     * Get global position (TVector3 version) of the origin of K0L
     * (always return (0,0,0) since we believe all K0Ls to originate at or
     * near primary vertex).
     * @return K0L origin coordinates.
     */
    inline TVector3 getPosition() const
    {return TVector3(0, 0, 0);}

    /**
     * Get momentum.
     * @return 4-Momentum.
     */
    inline TLorentzVector getMomentum() const
    { return TLorentzVector(m_pX, m_pY, m_pZ, m_e);}

    /**
     * Get ECL flag.
     * @return ECL flag.
     */
    bool getAssociatedEclClusterFlag() const
    {return m_eclClusterFound;}

    /**
     * Get KLM cluster momentum error matrix.
     * @return 7x7 KLM cluster momentum error matrix.
     */
    TMatrixFSym getErrorMatrix() const
    {return m_momentumErrorMatrix;}

    /**
     * Get track flag.
     * @return Track flag.
     */
    bool getAssociatedTrackFlag() const
    {return m_trackFound;}

    /**
     * Set ecl connected cluster flag.
     */
    inline void setAssociatedEclClusterFlag()
    {m_eclClusterFound = true;}

    /**
     * Set connected track flag.
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

