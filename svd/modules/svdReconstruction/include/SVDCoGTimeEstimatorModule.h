/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <string>

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

    /** Container for a RelationArray Lookup table */
    typedef std::vector<const RelationElement*> RelationLookup;

    /** Constructor defining the parameters */
    SVDCoGTimeEstimatorModule();

    /** default destructor*/
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

    /** store arrays*/
    StoreArray<SVDShaperDigit> m_storeShaper; /**<SVDShaperDigits Store Array*/
    StoreArray<SVDRecoDigit> m_storeReco; /**<SVDRecoDigits store array*/

    StoreArray<SVDTrueHit> m_storeTrueHits; /**<SVDTrueHits store array*/
    StoreArray<MCParticle> m_storeMCParticles; /**<MCParticles Store array*/

    StoreObjPtr<SVDEventInfo> m_storeSVDEvtInfo; /**<storage for SVDEventInfo object */

    /** The peak time estimation */
    float m_weightedMeanTime = 0;
    /** The peak time estimation error */
    float m_weightedMeanTimeError = 0;

    /** The shaper amplitude estimation */
    float m_amplitude = 0;
    /** The shaper amplitude estimation error */
    float m_amplitudeError = 0;

    /** Chi2, to be defined here */
    float m_chi2 = 0;

    /** Time width of a sampling */
    float m_DeltaT = 31.44; //ns

    /** To stop creation of the SVDShaperDigit if something is wrong */
    bool m_StopCreationReco = false;

  protected:

    /** Create lookup maps for relations
     * FIXME: This has to be significantly simplified here, we just copy the
     * relations, since there is a 1:1 correspondence. We don't even have to do
     * the copying digit-by-digit.
     *
     * We do not use the RelationIndex as we know much more about the
     * relations: we know the relations get created in a consolidated way by
     * the Digitizer and that they already point in the right direction so we
     * only need to speed up finding the correct element. We just create a
     * vector from digit id to relationElement pointer for fast lookup
     * @param relation RelationArray to build the lookuptable for
     * @param lookup Lookuptable to fill
     * @param digits number of digits in this event
     */
    void createRelationLookup(const RelationArray& relation, RelationLookup& lookup,
                              size_t digits);

    /** Add the relation from a given SVDShaperDigit index to a map
       * @param lookup Lookuptable to use for the relation
       * @param relation map to add the entries to
       * @param index index of the SVDDigit
       */
    void fillRelationMap(const RelationLookup& lookup, std::map<unsigned int, float>&
                         relation, unsigned int index);

    // Relation lookups
    /** Lookup table for SVDShaperDigit->MCParticle relation */
    RelationLookup m_mcRelation;
    /** Lookup table for SVDShaperDigit->SVDTrueHit relation */
    RelationLookup m_trueRelation;

    /** Name of the SVDEventInfo object */
    std::string m_svdEventInfoName = "SVDEventInfo";
    /** Name of the collection to use for the MCParticles */
    std::string m_storeMCParticlesName = "MCParticles";
    /** Name of the collection to use for the SVDTrueHits */
    std::string m_storeTrueHitsName = "SVDTrueHits";
    /** Name of the collection to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName = "SVDShaperDigits";
    /** Name of the collection to use for the SVDRecoDigits */
    std::string m_storeRecoDigitsName = "SVDRecoDigits";
    /** Name of the relation between SVDRecoDigits and SVDShaperDigits */
    std::string m_relRecoDigitShaperDigitName = "";

    /** Parameters for the corrections */
    bool m_calEventT0 = true; /**< calibration with EventT0*/
    bool m_corrPeakTime = true; /**< correction of peakTime per strip from local calibrations*/

    /** Name of the relation between SVDShaperDigits and MCParticles */
    std::string m_relShaperDigitMCParticleName = "";
    /** Name of the relation between SVDShaperDigits and SVDTrueHits */
    std::string m_relShaperDigitTrueHitName = "";
    /** Name of the relation between SVDRecoDigits and MCParticles */
    std::string m_relRecoDigitMCParticleName = "";
    /** Name of the relation between SVDRecoDigits and SVDTrueHits */
    std::string m_relRecoDigitTrueHitName = "";

    /** Approximate ADC error on each sample */
    float m_AmplitudeArbitraryError = 10;

    /** Function to calculate the peak time, obtained as the weighted mean of the time of the samples, weighted with the amplitude of each sample */
    float CalculateWeightedMeanPeakTime(Belle2::SVDShaperDigit::APVFloatSamples samples);
    /** Function to calculate the amplitude of the shaper, obtained as the largest of the 6 samples */
    float CalculateAmplitude(Belle2::SVDShaperDigit::APVFloatSamples samples);
    /** Function to calculate the peak time error */
    float CalculateWeightedMeanPeakTimeError(Belle2::SVDShaperDigit::APVFloatSamples samples);
    /** Function to calculate the amplitude error as the noise of the strip */
    float CalculateAmplitudeError(VxdID ThisSensorID, bool ThisSide, int ThisCellID);
    /** Function to calculate chi2, that is not used here, so just set at 0.01 */
    float CalculateChi2();

    //calibration objects
    SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShaper calibrations db object*/
    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibrations db object*/
    SVDCoGTimeCalibrations m_TimeCal; /**< SVD CoG Time calibrations db object*/

    /** number of samples*/
    int m_NumberOfAPVSamples = 6;

  };
}



























