/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio, Giulia Casarosa                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SVDCOGTIMEESTIMATOR_H
#define SVDCOGTIMEESTIMATOR_H


#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>

namespace Belle2 {

  /** This module builds the SVDRecoDigits (calibrated and fitted strips)
   * from the SVDShaperDigits.
   * The time of the hit is estimated as the weighted average of the
   * samples' time substracted by an offset sensor-depending;
   * the charge of the strips is estimated as the charge
   * of the highest sample
   */

  class SVDCoGTimeEstimatorModule : public Module {
  public:

    SVDCoGTimeEstimatorModule();

    virtual ~SVDCoGTimeEstimatorModule();

    /** Initialize the SVDCoGTimeEstimator.*/
    virtual void initialize() override;

    /** Called when entering a new run.     */
    virtual void beginRun() override;

    /** This method is the core of the SVDCoGTimeEstimator. */
    virtual void event() override;

    /** This method is called if the current run ends. */
    virtual void endRun() override;

    /** This method is called at the end of the event processing.   */
    virtual void terminate() override;


  private:

    /** vector containing the 6/3 samples*/
    Belle2::SVDShaperDigit::APVFloatSamples m_Samples_vec;

    /** The peak time estimation */
    float m_weightedMeanTime;
    /** The peak time estimation error */
    float m_weightedMeanTimeError;

    /** The shaper amplitude estimation */
    float m_amplitude;
    /** The shaper amplitude estimation error */
    float m_amplitudeError;

    /** Probabilities, to be defined here */
    std::vector<float> m_probabilities;
    /** Chi2, to be defined here */
    float m_chi2;

    /** Time width of a sampling */
    float DeltaT = 31.44; //ns

  protected:

    /** Name of the collection to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;
    /** Name of the collection to use for the SVDRecoDigits */
    std::string m_storeRecoDigitsName;
    /** Name of the relation between SVDRecoDigits and SVDShaperDigits */
    std::string m_relRecoDigitShaperDigitName;
    /** Width of the distribution of the times after having substracted the TriggerBin and the CalibrationPeakTime */
    float m_FinalShiftWidth;
    /** Approximate ADC error on each sample */
    float m_AmplitudeArbitraryError;

    /** Function to calculate the peak time, obtained as the weighted mean of the time of the samples, weighted with the amplitude of each sample */
    float CalculateWeightedMeanPeakTime(Belle2::SVDShaperDigit::APVFloatSamples samples);
    /** Function to calculate the peak time error, obtained as 1/10 of the time itself */
    float CalculateAmplitude(Belle2::SVDShaperDigit::APVFloatSamples samples);
    /** Function to calculate the amplitude of the shaper, obtained as the mean of the 6 samples */
    float CalculateWeightedMeanPeakTimeError();
    /** Function to calculate the amplitude error, obtained as 1/10 of the amplitude itself */
    float CalculateAmplitudeError(VxdID ThisSensorID, bool ThisSide, int ThisCellID);
    /** Function to calculate probabilities, that is not used here, so just set at 0.99 */
    void CalculateProbabilities();
    /** Function to calculate chi2, that is not used here, so just set at 0.01 */
    float CalculateChi2();

    //calibration objects
    SVDPulseShapeCalibrations m_PulseShapeCal;
    SVDNoiseCalibrations m_NoiseCal;

  };
}
#endif


























