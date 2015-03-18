//+
// File : PantherInutModule.cc
// Description : A module to read panther records in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contributors: Anze Zupanc, Thomas Keck
//-

#include <b2bii/modules/PantherInput/PantherInputModule.h>

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <cmath>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <memory>
#include <queue>

// Belle tables
#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"
#include "belle_legacy/tables/hepevt.h"
#include "belle_legacy/tables/run_info.h"

#include "belle_legacy/pntdb/TPntFDDB.h"
#include "belle_legacy/pntdb/TPntDB.h"

// Belle II dataobjects
#include <framework/dataobjects/EventMetaData.h>

// analysis dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

// Belle utilities
#include "b2bii/utility/BelleMdstToGenHepevt.h"

// Belle II utilities
#include "framework/gearbox/Const.h"
#include "framework/gearbox/Unit.h"

// ROOT classes
#include "TLorentzVector.h"
#include "TVector3.h"


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PantherInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PantherInputModule::PantherInputModule() : Module()
{
  //Set module properties
  setDescription("Panther input module");
  setPropertyFlags(c_Input);

  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "Panther MDST input file name.", string("PantherInput.sroot"));

  B2DEBUG(1, "PantherInput: Constructor done.");
}


PantherInputModule::~PantherInputModule()
{
}

void PantherInputModule::test_pntdb()
{

  Belle::Beam_energy_Manager& Beam_mgr = Belle::Beam_energy_Manager::get_manager();
  Beam_mgr.remove();

  // Open Database connection
  Belle::TPntFDDB master("rif");  // use default hostname
  Belle::TPntDB constant(master, "benergy");  // implicitly call Open fddb/db

  if (not constant.IsOK()) {
    B2ERROR("PantherInput: Couldn't connect to rif::benergy");
    return;
  }

  // Read out beam-energy of off-resonance data experiment 71!
  // exp: 71: run: 1 (run indepent number is stored in 1, NOT 0), version: 2
  if (constant.Get(71, 1 , 2) <= 0) {
    B2ERROR("PantherInput: Cannot get data from database correctly.");
    return;
  }

  Belle::Beam_energy_Manager::iterator beit = Beam_mgr.begin();
  if (beit == Beam_mgr.end()) {
    B2ERROR("PantherInput: There is no Beam Energy data.");
    return;
  } else {
    B2INFO("PantherInput: BeamEnergy is" << beit->E_beam());
  }


}

void PantherInputModule::initialize()
{
  // Initialize Panther
  BsInit(0);

  // Initialize Belle II DataStore
  initializeDataStore();

  test_pntdb();

  // Open data file
  m_fd = new Belle::Panther_FileIO(m_inputFileName.c_str(), BBS_READ);

  // Read first event (note that this is not a real event)
  m_fd->read();

  // Process first event
  Convert();

  B2INFO("PantherInput: initialized.");
}

void PantherInputModule::initializeDataStore()
{
  B2DEBUG(99, "[PantherInputModule::initializeDataStore] initialization of DataStore started");

  // event meta data Object pointer
  StoreObjPtr<EventMetaData>::registerPersistent();

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

  B2DEBUG(99, "[PantherInputModule::initializeDataStore] initialization of DataStore ended");
}


void PantherInputModule::beginRun()
{
  B2INFO("PantherInput: beginRun called.");
}


void PantherInputModule::event()
{
  m_nevt++;

  B2DEBUG(1, "[PantherInputModule::event] event #" << m_nevt);

  // First event is already loaded (skip it)
  if (m_nevt == 0) return;

  // Convert event
  Convert();
}

void PantherInputModule::Convert()
{
  B2DEBUG(99, "[PantherInputModule::Coversion] Started conversion for event #" << m_nevt);

  // Read event from Panther
  int rectype = -1;
  while (rectype < 0 && rectype != -2) {
    rectype = m_fd->read();
    B2DEBUG(99, "[PantherInputModule::Coversion] rectype = " << rectype);
  }
  if (rectype == -2) {   // EoF detected
    B2DEBUG(99, "[PantherInputModule::Coversion] Conversion stopped at event #" << m_nevt << ". EOF detected!");
    return;
  }

  // 1. Fill EventMetadata
  bool mc = convertBelleEventObject();

  // 2. Convert MC information
  if (mc) {
    convertGenHepEvtTable();
    convertMdstECLTable();
  }

}

bool PantherInputModule::convertBelleEventObject()
{
  // Get Belle_event_Manager
  Belle::Belle_event_Manager& evman = Belle::Belle_event_Manager::get_manager();
  Belle::Belle_event& evt = evman[0];

  // Fill EventMetaData
  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();

  // set exp/run/evt numbers
  evtmetadata->setExperiment(evt.ExpNo());
  evtmetadata->setRun(evt.RunNo());
  evtmetadata->setEvent(evt.EvtNo() & 0x0fffffff);

  // set generated weight (>0 for MC; <0 for real DATA)
  evtmetadata->setGeneratedWeight((evt.ExpMC() == 2) ? 1.0 : -1.0);

  B2DEBUG(90, "[PantherInputModule] Convert exp/run/evt: " << evt.ExpNo() << "/" << evt.RunNo() << "/" << int(
            evt.EvtNo() & 0x0fffffff));

  return (evt.ExpMC() == 2) ? true : false;
}

//-----------------------------------------------------------------------------
// CONVERT TABLES
//-----------------------------------------------------------------------------

void PantherInputModule::convertGenHepEvtTable()
{

  // clear the Gen_hepevt_ID <-> MCParticleGraphPosition map
  genHepevtToMCParticle.clear();

  // create MCParticle StoreArray
  StoreArray<MCParticle> mcParticles;
  mcParticles.create();

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

void PantherInputModule::convertMdstECLTable()
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

    // convert Belle::MDST_ECL -> Belle2::ECLCluster
    convertMdstECLObject(mdstEcl, mdstEclAux, B2EclCluster);

    // create ECLCluster -> MCParticle relation
    // step 1: MDST_ECL -> Gen_hepevt
    const Belle::Gen_hepevt hep(get_hepevt(mdstEcl, 0));
    if (hep) {
      // step 2: Gen_hepevt -> MCParticle
      if (genHepevtToMCParticle.count(hep.get_ID()) > 0) {
        int matchedMCParticle = genHepevtToMCParticle[hep.get_ID()];
        // step 3: set the relation
        eclClustersToMCParticles.add(B2EclCluster->getArrayIndex(), matchedMCParticle);
      } else {
        B2DEBUG(99, "Can not find MCParticle corresponding to this gen_hepevt (Panther ID = " << hep.get_ID() << ")");
        B2DEBUG(99, "Gen_hepevt: Panther ID = " << hep.get_ID() << "; idhep = " << hep.idhep() << "; isthep = " << hep.isthep());
      }
    }
  }
}

//-----------------------------------------------------------------------------
// CONVERT OBJECTS
//-----------------------------------------------------------------------------

void PantherInputModule::convertGenHepevtObject(const Belle::Gen_hepevt& genHepevt, MCParticleGraph::GraphParticle* mcParticle)
{
  B2DEBUG(80, "Gen_ehepevt: idhep " << genHepevt.idhep() << " (" << genHepevt.isthep() << ") with ID = " << genHepevt.get_ID());

  // updating the GraphParticle information from the Gen_hepevt information
  const int idHep = recoverMoreThan24bitIDHEP(genHepevt.idhep());

  // TODO: do not change 911 to 22
  if (idHep == 0 || idHep == 911) {
    mcParticle->setPDG(22);
  } else {
    mcParticle->setPDG(idHep);
  }

  if (genHepevt.isthep() > 0) {
    mcParticle->setStatus(Belle2::MCParticle::c_PrimaryParticle);
  }

  mcParticle->setMass(genHepevt.M());

  TLorentzVector p4(genHepevt.PX(), genHepevt.PY(), genHepevt.PZ(), genHepevt.E());
  mcParticle->set4Vector(p4);

  mcParticle->setProductionVertex(genHepevt.VX()*Unit::mm, genHepevt.VY()*Unit::mm, genHepevt.VZ()*Unit::mm);
  mcParticle->setProductionTime(genHepevt.T()*Unit::mm / Belle2::Const::speedOfLight);

  // decay time of this particle is production time of the daughter particle
  if (genHepevt.daFirst() > 0) {
    Belle::Gen_hepevt_Manager& genMgr = Belle::Gen_hepevt_Manager::get_manager();
    Belle::Gen_hepevt daughterParticle = genMgr(Belle::Panther_ID(genHepevt.daFirst()));
    mcParticle->setDecayTime(daughterParticle.T()*Unit::mm / Belle2::Const::speedOfLight);
    mcParticle->setDecayVertex(daughterParticle.VX()*Unit::mm, daughterParticle.VY()*Unit::mm, daughterParticle.VZ()*Unit::mm);
  }

  mcParticle->setValidVertex(true);
}

void PantherInputModule::convertMdstECLObject(const Belle::Mdst_ecl& ecl, const Belle::Mdst_ecl_aux& eclAux, ECLCluster* eclCluster)
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
  covarianceMatrix[0] = ecl.error(0);
  covarianceMatrix[1] = ecl.error(1);
  covarianceMatrix[2] = ecl.error(2);
  covarianceMatrix[3] = ecl.error(3);
  covarianceMatrix[4] = ecl.error(4);
  covarianceMatrix[5] = ecl.error(5);
  eclCluster->setError(covarianceMatrix);

  eclCluster->setEnedepSum(eclAux.mass());
  eclCluster->setE9oE25(eclAux.e9oe25());
  eclCluster->setHighestE(eclAux.seed());
  eclCluster->setTiming(eclAux.property(0));
  eclCluster->setNofCrystals(eclAux.nhits());
}

//-----------------------------------------------------------------------------
// MISC
//-----------------------------------------------------------------------------

int PantherInputModule::recoverMoreThan24bitIDHEP(int id)
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

void PantherInputModule::endRun()
{
  B2INFO("PantherInput: endRun done.");
}


void PantherInputModule::terminate()
{
  delete m_fd;
  B2INFO("PantherInput: terminate called")
}

