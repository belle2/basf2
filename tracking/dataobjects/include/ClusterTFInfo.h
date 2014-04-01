/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Ferstl                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <TObject.h>
#include <TVector3.h>
#include <string>


#include "BaseTFInfo.h"

namespace Belle2 {
  /** Cluster TF Info Class
   *
   *  This class is needed for information transport between the VXD Track Finder and the display.
   *
   * Members:
   *  m_realClusterId (int) = Cluster ID = Positon in the Store Array
   *  m_relativeClusterId (int) = Relative Cluster ID in TF used Cluster Array
   *  m_detectorType (int) = Detector Type for svd/pxd difference
   *  m_useCounter (int) = Countes the Hits using this Cluster (alive and connected)
   *  m_maxCounter (int) = max. m_useCounter
   *    m_particleID (int) = Particle ID
   *  m_isReal (int) => 0 = Particle is not real; 1 = Particle is real
   *
   * Important Methodes:
   *  isOverlapped: returns true if = used more then one time by Hits = Cluster is overlapped = m_useCounter > 1
   *
   */

  class ClusterTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    ClusterTFInfo() {
      m_realClusterId = -1;
      m_detectorType = -1;
      m_useCounter = 0;
      m_maxCounter = 0;
      m_particleID = -1;
      m_isReal = 0;
      m_relativeClusterId = -1;
      m_pdgCode = 0;
    }

    /** Standard constructor */
    ClusterTFInfo(int par_pass_index, int par_cluster_id, int par_detector): BaseTFInfo(par_pass_index) {
      m_realClusterId = par_cluster_id;
      m_detectorType = par_detector;
      m_useCounter = 0;
      m_maxCounter = 0;
      m_particleID = -1;
      m_isReal = 0;
      m_relativeClusterId = -1;
      m_pdgCode = 0;
    }

    /** getter - RealClusterID */
    int getRealClusterID()  { return m_realClusterId; }

    /** setter - RealClusterID */
    void setRealClusterID(int value) { m_realClusterId = value; }

    /** getter - RelativeClusterID */
    int getRelativeClusterID()  { return m_relativeClusterId; }

    /** setter - RelativeClusterID */
    void setRelativeClusterID(int value) { m_relativeClusterId = value; }

    /** getter - Detector Type */
    int getDetectorType()  { return m_detectorType; }

    /** setter - Detector Type */
    void setDetectorType(int value) { m_detectorType = value; }


    /** getter - UseCounter */
    int getUseCounter()  { return m_useCounter; }

    /** getMaxCounter */
    int getMaxCounter()  { return m_maxCounter; }

    /** setter - UseCounter */
    void setUseCounter(int value) {
      m_useCounter = value;
      if (m_useCounter > m_maxCounter) {
        m_maxCounter = m_useCounter;
      }
    }

    /** UseCounte add / minus */
    void changeUseCounter(int value) {
      m_useCounter = m_useCounter + value;
      if (m_useCounter > m_maxCounter) {
        m_maxCounter = m_useCounter;
      }
    }

    // Cluster is overlapped if it is used more then one time
    /** if the Cluster is overlaped */
    bool isOverlapped()  {
      if (m_useCounter > 1) {
        return true;
      } else {
        return false;
      }
    }

    /** getter - particleID */
    int getParticleID()  { return m_particleID; }

    /** setter - particleID */
    void setParticleID(int value) { m_particleID = value; }

    /** getter - isReal */
    int getIsReal()  { return m_isReal; }

    /** setter - isReal */
    void setIsReal(int value) { m_isReal = value; }

    /** getter - isReal */
    int getPDG()  { return m_pdgCode; }

    /** setter - isReal */
    void setPDG(int value) { m_pdgCode = value; }



  protected:

    int m_realClusterId;    /**< Store Arrays ID */
    int m_relativeClusterId;  /**< Relative Cluster ID in TF used Cluster Array */

    int m_detectorType;  /**< Detector Type */

    int m_useCounter;  /**< Countes the Hits using this Cluster (alive and connected)  */
    int m_maxCounter;  /**< max. m_useCounter */

    int m_particleID;  /**< Particle ID */
    int m_isReal;  /**< 0 = Particle is not real; 1 = Particle is real */

    int m_pdgCode;   /**< PDG Code */

    ClassDef(ClusterTFInfo, 1)
  };
}
