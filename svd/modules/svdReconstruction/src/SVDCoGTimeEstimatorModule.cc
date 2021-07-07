/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDCoGTimeEstimatorModule.h>

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
  setDescription("From SVDShaperDigit to SVDRecoDigit. Strip charge is evaluated as the max of the 6 samples; hit time is evaluated as a corrected Centre of Gravity (CoG) time.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDEventInfo", m_svdEventInfoName,
           "SVDEventInfo name", string(""));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "ShaperDigits collection name", string(""));
  addParam("RecoDigits", m_storeRecoDigitsName,
           "RecoDigits collection name", string(""));
  addParam("StripPeakTimeCorrection", m_corrPeakTime,
           "Correct for the different peaking times of the strips, obtained from local run calibration", true);
  addParam("CalibrationWithEventT0", m_calEventT0,
           "Use the timing informations of the EventT0 in order to calibrate the CoG.",
           true);

}


SVDCoGTimeEstimatorModule::~SVDCoGTimeEstimatorModule()
{
}


void SVDCoGTimeEstimatorModule::initialize()
{
  m_storeTrueHits.isOptional();
  m_storeMCParticles.isOptional();

  //Inizialization of needed store array
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  if (!m_storeSVDEvtInfo.isOptional(m_svdEventInfoName)) m_svdEventInfoName = "SVDEventInfoSim";
  m_storeSVDEvtInfo.isRequired(m_svdEventInfoName);

  //Initialize the new RecoDigit
  m_storeReco.registerInDataStore(m_storeRecoDigitsName, DataStore::c_ErrorIfAlreadyRegistered);

  RelationArray relRecoDigitShaperDigits(m_storeReco, m_storeShaper);
  relRecoDigitShaperDigits.registerInDataStore();
  RelationArray relRecoDigitTrueHits(m_storeReco, m_storeTrueHits);
  RelationArray relRecoDigitMCParticles(m_storeReco, m_storeMCParticles);
  RelationArray relShaperDigitTrueHits(m_storeShaper, m_storeTrueHits);
  RelationArray relShaperDigitMCParticles(m_storeShaper, m_storeMCParticles);

  //Relations to simulation objects only if the ancestor relations exist
  if (relShaperDigitTrueHits.isOptional())
    relRecoDigitTrueHits.registerInDataStore();
  if (relShaperDigitMCParticles.isOptional())
    relRecoDigitMCParticles.registerInDataStore();

  //Store names to speed up creation later
  m_storeTrueHitsName = m_storeTrueHits.getName();
  m_storeMCParticlesName = m_storeMCParticles.getName();

  m_relRecoDigitShaperDigitName = relRecoDigitShaperDigits.getName();
  m_relRecoDigitTrueHitName = relRecoDigitTrueHits.getName();
  m_relRecoDigitMCParticleName = relRecoDigitMCParticles.getName();
  m_relShaperDigitTrueHitName = relShaperDigitTrueHits.getName();
  m_relShaperDigitMCParticleName = relShaperDigitMCParticles.getName();

  B2DEBUG(25, " 1. COLLECTIONS:");
  B2DEBUG(25, " -->  MCParticles:        " << m_storeMCParticlesName);
  B2DEBUG(25, " -->  Digits:             " << m_storeShaperDigitsName);
  B2DEBUG(25, " -->  RecoDigits:           " << m_storeRecoDigitsName);
  B2DEBUG(25, " -->  TrueHits:           " << m_storeTrueHitsName);
  B2DEBUG(25, " -->  DigitMCRel:         " << m_relShaperDigitMCParticleName);
  B2DEBUG(25, " -->  RecoDigitMCRel:       " << m_relRecoDigitMCParticleName);
  B2DEBUG(25, " -->  RecoDigitDigitRel:    " << m_relRecoDigitShaperDigitName);
  B2DEBUG(25, " -->  DigitTrueRel:       " << m_relShaperDigitTrueHitName);
  B2DEBUG(25, " -->  RecoDigitTrueRel:     " << m_relRecoDigitTrueHitName);

}
void SVDCoGTimeEstimatorModule::beginRun()
{
}


void SVDCoGTimeEstimatorModule::event()
{

  /** Probabilities, to be defined here */
  std::vector<float> probabilities = {0.5};

  // If no digits or no SVDEventInfo, nothing to do
  if (!m_storeShaper || !m_storeShaper.getEntries() || !m_storeSVDEvtInfo.isValid()) return;

  SVDModeByte modeByte = m_storeSVDEvtInfo->getModeByte();
  size_t nDigits = m_storeShaper.getEntries();

  RelationArray relRecoDigitShaperDigit(m_storeReco, m_storeShaper,
                                        m_relRecoDigitShaperDigitName);
  if (relRecoDigitShaperDigit) relRecoDigitShaperDigit.clear();


  RelationArray relShaperDigitMCParticle(m_storeShaper, m_storeMCParticles, m_relShaperDigitMCParticleName);
  RelationArray relShaperDigitTrueHit(m_storeShaper, m_storeTrueHits, m_relShaperDigitTrueHitName);

  RelationArray relRecoDigitMCParticle(m_storeReco, m_storeMCParticles,
                                       m_relRecoDigitMCParticleName);
  if (relRecoDigitMCParticle) relRecoDigitMCParticle.clear();


  RelationArray relRecoDigitTrueHit(m_storeReco, m_storeTrueHits,
                                    m_relRecoDigitTrueHitName);
  if (relRecoDigitTrueHit) relRecoDigitTrueHit.clear();

  //Build lookup tables for relations
  createRelationLookup(relShaperDigitMCParticle, m_mcRelation, nDigits);
  createRelationLookup(relShaperDigitTrueHit, m_trueRelation, nDigits);

  //start loop on SVDSHaperDigits
  Belle2::SVDShaperDigit::APVFloatSamples samples_vec;

  m_NumberOfAPVSamples = m_storeSVDEvtInfo->getNSamples();

  B2DEBUG(25, "number of APV samples = " << m_NumberOfAPVSamples);

  for (const SVDShaperDigit& shaper : m_storeShaper) {

    m_StopCreationReco = false;


    if (m_StopCreationReco)
      continue;

    samples_vec = shaper.getSamples();

    //retrieve the VxdID, sensor and cellID of the current RecoDigit
    VxdID thisSensorID = shaper.getSensorID();
    bool thisSide = shaper.isUStrip();
    int thisCellID = shaper.getCellID();

    //call of the functions doomed to calculate the required quantities
    m_weightedMeanTime = CalculateWeightedMeanPeakTime(samples_vec);
    if (m_StopCreationReco)
      continue;
    m_amplitude = CalculateAmplitude(samples_vec);
    m_amplitudeError = CalculateAmplitudeError(thisSensorID, thisSide, thisCellID);

    //need the amplitudeError in ADC as the noise of the strip to computer the error on time
    m_weightedMeanTimeError = CalculateWeightedMeanPeakTimeError(samples_vec);

    m_chi2 = CalculateChi2();

    //check too high ADC
    if (m_amplitude > 255)
      B2DEBUG(25, "ERROR: m_amplitude = " << m_amplitude << ", should be <= 255");

    //CALIBRATION
    //convert ADC into #e- and apply offset to shift estimated peak time to hit time (to be completed)
    m_amplitude = m_PulseShapeCal.getChargeFromADC(thisSensorID, thisSide, thisCellID, m_amplitude);
    m_amplitudeError = m_PulseShapeCal.getChargeFromADC(thisSensorID, thisSide, thisCellID, m_amplitudeError);

    if (m_corrPeakTime)
      m_weightedMeanTime -= m_PulseShapeCal.getPeakTime(thisSensorID, thisSide, thisCellID);
    SVDModeByte::baseType triggerBin = modeByte.getTriggerBin();

    if (m_calEventT0) {
      m_weightedMeanTime = m_TimeCal.getCorrectedTime(thisSensorID, thisSide, thisCellID, m_weightedMeanTime, triggerBin);
      m_weightedMeanTimeError = m_TimeCal.getCorrectedTimeError(thisSensorID, thisSide, thisCellID, m_weightedMeanTime,
                                                                m_weightedMeanTimeError, triggerBin);
    }

    //check high charges and too high ADC
    if (m_amplitude > 100000) {
      B2DEBUG(25, "Charge = " << m_amplitude);
      B2DEBUG(25, "corresponding ADC = " << m_PulseShapeCal.getADCFromCharge(thisSensorID, thisSide, thisCellID, m_amplitude));
      B2DEBUG(25, "thisLayerNumber = " << thisSensorID.getLayerNumber());
      B2DEBUG(25, "thisLadderNumber = " << thisSensorID.getLadderNumber());
      B2DEBUG(25, "thisSensorNumber = " << thisSensorID.getSensorNumber());
      B2DEBUG(25, "thisSide = " << thisSide);
      B2DEBUG(25, "thisCellID = " << thisCellID);
      B2DEBUG(25, "-----");
    }

    //recording of the RecoDigit
    m_storeReco.appendNew(shaper.getSensorID(), shaper.isUStrip(), shaper.getCellID(), m_amplitude, m_amplitudeError,
                          m_weightedMeanTime, m_weightedMeanTimeError, probabilities, m_chi2);

    //Add digit to the RecoDigit->ShaperDigit relation list
    int recoDigitIndex = m_storeReco.getEntries() - 1;
    vector<pair<unsigned int, float> > digit_weights;
    digit_weights.reserve(1);
    digit_weights.emplace_back(shaper.getArrayIndex(), 1.0);
    relRecoDigitShaperDigit.add(recoDigitIndex, digit_weights.begin(), digit_weights.end());

    // Finally, we save the RecoDigit and its relations.
    map<unsigned int, float> mc_relations;
    map<unsigned int, float> truehit_relations;

    // Store relations to MCParticles and SVDTrueHits
    fillRelationMap(m_mcRelation, mc_relations, shaper.getArrayIndex());
    fillRelationMap(m_trueRelation, truehit_relations, shaper.getArrayIndex());

    //Create relations to the cluster
    if (!mc_relations.empty()) {
      relRecoDigitMCParticle.add(recoDigitIndex, mc_relations.begin(), mc_relations.end());
    }
    if (!truehit_relations.empty()) {
      relRecoDigitTrueHit.add(recoDigitIndex, truehit_relations.begin(), truehit_relations.end());
    }

  }
}


void SVDCoGTimeEstimatorModule::endRun()
{
}


void SVDCoGTimeEstimatorModule::terminate()
{
}


float SVDCoGTimeEstimatorModule::CalculateWeightedMeanPeakTime(Belle2::SVDShaperDigit::APVFloatSamples samples)
{
  float averagetime = 0;
  float sumAmplitudes = 0;
  //calculate weighted average time
  for (int k = 0; k < m_NumberOfAPVSamples; k ++) {
    averagetime += k * samples[k];
    sumAmplitudes += samples[k];
  }
  if (sumAmplitudes != 0) {
    averagetime /= (sumAmplitudes);
    averagetime *= m_DeltaT;
  } else {
    averagetime = -1;
    m_StopCreationReco = true;
    B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
  }

  return averagetime;
}

float SVDCoGTimeEstimatorModule::CalculateAmplitude(Belle2::SVDShaperDigit::APVFloatSamples samples)
{
  float amplitude = 0;
  //calculate amplitude
  for (int k = 0; k < m_NumberOfAPVSamples; k ++) {
    if (samples[k] > amplitude)
      amplitude = samples[k];
  }

  return amplitude;
}

float SVDCoGTimeEstimatorModule::CalculateWeightedMeanPeakTimeError(Belle2::SVDShaperDigit::APVFloatSamples samples)
{

  //assuming that noise of the samples are totally UNcorrelated
  //in MC this hypothesis is correct

  //sum of samples amplitudes
  float Atot = 0;
  //sum of time residuals squared
  float tmpResSq = 0;

  for (int k = 0; k < m_NumberOfAPVSamples; k ++) {
    Atot  += samples[k];
    tmpResSq += TMath::Power(k * m_DeltaT - m_weightedMeanTime, 2);
  }

  return m_amplitudeError / Atot * TMath::Sqrt(tmpResSq);

}

float SVDCoGTimeEstimatorModule::CalculateAmplitudeError(VxdID ThisSensorID, bool ThisSide, int ThisCellID)
{
  float stripnoise;
  stripnoise = m_NoiseCal.getNoise(ThisSensorID, ThisSide, ThisCellID);

  return stripnoise;
}

float SVDCoGTimeEstimatorModule::CalculateChi2()
{
  return 0.01;
}



void SVDCoGTimeEstimatorModule::createRelationLookup(const RelationArray& relation,
                                                     RelationLookup& lookup, size_t digits)
{
  lookup.clear();
  //If we don't have a relation we don't build a lookuptable
  if (!relation) return;
  //Resize to number of digits and set all values
  lookup.resize(digits);
  for (const auto& element : relation) {
    lookup[element.getFromIndex()] = &element;
  }
}

void SVDCoGTimeEstimatorModule::fillRelationMap(const RelationLookup& lookup,
                                                std::map<unsigned int, float>& relation, unsigned int index)
{
  //If the lookup table is not empty and the element is set
  if (!lookup.empty() && lookup[index]) {
    const RelationElement& element = *lookup[index];
    const unsigned int size = element.getSize();
    //Add all Relations to the map
    for (unsigned int i = 0; i < size; ++i) {
      //negative weights are from ignored particles, we don't like them and
      //thus ignore them :D
      if (element.getWeight(i) < 0) continue;
      relation[element.getToIndex(i)] += element.getWeight(i);
    }
  }
}
































