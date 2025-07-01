/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* External headers. */
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMatrixDSym.h>

/* Basf2 headers. */
#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

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
     * Get global position (ROOT::Math::XYZVector version).
     * @return Hit coordinates.
     */
    inline ROOT::Math::XYZVector getClusterPosition() const
    {return ROOT::Math::XYZVector(m_globalX, m_globalY, m_globalZ);}

    /**
     * Get global position (ROOT::Math::XYZVector version) of the origin of KLMCluster
     * (always return (0,0,0) since we believe all KLMClusters to originate at or
     * near primary vertex).
     * @return KLMCluster origin coordinates.
     */
    inline ROOT::Math::XYZVector getPosition() const
    {return ROOT::Math::XYZVector(0, 0, 0);}

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
    ROOT::Math::PxPyPzEVector getMomentum() const;

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
     * Get KLM cluster-track rotation angle.
     * @return KLM rotation angle to the nearest track [rad].
     */
    inline float getClusterTrackRotationAngle() const { return m_clusterTrackRotationAngle; }

    /**
     * Get KLM cluster-track separation angle.
     * @return KLM cluster separation angle to the nearest track [rad].
     */
    inline float getClusterTrackSeparationAngle() const { return m_clusterTrackSeparationAngle; }

    /**
     * Get KLM cluster-track distance.
     * @return KLM cluster separation from the nearest track [cm].
     */
    inline float getClusterTrackSeparation() const { return m_clusterTrackSeparation; }

    /**
     * Get std deviation of the 1st axis from PCA.
     * @return  std deviation of the 1st axis from PCA [cm].
     */
    inline float getShapeStdDev1() const
    {return m_shapeStdDev1;}

    /**
     * Get KLM std deviation of the 2nd axis from PCA.
     * @return std deviation of the 2nd axis from PCA.
     */
    inline float getShapeStdDev2() const
    {return m_shapeStdDev2;}

    /**
     * Get std deviation of the 3rd axis from PCA.
     * @return std deviation of the 3rd axis from PCA.
     */
    inline float getShapeStdDev3() const
    {return m_shapeStdDev3;}

    /**
     * Set KLM cluster-track rotation angle.
     * @param[in] rotation angle to the nearest track.
     */
    void setClusterTrackRotationAngle(float rotation)
    {
      m_clusterTrackRotationAngle = rotation;
    }

    /**
     * Set KLM cluster-track separation angle.
     * @param[in] separation angle to the nearest track.
     */
    void setClusterTrackSeparationAngle(float separation)
    {
      m_clusterTrackSeparationAngle = separation;
    }

    /**
     * Set KLM cluster-track distance.
     * @param[in] dist distance to the nearest track.
     */
    void setClusterTrackSeparation(float dist)
    {
      m_clusterTrackSeparation = dist;
    }

    /**
     * Set std deviation of the 1st axis, from PCA.
     * @param[in] std1 : std deviation of the 1st axis from PCA [cm].
     */
    void setShapeStdDev1(float std1)
    {m_shapeStdDev1 = std1;}

    /**
     * Set std deviation of the 2nd axis, from PCA.
     * @param[in] std2 : std deviation of the 2nd axis from PCA [cm].
     */
    void setShapeStdDev2(float std2)
    {m_shapeStdDev2 = std2;}

    /**
     * Set std deviation of the 3rd axis, from PCA.
     * @param[in] std3 : std deviation of the 3rd axis from PCA [cm].
     */
    void setShapeStdDev3(float std3)
    {m_shapeStdDev3 = std3;}


    /**
     * Set time.
     * @param[in] time Time [ns].
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
     * @param[in] globalX X coordinate [cm].
     * @param[in] globalY Y coordinate [cm].
     * @param[in] globalZ Z coordinate [cm].
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

    /** Track-cluster rotation angle. */
    float m_clusterTrackRotationAngle = Const::floatNaN;

    /** Track-cluster separation angle. */
    float m_clusterTrackSeparationAngle = Const::floatNaN;

    /** Track-cluster separation (distance). */
    float m_clusterTrackSeparation = Const::floatNaN;

    /** Std deviation of axis 1 (from Cluster Shape PCA). */
    float m_shapeStdDev1 = 0;

    /** Std deviation of axis 2 (from Cluster Shape PCA). */
    float m_shapeStdDev2 = 0;

    /** Std deviation of axis 3 (from Cluster Shape PCA). */
    float m_shapeStdDev3 = 0;

    /** Needed to make objects storable. */
    ClassDef(Belle2::KLMCluster, 4);

  };

}

