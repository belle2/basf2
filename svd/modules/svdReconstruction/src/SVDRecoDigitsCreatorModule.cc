/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDRecoDigitCreatorModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/reconstruction/SVDTimeReconstruction.h>
#include <svd/reconstruction/SVDChargeReconstruction.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDRecoDigitCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDRecoDigitCreatorModule::SVDRecoDigitCreatorModule() : Module(),
  m_useDB(true)
{
  //Set module properties
  setDescription("Reconstruct SVDShaperDigit in SVDRecoDigit");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name", string(""));
  addParam("RecoDigits", m_storeRecoDigitsName,
           "SVDRecoDigits collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "SVDCluster collection name", string(""));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2. Reconstruction
  addParam("timeAlgorithm6Samples", m_timeRecoWith6SamplesAlgorithm,
           " choose time algorithm for the 6-sample DAQ mode:  CoG6 = 6-sample CoG (default), CoG3 = 3-sample CoG,  ELS3 = 3-sample ELS",
           std::string("inRecoDBObject"));
  addParam("timeAlgorithm3Samples", m_timeRecoWith3SamplesAlgorithm,
           " choose time algorithm for the 3-sample DAQ mode:  CoG6 = 6-sample CoG, CoG3 = 3-sample CoG (default),  ELS3 = 3-sample ELS",
           std::string("inRecoDBObject"));
  addParam("chargeAlgorithm6Samples", m_chargeRecoWith6SamplesAlgorithm,
           " choose charge algorithm for 6-sample DAQ mode:  MaxSample (default), SumSamples,  ELS3 = 3-sample ELS",
           std::string("inRecoDBObject"));
  addParam("chargeAlgorithm3Samples", m_chargeRecoWith3SamplesAlgorithm,
           " choose charge algorithm for 3-sample DAQ mode:  MaxSample (default), SumSamples,  ELS3 = 3-sample ELS",
           std::string("inRecoDBObject"));

  addParam("useDB", m_useDB,
           "if false use clustering and reconstruction configuration module parameters", m_useDB);

}

void SVDRecoDigitCreatorModule::beginRun()
{

  if (m_useDB) {
    if (!m_recoConfig.isValid())
      B2ERROR("no valid configuration found for SVD reconstruction");
    else
      B2INFO("SVDRecoConfiguration: from now on we are using " << m_recoConfig->get_uniqueID());

    m_timeRecoWith6SamplesAlgorithm = m_recoConfig->getStripTimeRecoWith6Samples();
    m_timeRecoWith3SamplesAlgorithm = m_recoConfig->getStripTimeRecoWith3Samples();
    m_chargeRecoWith6SamplesAlgorithm = m_recoConfig->getStripChargeRecoWith6Samples();
    m_chargeRecoWith3SamplesAlgorithm = m_recoConfig->getStripChargeRecoWith3Samples();

  }


  B2INFO("SVD  6-sample DAQ, strip time algorithm:" << m_timeRecoWith6SamplesAlgorithm <<  ", strip charge algorithm: " <<
         m_chargeRecoWith6SamplesAlgorithm);

  B2INFO("SVD  3-sample DAQ, strip time algorithm:" << m_timeRecoWith3SamplesAlgorithm <<  ", strip charge algorithm: " <<
         m_chargeRecoWith3SamplesAlgorithm);
}

void SVDRecoDigitCreatorModule::initialize()
{
  //Register collections
  m_storeRecoDigits.registerInDataStore(m_storeRecoDigitsName, DataStore::c_ErrorIfAlreadyRegistered);
  m_storeClusters.isRequired(m_storeClustersName);
  m_storeDigits.isRequired(m_storeShaperDigitsName);
  m_storeTrueHits.isOptional(m_storeTrueHitsName);
  m_storeMCParticles.isOptional(m_storeMCParticlesName);

  /*
  RelationArray relClusterDigits(m_storeClusters, m_storeDigits);
  RelationArray relClusterTrueHits(m_storeClusters, m_storeTrueHits);
  RelationArray relClusterMCParticles(m_storeClusters, m_storeMCParticles);
  RelationArray relDigitTrueHits(m_storeDigits, m_storeTrueHits);
  RelationArray relDigitMCParticles(m_storeDigits, m_storeMCParticles);

  relClusterDigits.registerInDataStore();
  //Relations to simulation objects only if the ancestor relations exist
  if (relDigitTrueHits.isOptional())
    relClusterTrueHits.registerInDataStore();
  if (relDigitMCParticles.isOptional())
    relClusterMCParticles.registerInDataStore();

  //Store names to speed up creation later
  m_storeClustersName = m_storeClusters.getName();
  m_storeShaperDigitsName = m_storeDigits.getName();
  m_storeTrueHitsName = m_storeTrueHits.getName();
  m_storeMCParticlesName = m_storeMCParticles.getName();

  m_relClusterShaperDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relShaperDigitTrueHitName = relDigitTrueHits.getName();
  m_relShaperDigitMCParticleName = relDigitMCParticles.getName();
  */
  // Report:
  B2DEBUG(1, "SVDRecoDigitCreator Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(1, " 1. COLLECTIONS:");
  B2DEBUG(1, " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(1, " -->  SVDShaperDigits:      " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2DEBUG(1, " -->  SVDRecoDigits:      " << DataStore::arrayName<SVDRecoDigit>(m_storeRecoDigitsName));
  B2DEBUG(1, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2DEBUG(1, " -->  SVDTrueHits:        " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  /*  B2DEBUG(1, " -->  DigitMCRel:         " << m_relShaperDigitMCParticleName);
  B2DEBUG(1, " -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2DEBUG(1, " -->  ClusterDigitRel:    " << m_relClusterShaperDigitName);
  B2DEBUG(1, " -->  DigitTrueRel:       " << m_relShaperDigitTrueHitName);
  B2DEBUG(1, " -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  */
}



void SVDRecoDigitCreatorModule::event()
{

  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  int numberOfAcquiredSamples = 0;
  int daqMode = eventinfo->getModeByte().getDAQMode();
  if (daqMode == 2) numberOfAcquiredSamples = 6;
  if (daqMode == 1) numberOfAcquiredSamples = 3;
  if (daqMode == 0) numberOfAcquiredSamples = 1;

  int triggerBin = eventinfo->getModeByte().getTriggerBin();

  int nDigits = m_storeDigits.getEntries();
  if (nDigits == 0)
    return;

  m_storeRecoDigits.clear();

  //loop over the SVDShaperDigits
  int i = 0;

  while (i < nDigits) {

    float time = std::numeric_limits<float>::quiet_NaN();
    float timeError = std::numeric_limits<float>::quiet_NaN();
    float charge = std::numeric_limits<float>::quiet_NaN();
    float chargeError = std::numeric_limits<float>::quiet_NaN();
    int firstFrame = std::numeric_limits<int>::quiet_NaN();
    std::vector<float> probabilities = {0.5};

    double chi2 = std::numeric_limits<double>::quiet_NaN();

    SVDReconstructionBase* timeBase = new SVDReconstructionBase(*m_storeDigits[i]);
    SVDTimeReconstruction* timeReco = (SVDTimeReconstruction*)timeBase;
    timeReco->setTriggerBin(triggerBin);

    SVDReconstructionBase* chargeBase = new SVDReconstructionBase(*m_storeDigits[i]);
    SVDChargeReconstruction* chargeReco = (SVDChargeReconstruction*) chargeBase;

    if (numberOfAcquiredSamples == 6) {
      time = timeReco->getStripTime(m_timeRecoWith6SamplesAlgorithm);
      timeError = timeReco->getStripTimeError(m_timeRecoWith6SamplesAlgorithm);
      firstFrame = timeReco->getFirstFrame();
      charge = chargeReco->getStripCharge(m_chargeRecoWith6SamplesAlgorithm);
      chargeError = chargeReco->getStripChargeError(m_chargeRecoWith6SamplesAlgorithm);
    } else if (numberOfAcquiredSamples == 3) {
      time = timeReco->getStripTime(m_timeRecoWith3SamplesAlgorithm);
      timeError = timeReco->getStripTimeError(m_timeRecoWith3SamplesAlgorithm);
      firstFrame = timeReco->getFirstFrame();
      charge = chargeReco->getStripCharge(m_chargeRecoWith3SamplesAlgorithm);
      chargeError = chargeReco->getStripChargeError(m_chargeRecoWith3SamplesAlgorithm);
    } else
      B2ERROR("SVD Reconstruction not available for this cluster (unrecognized or not supported  number of acquired APV samples!!");

    // now go into FTSW reference frame
    time = time + eventinfo->getSVD2FTSWTimeShift(firstFrame);

    B2INFO("time = " << time);
    B2INFO("timeError = " << timeError);
    B2INFO("charge = " << charge);
    B2INFO("chargeError = " << chargeError);

    //append the new SVDRecoDigit to the StoreArray
    m_storeRecoDigits.appendNew(SVDRecoDigit(m_storeDigits[i]->getSensorID(), m_storeDigits[i]->isUStrip(),
                                             m_storeDigits[i]->getCellID(), charge, chargeError, time, timeError, probabilities, chi2));

    // write relations

    i++;
  } //exit loop on ShaperDigits

  B2DEBUG(1, "Number of strips: " << m_storeRecoDigits.getEntries());
}


