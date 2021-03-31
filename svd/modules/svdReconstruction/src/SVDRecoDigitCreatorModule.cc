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
#include <framework/logging/Logger.h>

#include <svd/dataobjects/SVDEventInfo.h>

#include <svd/reconstruction/SVDReconstructionBase.h>

#include <svd/reconstruction/SVDRecoTimeFactory.h>
#include <svd/reconstruction/SVDRecoChargeFactory.h>

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
      B2FATAL("no valid configuration found for SVD reconstruction");
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

  bool returnRawClusterTime = false;
  m_time6SampleClass = SVDRecoTimeFactory::NewTime(m_timeRecoWith6SamplesAlgorithm, returnRawClusterTime);
  m_time3SampleClass = SVDRecoTimeFactory::NewTime(m_timeRecoWith3SamplesAlgorithm, returnRawClusterTime);
  m_charge6SampleClass = SVDRecoChargeFactory::NewCharge(m_chargeRecoWith6SamplesAlgorithm);
  m_charge3SampleClass = SVDRecoChargeFactory::NewCharge(m_chargeRecoWith3SamplesAlgorithm);

  B2INFO("SVD  6-sample DAQ SVDRecoDigit, time algorithm: " << m_timeRecoWith6SamplesAlgorithm <<  ", charge algorithm: " <<
         m_chargeRecoWith6SamplesAlgorithm);

  B2INFO("SVD  3-sample DAQ SVDRecoDigit, time algorithm: " << m_timeRecoWith3SamplesAlgorithm <<  ", charge algorithm: " <<
         m_chargeRecoWith3SamplesAlgorithm);

}

void SVDRecoDigitCreatorModule::initialize()
{
  //Register collections
  m_storeReco.registerInDataStore(m_storeRecoDigitsName, DataStore::c_ErrorIfAlreadyRegistered);
  m_storeClusters.isOptional(m_storeClustersName);
  m_storeShaper.isOptional(m_storeShaperDigitsName);

  m_storeReco.registerRelationTo(m_storeShaper);
  m_storeClusters.registerRelationTo(m_storeReco);

  //Store names to speed up creation later
  m_storeClustersName = m_storeClusters.getName();
  m_storeShaperDigitsName = m_storeShaper.getName();

  // Report:
  B2DEBUG(25, "SVDRecoDigitCreator Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(25, " 1. COLLECTIONS:");
  B2DEBUG(25, " -->  SVDShaperDigits:      " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2DEBUG(25, " -->  SVDRecoDigits:      " << DataStore::arrayName<SVDRecoDigit>(m_storeRecoDigitsName));
  B2DEBUG(25, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
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

  int nShaperDigits = m_storeShaper.getEntries();

  //loop over the SVDShaperDigits
  for (int i = 0; i < nShaperDigits; ++i) {

    VxdID sensorID = m_storeShaper[i]->getSensorID();
    bool isU =  m_storeShaper[i]->isUStrip();
    int cellID = m_storeShaper[i]->getCellID();

    //build the StripInRawCluster and a fake RawCluster
    RawCluster rawCluster(sensorID, isU, 0, 0);
    Belle2::SVD::StripInRawCluster strip;
    strip.shaperDigitIndex = i;
    strip.cellID = cellID;
    strip.maxSample = m_storeShaper[i]->getMaxADCCounts();
    strip.noise = m_NoiseCal.getNoise(sensorID, isU, cellID);
    strip.samples = m_storeShaper[i]->getSamples();;
    strip.charge = std::numeric_limits<double>::quiet_NaN();;
    strip.time = std::numeric_limits<double>::quiet_NaN();;

    if (rawCluster.add(sensorID, rawCluster.isUSide(), strip)) {

      double time = std::numeric_limits<float>::quiet_NaN();
      double timeError = std::numeric_limits<float>::quiet_NaN();
      double charge = std::numeric_limits<float>::quiet_NaN();
      float chargeError = std::numeric_limits<float>::quiet_NaN();
      int firstFrame = std::numeric_limits<int>::quiet_NaN();
      std::vector<float> probabilities = {0.5};
      double chi2 = std::numeric_limits<double>::quiet_NaN();

      //dummy containers:
      double SNR = std::numeric_limits<double>::quiet_NaN();
      double seedCharge = std::numeric_limits<double>::quiet_NaN();

      if (numberOfAcquiredSamples == 6) {

        //time
        m_time6SampleClass->computeClusterTime(rawCluster, time, timeError, firstFrame);
        //charge
        m_charge6SampleClass->computeClusterCharge(rawCluster, charge, SNR, seedCharge);
      } else if (numberOfAcquiredSamples == 3) {
        //time
        m_time3SampleClass->computeClusterTime(rawCluster, time, timeError, firstFrame);

        //charge
        m_charge3SampleClass->computeClusterCharge(rawCluster, charge, SNR, seedCharge);

      } else
        B2ERROR("SVD Reconstruction not available for this strip: not supported number of acquired APV samples!!");

      // now go into FTSW reference frame
      time = eventinfo->getTimeInFTSWReference(time, firstFrame);

      //append the new SVDRecoDigit to the StoreArray
      SVDRecoDigit* recoDigit = m_storeReco.appendNew(sensorID, isU, cellID, charge, chargeError, time, timeError, probabilities, chi2);

      // set the relation SVDRecoDigit -> SVDShaperDigit
      recoDigit->addRelationTo(m_storeShaper[i]);
      // and SVDCluster -> SVDRecoDigit
      SVDCluster* cluster = m_storeShaper[i]->getRelated<SVDCluster>(m_storeClustersName);
      cluster->addRelationTo(recoDigit, recoDigit->getCharge());
    }
  } //exit loop on ShaperDigits
}


void SVDRecoDigitCreatorModule::endRun()
{

  delete m_time6SampleClass;
  delete m_time3SampleClass;
  delete m_charge6SampleClass;
  delete m_charge3SampleClass;

}
