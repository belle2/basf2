/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Benjamin Schwenker                                       *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <pxd/dbobjects/PXDClusterPositionEstimatorPar.h>
#include <pxd/dbobjects/PXDClusterShapeIndexPar.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  namespace PXD {
    /**
    * Singleton class that estimates cluster positions taking into account estimated track
    * incidence angles into the sensor. The class provides an interface to get the likelyhood
    * that a given cluster was created by a charged track with given incidence angles.
    */
    class PXDClusterPositionEstimator {

    public:

      /** Read cluster position corrections from DataBase */
      void initialize()
      {
        DBObjPtr<PXDClusterPositionEstimatorPar> dbObjPositionEstimator;
        if (!dbObjPositionEstimator) {
          // Check that we found the object and if not report the problem
          B2FATAL("No PXDClusterPositionEstimator payload found.");
        }
        m_positionEstimatorPar = *dbObjPositionEstimator;

        DBObjPtr<PXDClusterShapeIndexPar> dbObjShapeIndex;
        if (!dbObjShapeIndex) {
          // Check that we found the object and if not report the problem
          B2FATAL("No PXDClusterShapeIndexPar payload found.");
        }
        m_shapeIndexPar = *dbObjShapeIndex;

        // Remember that payloads are initialized now
        m_isInitialized = true;
      }

      /** Get initialization status. */
      bool isInitialized() const { return m_isInitialized; }

      /** Return cluster position estimator parameters from Database */
      const PXDClusterPositionEstimatorPar& getPositionEstimatorParameters() const {return  m_positionEstimatorPar;}

      /** Return shape index parameters from Database */
      const PXDClusterShapeIndexPar& getShapeIndexParameters() const {return  m_shapeIndexPar;}

      /** Return position-corrected cluster.
      * Create a temporary cluster to avoid accumulating corrections
      * Like this:
      * PXDCluster correctedCluster(*this->getCluster()); // "this" = PXDRecoHit
      * correctedCluster =
      *   PXD::PXDClusterPositionEstimator::getInstance().correctCluster(correctedCluster, tu, tv);
      * correctedCluster is a local object and dies with its scope.
      * @param cluster pointer to the cluster to be corrected
      * @param tu Track direction in u-coordinate
      * @param tv Track direction in v-coordinate
      * @return Corrected cluster
      */
      PXDCluster& correctCluster(PXDCluster& cluster, double tu, double tv) const;

      /** Return cluster shape likelyhood. */
      float getShapeLikelyhood(const PXDCluster& cluster, double tu, double tv) const;

      /** Main (and only) way to access the PXDClusterPositionEstimator. */
      static PXDClusterPositionEstimator& getInstance();

      /** Return the normed charge ratio between head and tail digit of cluster. */
      float computeEta(const PXDCluster& cluster, double thetaU, double thetaV) const;

      /** Return a name for the cluster shape constructed from the relative positions of head and tail digit. */
      const std::string getShortName(const PXDCluster& cluster, float thetaU, float thetaV) const;

      /** Return a name for the cluster shape constructed from the relative positions of all digits. */
      const std::string getFullName(const PXDCluster& cluster) const;

      /** Return type of corrector function */
      typedef std::function<PXDCluster&(PXDCluster&, double, double)> corrector_function_type;

      /** Return corrector function for this corrector */
      corrector_function_type getCorrectorFunction()
      {
        return ([this](PXDCluster & cluster, double tu, double tv)  -> PXDCluster& {
          return correctCluster(cluster, tu, tv);
        });
      }

    private:

      /** Return type of cluster needed to find cluster position correction. */
      int getClusterkind(const PXDCluster& cluster) const;

      /* Return reference to the head digit in the cluster. */
      const PXDDigit& getHeadDigit(const PXDCluster& cluster, float thetaU, float thetaV) const;

      /* Return reference to the tail digit in the cluster. */
      const PXDDigit& getTailDigit(const PXDCluster& cluster, float thetaU, float thetaV) const;

      /* Return reference to the last digit in cluster with given vOffset from vStart. */
      const PXDDigit& getLastDigitWithVOffset(const PXDCluster& cluster, int vOffset) const;

      /* Return reference to the first digit in cluster with given vOffset from vStart. */
      const PXDDigit& getFirstDigitWithVOffset(const PXDCluster& cluster, int vOffset) const;

      /** Singleton class, hidden constructor */
      PXDClusterPositionEstimator() {};
      /** Singleton class, forbidden copy constructor */
      PXDClusterPositionEstimator(const PXDClusterPositionEstimator&) = delete;
      /** Singleton class, forbidden assignment operator */
      PXDClusterPositionEstimator& operator=(const PXDClusterPositionEstimator&) = delete;

      /** Flag to indicate if Database payloads found. Set to true by a successful call to initialize(), otherwise false. */
      bool m_isInitialized = false;
      /** Map of cluster shape names and cluster shape and their index values*/
      PXDClusterShapeIndexPar m_shapeIndexPar;
      /** Lookup table with indexed shape offsets and likelyhoods*/
      PXDClusterPositionEstimatorPar m_positionEstimatorPar;
    };
  }
} //Belle2 namespace
