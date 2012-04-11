/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDCLUSTER_H
#define SVDCLUSTER_H

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {

  /** The SVD Cluster class
   * This class stores all information about reconstructed SVD clusters
    *
    * \addtogroup dataobjects
   */
  class SVDCluster: public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    SVDCluster():
      m_sensorID(0), m_isU(true), m_position(0), m_clsTime(0), m_clsTimeSigma(1),
      m_clsCharge(0), m_seedCharge(0), m_clsSize(0)
    {}

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param isU True if u strips, otherwise false.
     * @param position Seed strip coordinate.
     * @param clsTime The average of waveform maxima times of strips in the cluster.
     * @param clsTimeSigma The standard deviation of waveform maxima times.
     * @param clsCharge The cluster charge in electrons.
     * @param seedCharge The charge of the seed strip in electrons.
     * @param clsSize The size of the cluster in the corresponding strip pitch units.
     */
    SVDCluster(VxdID sensorID, bool isU, float position, double clsTime,
               double clsTimeSigma, float seedCharge, float clsCharge, unsigned short clsSize):
      m_sensorID(sensorID), m_isU(isU), m_position(position), m_clsTime(clsTime),
      m_clsTimeSigma(clsTimeSigma), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge), m_clsSize(clsSize)
    {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get the direction of strips.
     * @return true if u-strips, otherwise false.
     */
    bool isUCluster() const { return m_isU; }


    /** Get the coordinate of seed strip.
     * @return absolute coordinate of the seed strip.
     */
    float getPosition() const { return m_position; }

    /** Get average of waveform maximum times of cluster strip signals.
     * @return average waveform maximum times.
     */
    float getClsTime() const { return m_clsTime; }

    /** Get standard deviation of waveform maximum times of cluster strip signals.
     * @return standard deviation of waveform maximum times.
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

  protected:
    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if clusters of u-strips, otherwise false. */
    float m_position;          /**< Absolute seed strip position in r-phi or z. */
    double m_clsTime;          /**< Average waveform maximum time. */
    double m_clsTimeSigma;     /**< Standard deviation of waveform maximum times.*/
    float m_clsCharge;         /**< Deposited charge in electrons. */
    float m_seedCharge;        /**< Cluster seed charge in electrons. */
    unsigned short m_clsSize;  /**< Cluster size in pixels */


    ClassDef(SVDCluster, 1)

  };
} //Belle2 namespace
#endif
