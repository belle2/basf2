/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/TCConvertersTestModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>


using namespace Belle2;
using namespace std;

REG_MODULE(TCConvertersTest)

TCConvertersTestModule::TCConvertersTestModule() :
  Module()
{
  setDescription("Module for testing the functionality of the TrackCand converter modules and their underlying classes.");

  addParam("PXDClusters", m_PXDClusterName, "PXDCluster collection name. WARNING: it is only checked if these exist, they are not actually used at the moment!", string(""));
  addParam("SVDClusters", m_SVDClusterName, "SVDCluster collection name WARNING: it is only checked if these exist, they are not actually used at the moment!", string(""));
  addParam("SpacePointTCName", m_SPTCName, "Name of the container under which SpacePoints are stored in the DataStore", string(""));

  std::vector<std::string> emptyDefaultStringVec = { std::string("") };
  addParam("genfitTCNames", m_genfitTCNames, "Names of containers of genfit::TrackCands. WARNING: For this module provide two names! First is the name of the container with the genfit::TrackCands which were used to create SpacePointTrackCands (e.g. from MCTrackFinderTruth), second are the genfit::TrackCands obtaineed by the 'back conversion' from the SpacePointTrackCands!", emptyDefaultStringVec);
  addParam("SpacePointArrayNames", m_SpacePointArrayNames, "SpacePoints collection name(s) WARNING: it is only checked if these exist. At the moment all StoreArray<SpacePoint> are searched for SpacePoints!", emptyDefaultStringVec);
}

// --------------------------------- INITIALIZE -------------------------------------
void TCConvertersTestModule::initialize()
{
  B2INFO("TCConvertersTest ---------------------------- initialize ------------------ ");
  // check if all StoreArrays are present
  B2DEBUG(1, m_SPTCName);
  StoreArray<SpacePointTrackCand>::required(m_SPTCName);
  for (string aName : m_genfitTCNames) { StoreArray<genfit::TrackCand>::required(aName); }

  StoreArray<PXDCluster>::required(m_PXDClusterName);
  StoreArray<SVDCluster>::required(m_SVDClusterName);

  for (string aName : m_SpacePointArrayNames) { StoreArray<SpacePoint>::required(aName); }

  initializeCounters();
}

// ----------------------------------- EVENT -----------------------------------------
void TCConvertersTestModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "TCConvertersTest::event(). Processing event " << eventCounter << " --------");

  // this is very specific at the moment, but as it is only a testing module, I think it should work
  StoreArray<genfit::TrackCand> genfitTCs(m_genfitTCNames[0]);
  StoreArray<genfit::TrackCand> convertedGenfitTCs(m_genfitTCNames[1]);

  StoreArray<SpacePointTrackCand> SpacePointTCs(m_SPTCName);

  int nGenfitTCs = genfitTCs.getEntries();
  int nConvertedTCs = convertedGenfitTCs.getEntries();
  int nSpacePointTCs = SpacePointTCs.getEntries();

  m_genfitTCCtr += nGenfitTCs;
  m_convertedTCCtr += nConvertedTCs;
  m_SpacePointTCCtr += nSpacePointTCs;

  B2DEBUG(11, "Found " << nGenfitTCs << " genfit::TrackCands, " << nSpacePointTCs << " SpacePointTrackCands and " << nConvertedTCs << " genfit::TrackCands created by conversion from a SpacePointTrackCand");

  // count the 'simple' reasons for failure (if there are less SpacePointTCs than genfitTCs, there has to be a problem with the creation of SpacePointTCs which should result in a warning. The same applies for the back-conversion to genfitTCs)
  m_failedNoSPTC += (nGenfitTCs - nSpacePointTCs);
  m_failedNoGFTC += (nSpacePointTCs - nConvertedTCs);

  // have to loop over the SpacePointTrackCand as they inherit from RelationObject and therefore have the possibility to get Relations to or from them
  for (int iTC = 0; iTC < nSpacePointTCs; ++iTC) {
    const SpacePointTrackCand* trackCand = SpacePointTCs[iTC];

    // there SHOULD only exist one relation to each StoreArray of genfit::TrackCands for each SpacePointTrackCand (each converter module registers one, but since genfit::TrackCand is no RelationObject it is only possible to register RelationTo)
    const genfit::TrackCand* genfitTC = trackCand->getRelatedTo<genfit::TrackCand>(m_genfitTCNames[0]);
    const genfit::TrackCand* convertedTC = trackCand->getRelatedTo<genfit::TrackCand>(m_genfitTCNames[1]);

    // for debugging purposes, to reproduce why the comparison failed, if it did so. Have to comment in here if you want this output, as genfit::TrackCand::Print() prints to stdout and is thus not affected by setting another LogLevel in the steering script
//     genfitTC->Print();
//     convertedTC->Print();

    if (*genfitTC == *convertedTC) { continue; }
    else {
      B2DEBUG(11, "The two genfit::TrackCands related to the SpacePointTrackCand do not match");
      if (genfitTC->getNHits() > convertedTC->getNHits()) {
        B2WARNING("The originial TrackCand has more TrackCandHits");
        m_failedTooLittle += 1;
        continue;
      } else if (genfitTC->getNHits() < convertedTC->getNHits()) {
        B2WARNING("The original TrackCand has less TrackCandHits");
        m_failedTooMany += 1;
        continue;
      } else {
        std::vector< std::pair<int, int> > genfitIDs;
        std::vector< std::pair<int, int> > convertedIDs;
        vector<int> failedIDs;

        // get the detID and the hitID and store them together for later comparison
        for (unsigned int iHit = 0; iHit < genfitTC->getNHits(); ++iHit) {
          auto genfitHit = genfitTC->getHit(iHit);
          auto convertedHit = convertedTC->getHit(iHit);
          genfitIDs.push_back({genfitHit->getDetId(), genfitHit->getHitId()});
          convertedIDs.push_back({convertedHit->getDetId(), convertedHit->getHitId()});
          if (*genfitHit != *convertedHit) { failedIDs.push_back(iHit); }
        }

        // build a stringstream for output
        stringstream falseHitsStr;
        for (int iD : failedIDs) { falseHitsStr << iD << " "; }
        B2DEBUG(80, "The comparison of hits failed for HitIDs: " << falseHitsStr);

        // find the reason why the comparison of the TrackCand failed and categorize them accordingly
        bool wrongOrder = false;
        bool falseHits = false;
        for (auto aPair : genfitIDs) {
          if (std::find(convertedIDs.begin(), convertedIDs.end(), aPair) != convertedIDs.end()) {
            wrongOrder = true;
          } else {
            falseHits = true;
          }
        }

        if (wrongOrder && !falseHits) {
          B2WARNING("TrackCandidates contain the same TrackCandHits but in different order");
          m_failedWrongOrder += 1;
        } else if (falseHits) {
          B2WARNING("The TrackCands contain different TrackCandHits");
          m_failedNotSameHits += 1;
        } else {
          B2WARNING("TrackCands are not equal due to unknown reason");
          m_failedOther += 1;
        }
      }
    }
  }
}

// -------------------------------- TERMINATE -----------------------------------------
void TCConvertersTestModule::terminate()
{
  int nFailedConversions = m_failedNoGFTC;
  nFailedConversions += m_failedNoSPTC; nFailedConversions += m_failedOther; nFailedConversions += m_failedTooLittle; nFailedConversions += m_failedTooMany; nFailedConversions += m_failedNotSameHits;
  nFailedConversions += m_failedWrongOrder;

  B2INFO("TCConverterTest::terminate: There were " << m_genfitTCCtr << " 'original' genfit::TrackCands from which " << m_SpacePointTCCtr << " SpacePointTrackCands were created. " << m_convertedTCCtr << " were created by conversion form a SpacePointTrackCand. In " << nFailedConversions << " cases one of the two conversions went wrong");
  B2INFO("The reasons for the failure of the conversions are : \n" <<
         "No SpacePointTrackCand was created: " << m_failedNoSPTC << "\n" <<
         "No genfit::TrackCand was created: " << m_failedNoGFTC << "\n" <<
         "The converted TrackCand has too many TrackCandHits: " << m_failedTooMany << "\n" <<
         "The converted TrackCand has too little TrackCandHits: " << m_failedTooLittle << "\n" <<
         "The TrackCandidates contained the Hits in different orders: " << m_failedWrongOrder << "\n" <<
         "The TrackCandidates contained different Hits: " << m_failedNotSameHits << "\n" <<
         "The conversion failed due to another reason than stated above: " << m_failedOther);

}

void TCConvertersTestModule::initializeCounters()
{
  m_genfitTCCtr = 0;
  m_SpacePointTCCtr = 0;
  m_convertedTCCtr = 0;
  m_failedNoGFTC = 0;
  m_failedNoSPTC = 0;
  m_failedTooLittle = 0;
  m_failedTooMany = 0;
  m_failedOther = 0;
  m_failedNotSameHits = 0;
  m_failedWrongOrder = 0;
}