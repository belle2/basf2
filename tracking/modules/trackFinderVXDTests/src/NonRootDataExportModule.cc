/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackFinderVXDTests/NonRootDataExportModule.h"
// #include <boost/foreach.hpp>

#include <framework/gearbox/Const.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/GearDir.h> // needed for reading xml-files

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <genfit/TrackCand.h>
#include "tracking/vxdCaTracking/HitExporter.h"
#include <tracking/dataobjects/VXDTFSecMap.h>
#include <tracking/dataobjects/FullSecID.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <utility>      // std::pair
#include <fstream>

using namespace std;
using namespace Belle2;
// using namespace Belle2::Tracking;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(NonRootDataExport)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------



NonRootDataExportModule::NonRootDataExportModule() : Module()
{
  //Set module properties
  setDescription("allows export of various data objects into files");
  setPropertyFlags(c_ParallelProcessingCertified);
  int minTCLength = 1;
  std::vector<string> writeSecMaps;

  addParam("exportTrueHits", m_PARAMExportTrueHits,
           "allows you to export true hits. Please choose between 'all' hits, 'real' hits, 'background' hits or 'none', which is standard. Wrong input values will set to none with an error.",
           string("none"));
  addParam("exportGFTCs", m_PARAMExportGFTCs, "allows you to export mcInformation about whole tracks, set true for tcOutput",
           bool(false));
  addParam("detectorType", m_PARAMDetectorType,
           "set detectorype. Please choose between 'PXD', 'SVD' (standard) or 'VXD'. Wrong input values will set to SVD with an error.",
           string("SVD"));
  addParam("minTCLength", m_PARAMminTCLength, "tracks with less than minTCLength hits will be neglected", minTCLength);
  addParam("smearTrueHits", m_PARAMsmearTrueHits,
           "when using trueHits, hits and mcPoints have got exactly the same position. If you activate the smearing, the hits will be smeared using the sensor resolution",
           bool(false));
  addParam("outputFormat", m_PARAMoutputFormat,
           "this module can produce output-files with different styles, currently supported are 'gsi' and 'simpleMatlab'.",
           string("simpleMatlab"));
  // note to myself. Format needed by gsi is listed at personal log p.91, format needed by rudi (simpleMatlab) is listed in p. 162,165
  addParam("writeSecMaps", m_PARAMwriteSecMaps,
           "if you want to output an extra file for secMaps (sector-relations) simply write the names of the sectorMaps into that list of names here (same coding as for VXDTF)",
           writeSecMaps);
  addParam("eventCounter", m_PARAMeventCounter, "adds this number to the m_eventCounter (useful for looping shell scripts)", int(0));

  m_eventCounter = 0;
  m_runCounter = 0;
}



NonRootDataExportModule::~NonRootDataExportModule()
{
}



void NonRootDataExportModule::initialize()
{
  B2INFO("NonRootDataExportModule::initialize() \n checking and setting initial parameters");

  if (m_PARAMDetectorType != "PXD" and m_PARAMDetectorType != "SVD" and m_PARAMDetectorType != "VXD") {  // detectorType
    B2ERROR(" chosen value '" << m_PARAMDetectorType << "' for parameter detectorType is unknown, setting to 'SVD'");
    m_PARAMDetectorType = "SVD";
  }

  if (m_PARAMoutputFormat != "gsi" and m_PARAMoutputFormat != "simpleMatlab") {  // outputFormat
    B2ERROR(" chosen value '" << m_PARAMoutputFormat << "' for parameter outputFormat is unknown, setting to 'simpleMatlab'");
    m_PARAMoutputFormat = "simpleMatlab";
  }
  m_exportContainer.setOutputFormat(m_PARAMoutputFormat);

  if (m_PARAMExportTrueHits == "real" or m_PARAMExportTrueHits == "background" or m_PARAMExportTrueHits == "all") { // exportTrueHits
    if (m_PARAMDetectorType == "PXD" or m_PARAMDetectorType == "VXD") {
      StoreArray<PXDTrueHit>::required();
    }
    if (m_PARAMDetectorType == "SVD" or m_PARAMDetectorType == "VXD") {
      StoreArray<SVDTrueHit>::required();
    }
  } else {
    B2ERROR(" chosen value '" << m_PARAMExportTrueHits << "' for parameter exportTrueHits is unknown, setting to 'none'");
    m_PARAMExportTrueHits = "none";
  }

  if (m_PARAMExportGFTCs == true) {
    StoreArray<genfit::TrackCand>::required();
  }

  if (m_PARAMeventCounter < 0) {
    m_PARAMeventCounter = 0;
    B2ERROR("NonRootDataExport: eventCounter is set on illegal value, resetting to 0!");
  }
  StoreArray<MCParticle>::required();
  B2INFO("NonRootDataExportModule: chosen detectorType: " << m_PARAMDetectorType << ", exporting TrueHits: " << m_PARAMExportTrueHits
         << " and exporting GFTrackCands: " << m_PARAMExportGFTCs << ", minimal ndf number of hits for tcs: " << m_PARAMminTCLength <<
         " while using '" << m_PARAMoutputFormat << "/" << m_exportContainer.getOutputFormat() << "' as outputFormat");
}



void NonRootDataExportModule::beginRun()
{
  m_eventCounter = 0;
  m_runCounter = 0;

  importGeometry(&m_exportContainer);

  if (m_PARAMwriteSecMaps.size() != 0) {
    loadSecMap(m_PARAMwriteSecMaps);
    printSecMap();
  }
}



void NonRootDataExportModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  m_eventCounter += m_PARAMeventCounter;
  m_exportContainer.prepareEvent(m_eventCounter);
  for (HitExporter& aContainer : m_exportContainerVector) {
    aContainer.prepareEvent(m_eventCounter);
  }

  StoreArray<MCParticle> mcParticles;
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  /// TODO Instead of linking using RelationIndex -> simply use the following line:
//  RelationVector<MCParticle> mcRelations = hit->getRelationsFrom<MCParticle>();
//    B2DEBUG(10, " within HitExporter::storeGFTC: produced mcRelations-Vector with size: " << mcRelations.size() << " containing all McParticles responsible for a pxdHit")


  // looping over all mcparticles connected to each TrueHit, if there is at least one primary particle connected to current trueHit, the first found primary particle determines the state of the trueHit (means, if there is a primary particle attached to the trueHit, the trueHit is recognized as realhit, and recognized as background hit if otherwise, the particleID of the hit will be the first primary particle or the last secondary one attached to current trueHit):
  int nPXDTrueHits = 0, nSVDTrueHits = 0, particleID = -1, pdg = -1, isPrimary = 1;
  if (m_PARAMExportTrueHits != "none") { // storing trueHits

    if (m_PARAMDetectorType == "PXD" or m_PARAMDetectorType == "VXD") { // storing pxd truehits
      StoreArray<PXDTrueHit> pxdTrueHits; // carries all trueHits of event
      nPXDTrueHits = pxdTrueHits.getEntries();
      B2DEBUG(1, "NonRootDataExportModule event " << m_eventCounter << ": executing " << nPXDTrueHits << " PXDTrueHits");
      RelationIndex<MCParticle, PXDTrueHit> mcParticlesToPxdTrueHits; // allows us to find out which particle caused which truehit
      typedef RelationIndex<MCParticle, PXDTrueHit>::Element RelationElement;

      for (int i = 0; i < nPXDTrueHits; ++i) {
        const PXDTrueHit* aTrueHit = pxdTrueHits[i];

        for (const RelationElement& rel : mcParticlesToPxdTrueHits.getElementsTo(aTrueHit)) {
          const MCParticle* particle = rel.from;
          if (m_PARAMoutputFormat == "gsi") {
            particleID = particle->getIndex(); // 1-based index of Particle.
          } else if (m_PARAMoutputFormat == "simpleMatlab") {
            particleID = particle->getArrayIndex(); // 0-based index of Particle.
          } else { particleID = -1; }

          pdg = particle->getPDG();

          if ((particle->hasStatus(MCParticle::c_PrimaryParticle) == true) and (m_PARAMExportTrueHits == "real")) {
            isPrimary = 0;
            break; // if there are any other true or background particles, they shall be ignored
          }
        }
        B2DEBUG(10, " PXDTrueHit with ID " << i << " is attached to mcParticles with ID " << particleID <<
                " and hasStatusPrimary (0 = true) " << isPrimary);

        if ((m_PARAMExportTrueHits == "real" and isPrimary != 0) or (m_PARAMExportTrueHits == "background" and isPrimary != 1)
            or ((m_PARAMExportTrueHits == "all") and (isPrimary == -1))) {
          isPrimary = 1;
          particleID = -1;
          B2DEBUG(10, " PXD hit " << i << " neglected");
          continue;
        }
        string errorMessage = m_exportContainer.storePXDTrueHit(aGeometry, aTrueHit, i, m_PARAMsmearTrueHits, isPrimary, particleID, pdg);
        for (HitExporter& aContainer : m_exportContainerVector) {
          errorMessage += aContainer.storePXDTrueHit(aGeometry, aTrueHit, i, m_PARAMsmearTrueHits, isPrimary, particleID, pdg);
        }
        if (errorMessage != "") { B2DEBUG(1, "event " << m_eventCounter << ": " << errorMessage); }
        isPrimary = 1;
        particleID = -1;
      } // looping over all pxd trueHits
    } // storing pxdTrueHits


    if (m_PARAMDetectorType == "SVD" or m_PARAMDetectorType == "VXD") {
      StoreArray<SVDTrueHit> svdTrueHits; // carries all trueHits of event
      nSVDTrueHits = svdTrueHits.getEntries();
      B2DEBUG(5, "NonRootDataExportModule event " << m_eventCounter << ": executing " << nSVDTrueHits << " SVDTrueHits");
      RelationIndex<MCParticle, SVDTrueHit> mcParticlesToSvdTrueHits; // allows us to find out which particle caused which truehit
      typedef RelationIndex<MCParticle, SVDTrueHit>::Element RelationElement;

      for (int i = 0; i < nSVDTrueHits; ++i) {
        const SVDTrueHit* aTrueHit = svdTrueHits[i];

        for (const RelationElement& rel : mcParticlesToSvdTrueHits.getElementsTo(aTrueHit)) {
          const MCParticle* particle = rel.from;
          if (m_PARAMoutputFormat == "gsi") {
            particleID = particle->getIndex(); // 1-based index of Particle.
          } else if (m_PARAMoutputFormat == "simpleMatlab") {
            particleID = particle->getArrayIndex(); // 0-based index of Particle.
          } else { particleID = -1; }
          pdg = particle->getPDG();

          if ((particle->hasStatus(MCParticle::c_PrimaryParticle) == true) and (m_PARAMExportTrueHits == "real")) {
            isPrimary = 0;
            break; // if there are any other true or background particles, they shall be ignored
          }
        }
        B2DEBUG(10, " SVDTrueHit with ID " << i << " is attached to mcParticles with ID " << particleID <<
                " and hasStatusPrimary (0 = true) " << isPrimary);

        if ((m_PARAMExportTrueHits == "real" and isPrimary != 0) or (m_PARAMExportTrueHits == "background" and isPrimary != 1)
            or ((m_PARAMExportTrueHits == "all") and (isPrimary == -1))) {
          isPrimary = 1;
          particleID = -1;
          B2DEBUG(10, " SVD hit " << i << " neglected");
          continue;
        }

        string errorMessage = m_exportContainer.storeSVDTrueHit(aGeometry, aTrueHit, i + nPXDTrueHits, m_PARAMsmearTrueHits, isPrimary,
                                                                particleID, pdg); // nPXDTrueHits is 0 if detectorType is SVD, therefore no check here

        for (HitExporter& aContainer : m_exportContainerVector) {
          errorMessage += aContainer.storeSVDTrueHit(aGeometry, aTrueHit, i + nPXDTrueHits, m_PARAMsmearTrueHits, isPrimary, particleID, pdg);
        }
        if (errorMessage != "") { B2DEBUG(1, "event " << m_eventCounter << ": " << errorMessage); }
        isPrimary = 1;
        particleID = -1;
      } // looping over all svd trueHits
    } // storing svdTrueHits
  }


  // exporting GFTrackCands
  if (m_PARAMExportGFTCs == true) {
    StoreArray<genfit::TrackCand> gftcs; // carries all trueHits of event
    StoreArray<PXDTrueHit> pxdTrueHits;
    StoreArray<SVDTrueHit> svdTrueHits;
    vector<const PXDTrueHit*> pxdHits;
    vector<const SVDTrueHit*> svdHits;
    vector<int> hitIDs; // pxd and svd can use the same vector since modified hitIDs are stored which are unique for each hit
    int nTCs = gftcs.getEntries(), detID, hitID, countedTCs = 0;
    B2DEBUG(5, "NonRootDataExportModule event " << m_eventCounter << ": executing " << nTCs << " GFTrackCands");

    for (int i = 0; i < nTCs; ++i) {
      hitIDs.clear(); pxdHits.clear(); svdHits.clear();
      genfit::TrackCand* aTC = gftcs[i];
      int nHits = aTC->getNHits();
      B2DEBUG(10, "event " << m_eventCounter << ": TC " << i << " has got " << nHits << " hits");

      if (int(aTC->getNHits()) < m_PARAMminTCLength) {
        B2WARNING("NonRootDataExportModule - event " << m_eventCounter << ": GfTrackcand " << i << " has only " << nHits <<
                  " hits (threshold is " << m_PARAMminTCLength << "), neglecting tc...");
        continue;
      }

      for (int j = 0; j < nHits; ++j) {
        detID = 0;
        hitID = 0;
        aTC->getHit(j, detID, hitID); // sets detId and hitId for given hitIndex
        B2DEBUG(100, "----got Hitinfo. detID: " << detID << ", hitID: " << hitID);
        if (detID == Const::PXD and m_PARAMDetectorType != "SVD") { // pxd
          hitIDs.push_back(hitID);
          const PXDTrueHit* hit = pxdTrueHits[hitID];
          pxdHits.push_back(hit);
        } else if (detID == Const::SVD and m_PARAMDetectorType != "PXD") {
          hitIDs.push_back(hitID + nPXDTrueHits); // nPXDTrueHits is 0 if detectorType is SVD, therefore no check here
          const SVDTrueHit* hit = svdTrueHits[hitID];
          svdHits.push_back(hit);
        }
      }
      B2DEBUG(10, "storing GFTC " << i << " with " << pxdHits.size() << " pxd hits and " << svdHits.size() << " svd hits");
      m_exportContainer.storeGFTC(aGeometry, aTC, countedTCs, i, pxdHits, svdHits, hitIDs);
      for (HitExporter& aContainer : m_exportContainerVector) {
        aContainer.storeGFTC(aGeometry, aTC, countedTCs, i, pxdHits, svdHits, hitIDs);
      }
      ++countedTCs;
      hitIDs.clear();
    }
  }

  B2DEBUG(5, " event " << m_eventCounter << " got ID " << m_eventCounter << ", internal eventID of " <<
          m_exportContainer.getCurrentEventNumber() << " and got internal hits: " << m_exportContainer.getNumberOfHits() <<
          " hits, of these were " << m_exportContainer.getNumberOfPXDTrueHits() << " PXDTrueHits, and " <<
          m_exportContainer.getNumberOfSVDTrueHits() << " were SVDTrueHits");
}



void NonRootDataExportModule::endRun()
{
  m_runCounter++;
  if (m_PARAMoutputFormat == "gsi") {
    string extendedInfoOfExport = m_exportContainer.exportGsi(m_runCounter,
                                                              15.0); // second entry is the strength of the magnetic field in kiloGauss
    B2INFO("exportGsi: run " << m_runCounter << ": " << extendedInfoOfExport);
  } else if (m_PARAMoutputFormat == "simpleMatlab") {
    for (HitExporter& aContainer : m_exportContainerVector) {
      string extendedInfoOfExport = aContainer.exportSimpleMatlab(m_runCounter);
      B2INFO("exportSimpleMatlab: run " << m_runCounter << ": " << extendedInfoOfExport);
    }
//     string extendedInfoOfExport = m_exportContainer.exportSimpleMatlab(m_runCounter);
//     B2INFO("exportSimpleMatlab: run " << m_runCounter <<": " << extendedInfoOfExport)
  } else {
    B2FATAL("NonRootDataExportModule::endRun: outputFormat '" << m_PARAMoutputFormat << "'not recognized! Aborting export");
  }

  // cleanUp
  for (PassData* currentPass : m_passSetupVector) {
    for (secMapEntry aSector : currentPass->sectorMap) {
      delete aSector.second;
    }
    currentPass->sectorMap.clear();
    delete currentPass; // deleting struct itself
  }
}



void NonRootDataExportModule::terminate()
{

}



/** imports all sectorMaps listed in the vector */
void NonRootDataExportModule::loadSecMap(std::vector<std::string> secMaps)
{
  for (string aMapName : secMaps) {
    PassData* newPass = new PassData;
    newPass->sectorSetup = aMapName;

    VXDTFSecMap::Class(); // essential, needed for root, waiting for root 6 to be removed (hopefully)
    string directory = "/Detector/Tracking/CATFParameters/" + aMapName;
    const VXDTFSecMap* newMap = NULL;
    try {
      newMap = dynamic_cast<const VXDTFSecMap*>(Gearbox::getInstance().getTObject(directory.c_str()));
    } catch (exception& e) {
      B2ERROR("NonRootDataExportModule::loadSecMap: could not load sectorMap: " << aMapName << ". Reason: exception thrown: " << e.what()
              << ", this means you have to check whether the sectorMaps stored in ../tracking/data/VXDTFindex.xml and/or ../testbeam/vxd/data/VXDTFindexTF.xml are uncommented and locally unpacked and available! Skipping current map...");
      continue;
    }

    newPass->secConfigU = newMap->getSectorConfigU();
    newPass->secConfigV = newMap->getSectorConfigV();
    newPass->activateAllFilters();

    /// importing sectorMap including friend Information and friend specific cutoffs
    std::pair<int, int> countedFriendsAndCutoffs = newPass->importSectorMap(newMap->getSectorMap(), newMap->getDistances(), false);

    B2INFO("NonRootDataExportModule::loadSecMap: Pass-setup " << aMapName << ": importing secMap with " << newMap->getSectorMap().size()
           << " sectors -> imported: " << newPass->sectorMap.size() << "/" << countedFriendsAndCutoffs.first << "/" <<
           countedFriendsAndCutoffs.second << " sectors/friends/(friends w/o existing filters), a hitExporter has been attached to it...");

    m_passSetupVector.push_back(newPass);

    HitExporter newHitExporter;
    newHitExporter.setPass((*m_passSetupVector.rbegin()));
    newHitExporter.setOutputFormat(m_PARAMoutputFormat);
    importGeometry(&newHitExporter);
    m_exportContainerVector.push_back(newHitExporter);
  }
}

/** prints all sectorMaps to a file with the same name */
void NonRootDataExportModule::printSecMap()
{
  ofstream secMapFileStream;
  for (PassData* aPass : m_passSetupVector) {
    vector<int> currentCutOffTypes;

    secMapFileStream.open((aPass->sectorSetup + ".data").c_str(), std::ios_base::trunc); // trunc=overwrite app=append

    int sectorCtr = 0, friendCtr = 0, cutoffTypesCtr = 0; // counters
    for (auto& mapEntry : aPass->sectorMap) {  // looping through sectors
      const vector<unsigned int> currentFriends = mapEntry.second->getFriends();
      int nFriends = currentFriends.size();

      secMapFileStream << FullSecID(mapEntry.first) << " ";
      B2DEBUG(1, "Opening sector " << FullSecID(mapEntry.first) << " which has got " << nFriends << " friends");
      if (nFriends != mapEntry.second->getFriendMapSize()) {
        B2WARNING(" number of friends do not match in sector " << FullSecID(mapEntry.first) <<
                  ": friends by friendVector vs nEntries in FriendMa: " << nFriends << "/" << mapEntry.second->getFriendMapSize());
      }

      for (auto friendName : currentFriends) {  // looping through friends

        secMapFileStream << FullSecID(friendName) << " ";
        B2DEBUG(2, " > Opening sectorFriend " << FullSecID(friendName) << "...");
        currentCutOffTypes = mapEntry.second->getSupportedCutoffs(friendName);
        cutoffTypesCtr += currentCutOffTypes.size();
        ++friendCtr;
      }
      secMapFileStream << endl;
      ++sectorCtr;
    }

    secMapFileStream.close();
    B2DEBUG(1, "NonRootDataExportModule::loadSecMap: Pass-setup " << aPass->sectorSetup << ": manually counted a total of " << sectorCtr
            << "/" << friendCtr << "/" << cutoffTypesCtr << " setors/friends/cutoffs in sectorMap");
  }
}


/** importing whole geometry information.
 *
 * we don't need the whole geometry information, but only a guess for each layer, therefore we only store one sensor per layer.
 * Since the info is stored within a set which needs a key to reveal its info, I have to loop over all entries until I find the right one
 * */
void NonRootDataExportModule::importGeometry(HitExporter* hitExporter)
{
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  set< VxdID > layers = aGeometry.getLayers(); // SensorInfoBase::SensorType sensortype=SensorInfoBase::VXD
  double forwardWidth, backwardWidth, diff;
  for (VxdID layer : layers) {
    const set<VxdID>& ladders = aGeometry.getLadders(layer);
    bool stopLoop = false;
    for (VxdID ladder : ladders) {
      const set<VxdID>& sensors = aGeometry.getSensors(ladder);
      for (VxdID sensor : sensors) {
        const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
        if ((aSensorInfo.getType() == 0 and m_PARAMDetectorType == "SVD") or (aSensorInfo.getType() == 1
            and m_PARAMDetectorType == "PXD")) {
          stopLoop = true;
          break;
        }

        forwardWidth = aSensorInfo.getForwardWidth();
        backwardWidth = aSensorInfo.getBackwardWidth();
        diff = forwardWidth - backwardWidth;
        if (diff < 0.0001  and diff > -0.0001) {  // to prevent rounding errora, we don't want to store wedge/slanted sensors
          hitExporter->storeSensorInfo(aSensorInfo);
          stopLoop = true;
          break;
        }
      }
      if (stopLoop == true) { break; }
    }
  }
}
