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
     * Constructor.
     * @param[in] x               Vertex X coordinate.
     * @param[in] y               Vertex Y coordinate.
     * @param[in] z               Vertex Z coordinate.
     * @param[in] time            Decay (interaction) time.
     * @param[in] nLayers         Number of layers with signal.
     * @param[in] nInnermostLayer First layer.
     * @param[in] e               Energy.
     */
    KLMCluster(float x, float y, float z, float time, int nLayers,
               int nInnermostLayer, float e);

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
    TLorentzVector getMomentum() const;

    /**
     * Get KLM cluster momentum error matrix.
     * @return 7x7 KLM cluster momentum error matrix.
     */
    TMatrixFSym getErrorMatrix() const
    {return m_momentumErrorMatrix;}

    /**
     * Check for associated ECL clusters.
     * @return True if associated ECL clusters exist.
     */
    bool getAssociatedEclClusterFlag() const;

    /**
     * Check for associated tracks.
     * @return True if associated tracks exist.
     */
    bool getAssociatedTrackFlag() const;

  private:

    /** Decay time. */
    float m_time;

    /** Number of layers with hits. */
    int m_layers;

    /** Number of the innermost layer with hits. */
    int m_innermostLayer;

    /** Global position X coordinate. */
    float m_globalX;

    /** Global position Y coordinate. */
    float m_globalY;

    /** Global position Z coordinate. */
    float m_globalZ;

    /** Energy. */
    float m_e;

    /** error matrix 7x7 for KL momentum */
    TMatrixFSym m_momentumErrorMatrix;

    /** Needed to make objects storable. */
    ClassDef(Belle2::KLMCluster, 1);

  };

}

#endif

