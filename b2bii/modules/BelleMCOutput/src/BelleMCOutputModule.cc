/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <b2bii/modules/BelleMCOutput/BelleMCOutputModule.h>

/* Belle2 headers. */
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

/* Belle headers. */
#include <belle_legacy/tables/belletdf.h>
#include <belle_legacy/tables/hepevt.h>
#include <belle_legacy/tables/filespec.h>

/* ROOT headers. */
#include <TMatrixD.h>

using namespace Belle2;

REG_MODULE(BelleMCOutput)

BelleMCOutputModule::BelleMCOutputModule() :
  Module()
{
  setDescription("Output of MC particle list in Belle format.");
  setPropertyFlags(c_Output);
  addParam("outputFileName", m_OutputFileName,
           "Output file name.", std::string("belle_mc.mdst"));
  addParam("decayKsInGenerator", m_DecayKsInGenerator,
           "Decay K_S0 in generator.", false);
  addParam("decayLambdaInGenerator", m_DecayLambdaInGenerator,
           "Decay Lambda in generator.", false);
}

BelleMCOutputModule::~BelleMCOutputModule()
{
}

void BelleMCOutputModule::initialize()
{
  m_BelleFile = new Belle::Panther_FileIO(m_OutputFileName.c_str(), BBS_WRITE);
  m_BelleFile->write(BBS_FORMAT, 0);
  m_BelleFile->save_br("BELLE_FILE_SPECIFICATION");
  m_BelleFile->save_br("BELLE_RUNHEAD");
  m_BelleFile->save_br("BELLE_NOMINAL_BEAM");
  m_BelleFile->save("GEN_HEPEVT");
  B2WARNING(
    "************ ATTENTION! ************\n"
    "\n"
    "  Belle MC generation differs from Belle II. In order to generate MC "
    "correctly, you must do the following:\n"
    "\n"
    "  1. Enable vertex and beam-momentum smearing by adding the module "
    "\"OverrideGenerationFlags\" before the generator.\n"
    "  2. Disable smearing in Belle simulation by removing the module "
    "\"bpsmear\" from basf gsim scripts.\n"
    "\n"
    "It is also necessary to consider the following differences:\n"
    "\n"
    "  1. When converting the simulation result back to Belle II format, "
    "it is recommended to disable conversion of beam parameters "
    "(convertBeamParameters = False). Since the format of the table "
    "BELLE_NOMINAL_BEAM is not sufficient to store an arbitrary covariance "
    "matrix, the covariance matrix of IP position (BeamSpot) is not guaranteed "
    "to be exactly the same after conversion.\m"
    "  2. By default, decays of long-lived particles are removed and particles "
    "are declared to be stable in generator (ISTHEP == 1 in basf) because "
    "such decays are simulated by GEANT3 in basf. However, you may choose "
    "to decay such particles via module parameters. This results in direct "
    "passing of the decay products to simulation, i.e. material effects are "
    "ignored. In this case, you may need to perform an additional study "
    "of the difference between data and MC.\n"
    "\n"
    "************************************\n"
  );
}

void BelleMCOutputModule::beginRun()
{
  Belle::Belle_file_specification_Manager& fileManager =
    Belle::Belle_file_specification_Manager::get_manager();
  fileManager.remove();
  int position = 0;
  int length = m_OutputFileName.size();
  do {
    Belle::Belle_file_specification& file = fileManager.add();
    file.Spec(m_OutputFileName.substr(position, std::min(length, 8)).c_str());
    position += 8;
    length -= 8;
  } while (length > 0);
  Belle::Belle_runhead_Manager& runheadManager =
    Belle::Belle_runhead_Manager::get_manager();
  runheadManager.remove();
  Belle::Belle_runhead& runhead = runheadManager.add();
  runhead.ExpMC(2);
  runhead.ExpNo(m_EventMetaData->getExperiment());
  runhead.RunNo(m_EventMetaData->getRun());
  runhead.Time(time(nullptr));
  runhead.Type(0);
  TLorentzVector momentumLER = m_BeamParameters->getLER();
  TLorentzVector momentumHER = m_BeamParameters->getHER();
  runhead.ELER(momentumLER.E());
  runhead.EHER(momentumHER.E());
  Belle::Belle_nominal_beam_Manager& beamManager =
    Belle::Belle_nominal_beam_Manager::get_manager();
  beamManager.remove();
  Belle::Belle_nominal_beam& beam = beamManager.add();
  beam.px_high(momentumHER.X());
  beam.py_high(momentumHER.Y());
  beam.pz_high(momentumHER.Z());
  beam.px_low(momentumLER.X());
  beam.py_low(momentumLER.Y());
  beam.pz_low(momentumLER.Z());
  TMatrixDSym herCovariance = m_BeamParameters->getCovHER();
  beam.sigma_p_high(sqrt(herCovariance[0][0]));
  TMatrixDSym lerCovariance = m_BeamParameters->getCovLER();
  beam.sigma_p_low(sqrt(lerCovariance[0][0]));
  /*
   * The vertex parameters are in cm, as in basf2.
   * The unit is not the same as for particles in GEN_HEPEVT.
   */
  TVector3 vertex = m_BeamParameters->getVertex();
  beam.ip_x(vertex.X());
  beam.ip_y(vertex.Y());
  beam.ip_z(vertex.Z());
  TMatrixDSym vertexCovariance = m_BeamParameters->getCovVertex();
  beam.cang_high(momentumHER.Vect().Theta());
  beam.cang_low(M_PI - momentumLER.Vect().Theta());
  double angleIPZX = momentumHER.Vect().Theta() / 2;
  beam.angle_ip_zx(angleIPZX);
  /*
   * Transformation of error matrix. It is inverse to the transformation in
   * belle_legacy/ip/IpProfile.cc.
   */
  TRotation rotationY;
  rotationY.RotateY(-angleIPZX);
  TMatrixD rotationMatrix(3, 3);
  rotationMatrix[0][0] = rotationY.XX();
  rotationMatrix[0][1] = rotationY.XY();
  rotationMatrix[0][2] = rotationY.XZ();
  rotationMatrix[1][0] = rotationY.YX();
  rotationMatrix[1][1] = rotationY.YY();
  rotationMatrix[1][2] = rotationY.YZ();
  rotationMatrix[2][0] = rotationY.ZX();
  rotationMatrix[2][1] = rotationY.ZY();
  rotationMatrix[2][2] = rotationY.ZZ();
  TMatrixDSym vertexCovariance2 = vertexCovariance.Similarity(rotationMatrix);
  beam.sigma_ip_x(sqrt(vertexCovariance2[0][0]));
  beam.sigma_ip_y(sqrt(vertexCovariance2[1][1]));
  beam.sigma_ip_z(sqrt(vertexCovariance2[2][2]));
  m_BelleFile->write(BBS_BEGIN_RUN, 0);
}

void BelleMCOutputModule::addParticle(
  const MCParticle* particle, MCParticleGraph::GraphParticle* mother)
{
  MCParticleGraph::GraphParticle& part = m_MCParticleGraph.addParticle();
  part = *particle;
  if (mother != nullptr)
    part.comesFrom(*mother);
  int pdg = abs(particle->getPDG());
  if ((pdg == Const::muon.getPDGCode()) ||
      (pdg == Const::pion.getPDGCode()) ||
      (pdg == Const::kaon.getPDGCode()) ||
      ((pdg == Const::Kshort.getPDGCode()) && !m_DecayKsInGenerator) ||
      (pdg == Const::Klong.getPDGCode()) ||
      (pdg == Const::neutron.getPDGCode()) ||
      ((pdg == Const::Lambda.getPDGCode()) && !m_DecayLambdaInGenerator) ||
      (pdg == 3222) || // Sigma+
      (pdg == 3112) || // Sigma-
      (pdg == 3322) || // Xi0
      (pdg == 3312) || // Xi-
      (pdg == 3334)) { // Omega-
    part.addStatus(MCParticle::c_StableInGenerator);
    return;
  }
  std::vector<MCParticle*> daughters = particle->getDaughters();
  for (const MCParticle* daughter : daughters)
    addParticle(daughter, &part);
}

void BelleMCOutputModule::event()
{
  Belle::Gen_hepevt_Manager& hepevtManager =
    Belle::Gen_hepevt_Manager::get_manager();
  hepevtManager.remove();
  /*
   * The time shift applied by basf module "evtgen" (file beam.cc) is
   * vertex z coordinate [mm] / (2.0 * 2.99792458).
   * The vertex coordinate is calculated relative to the IP position.
   * The position correction happens at the simulation stage (bpsmear) in basf.
   */
  double timeShift = (m_MCInitialParticles->getVertex().Z() -
                      m_BeamParameters->getVertex().Z()) /
                     Unit::mm / (2.0 * 0.1 * Const::speedOfLight);
  /*
   * Regeneration of MCParticle array. It is necesary because in basf the
   * long-lived particles (K_S0, K_L0, Lambda, neutron, pi, K, mu)
   * are decayed by GEANT3.
   */
  m_MCParticleGraph.clear();
  for (const MCParticle& particle : m_MCParticles) {
    if (particle.getMother() == nullptr)
      addParticle(&particle, nullptr);
  }
  m_MCParticleGraph.generateList("", MCParticleGraph::c_setDecayInfo |
                                 MCParticleGraph::c_checkCyclic |
                                 MCParticleGraph::c_clearParticles);
  for (const MCParticle& particle : m_MCParticles) {
    Belle::Gen_hepevt& hepevt = hepevtManager.add();
    if (particle.hasStatus(MCParticle::c_Initial))
      hepevt.isthep(3);
    else if (particle.hasStatus(MCParticle::c_StableInGenerator))
      hepevt.isthep(1);
    else
      hepevt.isthep(2);
    hepevt.idhep(particle.getPDG());
    hepevt.reset_mother();
    const MCParticle* mother = particle.getMother();
    int motherIndex = 0;
    if (mother != nullptr)
      motherIndex = mother->getIndex();
    hepevt.moFirst(motherIndex);
    hepevt.moLast(motherIndex);
    hepevt.daFirst(particle.getFirstDaughter());
    hepevt.daLast(particle.getLastDaughter());
    TLorentzVector momentum = particle.get4Vector();
    hepevt.PX(momentum.Px());
    hepevt.PY(momentum.Py());
    hepevt.PZ(momentum.Pz());
    hepevt.E(momentum.E());
    hepevt.M(particle.getMass());
    TVector3 vertex = particle.getVertex();
    hepevt.VX(vertex.X() / Unit::mm);
    hepevt.VY(vertex.Y() / Unit::mm);
    hepevt.VZ(vertex.Z() / Unit::mm);
    hepevt.T(particle.getProductionTime() / Unit::mm * Const::speedOfLight +
             timeShift);
  }
  m_BelleFile->write(BBS_EVENT, m_EventMetaData->getEvent());
}

void BelleMCOutputModule::endRun()
{
}

void BelleMCOutputModule::terminate()
{
  delete m_BelleFile;
}
