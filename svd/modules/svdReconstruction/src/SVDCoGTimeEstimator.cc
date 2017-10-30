/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio, Giulia Casarosa                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDCoGTimeEstimator.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>

#include <string>
#include "TMath.h"
#include <algorithm>
#include <functional>


using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDCoGTimeEstimator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDCoGTimeEstimatorModule::SVDCoGTimeEstimatorModule() : Module()
{
  setDescription("From SVDShaperDigit to SVDRecoDigit. Strip charge is evaluated as the max of the 6 samples; hit time is evaluated as a shifted Centre of Gravity (CoG) time.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string(""));
  addParam("RecoDigits", m_storeRecoDigitsName,
           "RecoDigits collection name", string(""));
  addParam("FinalShiftWidth", m_FinalShiftWidth, "Width of the 3rd (final) time shift", float(6.0));

}


SVDCoGTimeEstimatorModule::~SVDCoGTimeEstimatorModule()
{
}


void SVDCoGTimeEstimatorModule::initialize()
{
  //Inizialization of useful store array
  StoreArray<SVDShaperDigit> storeShaper(m_storeShaperDigitsName);
  storeShaper.isRequired();

  //Initialize the new RecoDigit
  StoreArray<SVDRecoDigit> storeReco(m_storeRecoDigitsName);
  storeReco.registerInDataStore();

  RelationArray relRecoDigitShaperDigits(storeReco, storeShaper);
  relRecoDigitShaperDigits.registerInDataStore();

  //Store names to speed up creation later
  m_storeRecoDigitsName = storeReco.getName();
  m_storeShaperDigitsName = storeShaper.getName();
  m_relRecoDigitShaperDigitName = relRecoDigitShaperDigits.getName();

  B2INFO(" 1. COLLECTIONS:");
  B2INFO(" -->  ShaperDigits:        " << m_storeShaperDigitsName);
  B2INFO(" -->  RecoDigits:           " << m_storeRecoDigitsName);
  B2INFO(" -->  RecoDigitShaperDigitRel:    " << m_relRecoDigitShaperDigitName);


}
void SVDCoGTimeEstimatorModule::beginRun()
{
}


void SVDCoGTimeEstimatorModule::event()
{
  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDRecoDigit> SVDRecoDigits;

  // If no digits, nothing to do
  if (!SVDShaperDigits || !SVDShaperDigits.getEntries()) return;

  RelationArray relRecoDigitShaperDigit(SVDRecoDigits, SVDShaperDigits,
                                        m_relRecoDigitShaperDigitName);
  if (relRecoDigitShaperDigit) relRecoDigitShaperDigit.clear();


  for (const SVDShaperDigit& shaper : SVDShaperDigits) {
    m_Samples_vec = shaper.getSamples();

    //retrieve the VxdID, sensor and cellID of the current RecoDigit
    VxdID thisSensorID = shaper.getSensorID();
    bool thisSide = shaper.isUStrip();
    int thisCellID = shaper.getCellID();

    //call of the functions doomed to calculate the required quantities
    m_weightedMeanTime = CalculateWeightedMeanPeakTime(m_Samples_vec);
    m_weightedMeanTimeError = CalculateWeightedMeanPeakTimeError();
    m_amplitude = CalculateAmplitude(m_Samples_vec);
    m_amplitudeError = CalculateAmplitudeError(thisSensorID, thisSide, thisCellID);
    CalculateProbabilities();
    m_chi2 = CalculateChi2();

    //CALIBRATION
    //convert ADC into #e- and apply offset to shift estimated peak time to hit time (to be completed)
    m_amplitude = m_PulseShapeCal.getChargeFromADC(thisSensorID, thisSide, thisCellID, m_amplitude);
    m_amplitudeError = m_PulseShapeCal.getChargeFromADC(thisSensorID, thisSide, thisCellID, m_amplitudeError);
    m_weightedMeanTime -= m_PulseShapeCal.getPeakTime(thisSensorID, thisSide, thisCellID);
    SVDModeByte::baseType triggerBin = (shaper.getModeByte()).getTriggerBin();
    m_weightedMeanTime -= (DeltaT / 8 + ((int)triggerBin) * DeltaT / 4);
    m_weightedMeanTime -= m_PulseShapeCal.getTimeShiftCorrection(thisSensorID, thisSide, thisCellID);
    m_weightedMeanTime -= m_PulseShapeCal.getTriggerBinDependentCorrection(thisSensorID, thisSide, thisCellID, (int)triggerBin);

    //check high charges and too high ADC
    if (m_amplitude > 255)
      B2DEBUG(10, "SHIT: m_amplitude = " << m_amplitude);
    if (m_amplitude > 100000) {
      B2DEBUG(42, "Charge = " << m_amplitude);
      B2DEBUG(42, "corresponding ADC = " << m_PulseShapeCal.getADCFromCharge(thisSensorID, thisSide, thisCellID, m_amplitude));
      B2DEBUG(42, "thisLayerNumber = " << thisSensorID.getLayerNumber());
      B2DEBUG(42, "thisLadderNumber = " << thisSensorID.getLadderNumber());
      B2DEBUG(42, "thisSensorNumber = " << thisSensorID.getSensorNumber());
      B2DEBUG(42, "thisSide = " << thisSide);
      B2DEBUG(42, "thisCellID = " << thisCellID);
      B2DEBUG(42, "-----");
    }

    //recording of the RecoDigit
    SVDRecoDigits.appendNew(SVDRecoDigit(shaper.getSensorID(), shaper.isUStrip(), shaper.getCellID(), m_amplitude, m_amplitudeError,
                                         m_weightedMeanTime, m_weightedMeanTimeError, m_probabilities, m_chi2, shaper.getModeByte()));

    //Add digit to the RecoDigit->ShaperDigit relation list
    int recoDigitIndex = SVDRecoDigits.getEntries() - 1;
    vector<pair<unsigned int, float> > digit_weights;
    digit_weights.reserve(1);
    digit_weights.emplace_back(shaper.getArrayIndex(), 1.0);
    relRecoDigitShaperDigit.add(recoDigitIndex, digit_weights.begin(), digit_weights.end());
  }
}


void SVDCoGTimeEstimatorModule::endRun()
{

}


void SVDCoGTimeEstimatorModule::terminate()
{
}

//definition of the extra functions

float SVDCoGTimeEstimatorModule::CalculateWeightedMeanPeakTime(Belle2::SVDShaperDigit::APVFloatSamples samples)
{
  float averagetime = 0;
  float mean = 0;
  //calculate weighted average time and mean
  for (int k = 0; k < 6; k ++) {
    averagetime += k * samples[k];
    mean += samples[k];
  }
  averagetime /= (mean);
  averagetime *= DeltaT;

  return averagetime;
}

float SVDCoGTimeEstimatorModule::CalculateAmplitude(Belle2::SVDShaperDigit::APVFloatSamples samples)
{
  float amplitude = 0;
  //calculate amplitude
  for (int k = 0; k < 6; k ++) {
    if (samples[k] > amplitude)
      amplitude = samples[k];
  }

  return amplitude;
}

float SVDCoGTimeEstimatorModule::CalculateWeightedMeanPeakTimeError()
{
  return m_FinalShiftWidth;
}

float SVDCoGTimeEstimatorModule::CalculateAmplitudeError(VxdID ThisSensorID, bool ThisSide, int ThisCellID)
{
  float stripnoise;
  stripnoise = m_NoiseCal.getNoise(ThisSensorID, ThisSide, ThisCellID);

  return stripnoise;
}

void SVDCoGTimeEstimatorModule::CalculateProbabilities()
{
  m_probabilities.push_back(0.5);
}

float SVDCoGTimeEstimatorModule::CalculateChi2()
{
  return 0.01;
}



































