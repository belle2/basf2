/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TLorentzVector.h>
#include <TMatrixDSym.h>
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
     * @param[in] p               Absolute value of 3-momentum.
     */
    KLMCluster(float x, float y, float z, float time, int nLayers,
               int nInnermostLayer, float p);

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
    {return m_layers;}

    /**
     * Get number of the innermost layer with hits.
     * @return Number of the innermost layer with hits.
     */
    inline int getInnermostLayer() const
    {return m_innermostLayer;}

    /**
     * Get global position (TVector3 version).
     * @return Hit coordinates.
     */
    inline TVector3 getClusterPosition() const
    {return TVector3(m_globalX, m_globalY, m_globalZ);}

    /**
     * Get global position (TVector3 version) of the origin of KLMCluster
     * (always return (0,0,0) since we believe all KLMClusters to originate at or
     * near primary vertex).
     * @return KLMCluster origin coordinates.
     */
    inline TVector3 getPosition() const
    {return TVector3(0, 0, 0);}

    /**
     * Get momentum magnitude.
     * @return Absolute value of 3-momentum.
     */
    float getMomentumMag() const;

    /**
     * Get energy.
     * @return Full energy.
     */
    float getEnergy() const;

    /**
     * Get momentum.
     * @return 4-Momentum.
     */
    TLorentzVector getMomentum() const;

    /**
     * Get KLM cluster momentum error matrix.
     * @return 4x4 KLM cluster momentum error matrix.
     */
    TMatrixDSym getError4x4() const;

    /**
     * Get KLM cluster momentum error matrix.
     * @return 7x7 KLM cluster momentum error matrix.
     */
    TMatrixDSym getError7x7() const;

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

    /**
     * Set time.
     * @param[in] time Time.
     */
    void setTime(float time)
    {m_time = time;}

    /**
     * Set number of layers with hits.
     * @param[in] layers Number of layers with hits.
     */
    void setLayers(int layers)
    {m_layers = layers;}

    /**
     * Set number of the innermost layer with hits.
     * @param[in] innermostLayer Number of the innermost layer with hits.
     */
    void setInnermostLayer(int innermostLayer)
    {m_innermostLayer = innermostLayer;}

    /**
     * Set global position.
     * @param[in] globalX X coordinate.
     * @param[in] globalY Y coordinate.
     * @param[in] globalZ Z coordinate.
     */
    void setClusterPosition(float globalX, float globalY, float globalZ)
    {
      m_globalX = globalX;
      m_globalY = globalY;
      m_globalZ = globalZ;
    }

    /**
     * Set momentum magnitude.
     * @param[in] momentumMag Momentum magnitude.
     */
    void setMomentumMag(float momentumMag)
    {m_p = momentumMag;}

    /**
     * Set error of vertex X coordinate.
     * @param[in] errorX Error of vertex X coordinate.
     */
    void setErrorX(float errorX)
    {m_errorX = errorX;}

    /**
     * Set error of vertex Y coordinate.
     * @param[in] errorY Error of vertex Y coordinate.
     */
    void setErrorY(float errorY)
    {m_errorY = errorY;}

    /**
     * Set error of vertex Z coordinate.
     * @param[in] errorZ Error of vertex Z coordinate.
     */
    void setErrorZ(float errorZ)
    {m_errorZ = errorZ;}

    /**
     * Set error of momentum absolute value.
     * @param[in] errorP Error of momentum absolute value.
     */
    void setErrorP(float errorP)
    {m_errorP = errorP;}

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

    /** Absolute value of momentum, 0 means unknown. */
    float m_p;

    /** Error of vertex X coordinate. */
    float m_errorX;

    /** Error of vertex Y coordinate. */
    float m_errorY;

    /** Error of vertex Z coordinate. */
    float m_errorZ;

    /** Error of momentum absolute value. */
    float m_errorP;

    /** Needed to make objects storable. */
    ClassDef(Belle2::KLMCluster, 2);

  };

}

