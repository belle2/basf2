/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// framework
#include <framework/datastore/RelationsObject.h>
#include <framework/core/FrameworkExceptions.h>
// vxd
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
// pxd
#include <pxd/dataobjects/PXDCluster.h>
//svd
#include <svd/dataobjects/SVDCluster.h>

#include <framework/logging/Logger.h>

// stl:
#include <vector>
#include <utility> // std::pair
#include <math.h>

namespace Belle2 {
  /** The SpacePoint class.
   *
   *  This class stores a global space point with its position error and some extra infos
   */
  class SpacePoint: public RelationsObject {
  public:
    /** .first is a pointer to the SVDCluster, .second is the index number of its corresponding StoreArray */
    typedef std::pair< const SVDCluster*, unsigned int> SVDClusterInformation;

    /** exception for the case that the user filled an invalid number of Clusters into the Constructor */
    BELLE2_DEFINE_EXCEPTION(InvalidNumberOfClusters, "SpacePoint::Constructor: invalid numbers of Clusters given!");

    /** exception for the case that the user filled an invalid combination of clusters into the constructor (e.g. they are not from the same sensor or both are u clusters) */
    BELLE2_DEFINE_EXCEPTION(IncompatibleClusters, "SpacePoint::Constructor: given combination of SVDCluster is not allowed!");




    /** Default constructor for the ROOT IO. */
    SpacePoint()
    {}



    /** Constructor for the case of PXD Hits. For the case of TelHits, there will be a SpacePoint-Inheriting Class adding a TelCluster-feature
     *
     * first parameter is pointer to cluster (passing a null-pointer will throw an exception)
     * second is the index number of the cluster in its storeArray.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the constructor gets its own pointer to it.
     */
    SpacePoint(const PXDCluster* pxdCluster,
               unsigned int indexNumber,
               const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** Constructor for the case of SVD Hits. up to 2 clusters can be added this way
     *
     * first parameter is a container carrying pairs, where .first is the pointer to the svdCluster, and .second provides its indexNumber for the StoreArray.
     *  It should _not_ be filled with NULL-Pointers (passing a null-pointer will throw an exception).
     *  1 - 2 Clusters are allowed that way, if there are passed more than that or less, an exception will be thrown.
     * second parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the constructor gets its own pointer to it.
     *
     */
    SpacePoint(const std::vector<Belle2::SpacePoint::SVDClusterInformation>& clusters,
               const VXD::SensorInfoBase* aSensorInfo = NULL);



// getter:

    /** return the position vector in global coordinates */
    const TVector3& getPosition() const { return m_position; }



    /** return the hitErrors in sigma of the global position */
    const TVector3& getPositionError() const { return m_positionError; }



    /** return the VxdID of the sensor inhabiting the Cluster of the SpacePoint */
    VxdID::baseType getVxdID() const { return m_vxdID; }



    /** return the normalized local coordinates of the cluster in u (0 <= posU <= 1) */
    double getNormalizedLocalU() const { return m_normalizedLocal.first/*m_normalizedLocal[0]*/; }



    /** return the normalized local coordinates of the cluster in v (0 <= posV <= 1) */
    double getNormalizedLocalV() const { return m_normalizedLocal.second/*m_normalizedLocal[1]*/; }



    /** converts a local hit into sensor-independent relative coordinates.
     *
     * first parameter is the local hit (as provided by getU and getV!) stored as a pair of doubles.
     * second parameter is the coded vxdID, which carries the sensorID.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the member gets its own pointer to it.
     */
    static std::pair<double, double> convertLocalToNormalizedCoordinates(const std::pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** converts a hit in sensor-independent relative coordinates into local coordinate of given sensor.
     *
     * first parameter is the hit in sensor-independent (real) coordinates stored as a pair of floats.
     * second parameter is the coded vxdID, which carries the sensorID.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the member gets its own pointer to it.
     */
//     static std::pair<double, double> convertToLocalCoordinates(const std::pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** converts a local hit on a given sensor into global coordinates.
     *
     * so this practically does what sensorInfo::pointToGlobal is doing, the difference is, that you do not need to have the sensorInfo beforehand (it will be retrieved using the VxdID)
     * first parameter is the local hit (as provided by getU and getV!) stored as a pair of doubles.
     * second parameter is the coded vxdID, which carries the sensorID.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the member gets its own pointer to it.
     */
    static TVector3 getGlobalCoordinates(const std::pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** converts a hit in sensor-independent relative coordinates into local coordinate of given sensor.
    *
    * first parameter is the hit in sensor-independent normalized ! coordinates stored as a pair of floats.
    * second parameter is the coded vxdID, which carries the sensorID.
    * third parameter, a sensorInfo can be passed for testing purposes.
    *  If no sensorInfo is passed, the member gets its own pointer to it.
    */
    static std::pair<double, double> convertNormalizedToLocalCoordinates(const std::pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** checks first parameter for boundaries.
     *
     * does take second/third argument for checking for lower/upper boundary.
     * if boundary is crossed, value gets reset to boundary value
     * */
    static void boundaryCheck(double& value, double lower = 0, double higher = 1) {
      if (value < lower) { value = lower; }
      if (value > higher) { value = higher; }
    }



  protected:
    /** protected function to set the global position error.
     *
     * It takes care for the transformation of the local sigmas to global error values.
     */
    void setPositionError(double uSigma, double vSigma, const VXD::SensorInfoBase* aSensorInfo) {
      //As only variances, but not the sigmas transform linearly,
      // we need to use some acrobatics
      // (and some more (abs) since we do not really transform a vector).
      TVector3 globalizedVariances = aSensorInfo->vectorToGlobal(
                                       TVector3(
                                         uSigma * uSigma,
                                         vSigma * vSigma,
                                         0
                                       )
                                     );
      for (int i = 0; i < 3; i++) {
        m_positionError[i] = std::sqrt(std::abs(globalizedVariances[i]));
      }
    }



    /** Global position vector.
     *
     *  [0]: x , [1] : y, [2] : z
     */
    TVector3 m_position;



    /** Error "Vector" of global position in sigma.
     *
     *  [0]: x-uncertainty , [1] : y-uncertainty, [2] : z-uncertainty
     */
    TVector3 m_positionError;



    /** Position in local coordinates normalized to the sensor size between 0 and 1.
     *
     *  First entry is u, second is v
     */
    std::pair<double, double> m_normalizedLocal;
//     double m_normalizedLocal[2];



    /** stores the vxdID */
    VxdID::baseType m_vxdID;



    /** carries the index numbers of the linked clusters.
     *
     * These index numbers are for the storeArray of svdClusters, if the detector type is Const::SVD, If it is Const::PXD, its for the pxdCluster container, and for Const::Test it is for a telCluster-container carrying all the telescope hits */
    std::vector<unsigned int> m_indexNumbers;
    ClassDef(SpacePoint, 1)
  };
}
