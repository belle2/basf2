/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/geometry/B2Vector3.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <svd/geometry/SensorInfo.h>

#include <tracking/dataobjects/DATCONSimpleSVDCluster.h>
#include <tracking/dataobjects/DATCONSVDDigit.h>

#include <vxd/dataobjects/VxdID.h>

#include <genfit/PlanarMeasurement.h>

#include <vector>
#include <string>
#include <utility> // std::pair

namespace Belle2 {

  /** DATCONSVDSpacePoint typically is build from 1-2 DATCONSimpleSVDClusters.
  *
  *  It stores a global space point and some extra infos,
  *  such as a flag if it is already assigned.
  *
  *  For SVD only: <br>
  *  If relations to its attached Clusters are set, these have got the following meaning: <br>
  *  - relationweights code the type of the cluster. +1 for u and -1 for v clusters.
  */
  class DATCONSVDSpacePoint: public RelationsObject {

  public:
    //--- C'tors and D'tors -------------------------------------------------------------------------------------------
    /** Constructor DATCONSVDSpacePoint from ONE or TWO DATCONSimpleSVDClusters.
    *
    *  @param clusters            container carrying pointers to DATCONSimpleSVDCluster (1 or 2 (u+v), must not be nullptr).
    *  @param aSensorInfo         SensorInfoBase for testing purposes, usually derived from first cluster.
    */
    DATCONSVDSpacePoint(std::vector<DATCONSimpleSVDCluster const*>& clusters,
                        VXD::SensorInfoBase const* aSensorInfo = nullptr);

    /** Default constructor for the ROOT IO. */
    DATCONSVDSpacePoint() :
      m_vxdID(0), m_sensorType(VXD::SensorInfoBase::SensorType::VXD) // type is set to generic VXD
    {}

    /** Constructor for debugging or other special purposes.
    *
    *  @param pos                global DATCONSVDSpacePoint position.
    *  @param normalizedLocal    coordinates (element: [0;1]) of SP on given sensorID (.first "=" u, .second "=" v).
    *  @param clustersAssigned   states, if u (.first) or v (.second) is assigned.
    *  @param sensorID           VxdID of sensor the DATCONSVDSpacePoint shall be on.
    *  @param detID              SensorType detector-type (PXD, SVD, ...) to be used.
    */
    DATCONSVDSpacePoint(B2Vector3<double> pos, std::pair<double, double> normalizedLocal,
                        std::pair<bool, bool> clustersAssigned, VxdID sensorID, Belle2::VXD::SensorInfoBase::SensorType detID) :
      m_position(pos),
      m_normalizedLocal(normalizedLocal),
      m_clustersAssigned(clustersAssigned),
      m_vxdID(sensorID), m_sensorType(detID)
    {}

    /** Default Destructor. */
    virtual ~DATCONSVDSpacePoint() {}
    //-----------------------------------------------------------------------------------------------------------------

    /** overloaded '<<' stream operator. Print secID to stream by converting it to string */
    friend std::ostream& operator<< (std::ostream& out, const DATCONSVDSpacePoint& aSP) { return out << aSP.getName();}

    /** Compare, if two DATCONSVDSpacePoints are the same one.
    *
    *  As DATCONSVDSpacePoints should live in the DataStore and comparing positions doesn't guarantee,
    *  that different underlying clusters are used, we compare here only the indices of the DATCONSVDSpacePoint
    *  within their StoreArray --> DATCONSVDSpacePoints are equal, if the share the index.
    */
    bool operator == (const DATCONSVDSpacePoint& b) const
    {
      return getArrayIndex() == b.getArrayIndex();
    }

    /** Comparison for inequality with another DATCONSVDSpacePoint.*/
    bool operator != (const DATCONSVDSpacePoint& b) const
    {
      return !(*this == b);
    }

    //--- Global Coordinate Getters -----------------------------------------------------------------------------------
    /** return the x-value of the global position of the DATCONSVDSpacePoint */
    double X() const { return m_position.X(); }

    /** return the y-value of the global position of the DATCONSVDSpacePoint */
    double Y() const { return m_position.Y(); }

    /** return the z-value of the global position of the DATCONSVDSpacePoint */
    double Z() const { return m_position.Z(); }

    /** return the position vector in global coordinates */
    const B2Vector3<double>& getPosition() const { return m_position; }

    //---- Sensor Level Information Getters ---------------------------------------------------------------------------
    /** Return SensorType (PXD, SVD, ...) on which the DATCONSVDSpacePoint lives.*/
    Belle2::VXD::SensorInfoBase::SensorType getType() const { return m_sensorType; }

    /** Return the VxdID of the sensor on which the the cluster of the DATCONSVDSpacePoint lives.*/
    VxdID getVxdID() const { return m_vxdID; }

    /** Return normalized local coordinates of the cluster in u (0 <= posU <= 1).*/
    double getNormalizedLocalU() const { return m_normalizedLocal.first; }

    /** Return normalized local coordinates of the cluster in v (0 <= posV <= 1).*/
    double getNormalizedLocalV() const { return m_normalizedLocal.second; }
    //-----------------------------------------------------------------------------------------------------------------

    /** Returns, if u(v)-coordinate is based on cluster information.
    *
    * .first is true, if u-cluster information is present<br>
    * .second is true, if v-cluster information is present.
    */
    std::pair<bool, bool> getIfClustersAssigned() const { return m_clustersAssigned; }

    /** Returns the number of Clusters assigned to this DATCONSVDSpacePoint.*/
    unsigned short getNClustersAssigned() const
    {
      if (m_sensorType == VXD::SensorInfoBase::SensorType::SVD &&
          m_clustersAssigned.first && m_clustersAssigned.second) {
        return 2;
      }
      return 1;
    }

    /** Setter for association with a track.*/
    void setAssignmentState(bool isAssigned) const { m_isAssigned = isAssigned; }

    /// Returns true if the SP is single clustered and the cluster is a u cluster
    bool isUOnly() const { return m_clustersAssigned.first and not m_clustersAssigned.second; }
    /// Returns true if the SP is single clustered and the cluster is a v cluster
    bool isVOnly() const { return not m_clustersAssigned.first and m_clustersAssigned.second; }
    /// Returns true if the SP is not single clustered
    bool isUAndV() const { return m_clustersAssigned.first and m_clustersAssigned.second; }

    /** Getter for status of assignment to a track.*/
    bool getAssignmentState() const { return m_isAssigned; }


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
      * first parameter is the local hit (as provided by DATCONSVDSpacePoint::getUWedged(...) and Cluster::getV!) stored as a pair of doubles.
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
    * function kept only for backward compatibility with VXDTF1
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



    /** Enforce  @param value in the  range [ @param lower, @param higher ].
    * param = min ( max( param,lower)  ,higher )
    * @param value is the coordinate that must be constrained in the range
    * @param lower is the lower limit of the prescribed range
    * @param higher is the upper limit of the prescribed range
    * @param otherValue is for debugging and logging purposes
    * @param side is for debugging purposes: 0 for U side 1 for V side
    * @paramvxdID vxdID is for debugging purposes
    * */
    static void boundaryEnforce(double& value, const double& otherValue, double lower = 0, double higher = 1, unsigned int side = 0,
                                VxdID vxdID = VxdID())
    {
      // Times to times there are normalized coordinates that are out of the boundaries.
      // We do apply a smal sloppyness here

      double sloppyTerm = 1e-3;
      if (value < lower - sloppyTerm) {
        B2WARNING("DATCONSVDSpacePoint::boundaryEnforce: value had to be moved (lowerCheck)! old: " << value << ", new: " << lower);
        B2WARNING("On sensor: " << vxdID << " side: " << (side == 0 ? " U " : " V") <<
                  " when the other coordinate is: " << otherValue);

        value = lower;
      }
      if (value > higher + sloppyTerm) {
        B2WARNING("DATCONSVDSpacePoint::boundaryEnforce: value had to be moved (higherCheck)! old: " << value << ", new: " << higher);
        B2WARNING("On sensor: " << vxdID << " side: " << (side == 0 ? " U " : " V") <<
                  " when the other coordinate is: " << otherValue);
        value = higher;
      }

    }
    //---------------------------------------------------------------------------------------------------------------------

  protected:
    //--- Member variables --------------------------------------------------------------------------------------------
    /** Global position vector.
    *
    *  [0]: x, [1] : y, [2] : z
    */
    B2Vector3<double> m_position;

    /** Local position vector normalized to sensor size (0 <= x <= 1).
    *
    *  .first: u, .second: v
    */
    std::pair<double, double> m_normalizedLocal;

    /** The bool value is true, when correct information of the coordinate exists.
    *
    *  .first is true, if this DATCONSVDSpacePoint has a UCluster (only relevant for SVD, PXD always true),
    * .second is true, if this DATCONSVDSpacePoint has a VCluster (only relevant for SVD, PXD always true),
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

    /** Stores whether this DATCONSVDSpacePoint is connected to a track.
    *
    *  We assume, that const for DATCONSVDSpacePoint means, things like position et cetera remain constant.
    *  The assignment status is therefore a mutable on purpose and DATCONSVDSpacePoints can be const
    *  during tracking.
    */
    mutable bool m_isAssigned {false};


    ClassDef(DATCONSVDSpacePoint, 1)
  }; // end class
} // end namespace Belle2
