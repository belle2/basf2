/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDCLUSTER_H
#define SVDCLUSTER_H

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoCache.h>

#include <sstream>
#include <vector>

namespace Belle2 {

  /** The SVD Cluster class
   * This class stores all information about reconstructed SVD clusters.
   */
  class SVDCluster: public RelationsObject {
  public:

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param isU true if u strips, otherwise false.
     * @param position Hit position estimate.
     * @param positionSigma Error of hit postion estimate.
     * @param clsTime Hit time stimate.
     * @param clsTimeSigma Error of hit time estimate.
     * @param clsCharge The cluster charge in ADC units.
     * @param seedCharge The charge of the seed strip in ADC units.
     * @param clsSize Cluster size, number of fired strips that form the cluster.
     * @param clsSN Signal-to-noise ratio for the cluster
     * @param clsChi2 value for cluster fit
     * @param firstFrame
     */
    SVDCluster(VxdID sensorID, bool isU, float position, float positionSigma,
               double clsTime, double clsTimeSigma, float clsCharge, float seedCharge,
               unsigned short clsSize, float clsSN, float clsChi2, int firstFrame = 0):
      m_sensorID(sensorID), m_isU(isU),
      m_position(position), m_positionSigma(positionSigma),
      m_clsTime(clsTime), m_clsTimeSigma(clsTimeSigma), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge), m_clsSize(clsSize), m_clsSN(clsSN),
      m_clsChi2(clsChi2), m_firstFrame(firstFrame)
    {}

    /** Default constructor for the ROOT IO. */
    SVDCluster(): SVDCluster(0, false, 0.0, 1.0, 0.0, 100.0, 0.0, 0.0, 0, 1.0,
                               100.0)
    {}

    /** OLD STYLE 8-params constructor, just for backward compatibility.
     * Will be removed in future.
     * @param sensorID Sensor compact ID.
     * @param isU true if u strips, otherwise false.
     * @param position Hit position estimate.
     * @param clsTime Hit time stimate.
     * @param clsTimeSigma Error of hit time estimate.
     * @param clsCharge The cluster charge in ADC units.
     * @param seedCharge The charge of the seed strip in ADC units.
     * @param clsSize Cluster size, number of fired strips that form the cluster.
     */
    SVDCluster(VxdID sensorID, bool isU, float position, double clsTime,
               double clsTimeSigma, float clsCharge, float seedCharge,
               unsigned short clsSize):
      SVDCluster(sensorID, isU, position, 1.0, clsTime, clsTimeSigma,
                 clsCharge, seedCharge, clsSize, 1.0, 100.0)
    {}

    /** OLD STYLE Constructor, just for backward compatibility.
     * @param sensorID Sensor compact ID.
     * @param isU true if u strips, otherwise false.
     * @param position Hit position estimate.
     * @param positionSigma Error of hit postion estimate.
     * @param clsTime Hit time stimate.
     * @param clsTimeSigma Error of hit time estimate.
     * @param clsCharge The cluster charge in ADC units.
     * @param seedCharge The charge of the seed strip in ADC units.
     * @param clsSize Cluster size, number of fired strips that form the cluster.
     * @param clsSN Signal-to-noise ratio for the cluster
     * Will be removed in future.
     */
    SVDCluster(VxdID sensorID, bool isU, float position, float positionSigma,
               double clsTime, double clsTimeSigma, float clsCharge, float seedCharge,
               unsigned short clsSize, float clsSN):
      SVDCluster(sensorID, isU, position, positionSigma, clsTime, clsTimeSigma,
                 clsCharge, seedCharge, clsSize, clsSN, 100.0)
    {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get raw sensor ID */
    unsigned short getRawSensorID() const { return m_sensorID; }

    /** Get the direction of strips.
     * @return true if u-strips, otherwise false.
     */
    bool isUCluster() const { return m_isU; }

    /** Get the coordinate of reconstructed hit.
     * The returned value is now dependent of vCluster and valid only and
     * it is only relevant for wedged/slanted sensors because of their trapezoidal shape, for rectangular shapes, the value does not change
     * @return coordinate of the reconstructed hit.
     */
    float getPosition(double v = 0) const
    {
      if (v == 0) return m_position;
      else {
        const VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_sensorID);
        return (aSensorInfo->getWidth(v) / aSensorInfo->getWidth()) * m_position;
      }
    }

    /** Get the error of the reconstructed hit coordinate.
     * @return error of the reconstructed hit coordinate.
     */
    float getPositionSigma() const { return m_positionSigma; }

    /** Get average of waveform maximum times of cluster strip signals.
     * @return average waveform maximum times.
     */
    float getClsTime() const { return m_clsTime; }

    /** Get the error of the reconstructed hit time.
     * @return error of the reconstructed hit time.
     */
    float getClsTimeSigma() const { return m_clsTimeSigma; }

    /** Get collected charge.
     * @return charge collected in the cluster in e-.
     */
    float getCharge() const { return m_clsCharge; }

    /** Get seed charge.
     * @return charge of the seed strip in e-.
     */
    float getSeedCharge() const { return m_seedCharge; }

    /** Get cluster size.
     * @return number of strips contributing to the cluster.
     */
    unsigned short getSize() const { return m_clsSize; }

    /** Get cluster SNR.
     * @return cluster charge / std error of cluster charge
     */
    float getSNR() const { return m_clsSN; }

    /** Get chi2 of the time/amplitude fit.
     * @return Chi2/ndf for the fit
     */
    float getChi2() const { return m_clsChi2; }

    /** Get firstFrame of the MaxSum algorithm.
     * @return firstFrame of the MaxSum algorithm
     */
    int getFirstFrame() const { return m_firstFrame; }

    /** Get cluster quality indicator
    * @return probability that cluster is generated from signal hit.
    */
    double getQuality() const { return m_qualityIndicator; }

    /** Get cluster quality indicator error
    * @return error in probability that cluster is generated from signal hit.
    */
    double getQualityError() const { return m_qualityIndicatorError; }

    /** Get ID of the time-group.
     * @return time-group ID
     */
    const std::vector<int>& getTimeGroupId() const { return m_timeGroupId; }

    /** Get time-group parameters.
     * @return time-group parameters (integral, center, sigma)
     */
    const std::vector<std::tuple<float, float, float>>& getTimeGroupInfo() const { return m_timeGroupInfo; }

    /** Set position of the cluster
    *
    */
    void setPosition(const double position)
    {
      m_position = position;
    }

    /** Set time of the cluster
    *
    */
    void setClsTime(const double time)
    {
      m_clsTime = time;
    }

    /** Add quality indicator to object.
    *
    */
    void setQualityIndicator(const double qualityIndicator)
    {
      m_qualityIndicator = qualityIndicator;
    }

    /** Add quality indicator error to object.
    *
    */
    void setQualityIndicatorError(double qualityIndicatorError)
    {
      m_qualityIndicatorError = qualityIndicatorError;
    }

    /** Set ID of the time-group.
     * @return reference to time-group ID
     */
    std::vector<int>& setTimeGroupId() { return m_timeGroupId; }

    /** Set time-group parameters.
     * @return reference to the time-group parameters (integral, center, sigma)
     */
    std::vector<std::tuple<float, float, float>>& setTimeGroupInfo() { return m_timeGroupInfo; }

    /** Get a string representation of the cluster. */
    std::string print() const
    {
      VxdID thisSensorID(m_sensorID);
      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID)
         << " side: " << ((m_isU) ? "U" : "V") << " position: " << m_position
         << " +/- " << m_positionSigma << " time: " << m_clsTime << " +/- "
         << m_clsTimeSigma << " charge: " << m_clsCharge << " seed charge: "
         << m_seedCharge << " size: " << m_clsSize << " S/N: " << m_clsSN
         << " Fit Chi2: " << m_clsChi2 <<  " Signal Prob.: " << m_qualityIndicator
         << " Signal Prob. Error " << m_qualityIndicatorError << std::endl;
      return os.str();
    }

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if clusters of u-strips, otherwise false. */
    float m_position;          /**< Reconstructed hit position in r-phi or z. */
    float m_positionSigma;     /**< Standard deviation of hit position in r-phi or z.*/
    double m_clsTime;          /**< Average waveform maximum time. */
    double m_clsTimeSigma;     /**< Standard deviation of waveform maximum times.*/
    float m_clsCharge;         /**< Deposited charge in electrons. */
    float m_seedCharge;        /**< Cluster seed charge in electrons. */
    unsigned short m_clsSize;  /**< Cluster size in pixels */
    float m_clsSN;             /**< Cluster S/N ratio */
    float m_clsChi2;           /**< Chi2 for time/amplitude fit */
    int m_firstFrame;           /**< firstFrame computed with the MaxSum algorithm */
    std::vector<int> m_timeGroupId;    /**< Grouping of clusters in time */
    std::vector<std::tuple<float, float, float>> m_timeGroupInfo; /**< TimeGroup Gaussian Parameters,
                   (integral, center, sigma) */
    double m_qualityIndicator = {0.};  /**< Probability of signal cluster */
    double m_qualityIndicatorError = {0.};  /**< Error associated with probability calcualtion */

    ClassDef(SVDCluster, 8)

  };


} //Belle2 namespace
#endif
