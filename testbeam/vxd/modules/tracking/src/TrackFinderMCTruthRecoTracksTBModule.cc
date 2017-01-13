/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 * NOTE: this is a copy of tracking/modules/trackFinderMCTruth/include/TrackFinderMCTruthRecoTracksModule.cc *
 *                                                                        *
 *    where the access to cdc information is removed!
 *    For TestBeam usage ONLY!!!                                           *
 **************************************************************************/

#include <testbeam/vxd/modules/tracking/TrackFinderMCTruthRecoTracksTBModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/WireTrackCandHit.h>

#include <geometry/bfieldmap/BFieldMap.h>

#include <boost/foreach.hpp>

#include <TRandom.h>

#include <vector>
#include <string>
#include <sstream>
#include <cmath>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFinderMCTruthRecoTracksTB)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFinderMCTruthRecoTracksTBModule::TrackFinderMCTruthRecoTracksTBModule() : Module()
{
  //Set module properties
  setDescription("Uses the MC information to create genfit::TrackCandidates for primary MCParticles and Relations between them. "
                 "Fills the created genfit::TrackCandidates with all information (start values, hit indices) needed for the fitting.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits",
           m_usePXDHits,
           "Set true if PXDHits or PXDClusters should be used",
           true);
  addParam("UseSVDHits",
           m_useSVDHits,
           "Set true if SVDHits or SVDClusters should be used",
           true);
  addParam("UseOnlyAxialCDCHits",
           m_useOnlyAxialCDCHits,
           "Set true if only the axial CDCHits should be used",
           false);
  addParam("UseNLoops",
           m_useNLoops,
           "Set the number of loops to be included in the MC tracks. "
           "Default includes all",
           INFINITY);


  addParam("MinPXDHits",
           m_minPXDHits,
           "Minimum number of PXD hits needed to allow the created of a track candidate",
           0);
  addParam("MinSVDHits",
           m_minSVDHits,
           "Minimum number of SVD hits needed to allow the created of a track candidate",
           0);
  addParam("MinCDCAxialHits",
           m_minCDCAxialHits,
           "Minimum number of CDC hits form an axial wire needed to allow the created of a track candidate",
           0);
  addParam("MinCDCStereoHits",
           m_minCDCStereoHits,
           "Minimum number of CDC hits form a stereo wire needed to allow the created of a track candidate",
           0);
  addParam("MinimalNDF",
           m_minimalNdf,
           "Minimum number of total hits needed to allow the creation of a track candidate. "
           "It is called NDF (number of degrees of freedom) because it counts the dimensionality. "
           "2D hits are counted as 2",
           5);

  //choose for which particles a track candidate should be created
  //this is just an attempt to find out what is the most suitable way to select particles, if you have other/better ideas, communicate it to the tracking group...
  addParam("WhichParticles",
           m_whichParticles,
           "List of keywords to mark what properties particles must have to get a track candidate. "
           "If several properties are given all of them must be true: "
           "\"primary\" particle must come from the generator, "
           "\"PXD\", \"SVD\", \"CDC\", \"TOP\", \"ARICH\", \"ECL\" or \"KLM\" particle must have hits in the subdetector with that name. "
           "\"is:X\" where X is a PDG code: particle must have this code. "
           "\"from:X\" any of the particles's ancestors must have this (X) code",
           std::vector<std::string>(1, "primary"));

  addParam("EnergyCut",
           m_energyCut,
           "Track candidates are only created for MCParticles with energy larger than this cut ",
           double(0.1));

  addParam("Neutrals",
           m_neutrals,
           "Set true if track candidates should be created also for neutral particles",
           bool(false));

  addParam("SetTimeSeed",
           m_setTimeSeed,
           "Set true to forward the production time as time seed of the particles to the RecoTrack",
           true);

  //smearing of MCMomentum
  addParam("Smearing",
           m_smearing,
           "Smearing of MCMomentum/MCVertex prior to storing it in genfit::TrackCandidate (in %). "
           "A negative value will switch off smearing. This is also the default.",
           -1.0);

  addParam("SmearingCov",
           m_smearingCov,
           "Covariance matrix used to smear the true pos and mom before passed to track candidate. "
           "This matrix will also passed to Genfit as the initial covarance matrix. "
           "If any diagonal value is negative this feature will not be used. "
           "OFF DIAGNOLA ELEMENTS DO NOT HAVE AN EFFECT AT THE MOMENT",
           std::vector<double>(36, -1.0));
  // names of output containers
  addParam("RecoTracksStoreArrayName",
           m_recoTracksStoreArrayName,
           "Name of store array holding the RecoTracks (output)",
           std::string(""));

  addParam("TrueHitMustExist",
           m_enforceTrueHit,
           "If set true only cluster hits that have a relation to a TrueHit will be included in the track candidate",
           false);
}

void TrackFinderMCTruthRecoTracksTBModule::initialize()
{
  // deactivate the use of CDC by default
  m_useCDCHits = false;

  StoreArray<MCParticle> mcparticles;


  if (mcparticles.isOptional()) {
    m_mcParticlesPresent = true;

    //output store arrays have to be registered in initialize()
    StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
    recoTracks.registerInDataStore();

    recoTracks.registerRelationTo(mcparticles);

    RecoTrack::registerRequiredRelations(recoTracks);

    // build a bit mask with all properties a MCParticle should have to lead to the creation of a track candidate
    m_particleProperties = 0;
    int aPdgCode = 0;
    const int nProperties = m_whichParticles.size();
    for (int i = 0; i not_eq nProperties; ++i) {
      if (m_whichParticles[i] == "primary") {
        m_particleProperties += 1;
      } else if (m_whichParticles[i] == "PXD") {
        m_particleProperties += 2;
      } else if (m_whichParticles[i] == "SVD") {
        m_particleProperties += 4;
      } else if (m_whichParticles[i] == "CDC") {
        m_particleProperties += 8;
      } else if (m_whichParticles[i] == "TOP") {
        m_particleProperties += 16;
      } else if (m_whichParticles[i] == "ARICH") {
        m_particleProperties += 32;
      } else if (m_whichParticles[i] == "ECL") {
        m_particleProperties += 64;
      } else if (m_whichParticles[i] == "KLM") {
        m_particleProperties += 128;
      } else if (m_whichParticles[i].substr(0, 3) == "is:") {
        std::string pdgCodeString = m_whichParticles[i].substr(3);
        std::stringstream(pdgCodeString) >> aPdgCode;
        B2DEBUG(100, "PDG code added to m_particlePdgCodes " << aPdgCode << " *******");
        m_particlePdgCodes.push_back(aPdgCode);
      } else if (m_whichParticles[i].substr(0, 5) == "from:") {
        std::string pdgCodeString = m_whichParticles[i].substr(5);
        std::stringstream(pdgCodeString) >> aPdgCode;
        B2DEBUG(100, "PDG code added to m_fromPdgCodes " << aPdgCode << " *******");
        m_fromPdgCodes.push_back(aPdgCode);
      } else {
        B2FATAL("Invalid values were given to the MCTrackFinder parameter WhichParticles");
      }
    }



    //transfom the smearingCov vector into a TMatrixD
    //first check if it can be transformed into a 6x6 matrix
    if (m_smearingCov.size() != 36) {
      B2FATAL("SmearingCov does not have exactly 36 elements. So 6x6 covariance matrix can be formed from it");
    }
    m_initialCov.ResizeTo(6, 6);
    m_initialCov = TMatrixDSym(6, &m_smearingCov[0]);
    for (int i = 0; i != 6; ++i) {
      if (m_initialCov(i, i) < 0.0) {
        m_initialCov(0, 0) = -1.0; // if first element of matrix is negative this using this matrix will be switched off
      }
    }

    if (m_smearing > 0.0 && m_initialCov(0, 0) > 0.0) {
      B2FATAL("Both relative smearing (Smearing) and using a smearing cov (SmearingCov) is activated but only one of both can be used");
    }
  }
}

void TrackFinderMCTruthRecoTracksTBModule::beginRun()
{
  m_notEnoughtHitsCounter = 0;
  m_noTrueHitCounter = 0;
  m_nRecoTracks = 0;
}


void TrackFinderMCTruthRecoTracksTBModule::event()
{
  // Skip in the case there are no MC particles present.
  if (not m_mcParticlesPresent) {
    B2DEBUG(100, "Skipping MC Track Finder as there are no MC Particles registered in the DataStore.");
    return;
  }

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(100, "*******   MCTrackFinderModule processing event number: " << eventCounter << " *******");

  //all the input containers. First: MCParticles
  StoreArray<MCParticle> mcParticles;
  const int nMcParticles = mcParticles.getEntries();
  B2DEBUG(100, "MCTrackFinder: total Number of MCParticles: " << nMcParticles);

  //PXD trueHits
  StoreArray<PXDTrueHit> pxdTrueHits;
  const int nPXDHits = pxdTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of PXDTrueHits: " << nPXDHits);

  RelationArray mcPartToPXDTrueHits(mcParticles, pxdTrueHits);
  const int nMcPartToPXDHits = mcPartToPXDTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and PXDHits: " << nMcPartToPXDHits);

  //PXD clusters
  StoreArray<PXDCluster> pxdClusters;
  const int nPXDClusters = pxdClusters.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of PXDClusters: " << nPXDClusters);

  RelationArray pxdClusterToMCParticle(pxdClusters, mcParticles);
  const int nPxdClusterToMCPart = pxdClusterToMCParticle.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between PXDCluster and MCParticles: " << nPxdClusterToMCPart);

  //SVD truehits
  StoreArray<SVDTrueHit> svdTrueHits;
  const int nSVDHits = svdTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of SVDDHits: " << nSVDHits);

  RelationArray mcPartToSVDTrueHits(mcParticles, svdTrueHits);
  const int nMcPartToSVDHits = mcPartToSVDTrueHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and SVDHits: " << nMcPartToSVDHits);

  //SVD clusters
  StoreArray<SVDCluster> svdClusters;
  const int nSVDClusters = svdClusters.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of SVDClusters: " << nSVDClusters);

  RelationArray svdClusterToMCParticle(svdClusters, mcParticles);
  const int nSvdClusterToMCPart = svdClusterToMCParticle.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between SVDCluster and MCParticles: " << nSvdClusterToMCPart);

  //CDC
  StoreArray<CDCHit> cdcHits;
  const int nCDCHits = cdcHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of CDCHits: " << nCDCHits);

  RelationArray mcPartToCDCHits(mcParticles, cdcHits);
  const int nMcPartToCDCHits = mcPartToCDCHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between MCParticles and CDCHits: " << nMcPartToCDCHits);

  StoreArray<CDCSimHit> cdcSimHits("");
  const int nCDCSimHits = cdcSimHits.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of CDCSimHits: " << nCDCSimHits);

  RelationArray cdcSimHitToHitRel(cdcSimHits, cdcHits);
  const int nCdcSimHitToHitRel = cdcSimHitToHitRel.getEntries();
  B2DEBUG(100, "MCTrackFinder: Number of relations between CDCSimHit and CDCHits: " << nCdcSimHitToHitRel);


  //register StoreArray which will be filled by this module
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);

  // loop over MCParticles. And check several user selected properties. Make a track candidate only if MCParticle has properties wanted by user options.
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    MCParticle* aMcParticlePtr = mcParticles[iPart];
    // Ignore particles that didn't propagate significantly, they cannot make tracks.
    if ((aMcParticlePtr->getDecayVertex() - aMcParticlePtr->getProductionVertex()).Mag() < 1 * Unit::cm) {
      B2DEBUG(200, "Particle that did not propagate significantly cannot make track.");
      continue;
    }

    //set the property mask for this particle and compare it to the one generated from user input
    int mcParticleProperties = 0;
    if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle)) {
      mcParticleProperties += 1;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::PXD)) {
      mcParticleProperties += 2;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::SVD)) {
      mcParticleProperties += 4;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::CDC)) {
      mcParticleProperties += 8;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::TOP)) {
      mcParticleProperties += 16;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::ARICH)) {
      mcParticleProperties += 32;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::ECL)) {
      mcParticleProperties += 64;
    }
    if (aMcParticlePtr->hasSeenInDetector(Const::KLM)) {
      mcParticleProperties += 128;
    }
    // check all "seen in" properties that the mcparticle should have in one line.
    if ((mcParticleProperties bitand m_particleProperties) != m_particleProperties) {
      B2DEBUG(101, "PDG: " << aMcParticlePtr->getPDG() <<  " | property mask of particle " <<  mcParticleProperties <<
              " demanded property mask " << m_particleProperties);
      continue; //goto next mcParticle, do not make track candidate
    }
    //make links only for interesting MCParticles: energy cut
    if (aMcParticlePtr->getEnergy() < m_energyCut) {
      B2DEBUG(100, "particle energy too low.  MC particle will be skipped");
      continue; //goto next mcParticle, do not make track candidate
    }

    //check if particle has the pdg code the user wants to have. If user did not set any pdg code every code is fine for track candidate creation

    const int nPdgCodes = m_particlePdgCodes.size();
    if (nPdgCodes not_eq 0) {
      const int currentPdgCode = aMcParticlePtr->getPDG();
      int nFalsePdgCodes = 0;
      for (int i = 0; i not_eq nPdgCodes; ++i) {
        if (m_particlePdgCodes[i] not_eq currentPdgCode) {
          ++nFalsePdgCodes;
        }
      }
      if (nFalsePdgCodes == nPdgCodes) {
        B2DEBUG(100, "particle does not have one of the user provided pdg codes and will therefore be skipped");
        continue; //goto next mcParticle, do not make track candidate
      }
    }


    //check if particle has an ancestor selected by the user. If user did not set any pdg code every code is fine for track candidate creation
    const int nFromPdgCodes = m_fromPdgCodes.size();
    if (nFromPdgCodes not_eq 0) {
      MCParticle* currentMother = aMcParticlePtr->getMother();
      int nFalsePdgCodes = 0;
      int nAncestor = 0;
      while (currentMother not_eq NULL) {
        int currentMotherPdgCode = currentMother->getPDG();
        for (int i = 0; i not_eq nFromPdgCodes; ++i) {
          if (m_fromPdgCodes[i] not_eq currentMotherPdgCode) {
            ++nFalsePdgCodes;
          }
        }

        currentMother = currentMother->getMother();
        ++nAncestor;
      }
      if (nFalsePdgCodes == (nAncestor * nFromPdgCodes)) {
        B2DEBUG(100, "particle does not have and ancestor with one of the user provided pdg codes and will therefore be skipped");
        continue; //goto next mcParticle, do not make track candidate
      }
    }

    // Ignore baryons, except for deuteron.  The purpose is mainly to
    // avoid an error message when getCharge() is called below.
    if (abs(aMcParticlePtr->getPDG()) > 1000000000
        && abs(aMcParticlePtr->getPDG()) != 1000010020) {
      B2DEBUG(200, "Skipped Baryon.");
      continue; //goto next mcParticle, do not make track candidate

    }

    // ignore neutrals (unless requested)
    if (!m_neutrals && aMcParticlePtr->getCharge() == 0) {
      B2DEBUG(100, "particle does not have the right charge. MC particle will be skipped");
      continue; //goto next mcParticle, do not make track candidate
    }


    B2DEBUG(100, "Build a track for the MCParticle with index: " << iPart << " (PDG: " << aMcParticlePtr->getPDG() << ")");

    //assign indices of the Hits from all detectors.
    typedef std::tuple<double, int, Const::EDetector> TimeHitIDDetector;
    std::vector<TimeHitIDDetector> hitsWithTimeAndDetectorInformation;

    int ndf = 0; // count the ndf of one track candidate

    // create a list containing the indices to the PXDHits that belong to one track
    unsigned int hitCounter = 0;
    if (m_usePXDHits) {
      const RelationVector<PXDCluster>& relatedClusters = aMcParticlePtr->getRelationsFrom<PXDCluster>();

      for (size_t i = 0; i < relatedClusters.size(); ++i) {
        if (relatedClusters.weight(i) < 0) continue;  // skip hits from secondary particles

        const PXDCluster* pxdCluster = relatedClusters.object(i);
        const RelationVector<PXDTrueHit>& relatedTrueHits = pxdCluster->getRelationsTo<PXDTrueHit>();

        if (relatedTrueHits.size() == 0 and m_enforceTrueHit) {
          // there is not trueHit! throw away hit because there is no time information for sorting
          ++m_noTrueHitCounter;
          continue;
        }

        float time = NAN;
        for (const PXDTrueHit& pxdTrueHit : relatedTrueHits) {
          // make sure only a true hit is taken that really comes from the current mcParticle. This must be carefully checked because several trueHits from different real tracks can be melted into one cluster
          const RelationVector<MCParticle>& relatedMCParticles = pxdTrueHit.getRelationsFrom<MCParticle>();
          if (std::find_if(relatedMCParticles.begin(), relatedMCParticles.end(), [aMcParticlePtr](const MCParticle & mcParticle) { return &mcParticle == aMcParticlePtr; })
        != relatedMCParticles.end()) {
            time = pxdTrueHit.getGlobalTime();
            break;
          }
        }
        if (not std::isnan(time)) {
          hitsWithTimeAndDetectorInformation.emplace_back(time, pxdCluster->getArrayIndex(), Const::PXD);
          ++hitCounter;
          ndf += 2;
        }
      }

      B2DEBUG(100, "     add " << hitCounter << " PXDClusters. " << relatedClusters.size() - hitCounter <<
              " PXDClusters were not added because they do not have a corresponding PXDTrueHit");
    }
    if (hitCounter < static_cast<unsigned int>(m_minPXDHits)) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }

    // create a list containing the indices to the SVDHits that belong to one track
    hitCounter = 0;
    if (m_useSVDHits) {
      const RelationVector<SVDCluster>& relatedClusters = aMcParticlePtr->getRelationsFrom<SVDCluster>();

      for (size_t i = 0; i < relatedClusters.size(); ++i) {
        if (relatedClusters.weight(i) < 0) continue;  // skip hits from secondary particles

        const SVDCluster* svdCluster = relatedClusters.object(i);
        const RelationVector<SVDTrueHit>& relatedTrueHits = svdCluster->getRelationsTo<SVDTrueHit>();

        if (relatedTrueHits.size() == 0 and m_enforceTrueHit) {
          // there is not trueHit! throw away hit because there is no time information for sorting
          ++m_noTrueHitCounter;
          continue;
        }

        float time = NAN;
        for (const SVDTrueHit& svdTrueHit : relatedTrueHits) {
          // make sure only a true hit is taken that really comes from the current mcParticle. This must be carefully checked because several trueHits from different real tracks can be melted into one cluster
          const RelationVector<MCParticle>& relatedMCParticles = svdTrueHit.getRelationsFrom<MCParticle>();
          if (std::find_if(relatedMCParticles.begin(), relatedMCParticles.end(), [aMcParticlePtr](const MCParticle & mcParticle) { return &mcParticle == aMcParticlePtr; })
        != relatedMCParticles.end()) {
            time = svdTrueHit.getGlobalTime();
            break;
          }
        }
        if (not std::isnan(time)) {
          hitsWithTimeAndDetectorInformation.emplace_back(time, svdCluster->getArrayIndex(), Const::SVD);
          ++hitCounter;
          ndf += 1;
        }
      }

      B2DEBUG(100, "     add " << hitCounter << " SVDClusters. " << relatedClusters.size() - hitCounter <<
              " SVDClusters were not added because they do not have a corresponding SVDTrueHit");
    }
    if (hitCounter < static_cast<unsigned int>(m_minSVDHits)) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }

    // prepare rejection of CDC hits from higher order loops
    const TVector3 origin(0.0, 0.0, 0.0);
    const double Bz = BFieldMap::Instance().getBField(origin).Z();
    const double nLoops = m_useNLoops;
    auto isWithinNLoops = [Bz, nLoops](const CDCHit * cdcHit) {
      const CDCSimHit* cdcSimHit = cdcHit->getRelated<CDCSimHit>();
      if (not cdcSimHit) return false;
      const MCParticle* mcParticle = cdcSimHit->getRelated<MCParticle>();
      if (not mcParticle) return false;

      const double tof = cdcSimHit->getFlightTime();
      const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;
      const float absMom3D = mcParticle->getMomentum().Mag();

      const double loopLength = 2 * M_PI * absMom3D / (Bz * 0.00299792458);
      const double loopTOF =  loopLength / speed;
      if (tof > loopTOF * nLoops) {
        return false;
      } else {
        return true;
      }
    };

    // create a list containing the indices to the CDCHits that belong to one track
    int nAxialHits = 0;
    int nStereoHits = 0;
    if (m_useCDCHits) {
      const RelationVector<CDCHit>& relatedHits = aMcParticlePtr->getRelationsTo<CDCHit>();

      for (size_t i = 0; i < relatedHits.size(); ++i) {
        if (relatedHits.weight(i) < 0) continue;  // skip hits from secondary particles

        const CDCHit* cdcHit = relatedHits.object(i);

        // Reject higher order hits if requested
        if (not std::isnan(nLoops) and not isWithinNLoops(cdcHit)) continue;

        int superLayerId = cdcHit->getISuperLayer();

        const CDCSimHit* aCDCSimHitPtr = cdcHit->getRelatedFrom<CDCSimHit>();
        if (not aCDCSimHitPtr) {
          B2DEBUG(100, "     Skipping CDCHit without related CDCSimHit.");
          continue;
        }
        double time = aCDCSimHitPtr->getFlightTime();

        // Here it is hardcoded what superlayer has axial wires and what has stereo wires.
        // Maybe it would be better if the WireId would know this
        if (superLayerId % 2 == 0) {
          hitsWithTimeAndDetectorInformation.emplace_back(time, cdcHit->getArrayIndex(), Const::CDC);
          ndf += 1;
          ++nAxialHits;
        } else {
          if (not m_useOnlyAxialCDCHits) {
            hitsWithTimeAndDetectorInformation.emplace_back(time, cdcHit->getArrayIndex(), Const::CDC);
            ndf += 1;
            ++nStereoHits;
          }
        }
      }

      B2DEBUG(100, "    added " << nAxialHits << " axial and " << nStereoHits << " stereo CDCHits");
    }
    if (nAxialHits < m_minCDCAxialHits or (not m_useOnlyAxialCDCHits and nStereoHits < m_minCDCStereoHits)) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }


    if (m_initialCov(0, 0) > 0.0) { //using a user set initial cov and corresponding smearing of inital state adds information
      ndf += 5;
    }
    if (ndf < m_minimalNdf) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }

    std::sort(hitsWithTimeAndDetectorInformation.begin(), hitsWithTimeAndDetectorInformation.end(),
    [](const TimeHitIDDetector & rhs, const TimeHitIDDetector & lhs) {
      return std::get<0>(rhs) < std::get<0>(lhs);
    });

    //Now create TrackCandidate
    int counter = recoTracks.getEntries();
    B2DEBUG(100, "We came pass all filter of the MCPartile and hit properties. TrackCandidate " << counter <<
            " will be created from the MCParticle with index: " << iPart << " (PDG: " << aMcParticlePtr->getPDG() << ")");

    //set track parameters from MCParticle information
    TVector3 positionTrue = aMcParticlePtr->getProductionVertex();
    TVector3 momentumTrue = aMcParticlePtr->getMomentum();
    double timeTrue = aMcParticlePtr->getProductionTime();

    // if no kind of smearing is activated the initial values (seeds) for track fit will be the simulated truth
    TVector3 momentum = momentumTrue;
    TVector3 position = positionTrue;
    double time = timeTrue;
    TVectorD stateSeed(6); //this will
    TMatrixDSym covSeed(6);
    covSeed.Zero(); // just to be save
    covSeed(0, 0) = 1; covSeed(1, 1) = 1; covSeed(2, 2) = 2 * 2;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.2 * 0.2;
    //it may have positive effect on the fit not to start with exactly precise true values (or it may be just interesting to study this)
    //one can smear the starting momentum values with a gaussian
    //this calculation is always performed, but with the default value of m_smearing = 0 it has no effect on momentum and position (true values are taken)

    if (m_smearing > 0.0) {
      double smearing = m_smearing / 100.0;  //the module parameter m_smearing goes from 0 to 100, smearing should go from 0 to 1

      double smearedX = gRandom->Gaus(positionTrue.x(), smearing * positionTrue.x());
      double smearedY = gRandom->Gaus(positionTrue.y(), smearing * positionTrue.y());
      double smearedZ = gRandom->Gaus(positionTrue.z(), smearing * positionTrue.z());
      position.SetXYZ(smearedX, smearedY, smearedZ);
      double smearedPX = gRandom->Gaus(momentumTrue.x(), smearing * momentumTrue.x());
      double smearedPY = gRandom->Gaus(momentumTrue.y(), smearing * momentumTrue.y());
      double smearedPZ = gRandom->Gaus(momentumTrue.z(), smearing * momentumTrue.z());
      momentum.SetXYZ(smearedPX, smearedPY, smearedPZ);
    }

    //Errors for the position/momentum values can also be passed to genfit::TrackCandidate
    //Default values in Genfit are (1.,1.,1.,), they seem to be not good!!
    //The best way to set the 'correct' errors has to be investigated....
    if (m_initialCov(0, 0) > 0.0) { // alternative seamring with according to a covariance matrix
      double smearedX = gRandom->Gaus(positionTrue.x(), sqrt(m_initialCov(0, 0)));
      double smearedY = gRandom->Gaus(positionTrue.y(), sqrt(m_initialCov(1, 1)));
      double smearedZ = gRandom->Gaus(positionTrue.z(), sqrt(m_initialCov(2, 2)));
      position.SetXYZ(smearedX, smearedY, smearedZ);
      double smearedPX = gRandom->Gaus(momentumTrue.x(), sqrt(m_initialCov(3, 3)));
      double smearedPY = gRandom->Gaus(momentumTrue.y(), sqrt(m_initialCov(4, 4)));
      double smearedPZ = gRandom->Gaus(momentumTrue.z(), sqrt(m_initialCov(5, 5)));
      momentum.SetXYZ(smearedPX, smearedPY, smearedPZ);
      covSeed = m_initialCov;
    }

    //Finally create RecoTrack
    // TODO: In former times, the track candidate also stored the PDG code!!!
    short int charge = static_cast<short int>(aMcParticlePtr->getCharge());
    RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge);
    if (m_setTimeSeed) {
      newRecoTrack->setTimeSeed(time);
    }
    ++m_nRecoTracks;


    //create relation between the track candidates and the mcParticle (redundant to saving the MCId)
    newRecoTrack->addRelationTo(aMcParticlePtr);
    B2DEBUG(100, " --- Create relation between genfit::TrackCand " << counter << " and MCParticle " << iPart);

    // Removed that part is it uses the CDC geometry which is not present in the TB setting
    /*
    CDC::CDCGeometryPar& cdcGeometry = CDC::CDCGeometryPar::Instance();
    */

    hitCounter = 0;
    for (const TimeHitIDDetector& hitInformation : hitsWithTimeAndDetectorInformation) {
      const Const::EDetector& detectorInformation = std::get<2>(hitInformation);
      const int hitID = std::get<1>(hitInformation);

      if (detectorInformation == Const::CDC) {
        // no CDC for the testbeam
        /*
        const CDCHit* cdcHit = cdcHits[hitID];
        const CDCSimHit* aCDCSimHitPtr = cdcHit->getRelatedFrom<CDCSimHit>();

        //now determine the correct sign to resolve the left right ambiguity in the fitter
        TVector3 simHitPos = aCDCSimHitPtr->getPosTrack();
        TVector3 simMom = aCDCSimHitPtr->getMomentum();
        TVector3 simHitPosOnWire = aCDCSimHitPtr->getPosWire();

        const unsigned short isRightHit = cdcGeometry.getNewLeftRightRaw(simHitPosOnWire, simHitPos, simMom);

        if (isRightHit) {
          newRecoTrack->addCDCHit(cdcHit, hitCounter, RecoHitInformation::RightLeftInformation::c_right,
                                  RecoHitInformation::OriginTrackFinder::c_MCTrackFinder);
        } else {
          newRecoTrack->addCDCHit(cdcHit, hitCounter, RecoHitInformation::RightLeftInformation::c_left,
                                  RecoHitInformation::OriginTrackFinder::c_MCTrackFinder);
        }
        B2DEBUG(101, "CDC hit " << hitID << " has reft/right sign " << isRightHit);
        */
      } else if (detectorInformation == Const::PXD) {
        const PXDCluster* pxdCluster = pxdClusters[hitID];
        newRecoTrack->addPXDHit(pxdCluster, hitCounter, RecoHitInformation::OriginTrackFinder::c_MCTrackFinder);
      } else if (detectorInformation == Const::SVD) {
        const SVDCluster* svdCluster = svdClusters[hitID];
        newRecoTrack->addSVDHit(svdCluster, hitCounter, RecoHitInformation::OriginTrackFinder::c_MCTrackFinder);
      }

      ++hitCounter;
    }
  }//end loop over MCParticles
}

void TrackFinderMCTruthRecoTracksTBModule::endRun()
{
  if (m_notEnoughtHitsCounter != 0) {
    B2WARNING(m_notEnoughtHitsCounter << " tracks had not enough hits to have at least " << m_minimalNdf <<
              " number of degrees of freedom (NDF). No Track Candidates were created from them so they will not be passed to the track fitter");
  }
  if (m_noTrueHitCounter != 0) {
    B2WARNING(m_noTrueHitCounter <<
              " cluster hits did not have a relation to a true hit and were therefore not included in a track candidate");
  }
  B2INFO("The MCTrackFinder created a total of " << m_nRecoTracks << " track candidates");
}
