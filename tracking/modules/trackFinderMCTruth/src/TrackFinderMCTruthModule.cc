/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderMCTruth/TrackFinderMCTruthModule.h>

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
#include <genfit/TrackCand.h>
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
REG_MODULE(TrackFinderMCTruth)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFinderMCTruthModule::TrackFinderMCTruthModule() : Module()
{
  //Set module properties
  setDescription("Uses the MC information to create genfit::TrackCandidates for primary MCParticles and Relations between them. "
                 "Fills the created genfit::TrackCandidates with all information (start values, hit indices) needed for the fitting.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //Parameter definition
  addParam("UseClusters", m_useClusters, "Set true if you want to use PXD/SVD clusters instead of PXD/SVD trueHits", bool(true));

  //choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits",
           m_usePXDHits,
           "Set true if PXDHits or PXDClusters should be used",
           true);
  addParam("UseSVDHits",
           m_useSVDHits,
           "Set true if SVDHits or SVDClusters should be used",
           true);
  addParam("UseCDCHits",
           m_useCDCHits,
           "Set true if CDCHits should be used",
           true);
  addParam("UseOnlyAxialCDCHits",
           m_useOnlyAxialCDCHits,
           "Set true if only the axial CDCHits should be used",
           false);
  addParam("UseNLoops",
           m_useNLoops,
           "Set the number of loops to be included in the MC tracks. "
           "Default includes all",
           NAN);


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
  addParam("GFTrackCandidatesColName",
           m_gfTrackCandsColName,
           "Name of collection holding the genfit::TrackCandidates (output)",
           std::string(""));

  addParam("TrueHitMustExist",
           m_enforceTrueHit,
           "If set true only cluster hits that have a relation to a TrueHit will be included in the track candidate",
           false);
}


TrackFinderMCTruthModule::~TrackFinderMCTruthModule()
{

}


void TrackFinderMCTruthModule::initialize()
{
  B2FATAL("This module is depricated as it uses genfit::Track(Cand)s instead of RecoTracks. It will be removed in the future. If you need information on the transition, please contact Nils Braun (nils.braun@kit.edu).");
  StoreArray<MCParticle> mcparticles;
  mcparticles.isRequired();

  //output store arrays have to be registered in initialize()
  StoreArray<genfit::TrackCand> trackCandidates(m_gfTrackCandsColName);
  trackCandidates.registerInDataStore();

  trackCandidates.registerRelationTo(mcparticles);

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

void TrackFinderMCTruthModule::beginRun()
{
  m_notEnoughtHitsCounter = 0;
  m_noTrueHitCounter = 0;
  m_nTrackCands = 0;
}


void TrackFinderMCTruthModule::event()
{
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

  RelationIndex<MCParticle, PXDTrueHit> relMcPxdTrueHit;

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

  RelationIndex<MCParticle, SVDTrueHit> relMcSvdTrueHit;

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
  StoreArray<genfit::TrackCand> trackCandidates(m_gfTrackCandsColName);
  RelationArray gfTrackCandToMCPart(trackCandidates, mcParticles);

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


    B2DEBUG(100, "Search a track for the MCParticle with index: " << iPart << " (PDG: " << aMcParticlePtr->getPDG() << ")");

    int ndf = 0; // cout the ndf of one track candidate
    // create a list containing the indices to the PXDHits that belong to one track
    std::vector<int> pxdHitsIndices;
    if (m_usePXDHits == true) {
      if (m_useClusters == false) {
        for (int i = 0; i < nMcPartToPXDHits; ++i) {
          if (mcPartToPXDTrueHits[i].getFromIndex() == unsigned(iPart)) {
            for (unsigned int j = 0; j < mcPartToPXDTrueHits[i].getToIndices().size(); j++) {
              if (mcPartToPXDTrueHits[i].getWeight(j) < 0) continue;  // skip hits from secondary particles
              pxdHitsIndices.push_back(mcPartToPXDTrueHits[i].getToIndex(j));
              ndf += 2;
            }
          }
        }
      } else {
        for (int i = 0; i < nPxdClusterToMCPart; ++i) {
          for (unsigned int j = 0; j < pxdClusterToMCParticle[i].getToIndices().size(); j++) {
            if (pxdClusterToMCParticle[i].getToIndex(j) == unsigned(iPart)) {
              if (pxdClusterToMCParticle[i].getWeight(j) < 0) continue;  // skip hits from secondary particles
              pxdHitsIndices.push_back(pxdClusterToMCParticle[i].getFromIndex());
              ndf += 2;
            }
          }
        }
      }
    }
    if (pxdHitsIndices.size() < unsigned(m_minPXDHits)) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }
    // create a list containing the indices to the SVDHits that belong to one track
    std::vector<int> svdHitsIndices;
    if (m_useSVDHits == true) {
      if (m_useClusters == false) {
        for (int i = 0; i < nMcPartToSVDHits; ++i) {
          if (mcPartToSVDTrueHits[i].getFromIndex() == unsigned(iPart)) {
            for (unsigned int j = 0; j < mcPartToSVDTrueHits[i].getToIndices().size(); j++) {
              if (mcPartToSVDTrueHits[i].getWeight(j) < 0) continue;   // skip hits from secondary particles
              svdHitsIndices.push_back(mcPartToSVDTrueHits[i].getToIndex(j));
              ndf += 2;
            }
          }
        }
      } else {
        for (int i = 0; i < nSvdClusterToMCPart; ++i) {
          for (unsigned int j = 0; j < svdClusterToMCParticle[i].getToIndices().size(); j++) {
            if (svdClusterToMCParticle[i].getToIndex(j) == unsigned(iPart)) {
              if (svdClusterToMCParticle[i].getWeight(j) < 0) continue; // skip hits from secondary particles
              svdHitsIndices.push_back(svdClusterToMCParticle[i].getFromIndex());
              ndf += 1;
            }
          }
        }
      }
    }
    if (svdHitsIndices.size() < unsigned(m_minSVDHits)) {
      ++m_notEnoughtHitsCounter;
      continue; //goto next mcParticle, do not make track candidate
    }

    // prepare rejection of CDC hits from higher order loops
    const TVector3 origin(0.0, 0.0, 0.0);
    const double Bz = BFieldMap::Instance().getBField(origin).Z();
    const double nLoops = m_useNLoops;
    auto isWithinNLoops = [Bz, nLoops](const CDCHit * hit) {
      const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
      if (not simHit) return false;
      const MCParticle* mcParticle = simHit->getRelated<MCParticle>();
      if (not mcParticle) return false;

      const double tof = simHit->getFlightTime();
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
    std::vector<int> cdcHitsIndices;
    if (m_useCDCHits == true) {
      for (int i = 0; i < nMcPartToCDCHits; ++i) {
        if (mcPartToCDCHits[i].getFromIndex() == unsigned(iPart)) {
          for (unsigned int j = 0; j < mcPartToCDCHits[i].getToIndices().size(); j++) {
            if (mcPartToCDCHits[i].getWeight(j) < 0) continue;  // skip hits from secondary particles
            int cdcHitIndex = mcPartToCDCHits[i].getToIndex(j);

            const CDCHit* hit = cdcHits[cdcHitIndex];

            // Reject higher order hits if requested
            if (not std::isnan(nLoops) and not isWithinNLoops(hit)) continue;

            int superLayerId = hit->getISuperLayer();
            // Here it is hardcoded what superlayer has axial wires and what has stereo wires.
            // Maybe it would be better if the WireId would know this
            if (superLayerId % 2 == 0) {
              cdcHitsIndices.push_back(cdcHitIndex);
              ndf += 1;
              ++nAxialHits;
            } else {
              if (not m_useOnlyAxialCDCHits) {
                cdcHitsIndices.push_back(cdcHitIndex);
                ndf += 1;
                ++nStereoHits;
              }
            }
          }
        }
      }
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
    //Now create TrackCandidate
    int counter = trackCandidates.getEntries();
    B2DEBUG(100, "We came pass all filter of the MCPartile and hit properties. TrackCandidate " << counter <<
            " will be created from the MCParticle with index: " << iPart << " (PDG: " << aMcParticlePtr->getPDG() << ")");

    //create TrackCandidate
    trackCandidates.appendNew();
    ++m_nTrackCands;
    //set track parameters from MCParticle information
    TVector3 positionTrue = aMcParticlePtr->getProductionVertex();
    TVector3 momentumTrue = aMcParticlePtr->getMomentum();
    int pdg = aMcParticlePtr->getPDG();
    // if no kind of smearing is activated the initial values (seeds) for track fit will be the simulated truth
    TVector3 momentum = momentumTrue;
    TVector3 position = positionTrue;
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
    stateSeed(0) = position[0]; stateSeed(1) = position[1]; stateSeed(2) = position[2];
    stateSeed(3) = momentum[0]; stateSeed(4) = momentum[1]; stateSeed(5) = momentum[2];

    //Finally set the complete track seed
    trackCandidates[counter]->set6DSeedAndPdgCode(stateSeed, pdg);

    //Save the MCParticleID in the TrackCandidate
    trackCandidates[counter]->setMcTrackId(iPart);

    //create relation between the track candidates and the mcParticle (redundant to saving the MCId)
    gfTrackCandToMCPart.add(counter, iPart);
    B2DEBUG(100, " --- Create relation between genfit::TrackCand " << counter << " and MCParticle " << iPart);

    //member variable Dip is currently used to store the purity of the tracks, for MCTracks it is always 100 %
    //trackCandidates[counter]->setDip(100); //it is better to compare genfit::TrackCands from this module with a "real" track finder to get calculate the purity in a separate step than putting it into something with intended for something else

    //assign indices of the Hits from all detectors, their are distinguishable by their DetID. The official detector ids in basf2 are:
    // pxd 1
    //   svd 2
    //     cdc 3
    if (m_usePXDHits && m_useClusters == false) {
      BOOST_FOREACH(int hitID, pxdHitsIndices) {
        float time = pxdTrueHits[hitID]->getGlobalTime();
        trackCandidates[counter]->addHit(Const::PXD, hitID, -1,
                                         double(time)); // -1 means the hit will not compete with any other hit in the DAF
      }
      B2DEBUG(100, "     add " << pxdHitsIndices.size() << " PXDHits");
    }

    if (m_usePXDHits && m_useClusters) {
      RelationIndex<PXDCluster, PXDTrueHit> relPxdClusterTrueHit;
      unsigned int hitCounter = 0;
      BOOST_FOREACH(int hitID, pxdHitsIndices) {
        RelationIndex<PXDCluster, PXDTrueHit>::range_from iterPairCluTr = relPxdClusterTrueHit.getElementsFrom(pxdClusters[hitID]);
        if (iterPairCluTr.empty()
            && m_enforceTrueHit == true) { // there is not trueHit! trow away hit because there is no time information for sorting
          ++m_noTrueHitCounter;
          continue;
        }
        float time = -1;
        RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcTr = relMcPxdTrueHit.getElementsFrom(aMcParticlePtr);
        for (const auto& relElementCluTr :
             iterPairCluTr) {  // make sure only a true hit is taken that really comes from the current mcParticle. This must be carefully checked because several trueHits from different real tracks can be melted into one cluster
          if (time >= 0) break;
          for (const auto& relElementMcTr : iterPairMcTr) {
            if (relElementMcTr.to == relElementCluTr.to) {
              time = relElementCluTr.to->getGlobalTime();
              break;
            }
          }
        }

        trackCandidates[counter]->addHit(Const::PXD, hitID, -1, double(time));
        ++hitCounter;
      }
      B2DEBUG(100, "     add " << hitCounter << " PXDClusters. " << pxdHitsIndices.size() - hitCounter <<
              " PXDClusters were not added because they do not have a corresponding PXDTrueHit");
    }
    if (m_useSVDHits && m_useClusters == false) {
      BOOST_FOREACH(int hitID, svdHitsIndices) {
        float time = svdTrueHits[hitID]->getGlobalTime();
        trackCandidates[counter]->addHit(Const::SVD, hitID, -1, double(time));
      }
      B2DEBUG(100, "     add " << svdHitsIndices.size() << " SVDHits");
    }
    if (m_useSVDHits && m_useClusters) {
      RelationIndex<SVDCluster, SVDTrueHit> relSvdClusterTrueHit;
      unsigned int hitCounter = 0;
      BOOST_FOREACH(int hitID, svdHitsIndices) {
        RelationIndex<SVDCluster, SVDTrueHit>::range_from iterPairCluTr = relSvdClusterTrueHit.getElementsFrom(svdClusters[hitID]);
        if (iterPairCluTr.empty()
            && m_enforceTrueHit == true) { // there is not trueHit! throw away hit because there is no time information for sorting
          ++m_noTrueHitCounter;
          continue;
        }
        float time = -1;
        RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcTr = relMcSvdTrueHit.getElementsFrom(aMcParticlePtr);
        for (const auto& relElementCluTr :
             iterPairCluTr) {  // make sure only a true hit is taken that really comes from the current mcParticle. This must be carefully checked because several trueHits from different real tracks can be melted into one cluster
          if (time >= 0) break;
          for (const auto& relElementMcTr : iterPairMcTr) {
            if (relElementMcTr.to == relElementCluTr.to) {
              time = relElementCluTr.to->getGlobalTime();
              break;
            }
          }
        }
        trackCandidates[counter]->addHit(Const::SVD, hitID, -1, double(time));
        ++hitCounter;
      }
      B2DEBUG(100, "     add " << hitCounter << " SVDClusters. " << svdHitsIndices.size() - hitCounter <<
              " SVDClusters were not added because they do not have a corresponding SVDTrueHit");
    }


    if (m_useCDCHits) {
      CDC::CDCGeometryPar& cdcGeometry = CDC::CDCGeometryPar::Instance();
      float time = -1;
      BOOST_FOREACH(int hitID, cdcHitsIndices) {
        //set the time as the ordering parameter rho for genfit to do this search for any CDCSimHit that corresponds to the CDCHit and take the time from there
        //be aware that this is some kind of hack because it is not garanteed by the framework that the "first" simhit belonging to a cdchit is really comming from the original particle
        //it might also come from a secondary particle created by material effects. But the code here seems to work.
        CDCSimHit* aCDCSimHitPtr = NULL;
        for (int j = 0; j != nCdcSimHitToHitRel; ++j) {
          if (unsigned(hitID) == cdcSimHitToHitRel[j].getToIndex(0)) {
            aCDCSimHitPtr = cdcSimHits[cdcSimHitToHitRel[j].getFromIndex()];
            break;
          }
        }
        time = aCDCSimHitPtr->getFlightTime();
        //now determine the correct sign to resolve the left right ambiguity in the fitter
        TVector3 simHitPos = aCDCSimHitPtr->getPosTrack();
        TVector3 simMom = aCDCSimHitPtr->getMomentum();
        TVector3 simHitPosOnWire = aCDCSimHitPtr->getPosWire();
        char lrAmbiSign = cdcGeometry.getNewLeftRightRaw(simHitPosOnWire, simHitPos, simMom) ? 1 : -1;
        genfit::WireTrackCandHit* aCdcTrackCandHit = new genfit::WireTrackCandHit(Const::CDC, hitID, -1, time,
            lrAmbiSign); //do not delete! the genfit::TrackCand has ownership
        trackCandidates[counter]->addHit(aCdcTrackCandHit);
        B2DEBUG(101, "CDC hit " << hitID << " has reft/right sign " << int(lrAmbiSign));
      }
      B2DEBUG(100, "    add " << cdcHitsIndices.size() << " CDCHits");
    }



    // now after all the hits belonging to one track are added to a track candidate
    // bring them into the right order inside the trackCand objects using the rho/time parameter
    //     trackCandidates[counter]->Print();
    trackCandidates[counter]->sortHits();
    //     trackCandidates[counter]->Print();

  }//end loop over MCParticles
}

void TrackFinderMCTruthModule::endRun()
{
  if (m_notEnoughtHitsCounter != 0) {
    B2WARNING(m_notEnoughtHitsCounter << " tracks had not enough hits to have at least " << m_minimalNdf <<
              " number of degrees of freedom (NDF). No Track Candidates were created from them so they will not be passed to the track fitter");
  }
  if (m_noTrueHitCounter != 0) {
    B2WARNING(m_noTrueHitCounter <<
              " cluster hits did not have a relation to a true hit and were therefore not included in a track candidate");
  }
  B2INFO("The MCTrackFinder created a total of " << m_nTrackCands << " track candidates");
}

void TrackFinderMCTruthModule::terminate()
{
}
