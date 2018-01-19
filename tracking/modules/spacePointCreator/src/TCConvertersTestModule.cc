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

// #include <tracking/spacePointCreation/SpacePointTrackCand.h> // already in header
// #include <genfit/TrackCand.h> // already in header

using namespace Belle2;
using namespace std;

REG_MODULE(TCConvertersTest)

TCConvertersTestModule::TCConvertersTestModule() :
  Module()
{
  setDescription("Module for testing the functionality of the TrackCand converter modules and their underlying classes.");

  addParam("PXDClusters", m_PXDClusterName, "PXDCluster collection name. WARNING: it is only checked if these exist, "\
           "they are not actually used at the moment!", string(""));
  addParam("SVDClusters", m_SVDClusterName, "SVDCluster collection name WARNING: it is only checked if these exist, "\
           " they are not actually used at the moment!", string(""));
  addParam("SpacePointTCName", m_SPTCName, "Name of the container under which SpacePoints are stored in the DataStore", string(""));

  std::vector<std::string> emptyDefaultStringVec = { std::string("") };
  addParam("genfitTCNames", m_genfitTCNames, "Names of containers of genfit::TrackCands. "\
           "WARNING: For this module provide two names! First is the name of the container with the genfit::TrackCands"\
           " which were used to create SpacePointTrackCands (e.g. from MCTrackFinderTruth), second are the genfit::TrackCands"\
           " obtaineed by the 'back conversion' from the SpacePointTrackCands!", emptyDefaultStringVec);
  addParam("SpacePointArrayNames", m_SpacePointArrayNames, "SpacePoints collection name(s) WARNING: it is only checked if these"\
           " exist. At the moment all StoreArray<SpacePoint> are searched for SpacePoints!", emptyDefaultStringVec);

  initializeCounters(); // NOTE: they get initialized in initialize again!!
}

// --------------------------------- INITIALIZE -------------------------------------
void TCConvertersTestModule::initialize()
{
  B2INFO("TCConvertersTest ---------------------------- initialize ------------------ ");

  // check if all StoreArrays are present
  StoreArray<SpacePointTrackCand> SPTCs(m_SPTCName); SPTCs.isRequired(m_SPTCName);
  for (string aName : m_genfitTCNames) {
    StoreArray<genfit::TrackCand> TCs(aName);
    TCs.isRequired(aName);
  }

  StoreArray<PXDCluster> PXDClusters(m_PXDClusterName); PXDClusters.isRequired(m_PXDClusterName);
  StoreArray<SVDCluster> SVDClusters(m_SVDClusterName); SVDClusters.isRequired(m_SVDClusterName);

  for (string aName : m_SpacePointArrayNames) {
    StoreArray<SpacePoint> SPs(aName);
    SPs.isRequired(aName);
  }

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

  B2DEBUG(11, "Found " << nGenfitTCs << " genfit::TrackCands, " << nSpacePointTCs << " SpacePointTrackCands and " << \
          nConvertedTCs << " genfit::TrackCands created by conversion from a SpacePointTrackCand");

  // count the 'simple' reasons for failure (if there are less SpacePointTCs than genfitTCs,
  // there has to be a problem with the creation of SpacePointTCs which should result in a warning.
  // The same applies for the back-conversion to genfitTCs)
  m_failedNoSPTC += (nGenfitTCs - nSpacePointTCs);
  m_failedNoGFTC += (nSpacePointTCs - nConvertedTCs);

  // have to loop over the SpacePointTrackCand as they inherit from RelationObject and therefore have the possibility
  // to get Relations to or from them
  for (int iTC = 0; iTC < nSpacePointTCs; ++iTC) {
    const SpacePointTrackCand* trackCand = SpacePointTCs[iTC];

    B2DEBUG(20, "Now comparing genfit::TrackCands related from SpacePointTrackCand " << trackCand->getArrayIndex() << \
            " from Array " << trackCand->getArrayName());

    // there SHOULD only exist one relation to each StoreArray of genfit::TrackCands for each SpacePointTrackCand (each converter
    // module registers one, but since genfit::TrackCand is no RelationObject it is only possible to register RelationTo)
    const genfit::TrackCand* genfitTC = trackCand->getRelatedTo<genfit::TrackCand>(m_genfitTCNames[0]);
    const genfit::TrackCand* convertedTC = trackCand->getRelatedTo<genfit::TrackCand>(m_genfitTCNames[1]);

    // check if both trackCands are present (this should never happen, if the relations work correctly!)
    if (genfitTC == NULL) {
      B2DEBUG(50, "Found no original genfit::TrackCand related from SpacePointTrackCand " << trackCand->getArrayIndex() << \
              " from Array " << trackCand->getArrayName());
      ++m_failedNoRelationOrig;
      continue;
    }
    if (convertedTC == NULL) {
      B2DEBUG(50, "Found no converted genfit::TrackCand related from SpacePointTrackCand " << trackCand->getArrayIndex() << \
              " from Array " << trackCand->getArrayName());
      ++m_failedNoRelationConv;
      continue;
    }

    // only print in debug mode
    if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 100, PACKAGENAME())) {
      genfitTC->Print();
      convertedTC->Print();
    }

    // compare the two genfit::TrackCands
    if (*genfitTC == *convertedTC) {
      B2DEBUG(20, "The two genfit::TrackCands are equal!");
      continue;
    } else {
      B2DEBUG(20, "The two genfit::TrackCands differ!");
      if (analyzeMisMatch(genfitTC, convertedTC, trackCand)) {
        B2DEBUG(20, "It is OK for the two TrackCands to differ (i.e. the hits that miss are due to their absence in the SPTC!)");
        m_differButOKCtr++;
      } else {
        B2WARNING("Two genfit::TrackCands differ but there is no appearant reason why they should be allowed to do so!");
        m_failedOther++;
      }
    }
  }
}

// -------------------------------- TERMINATE -----------------------------------------
void TCConvertersTestModule::terminate()
{
  stringstream results; // put together BASIC results
  results << "There were " << m_failedOther << " cases that one should look into and " << \
          m_differButOKCtr << " cases where the failure of the comparison can be explained.";

  B2INFO("TCConverterTest::terminate(): Got " << m_genfitTCCtr << " 'original' genfit::TrackCands, " << \
         m_convertedTCCtr << " genfit::TrackCands to compare and " << m_SpacePointTCCtr << " SpacePointTrackCands. " << results.str());

  if (m_failedOther) { B2WARNING("There were cases during comparison that need to be looked at!"); } // should not happen anymore

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME())) {
    stringstream verbose;
    verbose << m_lessHitsCtr << " times the original GFTC had less hits than the converted" << endl;
    verbose << m_moreHitsCtr << " times the converted GFTC had less hits than the original" << endl;
    verbose << m_failedWrongSortingParam << " times the sorting parameters did not match" << endl;
    verbose << m_failedWrongOrder << " times the hits were contained in the wrong order " << endl;
    verbose << m_failedNotSameHits << " times there were hits in the converted GFTC that were not in the original" << endl;
    verbose << m_failedNoRelationConv << " times there was no related converted GFTC to a SPTC" << endl;
    verbose << m_failedNoRelationConv << " times there was no related original GFTC to a SPTC" << endl;
    verbose << "In " << m_failedNoSPTC << " cases the conversion from GFTC -> SPTC went wrong and in ";
    verbose << m_failedNoGFTC << " cases the conversion from SPTC -> GFTC went wrong" << endl;
    B2DEBUG(1, verbose.str());
  }
}

// ================================================ ANALYZE MISMATCH ==============================================================
bool TCConvertersTestModule::analyzeMisMatch(const genfit::TrackCand* origTC, const genfit::TrackCand* convTC,
                                             const Belle2::SpacePointTrackCand* spTC)
{
  // compare number of hits
  size_t nOrigHits = origTC->getNHits();
  size_t nConvHits = convTC->getNHits();
  bool diffNHits = nOrigHits != nConvHits;
  if (diffNHits) {
    B2DEBUG(100, "The number of hits do not match. original GFTC: " << nOrigHits << ", converted GFTC: " << nConvHits);
    if (nOrigHits > nConvHits) m_moreHitsCtr++; // the converted GFTC has less hits than the original
    else m_lessHitsCtr++;
    if (!spTC->hasRefereeStatus(SpacePointTrackCand::c_omittedClusters)) {
      // if there is a missing hit, but omitted hit is wrong, there is something wrong
      B2WARNING("The number of hits does not match but the referee status omittedClusters is not set for the SPTC!");
      return false;
    }
  }

  // get the hits to compare further on the hit level
  vector<trackCandHit> origHits = getTrackCandHits(origTC);
  vector<trackCandHit> convHits = getTrackCandHits(convTC);

  array<bool, 4> foundHits = checkHits(origHits, convHits);
  B2DEBUG(1999, "Contents of foundHits: " << get<0>(foundHits) << " " << get<1>(foundHits) << \
          " " << get<2>(foundHits) << " " << get<3>(foundHits));

  if (!foundHits[0]) { // if there are unfound hits: return false regardless of the refereeStatus of the SPTC
    B2WARNING("Not all hits from the converted GFTC are in the original GFTC!"); // warning for the moment, should not happen often
    m_failedNotSameHits++;
    return false;
  } else { // NOTE: for the moment only checking the order and the sorting params, but not the planeIDs!
    if (!foundHits[1]) { // if all hits are present, but they are in the wrong order
      B2DEBUG(1, "The hits appear in the wrong order in the converted GFTC compared to the original GFTC!");
      m_failedWrongOrder++;
    }
    if (!foundHits[3]) { // sorting params not matching
      B2WARNING("The sorting parameters are not matching!");
      m_failedWrongSortingParam++;
    }
  }

  return true;
}

// ====================================================== GET TRACKCANDHITS =======================================================
std::vector<TCConvertersTestModule::trackCandHit> TCConvertersTestModule::getTrackCandHits(const genfit::TrackCand* trackCand)
{
  vector<trackCandHit> tcHits;
  for (size_t iHit = 0; iHit < trackCand->getNHits(); ++iHit) {
    genfit::TrackCandHit* hit = trackCand->getHit(iHit);
    tcHits.push_back(make_tuple(hit->getDetId(), hit->getHitId(), hit->getPlaneId(), hit->getSortingParameter()));
  }
  return tcHits;
}

// ================================================== CHECK HITS ===================================================================
std::array<bool, 4> TCConvertersTestModule::checkHits(const std::vector<trackCandHit>& origHits,
                                                      const std::vector<trackCandHit>& convHits)
{
  std::array<bool, 4> checkResults = {{ true, true, true, true }};
  typedef vector<trackCandHit>::const_iterator vectorIt; // typedef for avoiding auto below

  int priorPos = -1; // first hit is 0 at least
  B2DEBUG(499, "Checking if all hits of converted TC can be found in the original TC!");
  stringstream allHitsOut;
  for (const trackCandHit& hit : origHits) {
    allHitsOut << get<0>(hit) << "\t" << get<1>(hit) << "\t" << get<2>(hit) << "\t" << get<3>(hit) << endl;
  }
  B2DEBUG(499, "Hits of original TC:\ndetId\thitId\tplaneId\tsort.Param:\n" << allHitsOut.str());

  for (size_t iHit = 0; iHit < convHits.size(); ++iHit) { // loop over all hits in convHits
    trackCandHit hit = convHits[iHit];
    int pos;
    stringstream hitOut;
    hitOut << "Checking hit " << get<0>(hit) << " " << get<1>(hit) << " " << get<2>(hit) << " " << get<3>(hit) << " -> ";
    vectorIt foundIt = find(origHits.begin(), origHits.end(), hit);

    if (foundIt == origHits.end()) { // check if a hit can be found without comparing the sorting parameters
      hitOut << "not found, now checking if omitting planeID and sortingParams helps:" << endl;
      vectorIt foundOnlyHitsIt = checkEntries<0, 1>(origHits, hit);
      if (foundOnlyHitsIt != origHits.end()) {
        pos = foundOnlyHitsIt - origHits.begin();
        hitOut << " hit is contained in origHits at position " << pos << "! Now checking what is not matching " << endl;
        vectorIt foundNoSortIt = checkEntries<0, 1, 2>(origHits, hit);
        vectorIt foundNoPlaneIdIt = checkEntries<0, 1, 3>(origHits, hit);
        if (foundNoSortIt == origHits.end()) {
          hitOut << " sorting parameters are not matching!" << endl;
          checkResults[3] = false;
        }
        if (foundNoPlaneIdIt == origHits.end()) {
          hitOut << " the planeIDs are not matching!" << endl;
          checkResults[2] = false;
        }
      } else {
        hitOut << " hit is not contained in origHits!" << endl;
        checkResults[0] = false;
        continue; // start over with next hit
      }
    } else {
      pos = foundIt - origHits.begin();
      hitOut << " hit found at position " << pos << endl;
    }
    B2DEBUG(499, hitOut.str());
    B2DEBUG(499, "This hit appears in position " << pos << " in the original TC " << " the last checked hit was on position " <<
            priorPos);

    if (pos <= priorPos) {
      B2DEBUG(499, "The ordering of hits is wrong!");
      checkResults[1] = false;
    }
    priorPos = pos;
  }
  return checkResults;
}

// ======================================================= INITIALIZE COUNTERS ====================================================
void TCConvertersTestModule::initializeCounters()
{
  B2INFO("TCConvertersTestModule initializing Counter variables");
  m_genfitTCCtr = 0;
  m_SpacePointTCCtr = 0;
  m_convertedTCCtr = 0;
  m_failedNoGFTC = 0;
  m_failedNoSPTC = 0;
  m_failedOther = 0;
  m_failedNotSameHits = 0;
  m_failedWrongOrder = 0;
  m_failedWrongSortingParam = 0;
  m_failedNoRelationOrig = 0;
  m_failedNoRelationConv = 0;

  m_lessHitsCtr = 0;
  m_moreHitsCtr = 0;
  m_differButOKCtr = 0;
}
