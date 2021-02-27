//+
// File : PantherInutModule.cc
// Description : A module to read panther records in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contributors: Anze Zupanc, Matic Lubej
//-

#include <b2bii/modules/B2BIIMdstInput/B2BIIConvertMdstModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/database/Database.h>
#include <framework/pcore/ProcHandler.h>

#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>

// Belle II utilities
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

// Belle II dataobjects
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/Helix.h>
#include <framework/dataobjects/UncertainHelix.h>

// Belle utilities
#include <b2bii/utility/BelleMdstToGenHepevt.h>

// ROOT
#include <TVector3.h>
#include <TLorentzVector.h>

#include <limits>
#include <algorithm>
#include <queue>
#include <utility>

#ifdef HAVE_EID
#include "belle_legacy/eid/eid.h"
#endif

#ifdef HAVE_KID_ACC
#include "belle_legacy/kid/kid_acc.h"
#include "belle_legacy/kid/kid_cdc.h"
#endif

#ifdef HAVE_FINDKS
#include "belle_legacy/findKs/findKs.h"
#endif

#ifdef HAVE_NISKSFINDER
#include "belle_legacy/nisKsFinder/nisKsFinder.h"
#endif

#ifdef HAVE_GOODLAMBDA
#include "belle_legacy/findLambda/findLambda.h"
#endif

#include "belle_legacy/benergy/BeamEnergy.h"
#include "belle_legacy/ip/IpProfile.h"
#include "belle_legacy/tables/evtcls.h"
#include "belle_legacy/tables/trg.h"


#include <cmath>
#include <cfloat>
#include <bitset>
using namespace Belle2;

const Const::ChargedStable B2BIIConvertMdstModule::c_belleHyp_to_chargedStable[c_nHyp] = { Const::electron, Const::muon, Const::pion, Const::kaon, Const::proton };

bool approximatelyEqual(float a, float b, float epsilon)
{
  return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

double adjustAngleRange(double phi)
{
  phi = phi - int(phi / TMath::TwoPi()) * TMath::TwoPi();
  return phi - int(phi / TMath::Pi()) * TMath::TwoPi();
}

void fill7x7ErrorMatrix(const TrackFitResult* tfr, TMatrixDSym& error7x7, const double mass, const double bField)
{
  short charge = tfr->getChargeSign();

  double d0    = tfr->getD0();
  double phi0  = tfr->getPhi0();
  double omega = tfr->getOmega();
  //double z0    = tfr->getZ0();
  double tanl  = tfr->getTanLambda();

  double alpha = tfr->getHelix().getAlpha(bField);

  double cosPhi0 = TMath::Cos(phi0);
  double sinPhi0 = TMath::Sin(phi0);

  double rho;
  if (omega != 0)
    rho = 1.0 / alpha / omega;
  else
    rho = (DBL_MAX);

  double energy = TMath::Sqrt(mass * mass + (1.0 + tanl * tanl) * rho * rho);

  const int iPx = 0;
  const int iPy = 1;
  const int iPz = 2;
  const int iE  = 3;
  const int iX  = 4;
  const int iY  = 5;
  const int iZ  = 6;

  const int iD0    = 0;
  const int iPhi0  = 1;
  const int iOmega = 2;
  const int iZ0    = 3;
  const int iTanl  = 4;

  TMatrixD jacobian(7, 5);
  jacobian.Zero();

  jacobian(iPx, iPhi0)  = - fabs(rho) * sinPhi0;
  jacobian(iPx, iOmega) = - charge * rho * rho * cosPhi0 * alpha;
  jacobian(iPy, iPhi0)  =   fabs(rho) * cosPhi0;
  jacobian(iPy, iOmega) = - charge * rho * rho * sinPhi0 * alpha;
  jacobian(iPz, iOmega) = - charge * rho * rho * tanl * alpha;
  jacobian(iPz, iTanl)  =   fabs(rho);
  if (omega != 0 && energy != 0) {
    jacobian(iE, iOmega) = - (1.0 + tanl * tanl) * rho * rho / omega / energy;
    jacobian(iE, iTanl)  = tanl * rho * rho / energy;
  } else {
    jacobian(iE, iOmega) = (DBL_MAX);
    jacobian(iE, iTanl)  = (DBL_MAX);
  }
  jacobian(iX, iD0)     =   sinPhi0;
  jacobian(iX, iPhi0)   = d0 * cosPhi0;
  jacobian(iY, iD0)     = - cosPhi0;
  jacobian(iY, iPhi0)   = d0 * sinPhi0;
  jacobian(iZ, iZ0)     = 1.0;

  TMatrixDSym error5x5 = tfr->getCovariance5();

  error7x7 = error5x5.Similarity(jacobian);
}
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2BIIConvertMdst)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2BIIConvertMdstModule::B2BIIConvertMdstModule() : Module(),
  m_mcMatchingMode(c_Direct)
{
  //Set module properties
  setDescription("Converts Belle mDST objects (Panther tables and records) to Belle II mDST objects.");

  addParam("convertBeamParameters", m_convertBeamParameters,
           "Convert beam parameters or use information stored in "
           "Belle II database.", true);
  addParam("use6x6CovarianceMatrix4Tracks", m_use6x6CovarianceMatrix4Tracks,
           "Use 6x6 (position, momentum) covariance matrix for charged tracks instead of 5x5 (helix parameters) covariance matrix", false);
  addParam("mcMatchingMode", m_mcMatchingModeString,
           "MC matching mode: 'Direct', or 'GeneratorLevel'",
           std::string("Direct"));
  addParam("matchType2E9oE25Threshold", m_matchType2E9oE25Threshold,
           "clusters with a E9/E25 value above this threshold are classified as neutral even if tracks are matched to their connected region (matchType == 2)",
           -1.1);

  addParam("convertEvtcls", m_convertEvtcls, "Flag to switch on conversion of Mdst_evtcls", true);
  addParam("nisKsInfo", m_nisEnable, "Flag to switch on conversion of nisKsFinder info", true);
  addParam("RecTrg", m_convertRecTrg, "Flag to switch on conversion of rectrg_summary3", false);
  addParam("TrkExtra", m_convertTrkExtra, " Flag to switch on conversion of first_x,y,z and last_x,y,z from Mdst_trk_fit", true);

  m_realData = false;

  B2DEBUG(1, "B2BIIConvertMdst: Constructor done.");
}


B2BIIConvertMdstModule::~B2BIIConvertMdstModule()
{
}

void B2BIIConvertMdstModule::initialize()
{
  // Initialize Belle II DataStore
  initializeDataStore();
  if (m_mcMatchingModeString == "Direct")
    m_mcMatchingMode = c_Direct;
  else if (m_mcMatchingModeString == "GeneratorLevel")
    m_mcMatchingMode = c_GeneratorLevel;
  else
    B2FATAL("Unknown MC matching mode: " << m_mcMatchingModeString);
  B2INFO("B2BIIConvertMdst: initialized.");
  if (!m_nisEnable)
    B2WARNING("nisKsFinder output has been disabled. ksnbVLike, ksnbNoLam, ksnbStandard will not be converted.");
}

void B2BIIConvertMdstModule::initializeDataStore()
{
  B2DEBUG(99, "[B2BIIConvertMdstModule::initializeDataStore] initialization of DataStore started");

  // list here all converted Belle2 objects
  m_eclClusters.registerInDataStore();
  m_klmClusters.registerInDataStore();
  m_tracks.registerInDataStore();
  m_trackFitResults.registerInDataStore();
  m_v0s.registerInDataStore();
  m_particles.registerInDataStore();

  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  extraInfoMap.registerInDataStore();

  if (m_convertEvtcls || m_convertRecTrg) m_evtInfo.registerInDataStore();

  if (m_convertTrkExtra) m_belleTrkExtra.registerInDataStore();

  StoreObjPtr<ParticleList> gammaParticleList("gamma:mdst");
  gammaParticleList.registerInDataStore();
  StoreObjPtr<ParticleList> pi0ParticleList("pi0:mdst");
  pi0ParticleList.registerInDataStore();
  StoreObjPtr<ParticleList> kShortParticleList("K_S0:mdst");
  kShortParticleList.registerInDataStore();
  StoreObjPtr<ParticleList> kLongParticleList("K_L0:mdst");
  kLongParticleList.registerInDataStore();
  StoreObjPtr<ParticleList> lambdaParticleList("Lambda0:mdst");
  lambdaParticleList.registerInDataStore();
  StoreObjPtr<ParticleList> antiLambdaParticleList("anti-Lambda0:mdst");
  antiLambdaParticleList.registerInDataStore();
  StoreObjPtr<ParticleList> gammaConversionsParticleList("gamma:v0mdst");
  gammaConversionsParticleList.registerInDataStore();

  m_pidLikelihoods.registerInDataStore();

  // needs to be registered, even if running over data, since this information is available only at the begin_run function
  m_mcParticles.registerInDataStore();

  //list here all Relations between Belle2 objects
  m_tracks.registerRelationTo(m_mcParticles);
  m_tracks.registerRelationTo(m_pidLikelihoods);
  if (m_convertTrkExtra) m_tracks.registerRelationTo(m_belleTrkExtra);
  m_eclClusters.registerRelationTo(m_mcParticles);
  m_tracks.registerRelationTo(m_eclClusters);
  m_klmClusters.registerRelationTo(m_tracks);
  m_klmClusters.registerRelationTo(m_eclClusters);
  m_particles.registerRelationTo(m_mcParticles);
  m_particles.registerRelationTo(m_pidLikelihoods);

  B2DEBUG(99, "[B2BIIConvertMdstModule::initializeDataStore] initialization of DataStore ended");
}


void B2BIIConvertMdstModule::beginRun()
{
  B2DEBUG(99, "B2BIIConvertMdst: beginRun called.");

  if (m_convertBeamParameters) {
    //BeamEnergy class updated by fixmdst module in beginRun()
    Belle::BeamEnergy::begin_run();
    convertBeamEnergy();
    Belle::BeamEnergy::dump();

    // load IP data from DB server
    Belle::IpProfile::begin_run();
    convertIPProfile(true);
    Belle::IpProfile::dump();
    bool usableIP = Belle::IpProfile::usable();
    B2DEBUG(99, "B2BIIConvertMdst: IpProfile is usable = " << usableIP);
  }

  //init eID
#ifdef HAVE_EID
  Belle::eid::init_data();
  Belle::eid::show_use("ALL");
#endif
}


void B2BIIConvertMdstModule::event()
{
  if (m_convertBeamParameters) {
    // Are we running on MC or DATA?
    Belle::Belle_event_Manager& evman = Belle::Belle_event_Manager::get_manager();
    Belle::Belle_event& evt = evman[0];

    if (evt.ExpMC() == 2)
      m_realData = false; // <- this is MC sample
    else
      m_realData = true;  // <- this is real data sample

    // 0. Convert IPProfile to BeamSpot
    convertIPProfile();

    // Make sure beam parameters are correct: if they are not found in the
    // database or different from the ones in the database we need to override them
    if (!m_beamSpotDB || !(m_beamSpot == *m_beamSpotDB) ||
        !m_collisionBoostVectorDB || !m_collisionInvMDB) {
      if ((!m_beamSpotDB || !m_collisionBoostVectorDB || !m_collisionInvMDB) && !m_realData) {
        B2INFO("No database entry for this run yet, create one");
        StoreObjPtr<EventMetaData> event;
        IntervalOfValidity iov(event->getExperiment(), event->getRun(), event->getExperiment(), event->getRun());
        Database::Instance().storeData("CollisionBoostVector", &m_collisionBoostVector, iov);
        Database::Instance().storeData("CollisionInvariantMass", &m_collisionInvM, iov);
        Database::Instance().storeData("BeamSpot", &m_beamSpot, iov);
      }
      if (m_realData) {
        B2ERROR("BeamParameters from condition database are different from converted "
                "ones, overriding database. Did you make sure the globaltag B2BII is used?");
      } else {
        B2INFO("BeamSpot, BoostVector, and InvariantMass from condition database are different from converted "
               "ones, overriding database");
      }
      if (ProcHandler::parallelProcessingUsed()) {
        B2FATAL("Cannot reliably override the Database content in parallel processing "
                "mode, please run the conversion in single processing mode");
      }
      DBStore::Instance().addConstantOverride("CollisionBoostVector", new CollisionBoostVector(m_collisionBoostVector), true);
      DBStore::Instance().addConstantOverride("CollisionInvariantMass", new CollisionInvariantMass(m_collisionInvM), true);
      DBStore::Instance().addConstantOverride("BeamSpot", new BeamSpot(m_beamSpot), true);
    }
  }

  // 1. Convert MC information
  convertGenHepEvtTable();

  // 2. Convert ECL information
  convertMdstECLTable();

  // 3. Convert KLM information
  convertMdstKLMTable();

  // 4. Convert Tracking information
  convertMdstChargedTable();

  // 5. Set Track -> ECLCluster relations
  setTracksToECLClustersRelations();

  // 6. Set KLMCluster -> Track, ECLCluster relations
  setKLMClustersRelations();

  // 7. Convert Gamma information
  convertMdstGammaTable();

  // 8. Convert Pi0 information
  convertMdstPi0Table();

  // 9. Convert V0s
  convertMdstVee2Table();

  // 10. Convert KLong information
  convertMdstKLongTable();

  // 11. Convert Evtcls panther table information
  if (m_convertEvtcls) convertEvtclsTable();

  // 12. Convert trigger information from rectrg_summary3
  if (m_convertRecTrg) convertRecTrgTable();

}


//-----------------------------------------------------------------------------
// CONVERT TABLES
//-----------------------------------------------------------------------------
void B2BIIConvertMdstModule::convertBeamEnergy()
{
  const double Eher = Belle::BeamEnergy::E_HER();
  const double Eler = Belle::BeamEnergy::E_LER();
  const double crossingAngle = Belle::BeamEnergy::Cross_angle();
  const double angleLer = M_PI; //parallel to negative z axis (different from Belle II!)
  const double angleHer = crossingAngle; //in positive z and x direction, verified to be consistent with Upsilon(4S) momentum
  const double mass_e = Const::electronMass;    //mass of electron: 0.0 in basf, 0.000510998902 in basf2;
  TMatrixDSym covariance(0);    //0 entries = no error
  HepLorentzVector p_beam = Belle::BeamEnergy::p_beam(); // Testing only

  // Get four momentum of LER and HER
  TLorentzVector P_her(0.0, 0.0, TMath::Sqrt(Eher * Eher - mass_e * mass_e), Eher);
  P_her.RotateY(angleHer);
  TLorentzVector P_ler(0.0, 0.0, TMath::Sqrt(Eler * Eler - mass_e * mass_e), Eler);
  P_ler.RotateY(angleLer);

  // Get four momentum of beam
  TLorentzVector P_beam = P_her + P_ler;

  m_collisionBoostVector.setBoost(P_beam.BoostVector(), covariance);
  m_collisionInvM.setMass(P_beam.M(), 0.0 , 0.0);

  B2DEBUG(99, "Beam Energy: E_HER = " << Eher << "; E_LER = " << Eler << "; angle = " << crossingAngle);
  B2DEBUG(99, "Beam Momentum (pre-convert) : P_X = " << p_beam.px() << "; P_Y = " << p_beam.py() << "; P_Z = " << p_beam.pz());
  B2DEBUG(99, "Beam Momentum (post-convert) : P_X = " << P_beam.Px() << "; P_Y = " << P_beam.Py() << "; P_Z = " << P_beam.Pz());
}

void B2BIIConvertMdstModule::convertIPProfile(bool beginRun)
{
  if (!Belle::IpProfile::usable()) {
    // No IPProfile for this run ...
    if (beginRun) {
      // no IPProfile, set vertex to NaN without errors for the full run
      m_beamSpot.setIP(
        TVector3(std::numeric_limits<double>::quiet_NaN(),
                 std::numeric_limits<double>::quiet_NaN(),
                 std::numeric_limits<double>::quiet_NaN()
                ), TMatrixTSym<double>()
      );
    }
    return;
  }
  HepPoint3D ip;
  CLHEP::HepSymMatrix ipErr;
  if (beginRun) {
    // use event independent average in begin run
    ip = Belle::IpProfile::position();
    ipErr = Belle::IpProfile::position_err();
  } else {
    // update evtbin
    Belle::IpProfile::set_evtbin_number();
    // check if it changed, if not there's nothing to do
    if (Belle::IpProfile::EvtBinNo() == m_lastIPProfileBin) return;
    // get event dependent position and error
    ip = Belle::IpProfile::e_position();
    ipErr = Belle::IpProfile::e_position_err();
  }
  // reset last ipprofile bin
  m_lastIPProfileBin = Belle::IpProfile::EvtBinNo();

  TMatrixDSym cov(ipErr.num_col());
  for (int i = 0; i < ipErr.num_row(); ++i) {
    for (int j = 0; j < ipErr.num_col(); ++j) {
      cov(i, j) = ipErr(i + 1, j + 1);
    }
  }
  m_beamSpot.setIP(TVector3(ip.x(), ip.y(), ip.z()), cov);
}

void B2BIIConvertMdstModule::convertMdstChargedTable()
{
  // Relations
  RelationArray tracksToMCParticles(m_tracks, m_mcParticles);

  // Loop over all Belle charged tracks
  Belle::Mdst_charged_Manager& m = Belle::Mdst_charged_Manager::get_manager();
  for (Belle::Mdst_charged_Manager::iterator chargedIterator = m.begin(); chargedIterator != m.end(); ++chargedIterator) {
    Belle::Mdst_charged belleTrack = *chargedIterator;

    auto track = m_tracks.appendNew();

    // convert MDST_Charged -> Track
    convertMdstChargedObject(belleTrack, track);

    convertPIDData(belleTrack, track);

    if (m_realData)
      continue;

    // create Track -> MCParticle relation
    // step 1: MDSTCharged -> Gen_hepevt
    const Belle::Gen_hepevt& hep0 = get_hepevt(belleTrack);
    if (hep0 == 0)
      continue;
    const Belle::Gen_hepevt* hep = nullptr;
    switch (m_mcMatchingMode) {
      case c_Direct:
        hep = &hep0;
        break;
      case c_GeneratorLevel:
        hep = &gen_level(hep0);
        break;
    }
    // step 2: Gen_hepevt -> MCParticle
    if (genHepevtToMCParticle.count(hep->get_ID()) > 0) {
      int matchedMCParticle = genHepevtToMCParticle[hep->get_ID()];

      // step 3: set the relation
      tracksToMCParticles.add(track->getArrayIndex(), matchedMCParticle);

      testMCRelation(*hep, m_mcParticles[matchedMCParticle], "Track");
    } else {
      B2DEBUG(99, "Can not find MCParticle corresponding to this gen_hepevt (Panther ID = " << hep->get_ID() << ")");
      B2DEBUG(99, "Gen_hepevt: Panther ID = " << hep->get_ID() << "; idhep = " << hep->idhep() << "; isthep = " << hep->isthep());
    }
  }
}

void B2BIIConvertMdstModule::convertMdstVee2Table()
{
  // Create and initialize K_S0 particle list
  StoreObjPtr<ParticleList> ksPList("K_S0:mdst");
  ksPList.create();
  ksPList->initialize(310, ksPList.getName());

  // Create and initialize Lambda0 and anti-Lamda0 particle list
  StoreObjPtr<ParticleList> lambda0PList("Lambda0:mdst");
  lambda0PList.create();
  lambda0PList->initialize(3122, lambda0PList.getName());

  StoreObjPtr<ParticleList> antiLambda0PList("anti-Lambda0:mdst");
  antiLambda0PList.create();
  antiLambda0PList->initialize(-3122, antiLambda0PList.getName());

  antiLambda0PList->bindAntiParticleList(*lambda0PList);

  // Create and initialize converted gamma particle list
  StoreObjPtr<ParticleList> convGammaPList("gamma:v0mdst");
  convGammaPList.create();
  convGammaPList->initialize(22, convGammaPList.getName());

  // Loop over all Belle Vee2 candidates
  Belle::Mdst_vee2_Manager& m = Belle::Mdst_vee2_Manager::get_manager();
  for (Belle::Mdst_vee2_Manager::iterator vee2Iterator = m.begin(); vee2Iterator != m.end(); ++vee2Iterator) {
    Belle::Mdst_vee2 belleV0 = *vee2Iterator;

    // +ve track
    Belle::Mdst_charged belleTrackP = belleV0.chgd(0);
    // -ve track
    Belle::Mdst_charged belleTrackM = belleV0.chgd(1);

    // type of V0
    Const::ChargedStable pTypeP(Const::pion);
    Const::ChargedStable pTypeM(Const::pion);
    int belleHypP = -1;
    int belleHypM = -1;

    switch (belleV0.kind()) {
      case 1 : // K0s -> pi+ pi-
        pTypeP = Const::pion;
        pTypeM = Const::pion;
        belleHypP = 2;
        belleHypM = 2;
        break;
      case 2 : // Lambda -> p+ pi-
        pTypeP = Const::proton;
        pTypeM = Const::pion;
        belleHypP = 4;
        belleHypM = 2;
        break;
      case 3 : // anti-Lambda -> pi+ anti-p-
        pTypeP = Const::pion;
        pTypeM = Const::proton;
        belleHypP = 2;
        belleHypM = 4;
        break;
      case 4 : // gamma -> e+ e-
        pTypeP = Const::electron;
        pTypeM = Const::electron;
        belleHypP = 0;
        belleHypM = 0;
        break;
      default :
        B2WARNING("Conversion of vee2 candidate of unknown kind! kind = " << belleV0.kind());
    }

    // This part is copied from Relation.cc in BASF
    int trackID[2] = {0, 0};
    unsigned nTrack = 0;
    Belle::Mdst_charged_Manager& charged_mag = Belle::Mdst_charged_Manager::get_manager();
    for (std::vector<Belle::Mdst_charged>::iterator chgIterator = charged_mag.begin(); chgIterator != charged_mag.end();
         ++chgIterator) {
      if (belleV0.chgd(0).get_ID() >= 1 && trackID[0] == 0 && belleV0.chgd(0).get_ID() == chgIterator->get_ID()) {
        trackID[0] = (int)(chgIterator->get_ID()); //+ve trac
        ++nTrack;
      }
      if (belleV0.chgd(1).get_ID() >= 1 && trackID[1] == 0 && belleV0.chgd(1).get_ID() == chgIterator->get_ID()) {
        trackID[1] = (int)(chgIterator->get_ID()); //-ve trac
        ++nTrack;
      }
      if (nTrack == 2)
        break;
    }

    HepPoint3D dauPivot(belleV0.vx(), belleV0.vy(), belleV0.vz());
    int trackFitPIndex = -1;
    int trackFitMIndex = -1;
    Particle daughterP, daughterM;
    CLHEP::HepLorentzVector momentumP;
    CLHEP::HepSymMatrix     error7x7P(7, 0);
    HepPoint3D       positionP;
    TMatrixFSym errMatrixP(7);
    CLHEP::HepLorentzVector momentumM;
    CLHEP::HepSymMatrix     error7x7M(7, 0);
    HepPoint3D       positionM;
    TMatrixFSym errMatrixM(7);
    CLHEP::HepSymMatrix error5x5(5, 0);
    if (trackID[0] >= 1) {
      if (belleV0.daut()) {
        std::vector<float> helixParam(5);
        std::vector<float> helixError(15);
        belleVeeDaughterHelix(belleV0, 1, helixParam, helixError);

        auto trackFitP = m_trackFitResults.appendNew(helixParam, helixError, pTypeP, 0.5, -1, -1, 0);
        trackFitPIndex = trackFitP->getArrayIndex();

        belleVeeDaughterToCartesian(belleV0, 1, pTypeP, momentumP, positionP, error7x7P);
        TrackFitResult* tmpTFR = new TrackFitResult(createTrackFitResult(momentumP, positionP, error7x7P, 1, pTypeP, 0.5, -1, -1, 0));
        // TrackFitResult internaly stores helix parameters at pivot = (0,0,0) so the momentum of the Particle will be wrong again.
        // Overwrite it.

        for (unsigned i = 0; i < 7; i++)
          for (unsigned j = 0; j < 7; j++)
            errMatrixP(i, j) = error7x7P[i][j];

        daughterP = Particle(trackID[0] - 1, tmpTFR, pTypeP);
        daughterP.updateMomentum(TLorentzVector(momentumP.px(), momentumP.py(), momentumP.pz(), momentumP.e()),
                                 TVector3(positionP.x(), positionP.y(), positionP.z()),
                                 errMatrixP, 0.5);
        delete tmpTFR;
      } else {
        Belle::Mdst_trk_fit& trk_fit = charged_mag[trackID[0] - 1].trk().mhyp(belleHypP);
        double pValue = TMath::Prob(trk_fit.chisq(), trk_fit.ndf());

        std::vector<float> helixParam(5);
        std::vector<float> helixError(15);
        convertHelix(trk_fit, HepPoint3D(0., 0., 0.), helixParam, helixError);

        // Checking for invalid helix curvature with parameter 2 equal to 0:
        if (helixParam[2] == 0) {
          B2WARNING("Helix parameter for curvature == 0. Skipping Track! The parameter is: " << helixParam[2] << "...");
          continue;
        }

        auto trackFitP = m_trackFitResults.appendNew(helixParam, helixError, pTypeP, pValue, -1, -1, 0);

        trackFitPIndex = trackFitP->getArrayIndex();

        daughterP = Particle(trackID[0] - 1, trackFitP, pTypeP);
        // set momentum/positions at pivot = V0 decay vertex
        getHelixParameters(trk_fit, pTypeP.getMass(), dauPivot,
                           helixParam,  error5x5,
                           momentumP, positionP, error7x7P);

        for (unsigned i = 0; i < 7; i++)
          for (unsigned j = 0; j < 7; j++)
            errMatrixP(i, j) = error7x7P[i][j];

        daughterP.updateMomentum(TLorentzVector(momentumP.px(), momentumP.py(), momentumP.pz(), momentumP.e()),
                                 TVector3(positionP.x(), positionP.y(), positionP.z()),
                                 errMatrixP, pValue);
      }
    }
    if (trackID[1] >= 1) {
      if (belleV0.daut()) {
        std::vector<float> helixParam(5);
        std::vector<float> helixError(15);
        belleVeeDaughterHelix(belleV0, -1, helixParam, helixError);

        auto trackFitM = m_trackFitResults.appendNew(helixParam, helixError, pTypeM, 0.5, -1, -1, 0);
        trackFitMIndex = trackFitM->getArrayIndex();

        belleVeeDaughterToCartesian(belleV0, -1, pTypeM, momentumM, positionM, error7x7M);
        TrackFitResult* tmpTFR = new TrackFitResult(createTrackFitResult(momentumM, positionM, error7x7M, -1, pTypeM, 0.5, -1, -1, 0));
        // TrackFitResult internaly stores helix parameters at pivot = (0,0,0) so the momentum of the Particle will be wrong again.
        // Overwrite it.
        for (unsigned i = 0; i < 7; i++)
          for (unsigned j = 0; j < 7; j++)
            errMatrixM(i, j) = error7x7M[i][j];

        daughterM = Particle(trackID[1] - 1, tmpTFR, pTypeM);
        daughterM.updateMomentum(TLorentzVector(momentumM.px(), momentumM.py(), momentumM.pz(), momentumM.e()),
                                 TVector3(positionM.x(), positionM.y(), positionM.z()),
                                 errMatrixM, 0.5);
        delete tmpTFR;
      } else {
        Belle::Mdst_trk_fit& trk_fit = charged_mag[trackID[1] - 1].trk().mhyp(belleHypM);
        double pValue = TMath::Prob(trk_fit.chisq(), trk_fit.ndf());

        std::vector<float> helixParam(5);
        std::vector<float> helixError(15);
        convertHelix(trk_fit, HepPoint3D(0., 0., 0.), helixParam, helixError);

        // Checking for invalid helix curvature with parameter 2 equal to 0:
        if (helixParam[2] == 0) {
          B2WARNING("Helix parameter for curvature == 0. Skipping Track! The parameter is: " << helixParam[2] << "...");
          continue;
        }

        auto trackFitM = m_trackFitResults.appendNew(helixParam, helixError, pTypeM, pValue, -1, -1, 0);

        trackFitMIndex = trackFitM->getArrayIndex();

        daughterM = Particle(trackID[1] - 1, trackFitM, pTypeM);
        // set momentum/positions at pivot = V0 decay vertex
        getHelixParameters(trk_fit, pTypeM.getMass(), dauPivot,
                           helixParam,  error5x5,
                           momentumM, positionM, error7x7M);

        for (unsigned i = 0; i < 7; i++)
          for (unsigned j = 0; j < 7; j++)
            errMatrixM(i, j) = error7x7M[i][j];

        daughterM.updateMomentum(TLorentzVector(momentumM.px(), momentumM.py(), momentumM.pz(), momentumM.e()),
                                 TVector3(positionM.x(), positionM.y(), positionM.z()),
                                 errMatrixM, pValue);
      }
    }

    Track* trackP = m_tracks[trackID[0] - 1];
    Track* trackM = m_tracks[trackID[1] - 1];

    TrackFitResult* trackFitP = m_trackFitResults[trackFitPIndex];
    TrackFitResult* trackFitM = m_trackFitResults[trackFitMIndex];

    m_v0s.appendNew(std::make_pair(trackP, trackFitP), std::make_pair(trackM, trackFitM));

    // create Ks Particle and add it to the 'K_S0:mdst' ParticleList
    const PIDLikelihood* pidP = trackP->getRelated<PIDLikelihood>();
    const PIDLikelihood* pidM = trackM->getRelated<PIDLikelihood>();
    const MCParticle* mcParticleP = trackP->getRelated<MCParticle>();
    const MCParticle* mcParticleM = trackM->getRelated<MCParticle>();

    Particle* newDaugP = m_particles.appendNew(daughterP);
    if (pidP)
      newDaugP->addRelationTo(pidP);
    if (mcParticleP)
      newDaugP->addRelationTo(mcParticleP);
    Particle* newDaugM = m_particles.appendNew(daughterM);
    if (pidM)
      newDaugM->addRelationTo(pidM);
    if (mcParticleM)
      newDaugM->addRelationTo(mcParticleM);

    TLorentzVector v0Momentum(belleV0.px(), belleV0.py(), belleV0.pz(), belleV0.energy());
    TVector3 v0Vertex(belleV0.vx(), belleV0.vy(), belleV0.vz());

    /*
     * Documentation of Mdst_vee2 vertex fit:
     *      /sw/belle/belle/b20090127_0910/share/tables/mdst.tdf (L96-L125)
     */
    auto appendVertexFitInfo = [](Belle::Mdst_vee2 & _belle_V0, Particle & _belle2_V0) {
      // Add chisq of vertex fit. chiSquared=10^10 means the fit fails.
      _belle2_V0.addExtraInfo("chiSquared", _belle_V0.chisq());
      // Ndf of the vertex kinematic fit is 1
      _belle2_V0.addExtraInfo("ndf", 1);
      // Add p-value to extra Info
      double prob = TMath::Prob(_belle_V0.chisq(), 1);
      _belle2_V0.setPValue(prob);
    };

    Particle* newV0 = nullptr;
    if (belleV0.kind() == 1) { // K0s -> pi+ pi-
      Particle KS(v0Momentum, 310);
      KS.appendDaughter(newDaugP);
      KS.appendDaughter(newDaugM);
      KS.setVertex(v0Vertex);
      appendVertexFitInfo(belleV0, KS);
      newV0 = m_particles.appendNew(KS);
      ksPList->addParticle(newV0);

      // append extra info: goodKs flag
      Belle::FindKs belleKSFinder;
      belleKSFinder.candidates(belleV0, Belle::IpProfile::position(1));
      newV0->addExtraInfo("goodKs", belleKSFinder.goodKs());

      /*
      std::cout << " ---- B1 Ks ---- " << std::endl;
      std::cout << " momentum = " << std::endl;
      v0Momentum.Print();
      std::cout << " position = " << std::endl;
      v0Vertex.Print();
      std::cout << " ---- B2 Ks ---- " << std::endl;
      std::cout << " momentum = " << std::endl;
      newKS->get4Vector().Print();
      std::cout << " position = " << std::endl;
      newKS->getVertex().Print();
      std::cout << " ---- B1 Ks.child(0) ---- " << std::endl;
      std::cout << " momentum = " << momentumP << std::endl;
      std::cout << " position = " << positionP << std::endl;
      std::cout << " error7x7 = " << error7x7P << std::endl;
      std::cout << " ---- B2 Ks.child(0) ---- " << std::endl;
      std::cout << " momentum = " << std::endl;
      newKS->getDaughter(0)->get4Vector().Print();
      std::cout << " position = " << std::endl;
      newKS->getDaughter(0)->getVertex().Print();
      std::cout << " error7x7 = " << std::endl;
      newKS->getDaughter(0)->getMomentumVertexErrorMatrix().Print();
      std::cout << " ---- B1 Ks.child(1) ---- " << std::endl;
      std::cout << " momentum = " << momentumM << std::endl;
      std::cout << " position = " << positionM << std::endl;
      std::cout << " error7x7 = " << error7x7M << std::endl;
      std::cout << " ---- B2 Ks.child(1) ---- " << std::endl;
      std::cout << " momentum = " << std::endl;
      newKS->getDaughter(1)->get4Vector().Print();
      std::cout << " position = " << std::endl;
      newKS->getDaughter(1)->getVertex().Print();
      std::cout << " error7x7 = " << std::endl;
      newKS->getDaughter(1)->getMomentumVertexErrorMatrix().Print();
      */
    } else if (belleV0.kind() == 2) { // Lambda -> p+ pi-
      Particle Lambda0(v0Momentum, 3122);
      Lambda0.appendDaughter(newDaugP);
      Lambda0.appendDaughter(newDaugM);
      Lambda0.setVertex(v0Vertex);
      appendVertexFitInfo(belleV0, Lambda0);
      newV0 = m_particles.appendNew(Lambda0);
      lambda0PList->addParticle(newV0);

      // GoodLambda flag as extra info
      Belle::FindLambda lambdaFinder;
      lambdaFinder.candidates(belleV0, Belle::IpProfile::position(1));
      newV0->addExtraInfo("goodLambda", lambdaFinder.goodLambda());
    } else if (belleV0.kind() == 3) { // anti-Lambda -> pi+ anti-p
      Particle antiLambda0(v0Momentum, -3122);
      antiLambda0.appendDaughter(newDaugM);
      antiLambda0.appendDaughter(newDaugP);
      antiLambda0.setVertex(v0Vertex);
      appendVertexFitInfo(belleV0, antiLambda0);
      newV0 = m_particles.appendNew(antiLambda0);
      antiLambda0PList->addParticle(newV0);

      // GoodLambda flag as extra info
      Belle::FindLambda lambdaFinder;
      lambdaFinder.candidates(belleV0, Belle::IpProfile::position(1));
      newV0->addExtraInfo("goodLambda", lambdaFinder.goodLambda());
    } else if (belleV0.kind() == 4) { // gamma -> e+ e-
      Particle gamma(v0Momentum, 22);
      gamma.appendDaughter(newDaugP);
      gamma.appendDaughter(newDaugM);
      gamma.setVertex(v0Vertex);
      appendVertexFitInfo(belleV0, gamma);
      newV0 = m_particles.appendNew(gamma);
      convGammaPList->addParticle(newV0);
    }
    // append extra info: nisKsFinder quality indicators
    if (m_nisEnable) {
      if (belleV0.kind() <= 3) { // K_S0, Lambda, anti-Lambda
        Belle::nisKsFinder ksnb;
        double protIDP = atcPID(pidP, 2, 4);
        double protIDM = atcPID(pidM, 2, 4);
        ksnb.candidates(belleV0, Belle::IpProfile::position(1), momentumP, protIDP, protIDM);
        // K_S0 and Lambda (inverse cut on ksnbNoLam for Lambda selection).
        newV0->addExtraInfo("ksnbVLike", ksnb.nb_vlike());
        newV0->addExtraInfo("ksnbNoLam", ksnb.nb_nolam());
        // K_S0 only
        if (belleV0.kind() == 1)
          newV0->addExtraInfo("ksnbStandard", ksnb.standard());
      }
    }
  }
}

void B2BIIConvertMdstModule::convertGenHepEvtTable()
{
  if (m_realData)
    return;

  // clear the Gen_hepevt_ID <-> MCParticleGraphPosition map
  genHepevtToMCParticle.clear();

  // check if the Gen_hepevt table has any entries
  Belle::Gen_hepevt_Manager& genMgr = Belle::Gen_hepevt_Manager::get_manager();
  if (genMgr.count() == 0) {
    return;
  }

  m_particleGraph.clear();

  int position = m_particleGraph.size();

  // Start with the root (mother-of-all) particle (1st particle in gen_hepevt table)
  m_particleGraph.addParticle();
  Belle::Gen_hepevt rootParticle = genMgr(Belle::Panther_ID(1));
  genHepevtToMCParticle[1] = position;

  MCParticleGraph::GraphParticle* p = &m_particleGraph[position];
  convertGenHepevtObject(rootParticle, p);

  // at this stage (before all other particles) all "motherless" particles (i.e. beam background)
  // have to be added to Particle graph
  /*
  // if this is uncommented then beam background hits will be added to the MCParticle array
  for (Belle::Gen_hepevt_Manager::iterator genIterator = genMgr.begin(); genIterator != genMgr.end(); ++genIterator) {
    Belle::Gen_hepevt hep = *genIterator;
    if (hep.moFirst() == 0 && hep.moLast() == 0 && hep.get_ID() > 1) {
      // Particle has no mother
      // put the particle in the graph:
      position = m_particleGraph.size();
      m_particleGraph.addParticle();
      genHepevtToMCParticle[hep.get_ID()] = position;

      MCParticleGraph::GraphParticle* graphParticle = &m_particleGraph[position];
      convertGenHepevtObject(hep, graphParticle);
    }
  }
  */

  typedef std::pair<MCParticleGraph::GraphParticle*, Belle::Gen_hepevt> halfFamily;
  halfFamily currFamily;
  halfFamily family;
  std::queue < halfFamily > heritancesQueue;

  for (int idaughter = rootParticle.daFirst(); idaughter <= rootParticle.daLast(); ++idaughter) {
    if (idaughter == 0) {
      B2DEBUG(95, "Trying to access generated daughter with Panther ID == 0");
      continue;
    }

    currFamily.first = p;
    currFamily.second = genMgr(Belle::Panther_ID(idaughter));
    heritancesQueue.push(currFamily);
  }

  //now we can go through the queue:
  while (!heritancesQueue.empty()) {
    currFamily = heritancesQueue.front(); //get the first entry from the queue
    heritancesQueue.pop(); //remove the entry.

    MCParticleGraph::GraphParticle* currMother = currFamily.first;
    Belle::Gen_hepevt& currDaughter = currFamily.second;

    // skip particle with idhep = 0
    if (currDaughter.idhep() == 0)
      continue;

    //putting the daughter in the graph:
    position = m_particleGraph.size();
    m_particleGraph.addParticle();
    genHepevtToMCParticle[currDaughter.get_ID()] = position;

    MCParticleGraph::GraphParticle* graphDaughter = &m_particleGraph[position];
    convertGenHepevtObject(currDaughter, graphDaughter);

    //add relation between mother and daughter to graph:
    currMother->decaysInto((*graphDaughter));

    int nGrandChildren = currDaughter.daLast() - currDaughter.daFirst() + 1;

    if (nGrandChildren > 0 && currDaughter.daFirst() != 0) {
      for (int igrandchild = currDaughter.daFirst(); igrandchild <= currDaughter.daLast(); ++igrandchild) {
        if (igrandchild == 0) {
          B2DEBUG(95, "Trying to access generated daughter with Panther ID == 0");
          continue;
        }

        family.first = graphDaughter;
        family.second = genMgr(Belle::Panther_ID(igrandchild));
        heritancesQueue.push(family);
      }
    }
  }

  m_particleGraph.generateList();
}

void B2BIIConvertMdstModule::convertMdstECLTable()
{
  // Relations
  RelationArray eclClustersToMCParticles(m_eclClusters, m_mcParticles);

  // Clear the mdstEcl <-> ECLCluster map
  mdstEclToECLCluster.clear();

  // Loop over all Belle Mdst_ecl
  Belle::Mdst_ecl_Manager& ecl_manager = Belle::Mdst_ecl_Manager::get_manager();
  Belle::Mdst_ecl_aux_Manager& ecl_aux_manager = Belle::Mdst_ecl_aux_Manager::get_manager();

  for (Belle::Mdst_ecl_Manager::iterator eclIterator = ecl_manager.begin(); eclIterator != ecl_manager.end(); ++eclIterator) {

    // Pull Mdst_ecl from manager
    Belle::Mdst_ecl mdstEcl = *eclIterator;
    Belle::Mdst_ecl_aux mdstEclAux(ecl_aux_manager(mdstEcl.get_ID()));

    // Create Belle II ECLCluster
    auto B2EclCluster = m_eclClusters.appendNew();

    // Convert Mdst_ecl -> ECLCluster and create map of indices
    convertMdstECLObject(mdstEcl, mdstEclAux, B2EclCluster);
    mdstEclToECLCluster[mdstEcl.get_ID()] = B2EclCluster->getArrayIndex();

    // set ConnectedRegionID and ClusterID to
    // cluster's array index + 1 and 1, respectively
    B2EclCluster->setConnectedRegionId(B2EclCluster->getArrayIndex() + 1);
    B2EclCluster->setClusterId(1);

    if (m_realData)
      continue;

    // Create ECLCluster -> MCParticle relation
    // Step 1: MDST_ECL -> Gen_hepevt
    const Belle::Gen_hepevt& hep0 = get_hepevt(mdstEcl);
    if (hep0 == 0)
      continue;
    const Belle::Gen_hepevt* hep = nullptr;
    switch (m_mcMatchingMode) {
      case c_Direct:
        hep = &hep0;
        break;
      case c_GeneratorLevel:
        hep = &gen_level(hep0);
        break;
    }
    if (hep->idhep() != 911) {
      // Step 2: Gen_hepevt -> MCParticle
      if (genHepevtToMCParticle.count(hep->get_ID()) > 0) {
        int matchedMCParticleID = genHepevtToMCParticle[hep->get_ID()];
        // Step 3: set the relation
        eclClustersToMCParticles.add(B2EclCluster->getArrayIndex(), matchedMCParticleID);
        testMCRelation(*hep, m_mcParticles[matchedMCParticleID], "ECLCluster");
      } else {
        B2DEBUG(79, "Cannot find MCParticle corresponding to this gen_hepevt (Panther ID = " << hep->get_ID() << ")");
        B2DEBUG(79, "Gen_hepevt: Panther ID = " << hep->get_ID() << "; idhep = " << hep->idhep() << "; isthep = " << hep->isthep());
      }
    }
  }
}

void B2BIIConvertMdstModule::convertMdstKLMTable()
{
  // There was no MC matching in Belle for KLM Clusters

  // Clear the mdstKlm <-> KLMCluster map
  mdstKlmToKLMCluster.clear();

  // Loop over all Belle Mdst_klm_cluster
  Belle::Mdst_klm_cluster_Manager& klm_cluster_manager = Belle::Mdst_klm_cluster_Manager::get_manager();

  for (Belle::Mdst_klm_cluster_Manager::iterator klmC_Ite = klm_cluster_manager.begin(); klmC_Ite != klm_cluster_manager.end();
       ++klmC_Ite) {

    // Pull Mdst_ecl from manager
    Belle::Mdst_klm_cluster mdstKlm_cluster = *klmC_Ite;

    // Create Belle II ECLCluster
    auto B2KlmCluster = m_klmClusters.appendNew();

    // Convert Mdst_klm_cluster -> KLMCluster and create map of indices
    convertMdstKLMObject(mdstKlm_cluster, B2KlmCluster);
    mdstKlmToKLMCluster[mdstKlm_cluster.get_ID()] = B2KlmCluster->getArrayIndex();

  }
}

void B2BIIConvertMdstModule::convertMdstGammaTable()
{
  // Relations
  RelationArray particlesToMCParticles(m_particles, m_mcParticles);

  // Clear the mdstGamma <-> Particle map
  mdstGammaToParticle.clear();

  // Create and initialize particle list
  StoreObjPtr<ParticleList> plist("gamma:mdst");
  plist.create();
  plist->initialize(22, "gamma:mdst");

  // Loop over all Belle Mdst_gamma
  Belle::Mdst_gamma_Manager& gamma_manager = Belle::Mdst_gamma_Manager::get_manager();

  for (Belle::Mdst_gamma_Manager::iterator gammaIterator = gamma_manager.begin(); gammaIterator != gamma_manager.end();
       ++gammaIterator) {

    // Pull Mdst_gamma from manager and Mdst_ecl from pointer to Mdst_ecl
    Belle::Mdst_gamma mdstGamma = *gammaIterator;
    Belle::Mdst_ecl mdstEcl = mdstGamma.ecl();
    if (!mdstEcl)
      continue;

    // Get ECLCluster from map
    ECLCluster* B2EclCluster = m_eclClusters[mdstEclToECLCluster[mdstEcl.get_ID()]];
    if (!B2EclCluster)
      continue;

    // Create Particle from ECLCluster, add to StoreArray, create gamma map entry
    Particle* B2Gamma = m_particles.appendNew(B2EclCluster);
    mdstGammaToParticle[mdstGamma.get_ID()] = B2Gamma->getArrayIndex();

    // Add particle to particle list
    plist->addParticle(B2Gamma);

    if (m_realData)
      continue;

    // Relation to MCParticle
    MCParticle* matchedMCParticle = B2EclCluster->getRelated<MCParticle>();
    if (matchedMCParticle)
      B2Gamma->addRelationTo(matchedMCParticle);
  }
}

void B2BIIConvertMdstModule::convertMdstPi0Table()
{
  // Create and initialize particle list
  StoreObjPtr<ParticleList> plist("pi0:mdst");
  plist.create();
  plist->initialize(111, "pi0:mdst");

  // Loop over all Mdst_pi0
  Belle::Mdst_pi0_Manager& pi0_manager = Belle::Mdst_pi0_Manager::get_manager();
  for (Belle::Mdst_pi0_Manager::iterator pi0Iterator = pi0_manager.begin(); pi0Iterator != pi0_manager.end(); ++pi0Iterator) {

    // Pull Mdst_pi0 from manager and Mdst_gammas from pointers to Mdst_gammas
    Belle::Mdst_pi0 mdstPi0 = *pi0Iterator;
    Belle::Mdst_gamma mdstGamma1 = mdstPi0.gamma(0);
    Belle::Mdst_gamma mdstGamma2 = mdstPi0.gamma(1);
    if (!mdstGamma1 || !mdstGamma2)
      continue;

    TLorentzVector p4(mdstPi0.px(), mdstPi0.py(), mdstPi0.pz(), mdstPi0.energy());

    // Create Particle from TLorentzVector and PDG code, add to StoreArray
    Particle* B2Pi0 = m_particles.appendNew(p4, 111);

    // Get Belle II photons from map
    Particle* B2Gamma1 = m_particles[mdstGammaToParticle[mdstGamma1.get_ID()]];
    Particle* B2Gamma2 = m_particles[mdstGammaToParticle[mdstGamma2.get_ID()]];
    if (!B2Gamma1 || !B2Gamma2)
      continue;

    // Append photons as pi0 daughters
    B2Pi0->appendDaughter(B2Gamma1);
    B2Pi0->appendDaughter(B2Gamma2);

    // Add chisq of mass-constrained Kfit
    B2Pi0->addExtraInfo("chiSquared", mdstPi0.chisq());

    // Ndf of a pi0 mass-constrained kinematic fit is 1
    B2Pi0->addExtraInfo("ndf", 1);

    // Add p-value to extra Info
    double prob = TMath::Prob(mdstPi0.chisq(), 1);
    B2Pi0->setPValue(prob);

    // Add particle to particle list
    plist->addParticle(B2Pi0);
  }
}

void B2BIIConvertMdstModule::convertMdstKLongTable()
{
  // Relations
  RelationArray particlesToMCParticles(m_particles, m_mcParticles);


  // Create and initialize particle list
  StoreObjPtr<ParticleList> plist("K_L0:mdst");
  plist.create();
  plist->initialize(Const::Klong.getPDGCode(), "K_L0:mdst");

  Belle::Mdst_klong_Manager& klong_manager = Belle::Mdst_klong_Manager::get_manager();
  for (Belle::Mdst_klong_Manager::iterator klong_Ite = klong_manager.begin(); klong_Ite != klong_manager.end(); ++klong_Ite) {

    // Pull Mdst_klong from manager and Mdst_klm from pointer to Mdst_klm
    Belle::Mdst_klong mdstKlong = *klong_Ite;
    Belle::Mdst_klm_cluster mdstKlm = mdstKlong.klmc();

    if (!mdstKlm)
      continue;


    // Get KLMCluster from map
    KLMCluster* B2KlmCluster = m_klmClusters[mdstKlmToKLMCluster[mdstKlm.get_ID()]];
    if (!B2KlmCluster)
      continue;

    // Extract cluster position from Klong and save it in KLMCluster
    B2KlmCluster->setClusterPosition(mdstKlong.cos_x(), mdstKlong.cos_y(), mdstKlong.cos_z());

    // Create Particle from KLMCluster, add to StoreArray, create Klong map entry
    Particle* B2Klong = m_particles.appendNew(B2KlmCluster);
    mdstKlongToParticle[mdstKlong.get_ID()] = B2Klong->getArrayIndex();

    // Add particle to particle list
    plist->addParticle(B2Klong);
  }

  // (Vague) MC Matching
  // There was no MC matching for KLongs in Belle , but a hack:
  // Check if MC KLong and reconstructed KLong (only without ecl) are within 15 degree for phi and theta, we set a relation
  // for the best reconstructed KLong to the MC KLong.
  // Taken and adapted from http://belle.kek.jp/secured/wiki/doku.php?id=physics:ckm:kleff

  if (!m_realData) {

    Belle::Gen_hepevt_Manager& GenMgr = Belle::Gen_hepevt_Manager::get_manager();
    const double dang(15. / 180.*M_PI); // check reconstructed candidates within 15 degrees

    for (Belle::Gen_hepevt_Manager::iterator klong_hep_it = GenMgr.begin(); klong_hep_it != GenMgr.end(); ++klong_hep_it) {

      if (abs((*klong_hep_it).idhep()) == Const::Klong.getPDGCode() && klong_hep_it->isthep() > 0) {

        CLHEP::HepLorentzVector gp4(klong_hep_it->PX(), klong_hep_it->PY(), klong_hep_it->PZ(), klong_hep_it->E());
        double sum(0.0);
        int bestRecKlongID(0);

        for (Belle::Mdst_klong_Manager::iterator klong_rec_it = klong_manager.begin(); klong_rec_it != klong_manager.end();
             ++klong_rec_it) {

          //  if((*klong_rec_it).klmc().ecl())continue; // check only klm cand.
          if ((*klong_rec_it).ecl())
            continue; // check only klm cand.
          CLHEP::Hep3Vector klp3(klong_rec_it->cos_x(), klong_rec_it->cos_y(), klong_rec_it->cos_z());

          if (cos(gp4.theta() - klp3.theta()) > cos(dang) && cos(gp4.phi() - klp3.phi()) > cos(dang)) {

            double tmp_sum = cos(gp4.theta() - klp3.theta()) + cos(gp4.phi() - klp3.phi());
            if (tmp_sum > sum) {
              bestRecKlongID = mdstKlongToParticle[(*klong_rec_it).get_ID()];
              sum = tmp_sum;
            }
          }

        }
        if (sum > 0.0) {
          int matchedMCParticleID = genHepevtToMCParticle[(*klong_hep_it).get_ID()];
          particlesToMCParticles.add(bestRecKlongID, matchedMCParticleID);
          testMCRelation((*klong_hep_it), m_mcParticles[matchedMCParticleID], "m_particles");
        }
      }
    }
  }
}

void B2BIIConvertMdstModule::convertEvtclsTable()
{
  // Create StoreObj if it is not valid
  if (not m_evtInfo.isValid()) {
    m_evtInfo.create();
  }
  // Pull Evtcls_flag(2) from manager
  Belle::Evtcls_flag_Manager& EvtFlagMgr = Belle::Evtcls_flag_Manager::get_manager();
  Belle::Evtcls_flag2_Manager& EvtFlag2Mgr = Belle::Evtcls_flag2_Manager::get_manager();

  // Pull Evtcls_hadronic_flag from manager
  Belle::Evtcls_hadronic_flag_Manager& EvtHadFlagMgr = Belle::Evtcls_hadronic_flag_Manager::get_manager();

  std::string name = "evtcls_flag";
  std::string name_had = "evtcls_hadronic_flag";
  // Only one entry in each event
  std::vector<Belle::Evtcls_flag>::iterator eflagIterator = EvtFlagMgr.begin();
  std::vector<Belle::Evtcls_flag2>::iterator eflag2Iterator = EvtFlag2Mgr.begin();
  std::vector<Belle::Evtcls_hadronic_flag>::iterator ehadflagIterator = EvtHadFlagMgr.begin();

  // Converting evtcls_flag(2)
  std::vector<int> flag(20);
  for (int index = 0; index < 20; ++index) {
    // flag(14, 16): not filled
    if (index == 14 || index == 16) continue;
    std::string iVar = name + std::to_string(index);
    // 0-9 corresponding to evtcls_flag.flag(0-9)
    if (index < 10) {
      m_evtInfo->addExtraInfo(iVar, (*eflagIterator).flag(index));
    } else {
      // 10-19 corresponding to evtcls_flag2.flag(0-9)
      m_evtInfo->addExtraInfo(iVar, (*eflag2Iterator).flag(index - 10));
    }
    B2DEBUG(99, "evtcls_flag(" << index << ") = " << m_evtInfo->getExtraInfo(iVar));
  }

  // Converting evtcls_hadronic_flag
  for (int index = 0; index < 6; ++index) {
    std::string iVar = name_had + std::to_string(index);
    m_evtInfo->addExtraInfo(iVar, (*ehadflagIterator).hadronic_flag(index));
    B2DEBUG(99, "evtcls_hadronic_flag(" << index << ") = " << m_evtInfo->getExtraInfo(iVar));
  }

}

void B2BIIConvertMdstModule::convertRecTrgTable()
{

  // Create StoreObj if it is not valid
  if (not m_evtInfo.isValid()) {
    m_evtInfo.create();
  }

  // Pull rectrg_summary3 from manager
  Belle::Rectrg_summary3_Manager& RecTrgSummary3Mgr = Belle::Rectrg_summary3_Manager::get_manager();

  std::string name = "rectrg_summary3_m_final";
  // Only one entry in each event
  std::vector<Belle::Rectrg_summary3>::iterator eflagIterator = RecTrgSummary3Mgr.begin();

  // Converting m_final(3)
  for (int index = 0; index < 3; ++index) {
    std::string iVar = name + std::to_string(index);
    m_evtInfo->addExtraInfo(iVar, (*eflagIterator).final(index));
    B2DEBUG(99, "m_final(" << index << ") = " << m_evtInfo->getExtraInfo(iVar));
  }

}


//-----------------------------------------------------------------------------
// CONVERT OBJECTS
//-----------------------------------------------------------------------------

#ifdef HAVE_KID_ACC
double B2BIIConvertMdstModule::acc_pid(const Belle::Mdst_charged& chg, int idp)
{
  static Belle::kid_acc acc_pdf(0);
  //static kid_acc acc_pdf(1);

  const double pmass[5] = { 0.00051099907, 0.105658389, 0.13956995, 0.493677, 0.93827231 };

  CLHEP::Hep3Vector mom(chg.px(), chg.py(), chg.pz());
  double cos_theta = mom.cosTheta();
  double pval      = mom.mag();

  double npe    = chg.acc().photo_electron();
  double beta   = pval / sqrt(pval * pval + pmass[idp] * pmass[idp]);
  double pdfval = acc_pdf.npe2pdf(cos_theta, beta, npe);

  return pdfval;
}

// this is CDC_prob5
double B2BIIConvertMdstModule::cdc_pid(const Belle::Mdst_charged& chg, int idp)
{
  CLHEP::Hep3Vector mom(chg.px(), chg.py(), chg.pz());
  double pval = mom.mag();

  Belle::kid_cdc kidCdc(5);
  float factor0 = kidCdc.factor0();
  float factor1 = kidCdc.factor1(idp, pval);

  if (factor0 == 1.0 && factor1 == 1.0) return chg.trk().pid(idp);
  //
  double m = chg.trk().dEdx() / factor0;
  double e = chg.trk().dEdx_exp(idp) * factor1;
  double s = chg.trk().sigma_dEdx(idp);
  double val = 1. / sqrt(2.*M_PI) / s * exp(-0.5 * (m - e) * (m - e) / s / s);

  return val;
}
#endif

void B2BIIConvertMdstModule::setLikelihoods(PIDLikelihood* pid, Const::EDetector det, double likelihoods[c_nHyp],
                                            bool discard_allzero)
{
  if (discard_allzero) {
    const double max_l = *std::max_element(likelihoods, likelihoods + c_nHyp);
    if (max_l <= 0.0) {
      return; //likelihoods broken, ignore
    }
  }

  for (int i = 0; i < c_nHyp; i++) {
    float logl = log(likelihoods[i]);
    pid->setLogLikelihood(det, c_belleHyp_to_chargedStable[i], logl);
  }
  //copy proton likelihood to deuterons
  pid->setLogLikelihood(det, Const::deuteron, pid->getLogL(Const::proton, det));
}

void B2BIIConvertMdstModule::convertPIDData(const Belle::Mdst_charged& belleTrack, const Track* track)
{
  PIDLikelihood* pid = m_pidLikelihoods.appendNew();
  track->addRelationTo(pid);

  //convert data handled by atc_pid: dE/dx (-> CDC), TOF (-> TOP), ACC ( -> ARICH)
  //this should result in the same likelihoods used when creating atc_pid(3, 1, 5, ..., ...)
  //and calling prob(const Mdst_charged & chg).

  double likelihoods[c_nHyp];
  double accL[c_nHyp];
  double tofL[c_nHyp];
  double cdcL[c_nHyp];
  for (int i = 0; i < c_nHyp; i++) {
    accL[i] = tofL[i] = cdcL[i] = 1.0;
  }
#ifdef HAVE_KID_ACC
  //accq0 = 3, as implemented in acc_prob3()
  const auto& acc = belleTrack.acc();
  if (acc and acc.quality() == 0) {
    for (int i = 0; i < c_nHyp; i++)
      accL[i] = likelihoods[i] = acc_pid(belleTrack, i);
    setLikelihoods(pid, Const::ARICH, likelihoods, true);
  }
#endif

  //tofq0 = 1, as implemented in tof_prob1()
  //uses p1 / (p1 + p2) to create probability, so this should map directly to likelihoods
  const Belle::Mdst_tof& tof = belleTrack.tof();
  if (tof and tof.quality() == 0) {
    for (int i = 0; i < c_nHyp; i++)
      tofL[i] = likelihoods[i] = tof.pid(i);
    setLikelihoods(pid, Const::TOP, likelihoods, true);
  }

  // cdcq0 = 5, as implemented in cdc_prob0() (which is used for all values of cdcq0!)
  //uses p1 / (p1 + p2) to create probability, so this should map directly to likelihoods
  // eID actually uses cdc_pid (cdc_prob5)
  const Belle::Mdst_trk& trk = belleTrack.trk();
  if (trk.dEdx() > 0) {
    for (int i = 0; i < c_nHyp; i++) {
      likelihoods[i] = trk.pid(i);
      cdcL[i] = cdc_pid(belleTrack, i);
    }
    setLikelihoods(pid, Const::CDC, likelihoods, true);
  }


  // eid
  // eid is combination of atc_pid and ecl related information
  // since atc_pid part is already converted above only the ECL part
  // is converted
  // ECL pdfs are available only for electrons and hadrons (assumed to be pions)
  // likelihoods for others are set to 0

#ifdef HAVE_EID
  Belle::eid electronID(belleTrack);
  float eclID_e_pdf = electronID.pdf_e_ecl();
  float eclID_h_pdf = electronID.pdf_h_ecl();
  float atcID_e_pdf = electronID.atc_pid_pdf(true,  accL, tofL, cdcL);
  float atcID_h_pdf = electronID.atc_pid_pdf(false, accL, tofL, cdcL);

  // eID
  float eclProb = eclID_e_pdf / (eclID_e_pdf + eclID_h_pdf);
  float atcProb = atcID_e_pdf / (atcID_e_pdf + atcID_h_pdf);

  if (atcProb > 0.999999) atcProb = 0.999999;
  // combine the two probabilities.
  double eidCombinedSig = eclProb * atcProb;
  double eidCombinedBkg = (1. - eclProb) * (1. - atcProb);

  likelihoods[0] = eidCombinedSig;
  likelihoods[1] = 0; // no muons
  likelihoods[2] = eidCombinedBkg;
  likelihoods[3] = 0; // no kaons
  likelihoods[4] = 0; // no protons

  setLikelihoods(pid, Const::ECL, likelihoods, true);

  //Hep3Vector mom(belleTrack.px(), belleTrack.py(), belleTrack.pz());
  //B2INFO(" p = " << mom.mag() << "   le_ecl = " << electronID.le_ecl());
#endif

  //muid
  //Note that though it says "_likelihood()" on the label, those are
  //actually likelihood ratios of the type L(hyp) / (L(mu) + L(pi) + L(K)),
  //which are set in the FixMdst module.
  int muid_trackid = belleTrack.muid_ID();
  if (muid_trackid) {
    //Using approach 2. from http://belle.kek.jp/secured/muid/usage_muid.html since
    //it's much simpler than what Muid_mdst does.
    Belle::Mdst_klm_mu_ex_Manager& ex_mgr = Belle::Mdst_klm_mu_ex_Manager::get_manager();
    Belle::Mdst_klm_mu_ex& ex = ex_mgr(Belle::Panther_ID(muid_trackid));

    //filter out tracks with insufficient #hits (equal to cut on Muid_mdst::Chi_2())
    if (ex.Chi_2() > 0) {
      likelihoods[0] = 0; //no electrons
      likelihoods[1] = ex.Muon_likelihood();
      likelihoods[2] = ex.Pion_likelihood();
      likelihoods[3] = ex.Kaon_likelihood();
      likelihoods[4] = 0; //no protons
      //Miss_likelihood should only be != 0 for tracks that do not pass the Chi_2 cut.

      // in some cases the ex.XYZ_likelihood() < 0; Set it to 0 in these cases.
      for (int i = 0; i < 5; i++)
        if (likelihoods[i] < 0)
          likelihoods[i] = 0;

      //note: discard_allzero = false since all likelihoods = 0 usually means that Junk_likelihood is 1
      //      PIDLikelihood::getProbability(hyp) will correctly return 0 then.
      setLikelihoods(pid, Const::KLM, likelihoods);

      /*
      const double tolerance = 1e-7;
      if (fabs(pid->getProbability(Const::muon, nullptr, Const::KLM) - ex.Muon_likelihood()) > tolerance ||
          fabs(pid->getProbability(Const::pion, nullptr, Const::KLM) - ex.Pion_likelihood()) > tolerance ||
          fabs(pid->getProbability(Const::kaon, nullptr, Const::KLM) - ex.Kaon_likelihood()) > tolerance) {

      B2INFO("muons: " <<  pid->getProbability(Const::muon, nullptr, Const::KLM) << " " << ex.Muon_likelihood());
      B2INFO("pion: " <<  pid->getProbability(Const::pion, nullptr, Const::KLM) << " " << ex.Pion_likelihood());
      B2INFO("kaon: " <<  pid->getProbability(Const::kaon, nullptr, Const::KLM) << " " << ex.Kaon_likelihood());
      B2INFO("miss/junk: " << ex.Miss_likelihood() << " " << ex.Junk_likelihood());
      }
      */
    }
  }
}

int B2BIIConvertMdstModule::getHelixParameters(const Belle::Mdst_trk_fit& trk_fit,
                                               const double mass,
                                               const HepPoint3D& newPivot,
                                               std::vector<float>& helixParams,
                                               CLHEP::HepSymMatrix& error5x5,
                                               CLHEP::HepLorentzVector& momentum,
                                               HepPoint3D& position,
                                               CLHEP::HepSymMatrix& error7x7, const double dPhi)
{
  const HepPoint3D pivot(trk_fit.pivot_x(),
                         trk_fit.pivot_y(),
                         trk_fit.pivot_z());

  CLHEP::HepVector  a(5);
  a[0] = trk_fit.helix(0);
  a[1] = trk_fit.helix(1);
  a[2] = trk_fit.helix(2);
  a[3] = trk_fit.helix(3);
  a[4] = trk_fit.helix(4);
  CLHEP::HepSymMatrix Ea(5, 0);
  Ea[0][0] = trk_fit.error(0);
  Ea[1][0] = trk_fit.error(1);
  Ea[1][1] = trk_fit.error(2);
  Ea[2][0] = trk_fit.error(3);
  Ea[2][1] = trk_fit.error(4);
  Ea[2][2] = trk_fit.error(5);
  Ea[3][0] = trk_fit.error(6);
  Ea[3][1] = trk_fit.error(7);
  Ea[3][2] = trk_fit.error(8);
  Ea[3][3] = trk_fit.error(9);
  Ea[4][0] = trk_fit.error(10);
  Ea[4][1] = trk_fit.error(11);
  Ea[4][2] = trk_fit.error(12);
  Ea[4][3] = trk_fit.error(13);
  Ea[4][4] = trk_fit.error(14);

  Belle::Helix helix(pivot, a, Ea);

  int charge = 0;
  if (helix.kappa() > 0)
    charge = 1;
  else
    charge = -1;

  if (newPivot.x() != 0. || newPivot.y() != 0. || newPivot.z() != 0.) {
    helix.pivot(newPivot);
    momentum = helix.momentum(dPhi, mass, position, error7x7);
  } else {
    if (pivot.x() != 0. || pivot.y() != 0. || pivot.z() != 0.) {
      helix.pivot(HepPoint3D(0., 0., 0.));
      momentum = helix.momentum(dPhi, mass, position, error7x7);
    } else {
      momentum = helix.momentum(dPhi, mass, position, error7x7);
    }
  }

  convertHelix(helix, helixParams, error5x5);

  return charge;
}

void B2BIIConvertMdstModule::convertHelix(const Belle::Mdst_trk_fit& trk_fit,
                                          const HepPoint3D& newPivot,
                                          std::vector<float>& helixParams, std::vector<float>& helixError)
{
  const HepPoint3D pivot(trk_fit.pivot_x(),
                         trk_fit.pivot_y(),
                         trk_fit.pivot_z());

  CLHEP::HepVector  a(5);
  a[0] = trk_fit.helix(0);
  a[1] = trk_fit.helix(1);
  a[2] = trk_fit.helix(2);
  a[3] = trk_fit.helix(3);
  a[4] = trk_fit.helix(4);
  CLHEP::HepSymMatrix Ea(5, 0);
  Ea[0][0] = trk_fit.error(0);
  Ea[1][0] = trk_fit.error(1);
  Ea[1][1] = trk_fit.error(2);
  Ea[2][0] = trk_fit.error(3);
  Ea[2][1] = trk_fit.error(4);
  Ea[2][2] = trk_fit.error(5);
  Ea[3][0] = trk_fit.error(6);
  Ea[3][1] = trk_fit.error(7);
  Ea[3][2] = trk_fit.error(8);
  Ea[3][3] = trk_fit.error(9);
  Ea[4][0] = trk_fit.error(10);
  Ea[4][1] = trk_fit.error(11);
  Ea[4][2] = trk_fit.error(12);
  Ea[4][3] = trk_fit.error(13);
  Ea[4][4] = trk_fit.error(14);

  Belle::Helix helix(pivot, a, Ea);

  if (newPivot.x() != 0. || newPivot.y() != 0. || newPivot.z() != 0.) {
    helix.pivot(newPivot);
  } else {
    if (pivot.x() != 0. || pivot.y() != 0. || pivot.z() != 0.) {
      helix.pivot(HepPoint3D(0., 0., 0.));
    }
  }

  CLHEP::HepSymMatrix error5x5(5, 0);
  convertHelix(helix, helixParams, error5x5);

  unsigned int size = 5;
  unsigned int counter = 0;
  for (unsigned int i = 0; i < size; i++)
    for (unsigned int j = i; j < size; j++)
      helixError[counter++] = error5x5[i][j];
}

void B2BIIConvertMdstModule::convertHelix(Belle::Helix& helix, std::vector<float>& helixParams, CLHEP::HepSymMatrix& error5x5)
{
  CLHEP::HepVector  a(5);
  CLHEP::HepSymMatrix Ea(5, 0);

  a = helix.a();
  Ea = helix.Ea();

  // param 0: d_0 = d_rho
  helixParams[0] = a[0];

  // param 1: phi = phi_0 + pi/2
  helixParams[1] = adjustAngleRange(a[1] +  TMath::Pi() / 2.0);

  // param 2: omega = Kappa * alpha = Kappa * B[Tesla] * speed_of_light[m/s] * 1e-11
  helixParams[2] = a[2] * KAPPA2OMEGA;

  // param 3: d_z = z0
  helixParams[3] = a[3];

  // param 4: tan(Lambda) = tanLambda
  helixParams[4] = a[4];

  unsigned int size = 5;
  for (unsigned int i = 0; i < size; i++) {
    for (unsigned int j = 0; j < size; j++) {
      error5x5[i][j] = Ea[i][j];
      if (i == 2)
        error5x5[i][j] *= KAPPA2OMEGA;
      if (j == 2)
        error5x5[i][j] *= KAPPA2OMEGA;

      if (std::isinf(error5x5[i][j])) {
        B2DEBUG(99, "Helix covariance matrix element found to be infinite. Setting value to DBL_MAX/2.0.");
        error5x5[i][j] = DBL_MAX / 2.0;
      }
    }
  }
}

void B2BIIConvertMdstModule::convertMdstChargedObject(const Belle::Mdst_charged& belleTrack, Track* track)
{
  Belle::Mdst_trk& trk = belleTrack.trk();

  for (int mhyp = 0 ; mhyp < c_nHyp; ++mhyp) {
    const Const::ChargedStable& pType = c_belleHyp_to_chargedStable[mhyp];
    double thisMass = pType.getMass();

    Belle::Mdst_trk_fit& trk_fit = trk.mhyp(mhyp);

    // Converted helix parameters
    std::vector<float> helixParam(5);
    // Converted 5x5 error matrix
    CLHEP::HepSymMatrix error5x5(5, 0);
    // 4-momentum
    CLHEP::HepLorentzVector momentum;
    // 7x7 (momentum, position) error matrix
    CLHEP::HepSymMatrix     error7x7(7, 0);
    // position
    HepPoint3D       position;

    getHelixParameters(trk_fit, thisMass, HepPoint3D(0., 0., 0.),
                       helixParam,  error5x5,
                       momentum, position, error7x7, 0.0);

    std::vector<float> helixError(15);
    unsigned int size = 5;
    unsigned int counter = 0;
    for (unsigned int i = 0; i < size; i++)
      for (unsigned int j = i; j < size; j++)
        helixError[counter++] = error5x5[i][j];

    double pValue = TMath::Prob(trk_fit.chisq(), trk_fit.ndf());

    // Create an empty cdc hitpattern and set the number of total hits
    // use hits from 0: axial-wire, 1:stereo-wire, 2:cathode
    // the actual cdc hitpattern is not converted

    int cdcNHits = 0;
    for (unsigned int i = 0; i < 3; i++)
      cdcNHits += trk_fit.nhits(i);

    HitPatternCDC patternCdc;
    patternCdc.setNHits(cdcNHits);

    // conversion of track position in CDC layers
    if (m_convertTrkExtra) {
      auto cdcExtraInfo = m_belleTrkExtra.appendNew(trk_fit.first_x(), trk_fit.first_y(), trk_fit.first_z(),
                                                    trk_fit.last_x(), trk_fit.last_y(), trk_fit.last_z());
      track->addRelationTo(cdcExtraInfo);
    }
    // conversion of the SVD hit pattern
    int svdHitPattern = trk_fit.hit_svd();
    // use hits from 3: SVD-rphi, 4: SVD-z
    // int svdNHits = trk_fit.nhits(3) + trk_fit.nhits(4);

    std::bitset<32> svdBitSet(svdHitPattern);

    HitPatternVXD patternVxd;

    unsigned short svdLayers;
    // taken from: http://belle.kek.jp/group/indirectcp/cpfit/cpfit-festa/2004/talks/Apr.14/CPfesta-2005-Higuchi(3).pdf
    StoreObjPtr<EventMetaData> event;
    // mask for the rphi hits, first 6 (8) bits/ 2 bits per layer
    std::bitset<32> svdUMask(static_cast<std::string>("00000000000000000000000000000011"));
    // mask for the z hits, second 6 (8) bits/ 2 bits per layer
    std::bitset<32> svdVMask;

    // find out if the SVD has 3 (4) layers; if exp <= (>) exp 27
    if (event->getExperiment() <= 27) {
      svdVMask = svdUMask << 6;
      svdLayers = 3;
    } else {
      svdVMask = svdUMask << 8;
      svdLayers = 4;
    }

    // loop over all svd layers (layer index is shifted + 3 for basf2)
    for (unsigned short layerId = 0; layerId < svdLayers; layerId++) {
      unsigned short uHits = (svdBitSet & svdUMask).count();
      unsigned short vHits = (svdBitSet & svdVMask).count();
      patternVxd.setSVDLayer(layerId + 3, uHits, vHits);
      // shift masks to the left
      svdUMask <<= 2;
      svdVMask <<= 2;
    }

    TrackFitResult helixFromHelix(helixParam, helixError, pType, pValue, -1, patternVxd.getInteger(), 0);

    if (m_use6x6CovarianceMatrix4Tracks) {
      TMatrixDSym cartesianCovariance(6);
      for (unsigned i = 0; i < 7; i++) {
        if (i == 3)
          continue;
        for (unsigned j = 0; j < 7; j++) {
          if (j == 3)
            continue;

          cartesianCovariance(ERRMCONV[i], ERRMCONV[j]) = error7x7[i][j];
        }
      }
      UncertainHelix helixFromCartesian(helixFromHelix.getPosition(), helixFromHelix.getMomentum(), helixFromHelix.getChargeSign(),
                                        BFIELD, cartesianCovariance, pValue);

      TMatrixDSym helixCovariance = helixFromCartesian.getCovariance();

      counter = 0;
      for (unsigned int i = 0; i < 5; ++i)
        for (unsigned int j = i; j < 5; ++j)
          helixError[counter++] = helixCovariance(i, j);
    }

    auto trackFit = m_trackFitResults.appendNew(helixParam, helixError, pType, pValue, patternCdc.getInteger(),
                                                patternVxd.getInteger(), trk_fit.ndf());
    track->setTrackFitResultIndex(pType, trackFit->getArrayIndex());
    /*
      B2INFO("--- B1 Track: ");
      std::cout << "Momentum = " << momentum << std::endl;
      std::cout << "Position = " << position << std::endl;
      std::cout << "7x7 error matrix = " << error7x7 << std::endl;
      B2INFO("--- B2 Track: ");
      std::cout << "Momentum = " << std::endl;
      trackFit->get4Momentum().Print();
      std::cout << "Position = " << std::endl;
      trackFit->getPosition().Print();
      std::cout << "6x6 error matrix = " << std::endl;
      trackFit->getCovariance6().Print();
      TMatrixDSym b2Error7x7(7);
      fill7x7ErrorMatrix(trackFit, b2Error7x7, thisMass, 1.5);
      std::cout << "7x7 error matrix = " << std::endl;
      b2Error7x7.Print();
    */
  }
}

void B2BIIConvertMdstModule::convertGenHepevtObject(const Belle::Gen_hepevt& genHepevt, MCParticleGraph::GraphParticle* mcParticle)
{
  //B2DEBUG(80, "Gen_ehepevt: idhep " << genHepevt.idhep() << " (" << genHepevt.isthep() << ") with ID = " << genHepevt.get_ID());

  // updating the GraphParticle information from the Gen_hepevt information
  const int idHep = recoverMoreThan24bitIDHEP(genHepevt.idhep());

  // TODO: do not change 911 to 22
  if (idHep == 0 || idHep == 911) {
    B2WARNING("[B2BIIConvertMdstModule] Trying to convert Gen_hepevt with idhep = " << idHep << ". This should enver happen.");
    mcParticle->setPDG(Const::photon.getPDGCode());
  } else {
    mcParticle->setPDG(idHep);
  }

  if (genHepevt.isthep() > 0) {
    mcParticle->setStatus(MCParticle::c_PrimaryParticle);
  }

  mcParticle->setMass(genHepevt.M());

  TLorentzVector p4(genHepevt.PX(), genHepevt.PY(), genHepevt.PZ(), genHepevt.E());
  mcParticle->set4Vector(p4);

  mcParticle->setProductionVertex(genHepevt.VX()*Unit::mm, genHepevt.VY()*Unit::mm, genHepevt.VZ()*Unit::mm);
  mcParticle->setProductionTime(genHepevt.T()*Unit::mm / Const::speedOfLight);

  // decay time of this particle is production time of the daughter particle
  if (genHepevt.daFirst() > 0) {
    Belle::Gen_hepevt_Manager& genMgr = Belle::Gen_hepevt_Manager::get_manager();
    Belle::Gen_hepevt daughterParticle = genMgr(Belle::Panther_ID(genHepevt.daFirst()));
    mcParticle->setDecayTime(daughterParticle.T()*Unit::mm / Const::speedOfLight);
    mcParticle->setDecayVertex(daughterParticle.VX()*Unit::mm, daughterParticle.VY()*Unit::mm, daughterParticle.VZ()*Unit::mm);
  } else {
    //otherwise, assume it's stable
    mcParticle->setDecayTime(std::numeric_limits<float>::infinity());
  }

  mcParticle->setValidVertex(true);
}

void B2BIIConvertMdstModule::convertMdstECLObject(const Belle::Mdst_ecl& ecl, const Belle::Mdst_ecl_aux& eclAux,
                                                  ECLCluster* eclCluster)
{
  if (eclAux.e9oe25() < m_matchType2E9oE25Threshold)
    eclCluster->setIsTrack(ecl.match() > 0);
  else
    eclCluster->setIsTrack(ecl.match() == 1);

  eclCluster->setEnergy(ecl.energy()); //must happen before setCovarianceMatrix()!
  eclCluster->setPhi(ecl.phi());
  eclCluster->setTheta(ecl.theta());
  eclCluster->setR(ecl.r());
  eclCluster->setdeltaL(ecl.quality());

  double covarianceMatrix[6];
  covarianceMatrix[0] = ecl.error(0); // error on energy
  covarianceMatrix[1] = ecl.error(1);
  covarianceMatrix[2] = ecl.error(2); // error on phi
  covarianceMatrix[3] = ecl.error(3);
  covarianceMatrix[4] = ecl.error(4);
  covarianceMatrix[5] = ecl.error(5); // error on theta
  eclCluster->setCovarianceMatrix(covarianceMatrix);

  eclCluster->setLAT(eclAux.width());
  eclCluster->setEnergyRaw(eclAux.mass());
  eclCluster->setE9oE21(eclAux.e9oe25());
  eclCluster->setEnergyHighestCrystal(eclAux.seed());
  eclCluster->setTime(eclAux.property(0));
  eclCluster->setNumberOfCrystals(eclAux.nhits());
}

void B2BIIConvertMdstModule::convertMdstKLMObject(const Belle::Mdst_klm_cluster& klm_cluster, KLMCluster* klmCluster)
{
  // note: Belle quality flag is not saved (no free int variable in Belle2 KLMCluster)
  klmCluster->setLayers(klm_cluster.layers());
  klmCluster->setInnermostLayer(klm_cluster.first_layer());
}


//-----------------------------------------------------------------------------
// RELATIONS
//-----------------------------------------------------------------------------
void B2BIIConvertMdstModule::setTracksToECLClustersRelations()
{
  // Relations
  RelationArray tracksToECLClusters(m_tracks, m_eclClusters);

  Belle::Mdst_ecl_trk_Manager& m = Belle::Mdst_ecl_trk_Manager::get_manager();
  Belle::Mdst_charged_Manager& chgMg = Belle::Mdst_charged_Manager::get_manager();

  // We first insert relations to tracks which are directly matched (type == 1)
  // secondly we had CR matched tracks (connected region) (type == 2)
  // finally tracks which are geometrically matched (type == 0)
  std::vector<int> insert_order_types = {1, 2, 0};
  for (auto& insert_type : insert_order_types) {
    for (Belle::Mdst_ecl_trk_Manager::iterator ecltrkIterator = m.begin(); ecltrkIterator != m.end(); ++ecltrkIterator) {
      Belle::Mdst_ecl_trk mECLTRK = *ecltrkIterator;

      if (mECLTRK.type() != insert_type)
        continue;

      Belle::Mdst_ecl mdstEcl = mECLTRK.ecl();
      Belle::Mdst_trk mTRK    = mECLTRK.trk();

      if (!mdstEcl)
        continue;

      // the numbering in mdst_charged
      // not necessarily the same as in mdst_trk
      // therfore have to find corresponding mdst_charged
      for (Belle::Mdst_charged_Manager::iterator chgIterator = chgMg.begin(); chgIterator != chgMg.end(); ++chgIterator) {
        Belle::Mdst_charged mChar = *chgIterator;
        Belle::Mdst_trk mTRK_in_charged = mChar.trk();

        if (mTRK_in_charged.get_ID() == mTRK.get_ID()) {
          // found the correct  mdst_charged
          tracksToECLClusters.add(mChar.get_ID() - 1, mdstEcl.get_ID() - 1, 1.0);
          break;
        }
      }
    }
  }
}


void B2BIIConvertMdstModule::setKLMClustersRelations()
{
  // Relations
  RelationArray klmClustersToTracks(m_klmClusters, m_tracks);
  RelationArray klmClustersToEclClusters(m_klmClusters, m_eclClusters);

  Belle::Mdst_klm_cluster_Manager& klm_cluster_manager = Belle::Mdst_klm_cluster_Manager::get_manager();


  for (Belle::Mdst_klm_cluster_Manager::iterator klmC_Ite = klm_cluster_manager.begin(); klmC_Ite != klm_cluster_manager.end();
       ++klmC_Ite) {

    Belle::Mdst_klm_cluster mdstKlm_cluster = *klmC_Ite;
    Belle::Mdst_trk mTRK    = mdstKlm_cluster.trk();
    Belle::Mdst_ecl mECL    = mdstKlm_cluster.ecl();

    if (mTRK) klmClustersToTracks.add(mdstKlm_cluster.get_ID() - 1, mTRK.get_ID() - 1);
    if (mECL) klmClustersToEclClusters.add(mdstKlm_cluster.get_ID() - 1, mECL.get_ID() - 1);
  }
}


//-----------------------------------------------------------------------------
// MISC
//-----------------------------------------------------------------------------

int B2BIIConvertMdstModule::recoverMoreThan24bitIDHEP(int id)
{
  /*
    QUICK CHECK: most of the normal particles are smaller than
    0x100000, while all the corrupt id has some of the high bits on.

    This bit check has to be revised when the table below is updated.
  */
  const int mask = 0x00f00000;
  int high_bits = id & mask;
  if (high_bits == 0 || high_bits == mask) return id;

  switch (id) {
    case   7114363:
      return      91000443; // X(3940)
    case   6114363:
      return      90000443; // Y(3940)
    case   6114241:
      return      90000321; // K_0*(800)+
    case   6114231:
      return      90000311; // K_0*(800)0
    case  -6865004:
      return       9912212; // p_diff+
    case  -6865104:
      return       9912112; // n_diffr
    case  -6866773:
      return       9910443; // psi_diff
    case  -6866883:
      return       9910333; // phi_diff
    case  -6866993:
      return       9910223; // omega_diff
    case  -6867005:
      return       9910211; // pi_diff+
    case  -6867103:
      return       9910113; // rho_diff0
    case  -7746995:
      return       9030221; // f_0(1500)
    case  -7756773:
      return       9020443; // psi(4415)
    case  -7756995:
      return       9020221; // eta(1405)
    case  -7766773:
      return       9010443; // psi(4160)
    case  -7776663:
      return       9000553; // Upsilon(5S)
    case  -7776773:
      return       9000443; // psi(4040)
    case  -7776783:
      return       9000433; // D_sj(2700)+
    case  -7776995:
      return       9000221; // f_0(600)
    case  -6114241:
      return     -90000321; // K_0*(800)-
    case  -6114231:
      return     -90000311; // anti-K_0*(800)0
    case   6865004:
      return      -9912212; // anti-p_diff-
    case   6865104:
      return      -9912112; // anti-n_diffr
    case   6867005:
      return      -9910211; // pi_diff-
    case   7776783:
      return      -9000433; // D_sj(2700)-
    default:
      return id;
  }
}

void B2BIIConvertMdstModule::testMCRelation(const Belle::Gen_hepevt& belleMC, const MCParticle* mcP, const std::string& objectName)
{
  int bellePDGCode   = belleMC.idhep();
  int belleIIPDGCode = mcP->getPDG();

  if (bellePDGCode == 0)
    B2WARNING("[B2BIIConvertMdstModule] " << objectName << " matched to Gen_hepevt with idhep = 0.");

  if (bellePDGCode != belleIIPDGCode && bellePDGCode != 911)
    B2WARNING("[B2BIIConvertMdstModule] " << objectName << " matched to different MCParticle! " << bellePDGCode << " vs. " <<
              belleIIPDGCode);

  double belleMomentum[]  = { belleMC.PX(), belleMC.PY(), belleMC.PZ() };
  double belle2Momentum[] = { mcP->get4Vector().Px(),  mcP->get4Vector().Py(),  mcP->get4Vector().Pz() };

  for (unsigned i = 0; i < 3; i++) {
    double relDev = (belle2Momentum[i] - belleMomentum[i]) / belleMomentum[i];

    if (relDev > 1e-3) {
      B2WARNING("[B2BIIConvertMdstModule] " << objectName << " matched to different MCParticle!");
      B2INFO(" - Gen_hepevt     [" << bellePDGCode << "] px/py/pz = " << belleMC.PX() << "/" << belleMC.PY() << "/" << belleMC.PZ());
      B2INFO(" - TrackFitResult [" << belleIIPDGCode << "] px/py/pz = " << mcP->get4Vector().Px() << "/" << mcP->get4Vector().Py() << "/"
             << mcP->get4Vector().Pz());
    }
  }
}

void B2BIIConvertMdstModule::belleVeeDaughterToCartesian(const Belle::Mdst_vee2& vee, const int charge,
                                                         const Const::ParticleType& pType,
                                                         CLHEP::HepLorentzVector& momentum, HepPoint3D& position, CLHEP::HepSymMatrix& error)
{
  const HepPoint3D pivot(vee.vx(), vee.vy(), vee.vz());
  CLHEP::HepVector  a(5);
  CLHEP::HepSymMatrix Ea(5, 0);
  if (charge > 0) {
    a[0] = vee.daut().helix_p(0); a[1] = vee.daut().helix_p(1);
    a[2] = vee.daut().helix_p(2); a[3] = vee.daut().helix_p(3);
    a[4] = vee.daut().helix_p(4);
    Ea[0][0] = vee.daut().error_p(0);  Ea[1][0] = vee.daut().error_p(1);
    Ea[1][1] = vee.daut().error_p(2);  Ea[2][0] = vee.daut().error_p(3);
    Ea[2][1] = vee.daut().error_p(4);  Ea[2][2] = vee.daut().error_p(5);
    Ea[3][0] = vee.daut().error_p(6);  Ea[3][1] = vee.daut().error_p(7);
    Ea[3][2] = vee.daut().error_p(8);  Ea[3][3] = vee.daut().error_p(9);
    Ea[4][0] = vee.daut().error_p(10); Ea[4][1] = vee.daut().error_p(11);
    Ea[4][2] = vee.daut().error_p(12); Ea[4][3] = vee.daut().error_p(13);
    Ea[4][4] = vee.daut().error_p(14);
  } else {
    a[0] = vee.daut().helix_m(0); a[1] = vee.daut().helix_m(1);
    a[2] = vee.daut().helix_m(2); a[3] = vee.daut().helix_m(3);
    a[4] = vee.daut().helix_m(4);
    Ea[0][0] = vee.daut().error_m(0);  Ea[1][0] = vee.daut().error_m(1);
    Ea[1][1] = vee.daut().error_m(2);  Ea[2][0] = vee.daut().error_m(3);
    Ea[2][1] = vee.daut().error_m(4);  Ea[2][2] = vee.daut().error_m(5);
    Ea[3][0] = vee.daut().error_m(6);  Ea[3][1] = vee.daut().error_m(7);
    Ea[3][2] = vee.daut().error_m(8);  Ea[3][3] = vee.daut().error_m(9);
    Ea[4][0] = vee.daut().error_m(10); Ea[4][1] = vee.daut().error_m(11);
    Ea[4][2] = vee.daut().error_m(12); Ea[4][3] = vee.daut().error_m(13);
    Ea[4][4] = vee.daut().error_m(14);
  }

  Belle::Helix helix(pivot, a, Ea);

  // this is Vee daughter momentum/position/error at pivot = V0 Decay Vertex
  momentum = helix.momentum(0., pType.getMass(), position, error);
}

void B2BIIConvertMdstModule::belleVeeDaughterHelix(const Belle::Mdst_vee2& vee, const int charge, std::vector<float>& helixParam,
                                                   std::vector<float>& helixError)
{
  const HepPoint3D pivot(vee.vx(), vee.vy(), vee.vz());
  CLHEP::HepVector  a(5);
  CLHEP::HepSymMatrix Ea(5, 0);
  if (charge > 0) {
    a[0] = vee.daut().helix_p(0); a[1] = vee.daut().helix_p(1);
    a[2] = vee.daut().helix_p(2); a[3] = vee.daut().helix_p(3);
    a[4] = vee.daut().helix_p(4);
    Ea[0][0] = vee.daut().error_p(0);
    Ea[1][0] = vee.daut().error_p(1);
    Ea[1][1] = vee.daut().error_p(2);
    Ea[2][0] = vee.daut().error_p(3);
    Ea[2][1] = vee.daut().error_p(4);
    Ea[2][2] = vee.daut().error_p(5);
    Ea[3][0] = vee.daut().error_p(6);
    Ea[3][1] = vee.daut().error_p(7);
    Ea[3][2] = vee.daut().error_p(8);
    Ea[3][3] = vee.daut().error_p(9);
    Ea[4][0] = vee.daut().error_p(10);
    Ea[4][1] = vee.daut().error_p(11);
    Ea[4][2] = vee.daut().error_p(12);
    Ea[4][3] = vee.daut().error_p(13);
    Ea[4][4] = vee.daut().error_p(14);
  } else {
    a[0] = vee.daut().helix_m(0); a[1] = vee.daut().helix_m(1);
    a[2] = vee.daut().helix_m(2); a[3] = vee.daut().helix_m(3);
    a[4] = vee.daut().helix_m(4);
    Ea[0][0] = vee.daut().error_m(0);
    Ea[1][0] = vee.daut().error_m(1);
    Ea[1][1] = vee.daut().error_m(2);
    Ea[2][0] = vee.daut().error_m(3);
    Ea[2][1] = vee.daut().error_m(4);
    Ea[2][2] = vee.daut().error_m(5);
    Ea[3][0] = vee.daut().error_m(6);
    Ea[3][1] = vee.daut().error_m(7);
    Ea[3][2] = vee.daut().error_m(8);
    Ea[3][3] = vee.daut().error_m(9);
    Ea[4][0] = vee.daut().error_m(10);
    Ea[4][1] = vee.daut().error_m(11);
    Ea[4][2] = vee.daut().error_m(12);
    Ea[4][3] = vee.daut().error_m(13);
    Ea[4][4] = vee.daut().error_m(14);
  }

  Belle::Helix helix(pivot, a, Ea);

  // go to the new pivot
  helix.pivot(HepPoint3D(0., 0., 0.));

  CLHEP::HepSymMatrix error5x5(5, 0);
  convertHelix(helix, helixParam, error5x5);

  unsigned int size = 5;
  unsigned int counter = 0;
  for (unsigned int i = 0; i < size; i++)
    for (unsigned int j = i; j < size; j++)
      helixError[counter++] = error5x5[i][j];
}

void B2BIIConvertMdstModule::belleHelixToHelix(const Belle::Mdst_trk_fit& trk_fit,
                                               std::vector<float>& helixParam, std::vector<float>& helixError)
{
  const HepPoint3D pivot(trk_fit.pivot_x(),
                         trk_fit.pivot_y(),
                         trk_fit.pivot_z());

  CLHEP::HepVector  a(5);
  a[0] = trk_fit.helix(0);
  a[1] = trk_fit.helix(1);
  a[2] = trk_fit.helix(2);
  a[3] = trk_fit.helix(3);
  a[4] = trk_fit.helix(4);
  CLHEP::HepSymMatrix Ea(5, 0);
  Ea[0][0] = trk_fit.error(0);
  Ea[1][0] = trk_fit.error(1);
  Ea[1][1] = trk_fit.error(2);
  Ea[2][0] = trk_fit.error(3);
  Ea[2][1] = trk_fit.error(4);
  Ea[2][2] = trk_fit.error(5);
  Ea[3][0] = trk_fit.error(6);
  Ea[3][1] = trk_fit.error(7);
  Ea[3][2] = trk_fit.error(8);
  Ea[3][3] = trk_fit.error(9);
  Ea[4][0] = trk_fit.error(10);
  Ea[4][1] = trk_fit.error(11);
  Ea[4][2] = trk_fit.error(12);
  Ea[4][3] = trk_fit.error(13);
  Ea[4][4] = trk_fit.error(14);
  Belle::Helix helix(pivot, a, Ea);

  helix.pivot(HepPoint3D(0., 0., 0.));

  // convert to Belle II helix parameters
  // param 0: d_0 = d_rho
  helixParam[0] = helix.a()[0];

  // param 1: phi = phi_0 + pi/2
  helixParam[1] = helix.a()[1] +  TMath::Pi() / 2.0;

  // param 2: omega = Kappa * alpha = Kappa * B[Tesla] * speed_of_light[m/s] * 1e-11
  helixParam[2] = helix.a()[2] * KAPPA2OMEGA;

  // param 3: d_z = z0
  helixParam[3] = helix.a()[3];

  // param 4: tan(Lambda) = cotTheta
  helixParam[4] = helix.a()[4];

  Ea = helix.Ea();

  helixError[0]  = Ea[0][0];
  helixError[1]  = Ea[1][0];
  helixError[2]  = Ea[2][0] * KAPPA2OMEGA;
  helixError[3]  = Ea[3][0];
  helixError[4]  = Ea[4][0];
  helixError[5]  = Ea[1][1];
  helixError[6]  = Ea[2][1] * KAPPA2OMEGA;
  helixError[7]  = Ea[3][1];
  helixError[8]  = Ea[4][1];
  helixError[9]  = Ea[2][2] * KAPPA2OMEGA * KAPPA2OMEGA;
  helixError[10] = Ea[3][2] * KAPPA2OMEGA;
  helixError[11] = Ea[4][2] * KAPPA2OMEGA;
  helixError[12] = Ea[3][3];
  helixError[13] = Ea[4][3];
  helixError[14] = Ea[4][4];
}

int B2BIIConvertMdstModule::belleHelixToCartesian(const Belle::Mdst_trk_fit& trk_fit, const double mass,
                                                  const HepPoint3D& newPivot,
                                                  CLHEP::HepLorentzVector& momentum, HepPoint3D& position, CLHEP::HepSymMatrix& error, double dPhi)
{
  const HepPoint3D pivot(trk_fit.pivot_x(),
                         trk_fit.pivot_y(),
                         trk_fit.pivot_z());

  CLHEP::HepVector  a(5);
  a[0] = trk_fit.helix(0);
  a[1] = trk_fit.helix(1);
  a[2] = trk_fit.helix(2);
  a[3] = trk_fit.helix(3);
  a[4] = trk_fit.helix(4);
  CLHEP::HepSymMatrix Ea(5, 0);
  Ea[0][0] = trk_fit.error(0);
  Ea[1][0] = trk_fit.error(1);
  Ea[1][1] = trk_fit.error(2);
  Ea[2][0] = trk_fit.error(3);
  Ea[2][1] = trk_fit.error(4);
  Ea[2][2] = trk_fit.error(5);
  Ea[3][0] = trk_fit.error(6);
  Ea[3][1] = trk_fit.error(7);
  Ea[3][2] = trk_fit.error(8);
  Ea[3][3] = trk_fit.error(9);
  Ea[4][0] = trk_fit.error(10);
  Ea[4][1] = trk_fit.error(11);
  Ea[4][2] = trk_fit.error(12);
  Ea[4][3] = trk_fit.error(13);
  Ea[4][4] = trk_fit.error(14);

  Belle::Helix helix(pivot, a, Ea);

  int charge = 0;
  if (helix.kappa() > 0)
    charge = 1;
  else
    charge = -1;

  if (newPivot.x() != 0. || newPivot.y() != 0. || newPivot.z() != 0.) {
    helix.pivot(newPivot);
    momentum = helix.momentum(dPhi, mass, position, error);
  } else {
    if (pivot.x() != 0. || pivot.y() != 0. || pivot.z() != 0.) {
      helix.pivot(HepPoint3D(0., 0., 0.));
      momentum = helix.momentum(dPhi, mass, position, error);
    } else {
      momentum = helix.momentum(dPhi, mass, position, error);
    }
  }
  return charge;
}

TrackFitResult B2BIIConvertMdstModule::createTrackFitResult(const CLHEP::HepLorentzVector& momentum,
                                                            const HepPoint3D&       position,
                                                            const CLHEP::HepSymMatrix&     error,
                                                            const short int charge,
                                                            const Const::ParticleType& pType,
                                                            const float pValue,
                                                            const uint64_t hitPatternCDCInitializer,
                                                            const uint32_t hitPatternVXDInitializer,
                                                            const uint16_t ndf)
{
  TVector3 pos(position.x(),  position.y(),  position.z());
  TVector3 mom(momentum.px(), momentum.py(), momentum.pz());

  TMatrixDSym errMatrix(6);
  for (unsigned i = 0; i < 7; i++) {
    if (i == 3)
      continue;
    for (unsigned j = 0; j < 7; j++) {
      if (j == 3)
        continue;

      if (i == j)
        errMatrix(ERRMCONV[i], ERRMCONV[i]) = error[i][i];
      else
        errMatrix(ERRMCONV[i], ERRMCONV[j]) = errMatrix(ERRMCONV[j], ERRMCONV[i]) = error[i][j];
    }
  }

  return TrackFitResult(pos, mom, errMatrix, charge, pType, pValue, BFIELD, hitPatternCDCInitializer, hitPatternVXDInitializer, ndf);
}

double B2BIIConvertMdstModule::atcPID(const PIDLikelihood* pid, int sigHyp, int bkgHyp)
{
  if (!pid) return 0.5;

  // ACC = ARICH
  Const::PIDDetectorSet set = Const::ARICH;
  double acc_sig = exp(pid->getLogL(c_belleHyp_to_chargedStable[sigHyp], set));
  double acc_bkg = exp(pid->getLogL(c_belleHyp_to_chargedStable[bkgHyp], set));
  double acc = 0.5;
  if (acc_sig + acc_bkg  > 0.0)
    acc = acc_sig / (acc_sig + acc_bkg);

  // TOF = TOP
  set = Const::TOP;
  double tof_sig = exp(pid->getLogL(c_belleHyp_to_chargedStable[sigHyp], set));
  double tof_bkg = exp(pid->getLogL(c_belleHyp_to_chargedStable[bkgHyp], set));
  double tof = 0.5;
  double tof_all = tof_sig + tof_bkg;
  if (tof_all != 0) {
    tof = tof_sig / tof_all;
    if (tof < 0.001) tof = 0.001;
    if (tof > 0.999) tof = 0.999;
  }

  // dE/dx = CDC
  set = Const::CDC;
  double cdc_sig = exp(pid->getLogL(c_belleHyp_to_chargedStable[sigHyp], set));
  double cdc_bkg = exp(pid->getLogL(c_belleHyp_to_chargedStable[bkgHyp], set));
  double cdc = 0.5;
  double cdc_all = cdc_sig + cdc_bkg;
  if (cdc_all != 0) {
    cdc = cdc_sig / cdc_all;
    if (cdc < 0.001) cdc = 0.001;
    if (cdc > 0.999) cdc = 0.999;
  }

  // Combined
  double pid_sig = acc * tof * cdc;
  double pid_bkg = (1. - acc) * (1. - tof) * (1. - cdc);

  return pid_sig / (pid_sig + pid_bkg);
}


void B2BIIConvertMdstModule::endRun()
{
  B2DEBUG(99, "B2BIIConvertMdst: endRun done.");
}


void B2BIIConvertMdstModule::terminate()
{
  B2DEBUG(99, "B2BIIConvertMdst: terminate called");
}

