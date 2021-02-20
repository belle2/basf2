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

/* Belle headers. */
#include <belle_legacy/tables/belletdf.h>
#include <belle_legacy/tables/hepevt.h>
#include <belle_legacy/tables/filespec.h>

using namespace Belle2;

REG_MODULE(BelleMCOutput)

BelleMCOutputModule::BelleMCOutputModule() :
  Module()
{
  setDescription("Output of MC particle list in Belle format.");
  setPropertyFlags(c_Output);
  addParam("outputFileName", m_OutputFileName,
           "Output file name.", std::string("belle_mc.mdst"));
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
  TVector3 vertex = m_BeamParameters->getVertex();
  beam.ip_x(vertex.X() / Unit::mm);
  beam.ip_y(vertex.Y() / Unit::mm);
  beam.ip_z(vertex.Z() / Unit::mm);
  TMatrixDSym vertexCovariance = m_BeamParameters->getCovVertex();
  beam.sigma_ip_x(sqrt(vertexCovariance[0][0]) / Unit::mm);
  beam.sigma_ip_y(sqrt(vertexCovariance[1][1]) / Unit::mm);
  beam.sigma_ip_z(sqrt(vertexCovariance[2][2]) / Unit::mm);
  beam.cang_high(momentumHER.Vect().Theta());
  beam.cang_low(M_PI - momentumLER.Vect().Theta());
  beam.angle_ip_zx(momentumHER.Vect().Theta() / 2);
  m_BelleFile->write(BBS_BEGIN_RUN, 0);
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
