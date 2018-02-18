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
#include <pxd/dbobjects/PXDClusterOffsetPar.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <framework/database/DBObjPtr.h>
#include <set>
#include <pxd/reconstruction/Pixel.h>

namespace Belle2 {

  namespace PXD {
    /**
    * Singleton class that estimates cluster positions taking into account the estimated track
    * incidence angles into the sensor. The class also provides an interface to get the likelyhood
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

      /** Return pointer to cluster offsets, can be nullptr */
      const PXDClusterOffsetPar* getClusterOffset(const PXDCluster& cluster, double tu, double tv) const;

      /** Return cluster shape likelyhood. */
      float getShapeLikelyhood(const PXDCluster& cluster, double tu, double tv) const;

      /** Main (and only) way to access the PXDClusterPositionEstimator. */
      static PXDClusterPositionEstimator& getInstance();

      /** Return the normed charge ratio between head and tail pixels. */
      float computeEta(const std::set<Pixel>& pixels, int vStart, int vSize, double thetaU, double thetaV) const;

      /** Return a name for the pixel set */
      const std::string getShortName(const std::set<Pixel>& pixels, int uStart, int vStart, int vSize, double thetaU,
                                     double thetaV) const;

      /** Return a name for the pixel set. */
      const std::string getFullName(const std::set<Pixel>& pixels, int uStart, int vStart) const;

      /** Return type of cluster needed to find cluster position correction. */
      int getClusterkind(const PXDCluster& cluster) const;

    private:

      /* Return reference to the head pixel in pixel set. */
      const Pixel& getHeadPixel(const std::set<Pixel>& pixels, int vStart, int vSize, double thetaU, double thetaV) const;

      /* Return reference to the tail pixel in pixel set. */
      const Pixel& getTailPixel(const std::set<Pixel>& pixels, int vStart, int vSize, double thetaU, double thetaV) const;

      /* Return reference to the last pixel in pixel set with given vOffset from vStart. */
      const Pixel& getLastPixelWithVOffset(const std::set<Pixel>& pixels, int vStart, int vOffset) const;

      /* Return reference to the first pixel in pixel set with given vOffset from vStart. */
      const Pixel& getFirstPixelWithVOffset(const std::set<Pixel>& pixels, int vStart, int vOffset) const;

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
