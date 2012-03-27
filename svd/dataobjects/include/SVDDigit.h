/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDIGIT_H
#define SVDDIGIT_H

#include <vxd/VxdID.h>

#include <root/TObject.h>

namespace Belle2 {

  /**
   * The SVD digit class.
   *
   * SVDDigit records data about signals in SVD strips.
   * This is a development implementation which is intentionally kept
   * somewhat bulky.
   * I record six samples of the waveform, and estimate of the waveform maximum.
   * I also record strip coordinates that won't be kept in future.
   */

  class SVDDigit : public TObject {

  public:

    enum {
      /** Number of waveform samples stored.*/
      WAVEFORM_SAMPLES = 6
    };

    /** Default constructor for the ROOT IO. */
    SVDDigit():
      m_sensorID(0), m_isU(true), m_cellID(0), m_cellPosition(0),
      m_charge(0), m_time(0)
    { for (int i = 0; i < WAVEFORM_SAMPLES; ++i) m_samples[i] = 0.0; }

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param isU True if v strip, false if v.
     * @param cellID Strip ID.
     * @param cellPosition Strip mid-line coordinate.
     * @param charge The charge collected on the strip.
     * @param time The time of waveform maximum.
     * @param samples Array of six consecutive samples of the strip signals.
     */
    SVDDigit(VxdID sensorID, bool isU, short cellID, float cellPosition,
             float charge, double time, const float samples[]):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID),
      m_cellPosition(cellPosition), m_charge(charge), m_time(time)
    { for (int i = 0; i < WAVEFORM_SAMPLES; ++i) m_samples[i] = samples[i]; }

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get strip direction.
     * @return true if u, false if v.
     */
    bool isUStrip() const { return m_isU; }

    /** Get strip ID.
     * @return ID of the strip.
     */
    short int getCellID() const { return m_cellID; }

    /** Get coordinate of strip midline.
     * @return coordinate of strip midline.
     */
    float getCellPosition() const { return m_cellPosition; }

    /** Get collected charge.
     * @return charge collected in the strip.
     */
    float getCharge() const { return m_charge; }

    /** Get time of waveform maximum.
     * @return time of waveform maximum.
     */
    double getTime() const { return m_time; }

    /** Get waveform samples.
     * @return array of waveform samples.
     */
    const float* getSamples() const { return m_samples; }

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if U, false if V. */
    short m_cellID;            /**< Strip coordinate in pitch units. */
    float m_cellPosition;      /**< Absolute strip position. */
    float m_charge;            /**< Strip signal at waveform maximum. */
    double m_time;             /**< Time of waveform maximum. */
    float m_samples[WAVEFORM_SAMPLES];        /**< WAVEFORM_SAMPLES samples of the strip signal (30 ns sampling time. */


    ClassDef(SVDDigit, 1)

  }; // class SVDDigit


} // end namespace Belle2

#endif
