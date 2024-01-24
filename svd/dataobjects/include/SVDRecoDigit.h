/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_RECODIGIT_H
#define SVD_RECODIGIT_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <cstdint>
#include <sstream>
#include <string>
#include <algorithm>
#include <numeric>

namespace Belle2 {

  /**
   * The SVD RecoDigit class.
   *
   * The SVDRecoDigit is a calibrated and time-fitted strip signal.
   * It contains strip identification data, plus fit
   * and fit quality information.
   * NB:
   * This class will be _bulky_ and is not intended for storage beyond the
   * current event.
   * NB:
   * I tried to make the class as little dependent on the waveform fitter
   * as possible. However, the current fitter produces a pdf for signal time
   * and this may not be the case with a different fitter.
   */

  class SVDRecoDigit : public RelationsObject {

  public:

    /** Types for internal storage of probability array.
     */
    typedef uint16_t StoredProbType;
    typedef std::vector<StoredProbType> StoredProbArray; /**< vector of StoreProbType objects*/
    static const uint16_t storedProbArrayNorm = UINT16_MAX; /**<some constant */

    /** Type for output probability array.
     * There is no link between this and what the fitter may produce.
     */
    typedef double OutputProbType;
    typedef std::vector<OutputProbType> OutputProbArray; /**< vector of OutProbType objects*/

    /** Constructor using a stl container of time bin probabilities.
     * @param sensorID Sensor VXD ID.
     * @param isU True if u strip, false if v.
     * @param cellID Strip ID.
     * @param fittedAmplitude amplitude estimate for the signal.
     * @param fittedAmplitudeError amplitude error estimate.
     * @param fittedTime fitted time-of-arrival estimate.
     * @param fittedTimeError time error estimate.
     * @param probabilities pdf for time estimate.
     * @param chi2 Standardized chi2 for the fit.
     */
    template<typename T>
    SVDRecoDigit(VxdID sensorID, bool isU, short cellID, float fittedAmplitude,
                 float fittedAmplitudeError, float fittedTime, float fittedTimeError,
                 const T& probabilities, float chi2):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID),
      m_fittedAmplitude(fittedAmplitude), m_fittedAmplitudeError(fittedAmplitudeError),
      m_fittedTime(fittedTime), m_fittedTimeError(fittedTimeError), m_fitChi2Ndf(chi2)
    {
      // Expecting input array normalized to 1, but don't rely on it.
      double inputNorm = std::accumulate(probabilities.begin(), probabilities.end(), 0.0);
      if (inputNorm < 0.1) inputNorm = 1.0;
      double normCoef = static_cast<double>(storedProbArrayNorm) / inputNorm;
      std::transform(probabilities.begin(), probabilities.end(),
                     std::back_inserter(m_probabilities),
                     [normCoef](typename T::value_type x)->StoredProbType
      { return static_cast<StoredProbType>(normCoef * x); }
                    );
    }

    /** Default constructor for the ROOT IO. */
    SVDRecoDigit() : SVDRecoDigit(
        0, true, 0, 0.0, 10.0, 0.0, 100.0, std::vector<double>({1.0}), 100.0
    )
    { }

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get raw sensor ID.
     * For use in Python
     * @return basetype ID of the sensor.
     */
    VxdID::baseType getRawSensorID() const { return m_sensorID; }

    /** Get strip direction.
     * @return true if u, false if v.
     */
    bool isUStrip() const { return m_isU; }

    /** Get strip ID.
     * @return ID of the strip.
     */
    short int getCellID() const { return m_cellID; }

    /** Get amplitude estimate
     * @return fitted amplitude in e-
     */
    float getAmplitude() const { return m_fittedAmplitude; }

    /** Get amplitude estimate, alternate getter name
     * @return fitted amplitude in e-
     */
    float getCharge() const { return m_fittedAmplitude; }

    /** Get amplitude error
     * @return amplitude error estimate in e-
     */
    float getAmplitudeError() const { return m_fittedAmplitudeError; }

    /** Get time estimate
     * @return fitted signal arrival time
     */
    float getTime() const { return m_fittedTime; }

    /** Get time error
     * @return time error estimate
     */
    float getTimeError() const { return m_fittedTimeError; }

    /** Get signal time pdf.
     * @return array containing bin probabilities, normalized to 1.
     */
    OutputProbArray getProbabilities() const
    {
      OutputProbArray outputPdf;
      // We need to get the norm, may be different
      int storeNorm = std::accumulate(m_probabilities.begin(), m_probabilities.end(), 0.0,
                                      [](int x, StoredProbType y) -> int { return x + y; });
      double normCoef = static_cast<double>(1.0 / storeNorm);
      if (normCoef < 1.0e-15) normCoef = 1.0;
      std::transform(m_probabilities.begin(), m_probabilities.end(),
                     std::back_inserter(outputPdf),
                     [normCoef](StoredProbType x)-> OutputProbType
      { return static_cast<OutputProbType>(normCoef * x); }
                    );
      return outputPdf;
    }

    /** Get waveform fit chi2/ndf
     * @return chi2/ndf for the fit
     */
    float getChi2Ndf() const { return m_fitChi2Ndf; }

    /** Display main parameters in this object */
    std::string toString() const
    {
      VxdID thisSensorID(m_sensorID);

      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID) << " strip: "
         << ((m_isU) ? "U-" : "V-") << m_cellID
         << " Amplitude: " << m_fittedAmplitude << " +/- " << m_fittedAmplitudeError
         << " Time: " << m_fittedTime << " +/- " << m_fittedTimeError << std::endl
         << " probabilities: ";
      std::copy(m_probabilities.begin(), m_probabilities.end(),
                std::ostream_iterator<StoredProbType>(os, " "));
      os << "Chi2/ndf: " << m_fitChi2Ndf << std::endl;
      return os.str();
    }

  private:

    VxdID::baseType m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU; /**< True if U, false if V. */
    short m_cellID; /**< Strip coordinate in pitch units. */
    //    float m_stripNoise; /**< Noise of the strip, from calibration. */
    float m_fittedAmplitude; /**< Fitted amplitude of the signal ("charge") */
    float m_fittedAmplitudeError; /**< Error estimate of amplitude fit. */
    float m_fittedTime; /**< Fitted arrival time of the signal. */
    float m_fittedTimeError; /**< Error estimate of time fit. */
    StoredProbArray m_probabilities; /**< pdf of the time estimate. */
    float m_fitChi2Ndf; /**< Standardized chi2 of the fit. */

    ClassDef(SVDRecoDigit, 3)

  }; // class SVDRecoDigit

} // end namespace Belle2

#endif // SVD_RECODIGIT_H
