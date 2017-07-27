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

#include <framework/geometry/B2Vector3.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <pxd/dataobjects/PXDCluster.h>

#include <svd/dataobjects/SVDCluster.h>

#include <genfit/PlanarMeasurement.h>

#include <vector>
#include <string>
#include <utility> // std::pair

namespace Belle2 {

  /** SpacePoint typically is build from 1 PXDCluster or 1-2 SVDClusters.
   *
   *  It stores a global space point with its position error and some extra infos,
   *  such as a flag, if it is already assigned.
   *
   *  For SVD only: <br>
   *  If relations to its attached Clusters are set, these have got the following meaning: <br>
   *  - relationweights code the type of the cluster. +1 for u and -1 for v clusters.
   */
  class SpacePoint: public RelationsObject {

  public:
    //--- C'tors and D'tors -------------------------------------------------------------------------------------------
    /** Constructor SpacePoint from one PXD Hit.
     *
     *  @param pxdCluster   Pointer to PXDCluster (typically owned by the DataStore).
     *  @param aSensorInfo  Only for testing purposes.
     */
    SpacePoint(const PXDCluster* pxdCluster,
               const VXD::SensorInfoBase* aSensorInfo = nullptr);

    /** Constructor SpacePoint from ONE or TWO SVDClusters.
     *
     *  @param clusters            container carrying pointers to SVDCluster (1 or 2 (u+v), must not be nullptr).
     *  @param aSensorInfo         SensorInfoBase for testing purposes, usually derived from first cluster.
     */
    SpacePoint(std::vector<SVDCluster const*>& clusters,
               VXD::SensorInfoBase const* aSensorInfo = nullptr);

    /** Default constructor for the ROOT IO. */
    SpacePoint() :
      m_positionError(1., 1., 1.), //TODO: Describe Design Decision for not using the default (0.,0.,0.)
      m_vxdID(0), m_sensorType(VXD::SensorInfoBase::SensorType::VXD) // type is set to generic VXD
    {}

    /** Constructor for debugging or other special purposes.
     *
     *  @param pos                global SpacePoint position.
     *  @param posError           uncertainty on position.
     *  @param normalizedLocal    coordinates (element: [0;1]) of SP on given sensorID (.first "=" u, .second "=" v).
     *  @param clustersAssigned   states, if u (.first) or v (.second) is assigned.
     *  @param sensorID           VxdID of sensor the SpacePoint shall be on.
     *  @param detID              SensorType detector-type (PXD, SVD, ...) to be used.
     */
    SpacePoint(B2Vector3<double> pos, B2Vector3<double> posError, std::pair<double, double> normalizedLocal,
               std::pair<bool, bool> clustersAssigned, VxdID sensorID, Belle2::VXD::SensorInfoBase::SensorType detID) :
      m_position(pos), m_positionError(posError), m_normalizedLocal(normalizedLocal),
      m_clustersAssigned(clustersAssigned),
      m_vxdID(sensorID), m_sensorType(detID)
    {}

    /** Currently SpacePoint is used as base class for test beam related TBSpacePoint. */
    virtual ~SpacePoint() {}
    //-----------------------------------------------------------------------------------------------------------------

    /** overloaded '<<' stream operator. Print secID to stream by converting it to string */
    friend std::ostream& operator<< (std::ostream& out, const SpacePoint& aSP) { return out << aSP.getName();}

    /** Compare, if two SpacePoints are the same one.
    *
    *  As SpacePoints should live in the DataStore and comparing positions doesn't guarantee,
    *  that different underlying clusters are used, we compare here only the indices of the SpacePoint
    *  within their StoreArray --> SpacePoints are equal, if the share the index.
    */
    bool operator == (const SpacePoint& b) const
    {
      return getArrayIndex() == b.getArrayIndex();
    }

    /** Comparison for inequality with another SpacePoint.*/
    bool operator != (const SpacePoint& b) const
    {
      return !(*this == b);
    }

    /** Print out some info for this SpacePoint.*/
    std::string getName() const override
    {
      return "SpacePoint with index: " + std::to_string(getArrayIndex()) +
             "and VxdID: " + std::to_string(VxdID(m_vxdID));
    }

    //--- Global Coordinate Getters -----------------------------------------------------------------------------------
    /** return the x-value of the global position of the SpacePoint */
    double X() const { return m_position.X(); }

    /** return the y-value of the global position of the SpacePoint */
    double Y() const { return m_position.Y(); }

    /** return the z-value of the global position of the SpacePoint */
    double Z() const { return m_position.Z(); }

    /** return the position vector in global coordinates */
    const B2Vector3<double>& getPosition() const { return m_position; }

    /** return the hitErrors in sigma of the global position */
    const B2Vector3<double>& getPositionError() const { return m_positionError; }

    //---- Sensor Level Information Getters ---------------------------------------------------------------------------
    /** Return SensorType (PXD, SVD, ...) on which the SpacePoint lives.*/
    Belle2::VXD::SensorInfoBase::SensorType getType() const { return m_sensorType; }

    /** Return the VxdID of the sensor on which the the cluster of the SpacePoint lives.*/
    VxdID getVxdID() const { return m_vxdID; }

    /** Return normalized local coordinates of the cluster in u (0 <= posU <= 1).*/
    double getNormalizedLocalU() const { return m_normalizedLocal.first; }

    /** Return normalized local coordinates of the cluster in v (0 <= posV <= 1).*/
    double getNormalizedLocalV() const { return m_normalizedLocal.second; }
    //-----------------------------------------------------------------------------------------------------------------

    /** returns a vector of genfit::PlanarMeasurement, which is needed for genfit::track.
    *
    * This member ensures compatibility with genfit2.
    * The return type is detector independent,
    * but each entry will be of the same detector type,
    * since a spacePoint can not contain clusters of different sensors
    * and therefore of different detector types.
    */
    virtual std::vector<genfit::PlanarMeasurement> getGenfitCompatible() const ;

    /** returns the current state of assignment - returns true if it is assigned and therefore blocked for reuse. */
    bool getAssignmentState() const {return m_isAssigned; }



    /** returns which local coordinate is based on assigned Cluster (only relevant for SVD, for other types, this is always true.
    *
    * .first is true, if this SpacePoint has a UCluster (only relevant for SVD, PXD always true),
    * .second is true, if this SpacePoint has a VCluster (only relevant for SVD, PXD always true),
    */
    std::pair<bool, bool> getIfClustersAssigned() const {return m_clustersAssigned; }


    /** returns the number of Clusters assigned to this SpacePoint (0,1, or 2) */
    unsigned short getNClustersAssigned() const
    {
      if (m_sensorType == VXD::SensorInfoBase::SensorType::SVD) {
        unsigned short nClusters = 0;
        nClusters += m_clustersAssigned.first;
        nClusters += m_clustersAssigned.second;
        return nClusters;
      }
      if (m_clustersAssigned.first && m_clustersAssigned.second) return 1;
      return 0;
    }


    /** returns the current estimation for the quality of that spacePoint.
     *
     * returns value between 0-1, 1 means "good", 0 means "bad".
     * */
    unsigned int getQualityEstimation() const {return m_qualityIndicator; }



// static converter functions:

    /** converts a local hit on a given sensor into global coordinates.
     *
     * so this practically does what sensorInfo::pointToGlobal is doing, the difference is, that you do not need to have the sensorInfo beforehand (it will be retrieved using the VxdID)
     * first parameter is the local hit (as provided by getU and getV!) stored as a pair of doubles.
     * second parameter is the coded vxdID, which carries the sensorID.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the member gets its own pointer to it.
    *
    * ATTENTION: this function assumes, that for wedged sensors, the uCoordinate is already adapted to the vCoordinate!
     */
    static B2Vector3<double> getGlobalCoordinates(const std::pair<double, double>& hitLocal, VxdID vxdID,
                                                  const VXD::SensorInfoBase* aSensorInfo = nullptr);



    /** converts a local hit into sensor-independent relative coordinates.
       *
       * first parameter is the local hit (as provided by SpacePoint::getUWedged(...) and Cluster::getV!) stored as a pair of doubles.
       * second parameter is the coded vxdID, which carries the sensorID.
       * third parameter, a sensorInfo can be passed for testing purposes.
       *  If no sensorInfo is passed, the member gets its own pointer to it.
     *
     * ATTENTION: this function assumes, that for wedged sensors, the uCoordinate is already adapted to the vCoordinate!
     * The normalized coordinates are independent of wedged-sensor-issues
       */
    static std::pair<double, double> convertLocalToNormalizedCoordinates(const std::pair<double, double>& hitLocal,
        VxdID vxdID, const VXD::SensorInfoBase* aSensorInfo = nullptr);






    /** converts a hit in sensor-independent relative coordinates into local coordinate of given sensor.
    *
    * first parameter is the hit in sensor-independent normalized ! coordinates stored as a pair of floats.
    * second parameter is the coded vxdID, which carries the sensorID.
    * third parameter, a sensorInfo can be passed for testing purposes.
    *  If no sensorInfo is passed, the member gets its own pointer to it.
    */
    static std::pair<double, double> convertNormalizedToLocalCoordinates(const std::pair<double, double>& hitNormalized,
        Belle2::VxdID vxdID, const Belle2::VXD::SensorInfoBase* aSensorInfo = nullptr);



    /** takes a general uCoordinate, and transforms it to corrected uCoordinate for wedged sensors.
     *
     * Use this if you want to add the information of the vCluster to the local uPosition.
     * The returned value is now dependent of vCluster and valid only for this cluster!
     * This is only relevant for wedged/slanted sensors because of their trapezoidal shape, for rectangular shapes, the value does not change
     *
     */
    static double getUWedged(const std::pair<double, double>& hitLocalUnwedged, VxdID vxdID,
                             const VXD::SensorInfoBase* aSensorInfo = nullptr)
    {
      if (aSensorInfo == nullptr) { aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID); }
      return (aSensorInfo->getWidth(hitLocalUnwedged.second) / aSensorInfo->getWidth()) * hitLocalUnwedged.first;
    }



    /** takes a wedged uCoordinate, and transforms it to general uCoordinate.
     *
     * Use this if you want to "unwedge" your u-coordinate.
     * The returned value shall be like the value delivered by a uCluster without information of v (== aCluster.getPosition() ).
     * This is only relevant for wedged/slanted sensors because of their trapezoidal shape, for rectangular shapes, the value does not change
     */
    static double getUUnwedged(const std::pair<double, double>& hitLocalWedged, VxdID::baseType vxdID,
                               const VXD::SensorInfoBase* aSensorInfo = nullptr)
    {
      if (aSensorInfo == nullptr) { aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID); }
      return (aSensorInfo->getWidth() / aSensorInfo->getWidth(hitLocalWedged.second)) * hitLocalWedged.first;
    }



    /** checks first parameter for boundaries.
     *
     * does take second/third argument for checking for lower/upper boundary.
     * if boundary is crossed, value gets reset to boundary value
     * */
    static void boundaryCheck(double& value, double lower = 0, double higher = 1)
    {
      if (value < lower) {
        B2WARNING("SpacePoint::boundaryCheck: value had to be moved (lowerCheck)! old: " << value << ", new: " << lower);
        value = lower;
      }
      if (value > higher) {
        B2WARNING("SpacePoint::boundaryCheck: value had to be moved (higherCheck)! old: " << value << ", new: " << higher);
        value = higher;
      }

    }



// setter:

    /** sets the state of assignment - set true if it is assigned and therefore blocked for reuse. */
    void setAssignmentState(bool newState) const { m_isAssigned = newState; }



    /** sets the estimation for the quality of that spacePoint.
     *
     * set value between 0-1, 1 means "good", 0 means "bad".
     * */
    void setQualityEstimation(unsigned int newQI) {m_qualityIndicator = newQI; }


  protected:


    /** protected function to set the global position error.
     *
     * It takes care for the transformation of the local sigmas to global error values.
     */
    void setPositionError(double uSigma, double vSigma, const VXD::SensorInfoBase* aSensorInfo)
    {
      //As only variances, but not the sigmas transform linearly,
      // we need to use some acrobatics
      // (and some more (abs) since we do not really transform a vector).
      m_positionError = aSensorInfo->vectorToGlobal(
                          TVector3(
                            uSigma * uSigma,
                            vSigma * vSigma,
                            0
                          )
                        );
      m_positionError.Sqrt();
    }



    /** Global position vector.
     *
     *  [0]: x , [1] : y, [2] : z
     */
    B2Vector3<double> m_position;



    /** Error "Vector" of global position in sigma.
     *
     *  [0]: x-uncertainty , [1] : y-uncertainty, [2] : z-uncertainty
     */
    B2Vector3<double> m_positionError;



    /** Position in local coordinates normalized to the sensor size between 0 and 1.
     *
     *  First entry is u, second is v
     */
    std::pair<double, double> m_normalizedLocal;
//     double m_normalizedLocal[2];



    /** The bool value is true, when correct information of the coordinate exists.
     *
     *  .first is true, if this SpacePoint has a UCluster (only relevant for SVD, PXD always true),
     * .second is true, if this SpacePoint has a VCluster (only relevant for SVD, PXD always true),
     */
    std::pair<bool, bool> m_clustersAssigned {false, false};

    /** Stores the VxdID. */
    VxdID::baseType m_vxdID;

    /** Stores the SensorType using the scheme of SensorInfoBase.
     *
     *  Currently there are the following types possible:<br>
     *  PXD, SVD, TEL, VXD
     */
    VXD::SensorInfoBase::SensorType m_sensorType;

    /** Stores a quality indicator.
     *
     *  The value shall be between 0. and 1., where 1. means "good" and 0. means "bad".
     */
    double m_qualityIndicator {0.5};

    /** Stores whether this SpacePoint is connected to a track.
     *
     *  We assume, that const for SpacePoint means, things like position et cetera remain constant.
     *  The assignment status is therefore a mutable on purpose and SpacePoints can be const
     *  during tracking.
     */
    mutable bool m_isAssigned {false};

    ClassDefOverride(SpacePoint, 10) // last member changed: double float -> double!
  };
}
