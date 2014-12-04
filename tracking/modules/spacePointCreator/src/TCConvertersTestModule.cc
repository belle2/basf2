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


#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

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
  StoreArray<SpacePointTrackCand>::required(m_SPTCName);
  for (string aName : m_genfitTCNames) { StoreArray<genfit::TrackCand>::required(aName); }

  StoreArray<PXDCluster>::required(m_PXDClusterName);
  StoreArray<SVDCluster>::required(m_SVDClusterName);

  for (string aName : m_SpacePointArrayNames) { StoreArray<SpacePoint>::required(aName); }

  // TODO: check why this does not work properly (not all variables are initialized to 0 here!)
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

    B2DEBUG(20, "Now comparing genfit::TrackCands related from SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName());

    // there SHOULD only exist one relation to each StoreArray of genfit::TrackCands for each SpacePointTrackCand (each converter module registers one, but since genfit::TrackCand is no RelationObject it is only possible to register RelationTo)
    const genfit::TrackCand* genfitTC = trackCand->getRelatedTo<genfit::TrackCand>(m_genfitTCNames[0]);
    const genfit::TrackCand* convertedTC = trackCand->getRelatedTo<genfit::TrackCand>(m_genfitTCNames[1]);

    // check if both trackCands are present (this should never happen, if the relations work correctly!)
    if (genfitTC == NULL) {
      B2DEBUG(50, "Found no original genfit::TrackCand related from SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName());
      ++m_failedNoRelationOrig;
      continue;
    }
    if (convertedTC == NULL) {
      B2DEBUG(50, "Found no converted genfit::TrackCand related from SpacePointTrackCand " << trackCand->getArrayIndex() << " from Array " << trackCand->getArrayName());
      ++m_failedNoRelationConv;
      continue;
    }

    // for debugging purposes, to reproduce why the comparison failed, if it did so. Have to comment in here if you want this output, as genfit::TrackCand::Print() prints to stdout and is thus not affected by setting another LogLevel in the steering script
    genfitTC->Print();
    convertedTC->Print();

    // compare the two genfit::TrackCands
    if (*genfitTC == *convertedTC) {
      B2DEBUG(20, "The two genfit::TrackCands are equal!")
      continue;
    } else {
      B2WARNING("The two genfit::TrackCands related to the SpacePointTrackCand " << trackCand->getArrayIndex() << " do not match");
      if (genfitTC->getNHits() > convertedTC->getNHits()) {
        B2DEBUG(80, "The originial TrackCand has more TrackCandHits");
        ++m_failedTooLittle;;
        continue;
      } else if (genfitTC->getNHits() < convertedTC->getNHits()) {
        B2DEBUG(80, "The original TrackCand has less TrackCandHits");
        ++m_failedTooMany;
        continue;
      } else {
        ++m_failedHitLevel;

        // COULDDO: typedef this
        std::vector< boost::tuple<int, int, double> > genfitIDs;
        std::vector< boost::tuple<int, int, double> > convertedIDs;
        vector<int> failedIDs;

        // get the detID and the hitID and store them together for later comparison
        for (unsigned int iHit = 0; iHit < genfitTC->getNHits(); ++iHit) {
          auto genfitHit = genfitTC->getHit(iHit);
          auto convertedHit = convertedTC->getHit(iHit);
          boost::tuple<int, int, double> genfitTuple(genfitHit->getDetId(), genfitHit->getHitId(), genfitHit->getSortingParameter());
          boost::tuple<int, int, double> convertedTuple(convertedHit->getDetId(), convertedHit->getHitId(), convertedHit->getSortingParameter());
          genfitIDs.push_back(genfitTuple);
          convertedIDs.push_back(convertedTuple);
          if (*genfitHit != *convertedHit) { failedIDs.push_back(iHit); }
        }

        // build a stringstream for output
        stringstream falseHitsStr;
        for (int iD : failedIDs) { falseHitsStr << iD << " "; }
        B2DEBUG(80, "The comparison of hits failed for HitIDs: " << falseHitsStr.str());

        // find the reason why the comparison of the TrackCand failed and categorize them accordingly
        bool wrongSortParam = false;
        bool falseHits = false;
        bool wrongOrder = false;

        for (auto aTuple : genfitIDs) {
          B2DEBUG(250, "Checking aTuple: " << aTuple.get<0>() << "," << aTuple.get<1>() << "," << aTuple.get<2>())
          // first check if the whole tuple can be found in the converted TrackCand, if so: the TrackCandHits are in wrong order, if not -> make further checks
          if (std::find(convertedIDs.begin(), convertedIDs.end(), aTuple) != convertedIDs.end()) {
            B2DEBUG(250, "Found with complete Tuple");
            wrongOrder = true;
          }
          // if this is not the case, look if the combination of (detID, hitID) can be found (omitting the sorting parameter), then only the sorting Parameters are wrong, if this is not the case, there are wrong hits
          else {
            std::pair<int, int> detHitID = {aTuple.get<0>(), aTuple.get<1>()};
            if (std::find_if(convertedIDs.begin(), convertedIDs.end(), [&detHitID](const boost::tuple<int, int, double>& mTuple) { return mTuple.get<0>() == detHitID.first && mTuple.get<1>() == detHitID.second; }) != convertedIDs.end()) {
              B2DEBUG(250, "Found with only detId, hitID");
              wrongSortParam = true;
            } else {
              B2DEBUG(250, "Not found at all");
              falseHits = true;
            }
          }
        }

        if (wrongOrder && !falseHits) {
          B2DEBUG(80, "TrackCandidates contain the same TrackCandHits but in different order");
          ++m_failedWrongOrder;
        }
        if (wrongSortParam && !falseHits) {
          B2DEBUG(80, "The Sorting Parameters do not match");
          ++m_failedWrongSortingParam;
        } else if (falseHits) {
          B2DEBUG(80, "The TrackCands contain different Hits");
        } else {
          B2DEBUG(80, "TrackCands are not equal due to an unknown reason");
          ++m_failedOther;
        }
      }
    }
  }
}

// -------------------------------- TERMINATE -----------------------------------------
void TCConvertersTestModule::terminate()
{
  int nFailedConversions = m_failedNoGFTC;
  nFailedConversions += m_failedNoSPTC; nFailedConversions += m_failedTooLittle; nFailedConversions += m_failedTooMany; nFailedConversions += m_failedHitLevel;
  nFailedConversions += m_failedNoRelationOrig; nFailedConversions += m_failedNoRelationOrig;

  B2INFO("TCConverterTest::terminate: There were " << m_genfitTCCtr << " 'original' genfit::TrackCands from which " << m_SpacePointTCCtr << " SpacePointTrackCands were created. " << m_convertedTCCtr << " were created by conversion from a SpacePointTrackCand. In " << nFailedConversions << " cases one of the two conversions went wrong");
  B2INFO("The reasons for the failure of the conversions are : \n" <<
         "No SpacePointTrackCand was created: " << m_failedNoSPTC << "\n" <<
         "No genfit::TrackCand was created: " << m_failedNoGFTC << "\n" <<
         "No related original genfit::TrackCand was found: " << m_failedNoRelationOrig << "\n" <<
         "No related converted genfit::TrackCand was found: " << m_failedNoRelationConv << "\n" <<
         "The converted TrackCand has too many TrackCandHits: " << m_failedTooMany << "\n" <<
         "The converted TrackCand has too little TrackCandHits: " << m_failedTooLittle << "\n" <<
         "The conversion failed on hit Level: " << m_failedHitLevel << ". The reasons the for the failure of the comparison on hit level are (multiple reasons possible):\n" <<
         "The TrackCandidates contained the Hits in different orders: " << m_failedWrongOrder << "\n" <<
         "The Sorting Parameters did not match: " << m_failedWrongSortingParam << "\n" <<
         "The TrackCandidates contained different Hits: " << m_failedNotSameHits << "\n" <<
         "The conversion failed due to another reason than stated above: " << m_failedOther);
}

void TCConvertersTestModule::initializeCounters()
{
  B2INFO("TCConvertersTestModule initializing Counter variables");
  m_genfitTCCtr = 0;
  m_SpacePointTCCtr = 0;
  m_convertedTCCtr = 0;
  m_failedNoGFTC = 0;
  m_failedNoSPTC = 0;
  m_failedHitLevel = 0;
  m_failedTooLittle = 0;
  m_failedTooMany = 0;
  m_failedOther = 0;
  m_failedNotSameHits = 0;
  m_failedWrongOrder = 0;
  m_failedWrongSortingParam = 0;
  m_failedNoRelationOrig = 0;
  m_failedNoRelationConv = 0;
}