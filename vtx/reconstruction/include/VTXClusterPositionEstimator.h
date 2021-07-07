/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vtx/dbobjects/VTXClusterPositionEstimatorPar.h>
#include <vtx/dbobjects/VTXClusterShapeIndexPar.h>
#include <vtx/dataobjects/VTXCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/database/DBObjPtr.h>
#include <set>
#include <vector>
#include <vtx/reconstruction/Pixel.h>
#include <memory>

namespace Belle2 {

  namespace VTX {
    /**
    * Singleton class that estimates cluster positions taking into account the estimated track
    * incidence angles into the sensor. The class also provides an interface to get the likelyhood
    * that a given cluster was created by a charged track with given incidence angles.
    */
    class VTXClusterPositionEstimator {

    public:

      /** Initialize VTXClusterPositionEstimator from DB */
      void initialize();

      /** Set ShapeIndex from DB. */
      void setShapeIndexFromDB();

      /** Set PositionEstimator from DB. */
      void setPositionEstimatorFromDB();

      /** Return cluster position estimator parameters from Database */
      const VTXClusterPositionEstimatorPar& getPositionEstimatorParameters() const {return  m_positionEstimatorPar;}

      /** Return shape index parameters from Database */
      const VTXClusterShapeIndexPar& getShapeIndexParameters() const {return  m_shapeIndexPar;}

      /** Return pointer to cluster offsets, can be nullptr */
      const VTXClusterOffsetPar* getClusterOffset(const VTXCluster& cluster, double tu, double tv) const;

      /** Return cluster shape likelyhood. */
      float getShapeLikelyhood(const VTXCluster& cluster, double tu, double tv) const;

      /** Main (and only) way to access the VTXClusterPositionEstimator. */
      static VTXClusterPositionEstimator& getInstance();

      /** Return the normed charge ratio between head and tail pixels (size>=2) or the charge of the seed (size=1) . */
      float computeEta(const std::set<Pixel>& pixels, int vStart, int vSize, double thetaU, double thetaV) const;

      /** Return the shape index of the pixels */
      int computeShapeIndex(const std::set<Pixel>& pixels, int uStart, int vStart, int vSize, double thetaU,
                            double thetaV) const;

      /** Return the name for the pixel set */
      const std::string getShortName(const std::set<Pixel>& pixels, int uStart, int vStart, int vSize, double thetaU,
                                     double thetaV) const;

      /** Return the mirrored name for the pixel set */
      const std::string getMirroredShortName(const std::set<Pixel>& pixels, int uStart, int vStart, int vSize, double thetaU,
                                             double thetaV) const;

      /** Return a name for the pixel set. */
      const std::string getFullName(const std::set<Pixel>& pixels, int uStart, int vStart) const;

      /** Return kind of cluster needed to find cluster position correction. */
      int getClusterkind(const VTXCluster& cluster) const;

      /** Return kind of cluster needed to find cluster position correction. */
      int getClusterkind(const std::vector<Pixel>& pixels, const VxdID& sensorID) const;

      /** Get sector index from angles. Sectors in thetaU and thetaV are numbered ++, -+, --, +-. */
      int getSectorIndex(double thetaU, double thetaV) const;

    private:

      /** Return reference to the head pixel in pixel set. */
      const Pixel& getHeadPixel(const std::set<Pixel>& pixels, int vStart, int vSize, double thetaU, double thetaV) const;

      /** Return reference to the tail pixel in pixel set. */
      const Pixel& getTailPixel(const std::set<Pixel>& pixels, int vStart, int vSize, double thetaU, double thetaV) const;

      /** Return reference to the last pixel in pixel set with given vOffset from vStart. */
      const Pixel& getLastPixelWithVOffset(const std::set<Pixel>& pixels, int vStart, int vOffset) const;

      /** Return reference to the first pixel in pixel set with given vOffset from vStart. */
      const Pixel& getFirstPixelWithVOffset(const std::set<Pixel>& pixels, int vStart, int vOffset) const;

      /** Singleton class, hidden constructor */
      VTXClusterPositionEstimator() {};
      /** Singleton class, forbidden copy constructor */
      VTXClusterPositionEstimator(const VTXClusterPositionEstimator&) = delete;
      /** Singleton class, forbidden assignment operator */
      VTXClusterPositionEstimator& operator=(const VTXClusterPositionEstimator&) = delete;

      /** VTXClusterShapeIndex retrieved from DB. */
      std::unique_ptr<DBObjPtr<VTXClusterShapeIndexPar>> m_shapeIndexFromDB;
      /** VTXClusterPositionEstimatorPar retrieved from DB. */
      std::unique_ptr<DBObjPtr<VTXClusterPositionEstimatorPar>> m_positionEstimatorFromDB;

      /** Current valid VTXClusterShapeIndex*/
      VTXClusterShapeIndexPar m_shapeIndexPar;
      /** Currrent valid VTXClusterPositionEstimatorPar*/
      VTXClusterPositionEstimatorPar m_positionEstimatorPar;
    };
  }
} //Belle2 namespace
