//+
// File : PantherInutModule.cc
// Description : A module to read panther records in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contributors: Anze Zupanc
//-

#include <b2bii/modules/B2BIIConvertMdst/B2BIIConvertMdstModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

// Belle II utilities
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

// Belle II dataobjects
#include <framework/dataobjects/EventMetaData.h>

// Belle utilities
#include <b2bii/utility/BelleMdstToGenHepevt.h>

// ROOT
#include <TVector3.h>

#include <limits>
#include <queue>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2BIIConvertMdst)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2BIIConvertMdstModule::B2BIIConvertMdstModule() : Module()
{
  //Set module properties
  setDescription("Converts Belle mdst objects (Panther tables and records) to Belle II msdt objects.");

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

  B2INFO("B2BIIConvertMdst: initialized.");
}

void B2BIIConvertMdstModule::initializeDataStore()
{
  B2DEBUG(99, "[B2BIIConvertMdstModule::initializeDataStore] initialization of DataStore started");

  // list here all converted Belle2 objects
  StoreArray<ECLCluster> eclClusters;
  eclClusters.registerInDataStore();

  StoreArray<Track> tracks;
  tracks.registerInDataStore();

  StoreArray<TrackFitResult> trackFitResults;
  trackFitResults.registerInDataStore();

  // needs to be registered, even if running over data, since this information is available only at the begin_run function
  // TODO: Change to module parameter and check if consistent?
  StoreArray<MCParticle> mcParticles;
  mcParticles.registerInDataStore();

  //list here all Relations between Belle2 objects
  tracks.registerRelationTo(mcParticles);
  eclClusters.registerRelationTo(mcParticles);
  eclClusters.registerRelationTo(tracks);

  B2DEBUG(99, "[B2BIIConvertMdstModule::initializeDataStore] initialization of DataStore ended");
}


void B2BIIConvertMdstModule::beginRun()
{
  B2INFO("B2BIIConvertMdst: beginRun called.");
}


void B2BIIConvertMdstModule::event()
{
  // Are we running on MC or DATA?
  Belle::Belle_event_Manager& evman = Belle::Belle_event_Manager::get_manager();
  Belle::Belle_event& evt = evman[0];

  if (evt.ExpMC() == 2)
    m_realData = false; // <- this is MC sample
  else
    m_realData = true;  // <- this is real data sample

  // 1. Convert MC information
  convertGenHepEvtTable();

  // 2. Convert ECL information
  convertMdstECLTable();

  // 3. Convert Tracking information
  convertMdstChargedTable();

  // 4. Set ECLCluster -> Track relations
  setECLClustersToTracksRelations();
}

//-----------------------------------------------------------------------------
// CONVERT TABLES
//-----------------------------------------------------------------------------
void B2BIIConvertMdstModule::convertMdstChargedTable()
{
  // at this point MCParticles StoreArray should already exist
  StoreArray<MCParticle> mcParticles;

  // StoreArrays
  StoreArray<Track> tracks;
  StoreArray<TrackFitResult> trackFitResults;
  tracks.create();
  trackFitResults.create();

  // Relations
  RelationArray tracksToMCParticles(tracks, mcParticles);

  // Loop over all Belle charged tracks
  Belle::Mdst_charged_Manager& m = Belle::Mdst_charged_Manager::get_manager();
  for (Belle::Mdst_charged_Manager::iterator chargedIterator = m.begin(); chargedIterator != m.end(); chargedIterator++) {
    Belle::Mdst_charged belleTrack = *chargedIterator;

    auto track = tracks.appendNew();

    // convert MDST_Charged -> Track
    convertMdstChargedObject(belleTrack, track);

    if (m_realData)
      continue;

    // create Track -> MCParticle relation
    // step 1: MDSTCharged -> Gen_hepevt
    const Belle::Gen_hepevt& hep(gen_level(get_hepevt(belleTrack)));
    if (hep) {
      // step 2: Gen_hepevt -> MCParticle
      if (genHepevtToMCParticle.count(hep.get_ID()) > 0) {
        int matchedMCParticle = genHepevtToMCParticle[hep.get_ID()];

        // step 3: set the relation
        tracksToMCParticles.add(track->getArrayIndex(), matchedMCParticle);

        testMCRelation(hep, mcParticles[matchedMCParticle], "Track");
      } else {
        B2DEBUG(99, "Can not find MCParticle corresponding to this gen_hepevt (Panther ID = " << hep.get_ID() << ")");
        B2DEBUG(99, "Gen_hepevt: Panther ID = " << hep.get_ID() << "; idhep = " << hep.idhep() << "; isthep = " << hep.isthep());
      }
    }
  }
}

void B2BIIConvertMdstModule::convertGenHepEvtTable()
{
  // create MCParticle StoreArray
  StoreArray<MCParticle> mcParticles;
  mcParticles.create();

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
  int nParticles = 0;

  // Start with the root (mother-of-all) particle (1st particle in gen_hepevt table)
  m_particleGraph.addParticle(); nParticles++;
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
      m_particleGraph.addParticle(); nParticles++;
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
    m_particleGraph.addParticle(); nParticles++;
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
  // at this point MCParticles StoreArray should already exist
  StoreArray<MCParticle> mcParticles;

  // create ECLCluster StoreArray
  StoreArray<ECLCluster> eclClusters;
  eclClusters.create();

  // Relations
  RelationArray eclClustersToMCParticles(eclClusters, mcParticles);

  // Loop over all Belle ECL clusters
  Belle::Mdst_ecl_Manager& ecl_manager = Belle::Mdst_ecl_Manager::get_manager();
  Belle::Mdst_ecl_aux_Manager& ecl_aux_manager = Belle::Mdst_ecl_aux_Manager::get_manager();

  for (Belle::Mdst_ecl_Manager::iterator eclIterator = ecl_manager.begin(); eclIterator != ecl_manager.end(); eclIterator++) {

    Belle::Mdst_ecl mdstEcl = *eclIterator;
    Belle::Mdst_ecl_aux mdstEclAux(ecl_aux_manager(Belle::Panther_ID(mdstEcl.get_ID())));

    auto B2EclCluster = eclClusters.appendNew();

    // convert Belle::MDST_ECL -> ECLCluster
    convertMdstECLObject(mdstEcl, mdstEclAux, B2EclCluster);

    if (m_realData)
      continue;

    // create ECLCluster -> MCParticle relation
    // step 1: MDST_ECL -> Gen_hepevt
    const Belle::Gen_hepevt hep(get_hepevt(mdstEcl, 0));
    if (hep && hep.idhep() != 911) {
      // step 2: Gen_hepevt -> MCParticle
      if (genHepevtToMCParticle.count(hep.get_ID()) > 0) {
        int matchedMCParticle = genHepevtToMCParticle[hep.get_ID()];
        // step 3: set the relation
        eclClustersToMCParticles.add(B2EclCluster->getArrayIndex(), matchedMCParticle);

        testMCRelation(hep, mcParticles[matchedMCParticle], "ECLCluster");
      } else {
        B2DEBUG(79, "Can not find MCParticle corresponding to this gen_hepevt (Panther ID = " << hep.get_ID() << ")");
        B2DEBUG(79, "Gen_hepevt: Panther ID = " << hep.get_ID() << "; idhep = " << hep.idhep() << "; isthep = " << hep.isthep());
      }
    }
  }
}

//-----------------------------------------------------------------------------
// CONVERT OBJECTS
//-----------------------------------------------------------------------------

void B2BIIConvertMdstModule::convertMdstChargedObject(const Belle::Mdst_charged& belleTrack, Track* track)
{
  StoreArray<TrackFitResult> trackFitResults;

  Belle::Mdst_trk& trk = belleTrack.trk();

  for (int mhyp = 0 ; mhyp < 5; ++mhyp) {
    std::shared_ptr<Const::ParticleType> pType;

    if (mhyp == 0) {
      pType = std::make_shared<Const::ParticleType>(Const::electron);
    } else if (mhyp == 1) {
      pType = std::make_shared<Const::ParticleType>(Const::muon);
    } else if (mhyp == 2) {
      pType = std::make_shared<Const::ParticleType>(Const::pion);
    } else if (mhyp == 3) {
      pType = std::make_shared<Const::ParticleType>(Const::kaon);
    } else {
      pType = std::make_shared<Const::ParticleType>(Const::proton);
    }

    Belle::Mdst_trk_fit& trk_fit = trk.mhyp(mhyp);

    // Convert helix parameters
    std::vector<float> helixParam(5);

    // param 0: d_0 = d_rho
    helixParam[0] = trk_fit.helix(0);

    // param 1: phi = phi_0 + pi/2
    helixParam[1] = trk_fit.helix(1) +  TMath::Pi() / 2.0;

    // param 2: omega = Kappa * alpha = Kappa * B[Tesla] * speed_of_light[m/s] * 1e-11
    helixParam[2] = trk_fit.helix(2) * 1.5 * TMath::C() * 1E-11;

    // param 3: d_z = z0
    helixParam[3] = trk_fit.helix(3);

    // param 4: tan(Lambda) = cotTheta
    helixParam[4] = trk_fit.helix(4);

    // Convert helix error matrix
    // only elements related with omega (Kappa) need to be scaled by 1.5 * TMath::C() * 1E-11;
    /*
       Belle's definition of the error matrix array
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

       Belle II's definition of the error matrix array
       Ea[0][0] = helixError(0);
       Ea[0][1] = helixError(1);
       Ea[0][2] = helixError(2);  *
       Ea[0][3] = helixError(3);
       Ea[0][4] = helixError(4);
       Ea[1][1] = helixError(5);
       Ea[1][2] = helixError(6);  *
       Ea[1][3] = helixError(7);
       Ea[1][4] = helixError(8);
       Ea[2][2] = helixError(9);  *
       Ea[2][3] = helixError(10); *
       Ea[2][4] = helixError(11); *
       Ea[3][3] = helixError(12);
       Ea[3][4] = helixError(13);
       Ea[4][4] = helixError(14);
    */

    std::vector<float> helixError(15);
    helixError[0]  = trk_fit.error(0);
    helixError[1]  = trk_fit.error(1);
    helixError[2]  = trk_fit.error(3) * 1.5 * TMath::C() * 1E-11;
    helixError[3]  = trk_fit.error(6);
    helixError[4]  = trk_fit.error(10);
    helixError[5]  = trk_fit.error(2);
    helixError[6]  = trk_fit.error(4) * 1.5 * TMath::C() * 1E-11;
    helixError[7]  = trk_fit.error(7);
    helixError[8]  = trk_fit.error(11);
    helixError[9]  = trk_fit.error(5) * 1.5 * TMath::C() * 1E-11;
    helixError[10] = trk_fit.error(8) * 1.5 * TMath::C() * 1E-11;
    helixError[11] = trk_fit.error(12) * 1.5 * TMath::C() * 1E-11;
    helixError[12] = trk_fit.error(9);
    helixError[13] = trk_fit.error(13);
    helixError[14] = trk_fit.error(14);

    double pValue = TMath::Prob(trk_fit.chisq(), trk_fit.ndf());

    //TODO what are hitPatternCDCInitializer and hitPatternVXDInitializer?
    auto trackFit = trackFitResults.appendNew(helixParam, helixError, *pType, pValue, -1, -1);

    track->setTrackFitResultIndex(*pType, trackFit->getArrayIndex());

    // test conversion (pion hypothesis only)
    if (mhyp == 2)
      testTrackConversion(belleTrack, trackFit);
  }
}

void B2BIIConvertMdstModule::convertGenHepevtObject(const Belle::Gen_hepevt& genHepevt, MCParticleGraph::GraphParticle* mcParticle)
{
  //B2DEBUG(80, "Gen_ehepevt: idhep " << genHepevt.idhep() << " (" << genHepevt.isthep() << ") with ID = " << genHepevt.get_ID());

  // updating the GraphParticle information from the Gen_hepevt information
  const int idHep = recoverMoreThan24bitIDHEP(genHepevt.idhep());

  // TODO: do not change 911 to 22
  if (idHep == 0 || idHep == 911) {
    B2WARNING("[B2BIIConvertMdstModule] Trying to convert Gen_hepevt with idhep = " << idHep << ". This should enver happen.")
    mcParticle->setPDG(22);
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
  if (ecl.match() > 0)
    eclCluster->setisTrack(true);
  else
    eclCluster->setisTrack(false);

  eclCluster->setEnergy(ecl.energy());
  eclCluster->setPhi(ecl.phi());
  eclCluster->setTheta(ecl.theta());
  eclCluster->setR(ecl.r());

  // TODO: check
  // TODO: ECLCluster is an unclear mess
  float covarianceMatrix[6];
  covarianceMatrix[0] = sqrt(ecl.error(0)); // error on energy
  covarianceMatrix[1] = ecl.error(1);
  covarianceMatrix[2] = sqrt(ecl.error(2)); // error on phi
  covarianceMatrix[3] = ecl.error(3);
  covarianceMatrix[4] = ecl.error(4);
  covarianceMatrix[5] = sqrt(ecl.error(5)); // error on theta
  eclCluster->setError(covarianceMatrix);

  eclCluster->setEnedepSum(eclAux.mass());
  eclCluster->setE9oE25(eclAux.e9oe25());
  eclCluster->setHighestE(eclAux.seed());
  eclCluster->setTiming(eclAux.property(0));
  eclCluster->setNofCrystals(eclAux.nhits());
}

//-----------------------------------------------------------------------------
// RELATIONS
//-----------------------------------------------------------------------------
void B2BIIConvertMdstModule::setECLClustersToTracksRelations()
{
  StoreArray<Track> tracks;
  StoreArray<ECLCluster> eclClusters;

  // Relations
  RelationArray eclClustersToTracks(eclClusters, tracks);

  Belle::Mdst_ecl_trk_Manager& m = Belle::Mdst_ecl_trk_Manager::get_manager();
  Belle::Mdst_charged_Manager& chgMg = Belle::Mdst_charged_Manager::get_manager();
  for (Belle::Mdst_ecl_trk_Manager::iterator ecltrkIterator = m.begin(); ecltrkIterator != m.end(); ecltrkIterator++) {
    Belle::Mdst_ecl_trk mECLTRK = *ecltrkIterator;

    Belle::Mdst_ecl mdstEcl = mECLTRK.ecl();
    Belle::Mdst_trk mTRK    = mECLTRK.trk();

    if (!mdstEcl)
      continue;

    // the numbering in mdst_charged
    // not necessarily the same as in mdst_trk
    // therfore have to find corresponding mdst_charged
    for (Belle::Mdst_charged_Manager::iterator chgIterator = chgMg.begin(); chgIterator != chgMg.end(); chgIterator++) {
      Belle::Mdst_charged mChar = *chgIterator;
      Belle::Mdst_trk mTRK_in_charged = mChar.trk();

      if (mTRK_in_charged.get_ID() == mTRK.get_ID()) {
        // found the correct  mdst_charged
        eclClustersToTracks.add(mdstEcl.get_ID() - 1, mChar.get_ID() - 1, mECLTRK.type() * 1.0);
        break;
      }
    }
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
    case   7114363: return      91000443; // X(3940)
    case   6114363: return      90000443; // Y(3940)
    case   6114241: return      90000321; // K_0*(800)+
    case   6114231: return      90000311; // K_0*(800)0
    case  -6865004: return       9912212; // p_diff+
    case  -6865104: return       9912112; // n_diffr
    case  -6866773: return       9910443; // psi_diff
    case  -6866883: return       9910333; // phi_diff
    case  -6866993: return       9910223; // omega_diff
    case  -6867005: return       9910211; // pi_diff+
    case  -6867103: return       9910113; // rho_diff0
    case  -7746995: return       9030221; // f_0(1500)
    case  -7756773: return       9020443; // psi(4415)
    case  -7756995: return       9020221; // eta(1405)
    case  -7766773: return       9010443; // psi(4160)
    case  -7776663: return       9000553; // Upsilon(5S)
    case  -7776773: return       9000443; // psi(4040)
    case  -7776783: return       9000433; // D_sj(2700)+
    case  -7776995: return       9000221; // f_0(600)
    case  -6114241: return     -90000321; // K_0*(800)-
    case  -6114231: return     -90000311; // anti-K_0*(800)0
    case   6865004: return      -9912212; // anti-p_diff-
    case   6865104: return      -9912112; // anti-n_diffr
    case   6867005: return      -9910211; // pi_diff-
    case   7776783: return      -9000433; // D_sj(2700)-
    default:
      return id;
  }
}

void B2BIIConvertMdstModule::testTrackConversion(const Belle::Mdst_charged& belleTrack, const TrackFitResult* trackFit)
{

  double belle2Momentum[] = { trackFit->getMomentum().X(), trackFit->getMomentum().Y(), trackFit->getMomentum().Z() };
  double belleMomentum[] =  { belleTrack.px(), belleTrack.py(), belleTrack.pz() };

  for (unsigned i = 0; i < 3; i++) {
    double relDev = (belle2Momentum[i] - belleMomentum[i]) / belleMomentum[i];

    if (relDev > 1e-3) {
      B2WARNING("[B2BIIConvertMdstModule] conversion of Belle's track #" << belleTrack.get_ID() << " momentum has large deviation: " <<
                relDev);
      B2INFO(" - MDST_Charged   px/py/pz = " << belleTrack.px() << "/" << belleTrack.py() << "/" << belleTrack.pz());
      B2INFO(" - TrackFitResult px/py/pz = " << trackFit->getMomentum().X() << "/" << trackFit->getMomentum().Y() << "/" <<
             trackFit->getMomentum().Z());
    }
  }
}

void B2BIIConvertMdstModule::testMCRelation(const Belle::Gen_hepevt& belleMC, const MCParticle* mcP, std::string objectName)
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

void B2BIIConvertMdstModule::endRun()
{
  B2INFO("B2BIIConvertMdst: endRun done.");
}


void B2BIIConvertMdstModule::terminate()
{
  B2INFO("B2BIIConvertMdst: terminate called")
}

