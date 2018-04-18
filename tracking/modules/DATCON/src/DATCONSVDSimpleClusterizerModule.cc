/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONSVDSimpleClusterizerModule.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DATCONSVDSimpleClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DATCONSVDSimpleClusterizerModule::DATCONSVDSimpleClusterizerModule() : Module()
{
  //Set module properties
  setDescription("Clusterize DATCONSVDDigits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("DATCONSVDDigits", m_storeDATCONSVDDigitsListName,
           "DATCONSVDDigits collection name", string(""));
  addParam("DATCONSimpleSVDCluster", m_storeDATCONSimpleSVDClustersName,
           "DATCONSimpleSVDCluster collection name", string(""));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2. Clustering
  addParam("NoiseLevelADU", m_NoiseLevelInADU,
           "Simple assumption of the noise level of the sensors in ADU's", (unsigned short)(0));
  addParam("NoiseCutADU", m_NoiseCutInADU,
           "Simple assumption of the noise level of the sensors in ADU's", (unsigned short)(0));
  addParam("useSimpleClustering", m_useSimpleClustering,
           "Use the simple clustering that is currently done on FPGA for phase 2 (Feb. 2018)", bool(true));
  addParam("maxClusterSize", m_maxClusterSize,
           "Maximum cluster size in count of strips.", (unsigned short)(5));

}


void DATCONSVDSimpleClusterizerModule::initialize()
{
  //Register collections
  storeDATCONSimpleSVDClusters.registerInDataStore(m_storeDATCONSimpleSVDClustersName, DataStore::c_DontWriteOut);

  m_storeDATCONSimpleSVDClustersName = storeDATCONSimpleSVDClusters.getName();

  storeDATCONSVDDigits.isRequired(m_storeDATCONSVDDigitsListName);
  m_storeDATCONSVDDigitsListName = storeDATCONSVDDigits.getName();
  storeDATCONSimpleSVDClusters.registerRelationTo(storeDATCONSVDDigits, DataStore::c_Event, DataStore::c_DontWriteOut);

  storeTrueHits.isOptional(m_storeTrueHitsName);
  if (storeTrueHits.isValid()) {
    m_storeTrueHitsName = storeTrueHits.getName();
    storeDATCONSimpleSVDClusters.registerRelationTo(storeTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  storeMCParticles.isOptional(m_storeMCParticlesName);
  if (storeMCParticles.isValid()) {
    m_storeMCParticlesName = storeMCParticles.getName();
    storeDATCONSimpleSVDClusters.registerRelationTo(storeMCParticles, DataStore::c_Event, DataStore::c_DontWriteOut);
  }
}



void DATCONSVDSimpleClusterizerModule::event()
{
  int nDigits = storeDATCONSVDDigits.getEntries();
  if (nDigits == 0)
    return;

  storeDATCONSimpleSVDClusters.clear();
  clusterCandidates.clear();

  //create a dummy cluster just to start
  DATCONSVDSimpleClusterCandidate clusterCandidate(storeDATCONSVDDigits[0]->getSensorID(), storeDATCONSVDDigits[0]->isUStrip(),
                                                   m_maxClusterSize);

  unsigned short digitindex = 0;

  //loop over the DATCONSVDDigits
  for (auto& datconsvddigit : storeDATCONSVDDigits) {

    if (!noiseFilter(datconsvddigit)) {
      digitindex++;
      continue;
    }

    //retrieve the VxdID, sensor and cellID of the current DATCONSVDDigit
    VxdID thisSensorID = datconsvddigit.getSensorID();
    bool thisSide = datconsvddigit.isUStrip();
    unsigned short thisCellID = datconsvddigit.getCellID();
    unsigned short thisCharge = datconsvddigit.getMaxSampleCharge();

    //try to add the strip to the existing cluster
    if (! clusterCandidate.add(thisSensorID, thisSide, digitindex, thisCharge, thisCellID)) {
      //if the strip is not added, write the cluster, if present and good:
      if (clusterCandidate.size() > 0) {
        if (m_useSimpleClustering) {
          clusterCandidate.finalizeSimpleCluster();
        } else {
          B2WARNING("This one is not yet implemented, so no DATCONSimpleSVDClusters will be created! Skipping...");
        }
        if (clusterCandidate.isGoodCluster()) {
          clusterCandidates.push_back(clusterCandidate);
        }
      }

      //prepare for the next cluster:
      clusterCandidate = DATCONSVDSimpleClusterCandidate(thisSensorID, thisSide, m_maxClusterSize);

      //start another cluster:
      if (! clusterCandidate.add(thisSensorID, thisSide, digitindex, thisCharge, thisCellID))
        B2WARNING("this state is forbidden!!");
    }
    digitindex++;
  } //exit loop on ShaperDigits

  //write the last cluster, if good
  if (clusterCandidate.size() > 0) {
    if (m_useSimpleClustering) {
      clusterCandidate.finalizeSimpleCluster();
    } else {
      B2WARNING("This one is not yet implemented, so no DATCONSimpleSVDClusters will be created! Skipping...");
    }
    if (clusterCandidate.isGoodCluster())
      clusterCandidates.push_back(clusterCandidate);
  }

  saveClusters();

  B2DEBUG(1, "Number of clusters: " << storeDATCONSimpleSVDClusters.getEntries());
}


void DATCONSVDSimpleClusterizerModule::saveClusters()
{

  for (auto clustercandit = clusterCandidates.begin(); clustercandit != clusterCandidates.end(); clustercandit++) {

    DATCONSVDSimpleClusterCandidate clustercand = *clustercandit;

    VxdID sensorID = clustercand.getSensorID();
    bool isU = clustercand.isUSide();
    float position = clustercand.getPosition();
    unsigned short seedCharge = clustercand.getSeedCharge();
    unsigned short charge = clustercand.getCharge();
    unsigned short size = clustercand.size();
    unsigned short seedStripIndex = clustercand.getSeedStripIndex();

    DATCONSimpleSVDCluster* datconsvdcluster =
      storeDATCONSimpleSVDClusters.appendNew(DATCONSimpleSVDCluster(sensorID, isU, position, charge,
                                             seedCharge, seedStripIndex, size));

    vector<unsigned short> indices = clustercand.getIndexVector();

    for (auto digitindexit = indices.begin(); digitindexit != indices.end(); digitindexit++) {

      DATCONSVDDigit* datconsvddigit = storeDATCONSVDDigits[*digitindexit];
      RelationVector<MCParticle> relatedMC = datconsvddigit->getRelationsTo<MCParticle>();
      RelationVector<SVDTrueHit> relatedSVDTrue = datconsvddigit->getRelationsTo<SVDTrueHit>();

      // Register relation to the DATCONSVDDigits this cluster belongs to
      datconsvdcluster->addRelationTo(datconsvddigit);

      // Register relations to the MCParticles and SVDTrueHits
      if (relatedMC.size() > 0) {
        for (unsigned int relmcindex = 0; relmcindex < relatedMC.size(); relmcindex++) {
          datconsvdcluster->addRelationTo(relatedMC[relmcindex], relatedMC.weight(relmcindex));
        }
      }
      if (relatedSVDTrue.size() > 0) {
        for (unsigned int reltruehitindex = 0; reltruehitindex < relatedSVDTrue.size(); reltruehitindex++) {
          datconsvdcluster->addRelationTo(relatedSVDTrue[reltruehitindex], relatedSVDTrue.weight(reltruehitindex));
        }
      }
    }
  }
}

/*
* Simple noise filter.
* Run the noise filter over the given numbers of samples.
* If it fulfills the requirements true is returned.
* TODO: Improve!!!
*/
bool DATCONSVDSimpleClusterizerModule::noiseFilter(DATCONSVDDigit datconsvddigit)
{
  bool passNoiseFilter = false;  /**< initialize the passNoiseFilter with false as default */
  unsigned short maxSampleIndex = datconsvddigit.getMaxSampleIndex();
  DATCONSVDDigit::APVRawSamples sample = datconsvddigit.getRawSamples();
//   unsigned short noiseCut = sample[maxSampleIndex] / 2;  // the initial idea was to declare a strip signal as noise if the samples neighbouring the maximum samle (i.e. maxSampleIndex-1 and maxSampleIndex+1) had a charge (in ADU) of less then getMaxSampleCharge / 2
  unsigned short noiseCut = m_NoiseCutInADU; //sample[maxSampleIndex] / 8;

  if (sample[maxSampleIndex] <= m_NoiseLevelInADU /*|| noiseCut < m_NoiseLevelInADU*/) {
    return false;
  }

  /* The maximum sample is expected to be 2nd or 3rd, so maybe change the conditions
   * in the if-clause accordingly to maxSampleIndex > 1 && maxSampleIndex < 4
   */
  if (maxSampleIndex > 0 && maxSampleIndex < (sample.size() - 1)) {
    if (sample[maxSampleIndex - 1] >= noiseCut && sample[maxSampleIndex + 1] >= noiseCut) {
      passNoiseFilter = true;
    }
  }

  return passNoiseFilter;
}
