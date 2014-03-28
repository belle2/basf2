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
   *  m_real_cluster_id (int) = Cluster ID = Positon in the Store Array
   *  n_relative_cluster_id (int) = Relative Cluster ID in TF used Cluster Array
   *  m_detectorType (int) = Detector Type for svd/pxd difference
   *  m_use_counter (int) = Countes the Hits using this Cluster (alive and connected)
   *  m_max_counter (int) = max. m_use_counter
   *    m_particleID (int) = Particle ID
   *  m_is_real (int) => 0 = Particle is not real; 1 = Particle is real
   *
   * Important Methodes:
   *  isOverlapped: returns true if = used more then one time by Hits = Cluster is overlapped = m_use_counter > 1
   *
   */

  class ClusterTFInfo: public BaseTFInfo {
  public:

    /** Default constructor for the ROOT IO. */
    ClusterTFInfo() {
      m_real_cluster_id = -1;
      m_detectorType = -1;
      m_use_counter = 0;
      m_max_counter = 0;
      m_particleID = -1;
      m_is_real = 0;
      n_relative_cluster_id = -1;
      m_pdg_code = 0;
    }

    /** Standard constructor */
    ClusterTFInfo(int par_pass_index, int par_cluster_id, int par_detector): BaseTFInfo(par_pass_index) {
      m_real_cluster_id = par_cluster_id;
      m_detectorType = par_detector;
      m_use_counter = 0;
      m_max_counter = 0;
      m_particleID = -1;
      m_is_real = 0;
      n_relative_cluster_id = -1;
      m_pdg_code = 0;
    }

    /** getter - RealClusterID */
    int getRealClusterID()  { return m_real_cluster_id; }

    /** setter - RealClusterID */
    void setRealClusterID(int value) { m_real_cluster_id = value; }

    /** getter - RelativeClusterID */
    int getRelativeClusterID()  { return n_relative_cluster_id; }

    /** setter - RelativeClusterID */
    void setRelativeClusterID(int value) { n_relative_cluster_id = value; }

    /** getter - Detector Type */
    int getDetectorType()  { return m_detectorType; }

    /** setter - Detector Type */
    void setDetectorType(int value) { m_detectorType = value; }


    /** getter - UseCounter */
    int getUseCounter()  { return m_use_counter; }

    /** getMaxCounter */
    int getMaxCounter()  { return m_max_counter; }

    /** setter - UseCounter */
    void setUseCounter(int value) {
      m_use_counter = value;
      if (m_use_counter > m_max_counter) {
        m_max_counter = m_use_counter;
      }
    }

    /** UseCounte add / minus */
    void changeUseCounter(int value) {
      m_use_counter = m_use_counter + value;
      if (m_use_counter > m_max_counter) {
        m_max_counter = m_use_counter;
      }
    }

    // Cluster is overlapped if it is used more then one time
    /** if the Cluster is overlaped */
    bool isOverlapped()  {
      if (m_use_counter > 1) {
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
    int getIsReal()  { return m_is_real; }

    /** setter - isReal */
    void setIsReal(int value) { m_is_real = value; }

    /** getter - isReal */
    int getPDG()  { return m_pdg_code; }

    /** setter - isReal */
    void setPDG(int value) { m_pdg_code = value; }



  protected:

    int m_real_cluster_id;    /**< Store Arrays ID */
    int n_relative_cluster_id;  /**< Relative Cluster ID in TF used Cluster Array */

    int m_detectorType;  /**< Detector Type */

    int m_use_counter;  /**< Countes the Hits using this Cluster (alive and connected)  */
    int m_max_counter;  /**< max. m_use_counter */

    int m_particleID;  /**< Particle ID */
    int m_is_real;  /**< 0 = Particle is not real; 1 = Particle is real */

    int m_pdg_code;   /**< PDG Code */

    ClassDef(ClusterTFInfo, 1)
  };
}
