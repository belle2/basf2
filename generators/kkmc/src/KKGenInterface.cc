/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <generators/kkmc/KKGenInterface.h>

/* Basf2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <framework/dataobjects/MCInitialParticles.h>
#include <mdst/dataobjects/MCParticleGraph.h>

/* ROOT headers. */
#include <THashList.h>
#include <Math/LorentzRotation.h>
#include <Math/Boost.h>
#include <Math/Vector3D.h>

/* C++ headers. */
#include <cmath>
#include <string>
#include <utility>

#include <Eigen/Dense>


using namespace Belle2;

int KKGenInterface::setup(const std::string& KKdefaultFileName, const std::string& tauinputFileName,
                          const std::string& taudecaytableFileName, const std::string& KKMCOutputFileName)
{
  B2DEBUG(20, "Begin initialisation of KKGen Interface.");

  // This is to avoid character corruption of TAUOLA output
  char DatX_d[132], DatX_u[132], DatX_p[132], DatX_o[132];
  for (int i = 0; i < 132; ++i) {
    DatX_d[i] = ' ';
    DatX_u[i] = ' ';
    DatX_p[i] = ' ';
    DatX_o[i] = ' ';
  }
  strcpy(DatX_d, KKdefaultFileName.c_str());
  int length = strlen(DatX_d);
  DatX_d[length] = ' ';
  strcpy(DatX_u, tauinputFileName.c_str());
  length = strlen(DatX_u);
  DatX_u[length] = ' ';
  strcpy(DatX_p, taudecaytableFileName.c_str());
  length = strlen(DatX_p);
  DatX_p[length] = ' ';
  strcpy(DatX_o, KKMCOutputFileName.c_str());
  length = strlen(DatX_o);
  DatX_o[length] = ' ';
  int irand = 0;
  kk_init_(DatX_d, DatX_u, DatX_p, &irand, DatX_o);

  // seed of random generator should be set here
  kk_init_seed_();

  // create mapping from pythia id to pdg code
  auto particlelist = EvtGenDatabasePDG::Instance()->ParticleList();
  m_mapPythiaIDtoPDG.reserve(particlelist->GetEntries());
  for (TObject* object : *particlelist) {
    EvtGenParticlePDG* particle = dynamic_cast<EvtGenParticlePDG*>(object);
    if (!particle) {
      B2FATAL("Something is wrong with the EvtGenDatabasePDG, got object not inheriting from EvtGenParticlePDG");
    }
    m_mapPythiaIDtoPDG[particle->PythiaID()] = particle->PdgCode();
  }

  B2DEBUG(20, "End initialisation of KKGen Interface.");

  return 0;
}


void KKGenInterface::set_beam_info(double Ecms0, double Ecms0Spread)
{
  if (Ecms0 > 0. && Ecms0Spread >= 0.) {
    // KKMC requires spread of energy of a single beam in CMS system
    double E0spread = Ecms0Spread / std::sqrt(2);
    kk_begin_run_(&Ecms0, &E0spread);
  } else {
    B2DEBUG(100, "Wrong beam info");
  }
}


int KKGenInterface::simulateEvent(MCParticleGraph& graph, const ConditionalGaussGenerator& lorentzGenerator,
                                  ROOT::Math::XYZVector vertex)
{
  B2DEBUG(20, "Start simulation of KKGen Interface.");
  int status = 0;
  kk_event_(&status);

  ROOT::Math::PxPyPzEVector pHERorg(hepevt_.phep[0][0], hepevt_.phep[0][1], hepevt_.phep[0][2], hepevt_.phep[0][3]);
  ROOT::Math::PxPyPzEVector pLERorg(hepevt_.phep[1][0], hepevt_.phep[1][1], hepevt_.phep[1][2], hepevt_.phep[1][3]);
  ROOT::Math::PxPyPzEVector pTotOrg = pHERorg + pLERorg;

  // KKMC allows generation of events with E-spread of beams, where
  // without spread both energies are equal and momenta aligned along z-asis
  // When spread it on, the system is no more CMS, so we transform to it
  ROOT::Math::LorentzRotation rotKKMC(ROOT::Math::Boost(pTotOrg.BoostToCM()));

  // CMS energy from KKMC used for conditional generator
  double EcmsNow = pTotOrg.M();

  // Calculate Lorentz transformation to LAB for this event
  Eigen::VectorXd               transVec = lorentzGenerator.generate(EcmsNow);
  ROOT::Math::LorentzRotation   rot =  MCInitialParticles::cmsToLab(transVec[1], transVec[2], transVec[3], transVec[4], transVec[5]);

  // Total Lorentz transformation
  ROOT::Math::LorentzRotation rotTot = rot * rotKKMC;

  for (int i = 0; i < hepevt_.nhep; ++i) {
    ROOT::Math::PxPyPzEVector p4cms(hepevt_.phep[i][0], hepevt_.phep[i][1], hepevt_.phep[i][2], hepevt_.phep[i][3]);

    // transform to LAB
    ROOT::Math::PxPyPzEVector p4lab = rotTot * p4cms;

    hepevt_.phep[i][0] = p4lab.Px();
    hepevt_.phep[i][1] = p4lab.Py();
    hepevt_.phep[i][2] = p4lab.Pz();
    hepevt_.phep[i][3] = p4lab.E();
    //hepevt_.phep[i][4] = p4lab.M();
  }


  // Shift vertex point due to tau life time
  kk_shifttaudecayvtx_();


  // before storing event to MCParticle, check /hepevt/ common block
  B2DEBUG(100, "HepEVT table:");

  for (int i = 0; i < hepevt_.nhep; ++i) {
    char buf[200];
    sprintf(buf,
            "IntA: %3d %4d %8d %4d %4d %4d %9.4f %9.4f %9.4f %9.4f %9.4f",
            i + 1, hepevt_.isthep[i], hepevt_.idhep[i],
            hepevt_.jmohep[i][0], hepevt_.jdahep[i][0],
            hepevt_.jdahep[i][1], hepevt_.phep[i][0],
            hepevt_.phep[i][1], hepevt_.phep[i][2],
            hepevt_.phep[i][3], hepevt_.phep[i][4]);
    B2DEBUG(100, buf);
  }

  int npar = addParticles2Graph(graph, vertex);
  graph.generateList("", MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);
  B2DEBUG(100, "GraphParticles:");

  // check MCParticleGraph
  for (int i = 0; i < npar; ++i) {
    MCParticleGraph::GraphParticle* p = &graph[i];
    int moID = 0;
    char buf[200];
    sprintf(buf, "IntB: %3d %4u %8d %4d %4d %4d %9.4f %9.4f %9.4f %9.4f",
            p->getIndex(),  p->getStatus(),  p->getPDG(),  moID,
            p->getFirstDaughter(),  p->getLastDaughter(),
            p->get4Vector().Px(),  p->get4Vector().Py(),
            p->get4Vector().Pz(),  p->get4Vector().E());
    B2DEBUG(100, buf);

  }

  B2DEBUG(20, "End simulation of KKGen Interface.");
  return hepevt_.nhep; //returns the number of generated particles from KKMC
}



int KKGenInterface::addParticles2Graph(MCParticleGraph& graph, ROOT::Math::XYZVector vertex)
{
  // KKMC generates at least five particles:
  // beam (e+ e-), intermediate gamma/Z, f+ f- (f=mu, tau, ...)
  if (hepevt_.nhep < 5) {
    B2ERROR("KKMC-generated event has not been produced correctty!");
    return hepevt_.nhep;
  }

  std::vector <MCParticleGraph::GraphParticle*> MCPList;

  //Fill top particle in the tree & starting the queue:
  for (int i = 1; i <= hepevt_.nhep; ++i) {
    int position = graph.size();
    graph.addParticle();
    MCParticleGraph::GraphParticle* p = &graph[position];
    updateGraphParticle(i, p, vertex);
    MCPList.push_back(p);
  }

  // From produced particles, its mother is assigned
  for (int i = 4; i <= hepevt_.nhep; ++i) {
    MCParticleGraph::GraphParticle* p = MCPList[i - 1];
    if (hepevt_.jmohep[i - 1][0] > 0 && hepevt_.jmohep[i - 1][0] <= hepevt_.nhep) {
      int j = hepevt_.jmohep[i - 1][0];
      MCParticleGraph::GraphParticle* q = MCPList[j - 1];
      p->comesFrom((*q));
    }
  }
  return hepevt_.nhep;
}


void KKGenInterface::updateGraphParticle(int index, MCParticleGraph::GraphParticle* gParticle, ROOT::Math::XYZVector vertex)
{
  if (index < 1 || index > hepevt_.nhep)
    return;
  //updating the GraphParticle information from /hepevt/ common block information

  // convert PYTHIA ID to PDG ID
  auto iter = m_mapPythiaIDtoPDG.find(hepevt_.idhep[index - 1]);
  if (iter != end(m_mapPythiaIDtoPDG)) {
    gParticle->setPDG(iter->second);
  } else {
    //not in the map, set pythia id directly
    gParticle->setPDG(hepevt_.idhep[index - 1]);
  }

  //all(!) particles from the generator have to be primary
  gParticle->addStatus(MCParticleGraph::GraphParticle::c_PrimaryParticle);

  // initial beam electron and positron should be Initial.
  if (abs(hepevt_.idhep[index - 1]) == 11 &&
      hepevt_.jmohep[index - 1][0] == 0 &&
      hepevt_.jmohep[index - 1][1] == 0 &&
      hepevt_.isthep[index - 1] == 3 &&
      index < 3) {
    gParticle->addStatus(MCParticle::c_Initial);
  }

  // Z0 or W+/W- must be flagged as virtual
  if (hepevt_.idhep[index - 1] == 23 || std::abs(hepevt_.idhep[index - 1]) == 24) {
    gParticle->addStatus(MCParticleGraph::GraphParticle::c_IsVirtual);
  } else if (hepevt_.isthep[index - 1] == 1) {
    gParticle->addStatus(MCParticleGraph::GraphParticle::c_StableInGenerator);
  }

  // set photon flags (for now: set ISR and FSR (CEEX is undefined by definition, unsure about IFI))
  // PHOTOS could be called by TAUOLA, not sure what to do about that
  if (hepevt_.idhep[index - 1] == 22) {
    if (hepevt_.jmohep[index - 1][0] == 1) {
      gParticle->addStatus(MCParticleGraph::GraphParticle::c_IsISRPhoton);
      gParticle->addStatus(MCParticleGraph::GraphParticle::c_IsFSRPhoton);
    }
  }

  ROOT::Math::PxPyPzEVector p4(hepevt_.phep[index - 1][0],
                               hepevt_.phep[index - 1][1],
                               hepevt_.phep[index - 1][2],
                               hepevt_.phep[index - 1][3]);
  gParticle->setMass(hepevt_.phep[index - 1][4]);
  gParticle->set4Vector(p4);

  //set vertex including smearing (if user requested)
  ROOT::Math::XYZVector pProductionVertex(hepevt_.vhep[index - 1][0]*Unit::mm,
                                          hepevt_.vhep[index - 1][1]*Unit::mm,
                                          hepevt_.vhep[index - 1][2]*Unit::mm);
  if (!gParticle->hasStatus(MCParticle::c_Initial)) {
    pProductionVertex = pProductionVertex + vertex;
  }
  gParticle->setProductionVertex(pProductionVertex);
  gParticle->setProductionTime(hepevt_.vhep[index - 1][3]*Unit::mm / Const::speedOfLight);
  gParticle->setValidVertex(true);
}

void KKGenInterface::term()
{
  double xsec = 0.;
  double xsecerr = 0.;
  kk_term_(&xsec, &xsecerr);
}
