/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
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

  //check that all algorithms are available, otherwise use the default one
  SVDReconstructionBase recoBase;

  if (!recoBase.isTimeAlgorithmAvailable(m_timeRecoWith6SamplesAlgorithm)) {
    B2WARNING("strip time algorithm " << m_timeRecoWith6SamplesAlgorithm << " is NOT available, using CoG6");
    m_timeRecoWith6SamplesAlgorithm = "CoG6";
  };

  if (!recoBase.isTimeAlgorithmAvailable(m_timeRecoWith3SamplesAlgorithm)) {
    B2WARNING("strip time algorithm " << m_timeRecoWith3SamplesAlgorithm << " is NOT available, using CoG3");
    m_timeRecoWith3SamplesAlgorithm = "CoG3";
  };
  if (!recoBase.isChargeAlgorithmAvailable(m_chargeRecoWith6SamplesAlgorithm)) {
    B2WARNING("strip charge algorithm " << m_chargeRecoWith6SamplesAlgorithm << " is NOT available, using MaxSample");
    m_chargeRecoWith6SamplesAlgorithm = "MaxSample";
  };
  if (!recoBase.isChargeAlgorithmAvailable(m_chargeRecoWith3SamplesAlgorithm)) {
    B2WARNING("strip charge algorithm " << m_chargeRecoWith3SamplesAlgorithm << " is NOT available, using MaxSample");
    m_chargeRecoWith3SamplesAlgorithm = "MaxSample";
  };

  B2INFO("SVD  6-sample DAQ, strip time algorithm: " << m_timeRecoWith6SamplesAlgorithm <<  ", strip charge algorithm: " <<
         m_chargeRecoWith6SamplesAlgorithm);

  B2INFO("SVD  3-sample DAQ, strip time algorithm: " << m_timeRecoWith3SamplesAlgorithm <<  ", strip charge algorithm: " <<
         m_chargeRecoWith3SamplesAlgorithm);
}

void SVDRecoDigitCreatorModule::initialize()
{
  //Register collections
  m_storeReco.registerInDataStore(m_storeRecoDigitsName, DataStore::c_ErrorIfAlreadyRegistered);
  m_storeClusters.isRequired(m_storeClustersName);
  m_storeShaper.isRequired(m_storeShaperDigitsName);

  RelationArray relRecoToShaper(m_storeReco, m_storeShaper);
  relRecoToShaper.registerInDataStore();
  RelationArray relClusterToReco(m_storeClusters, m_storeReco);
  relClusterToReco.registerInDataStore();

  //Store names to speed up creation later
  m_storeClustersName = m_storeClusters.getName();
  m_storeShaperDigitsName = m_storeShaper.getName();

  m_relRecoToShaperName = relRecoToShaper.getName();
  m_relClusterToRecoName = relClusterToReco.getName();

  // Report:
  B2DEBUG(25, "SVDRecoDigitCreator Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(25, " 1. COLLECTIONS:");
  B2DEBUG(25, " -->  SVDShaperDigits:      " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2DEBUG(25, " -->  SVDRecoDigits:      " << DataStore::arrayName<SVDRecoDigit>(m_storeRecoDigitsName));
  B2DEBUG(25, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2DEBUG(25, " -->  ClusterToRecoRel:    " << m_relClusterToRecoName);
  B2DEBUG(25, " -->  RecoToShaperRel:    " << m_relRecoToShaperName);
}



void SVDRecoDigitCreatorModule::event()
{

  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  int numberOfAcquiredSamples = eventinfo->getNSamples();

  int triggerBin = eventinfo->getModeByte().getTriggerBin();

  int nDigits = m_storeShaper.getEntries();
  if (nDigits == 0)
    return;

  m_storeReco.clear();

  //loop over the SVDShaperDigits
  int i = 0;

  while (i < nDigits) {

    VxdID sensorID = m_storeShaper[i]->getSensorID();
    bool isU =  m_storeShaper[i]->isUStrip();
    int cellID = m_storeShaper[i]->getCellID();
    float averageNoiseInADC =  m_NoiseCal.getNoise(sensorID, isU, cellID);
    float averageNoiseInElectrons =  m_NoiseCal.getNoiseInElectrons(sensorID, isU, cellID);

    float time = std::numeric_limits<float>::quiet_NaN();
    float timeError = std::numeric_limits<float>::quiet_NaN();
    float charge = std::numeric_limits<float>::quiet_NaN();
    float chargeError = std::numeric_limits<float>::quiet_NaN();
    int firstFrame = std::numeric_limits<int>::quiet_NaN();
    std::vector<float> probabilities = {0.5};
    double chi2 = std::numeric_limits<double>::quiet_NaN();


    // build SVDTimeReconstuction and SVDChargeReconstruction classes with the SVDShaperDigit
    SVDTimeReconstruction* timeReco = new SVDTimeReconstruction(*m_storeShaper[i]);
    timeReco->setAverageNoise(averageNoiseInADC, averageNoiseInElectrons);
    timeReco->setTriggerBin(triggerBin);
    SVDChargeReconstruction* chargeReco = new SVDChargeReconstruction(*m_storeShaper[i]);
    chargeReco->setAverageNoise(averageNoiseInADC, averageNoiseInElectrons);


    // get first frame, strip time and charge and their errors
    if (numberOfAcquiredSamples == 6) {
      std::pair<int, double> FFandTime = timeReco->getFirstFrameAndStripTime(m_timeRecoWith6SamplesAlgorithm);
      firstFrame = FFandTime.first;
      time = FFandTime.second;
      timeError = timeReco->getStripTimeError(m_timeRecoWith6SamplesAlgorithm);
      charge = chargeReco->getStripCharge(m_chargeRecoWith6SamplesAlgorithm);
      chargeError = chargeReco->getStripChargeError(m_chargeRecoWith6SamplesAlgorithm);
    } else if (numberOfAcquiredSamples == 3) {
      std::pair<int, double> FFandTime = timeReco->getFirstFrameAndStripTime(m_timeRecoWith3SamplesAlgorithm);
      firstFrame = FFandTime.first;
      time = FFandTime.second;
      timeError = timeReco->getStripTimeError(m_timeRecoWith3SamplesAlgorithm);
      charge = chargeReco->getStripCharge(m_chargeRecoWith3SamplesAlgorithm);
      chargeError = chargeReco->getStripChargeError(m_chargeRecoWith3SamplesAlgorithm);
    } else
      B2ERROR("SVD Reconstruction not available for this strip: not supported number of acquired APV samples!!");

    // now go into FTSW reference frame
    time = time + eventinfo->getSVD2FTSWTimeShift(firstFrame);

    //append the new SVDRecoDigit to the StoreArray
    m_storeReco.appendNew(SVDRecoDigit(sensorID, isU, cellID, charge, chargeError, time, timeError, probabilities, chi2));

    // write relations SVDRecoDigit -> SVDShaperDigit
    RelationArray relRecoToShaper(m_storeReco, m_storeShaper,
                                  m_relRecoToShaperName);
    if (relRecoToShaper) relRecoToShaper.clear();

    int recoIndex = m_storeReco.getEntries() - 1;
    int shaperIndex = i;
    if (recoIndex != shaperIndex)
      B2ERROR("incompatible SVDShaperDigit and sVDRecoDigit indexes, they are supposed to be the same, while: SVDRecoDigit Index = " <<
              recoIndex << " != " << shaperIndex << " = SVDShaperDigit index");

    vector<pair<unsigned int, float> > digit_weights;
    digit_weights.reserve(1);
    digit_weights.emplace_back(shaperIndex, 1.0);
    relRecoToShaper.add(recoIndex, digit_weights.begin(), digit_weights.end());


    i++;
  } //exit loop on ShaperDigits

  B2DEBUG(25, "Number of strips: " << m_storeReco.getEntries());

  //write relations: SVDCluster -> SVDRecoDigit
  //1. loop on clusters
  //2. take related shaper digits
  //3. build relation with reco digit with the same index, using reco charge as weight

  RelationArray relClusterToReco(m_storeClusters, m_storeReco,
                                 m_relClusterToRecoName);
  if (relClusterToReco) relClusterToReco.clear();

  for (const SVDCluster& cluster : m_storeClusters) {

    int clusterIndex = cluster.getArrayIndex();

    vector<pair<unsigned int, float> > digit_weights;
    digit_weights.reserve(cluster.getSize());

    RelationVector<SVDShaperDigit> theShaperDigits = DataStore::getRelationsWithObj<SVDShaperDigit>(&cluster);

    for (int s = 0; s < (int)theShaperDigits.size(); s++)
      digit_weights.push_back(make_pair(s, m_storeReco[s]->getCharge()));

    relClusterToReco.add(clusterIndex, digit_weights.begin(), digit_weights.end());
  }

}


