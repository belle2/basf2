/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/trepsinput/TrepsInputModule.h>
#include <string.h>
#include <string>
#include <boost/filesystem.hpp>
#include <TFile.h>
#include <TRandom.h>

#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrepsInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrepsInputModule::TrepsInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  // Set module properties
  setDescription("Input from TREPS generator (No-tag), Input from TREPS generator for ee->ee hadrons");

  // Parameter definitions
  addParam("ParameterFile", m_parameterFile,
           "parameter file for TREPS input", std::string("treps_par.dat"));
  addParam("DifferentialCrossSectionFile", m_differentialCrossSectionFile,
           "file name for differential cross section table input. If UseDiscreteAndSortedW is true, the file is used",
           std::string("pipidcs.dat"));
  addParam("WListTableFile", m_wListTableFile,
           "file name for W-List table input. If UseDiscreteAndSortedW is false (default), the file is used", std::string("wlist_table.dat"));
  addParam("UseDiscreteAndSortedW", m_useDiscreteAndSortedW,
           "if true, use WListTable for discrete and sorted W. if false (default), use DifferentialCrossSection", false);

  addParam("MaximalQ2", m_maximalQ2,
           "Maximal :math:`Q^2 = -q^2`, where q is the difference between the initial "
           "and final electron or positron momenta. Negative means no cut.",
           -1.0);
  addParam("MaximalAbsCosTheta", m_maximalAbsCosTheta,
           "Maximal :math:`|\\cos(\\theta)|`, where :math:`\\theta` is the final-state particle "
           "polar angle.", 1.01);
  addParam("ApplyCosThetaCutCharged", m_applyCosThetaCutCharged,
           "Whether to apply cut on :math:`|cos(theta)|` for charged particles only.",
           true);
  addParam("MinimalTransverseMomentum", m_minimalTransverseMomentum,
           "Minimal transverse momentum of the final-state particles.", 0.0);
  addParam("ApplyTransverseMomentumCutCharged",
           m_applyTransverseMomentumCutCharged,
           "Whether to apply cut on the minimal transverse momentum for "
           "charged particles only.", true);

}

void TrepsInputModule::initialize()
{
  m_initial.initialize();
  m_mcparticles.registerInDataStore();
}

void TrepsInputModule::event()
{
  /*
   * Check if the BeamParameters have changed (if they do, abort the job;
   * otherwise cross section calculation will be a nightmare).
   */
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
      B2INFO("Initialized the TREPS generator!");
    } else {
      B2FATAL("TrepsInputModule::event(): BeamParameters have changed within "
              "a job, this is not supported for TREPS!");
    }
  }

  /* Generation of the initial particle from beam parameters. */
  const MCInitialParticles& initial = m_initial.generate();
  TVector3 vertex = initial.getVertex();

  if (m_useDiscreteAndSortedW) {
    if (m_generator.inmode != 0) return;

    m_generator.wtcount++;
    m_generator.wf = (float)(m_generator.wtable(1));

    if (abs((double)m_generator.wf - m_generator.w) >= 0.001 && m_generator.wf > 0.01) {
      B2INFO(" W value changed. " << m_generator.w << " to " << m_generator.wf);
      m_generator.w = (double)m_generator.wf;
      m_generator.updateW();
    }
  } else {
    m_generator.w = simulateW();
    m_generator.updateW();
  }

  int idummy = 0;
  int iret = m_generator.event_gen(idummy);
  m_mpg.clear();

  // fill data of the initial electron and positron
  MCParticleGraph::GraphParticle& electron = m_mpg.addParticle();
  electron.addStatus(MCParticle::c_Initial | MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
  electron.setPDG(11);
  electron.setMomentum(m_generator.getElectronMomentum());
  electron.setMass(m_generator.me);
  electron.setEnergy(sqrt(m_generator.me * m_generator.me + m_generator.getElectronMomentum().Mag2()));

  MCParticleGraph::GraphParticle& positron = m_mpg.addParticle();
  positron.addStatus(MCParticle::c_Initial | MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
  positron.setPDG(-11);
  positron.setMomentum(m_generator.getPositronMomentum());
  positron.setMass(m_generator.me);
  positron.setEnergy(sqrt(m_generator.me * m_generator.me + m_generator.getPositronMomentum().Mag2()));

  if (iret >= 1) {

    const Part_gen* part = m_generator.partgen;

    // fill data of the final-state particles
    for (int i = 0; i < m_generator.npart ; i++) {
      auto& p = m_mpg.addParticle();
      p.setPDG(part[i].part_prop.icode);
      p.set4Vector(part[i].p);
      p.setMass(part[i].part_prop.pmass);
      p.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
      p.setProductionVertex(vertex);
      p.setValidVertex(true);
    }
    // fill data of the recoil electron and positron
    auto& p1 = m_mpg.addParticle();
    p1.setPDG(11);
    p1.set4Vector(m_generator.pe);
    p1.setMass(m_generator.me);
    p1.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
    p1.setProductionVertex(vertex);
    p1.setValidVertex(true);

    auto& p2 = m_mpg.addParticle();
    p2.setPDG(-11);
    p2.set4Vector(m_generator.pp);
    p2.setMass(m_generator.me);
    p2.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
    p2.setProductionVertex(vertex);
    p2.setValidVertex(true);

  }
  //Fill MCParticle List
  m_mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);

}

void TrepsInputModule::terminate()
{
}

double TrepsInputModule::getCrossSection(double W)
{
  if (m_generator.diffCrossSectionOfW.size() == 0) {
    B2FATAL("Cross Section Table is empty !!!");
    return 0.;
  }

  // lower_bound returns first iterator which meets >=W condition. --> upper side
  auto it_upper = m_generator.diffCrossSectionOfW.lower_bound(W);
  auto it_lower = it_upper;
  it_lower--;

  return (it_upper->second - it_lower->second) / (it_upper->first - it_lower->first) * (W - it_lower->first) + it_lower->second;
}

double TrepsInputModule::simulateW()
{

  while (1) {
    double crossSectionForMC = gRandom->Uniform(0.0, m_generator.totalCrossSectionForMC);

    auto it_upper = m_generator.WOfCrossSectionForMC.lower_bound(crossSectionForMC);
    auto it_lower = it_upper;
    it_lower--;

    double diffCrossSectionAtUpper = m_generator.diffCrossSectionOfW.at(it_upper->second);
    double diffCrossSectionAtLower = m_generator.diffCrossSectionOfW.at(it_lower->second);
    double limit = (diffCrossSectionAtUpper > diffCrossSectionAtLower) ? diffCrossSectionAtUpper * 1.01 : diffCrossSectionAtLower *
                   1.01;
    // Higher diffCrossSection * 1.01 is set as limit. Same definition of limit is used in TrepsB::wtable().

    double W = (crossSectionForMC - it_lower->first) / limit + it_lower->second ;
    if (W < m_generator.diffCrossSectionOfW.begin()->first or W > m_generator.diffCrossSectionOfW.rbegin()->first)
      B2FATAL("W has to be in [" << m_generator.diffCrossSectionOfW.begin()->first << ", "
              << m_generator.diffCrossSectionOfW.rbegin()->first
              << "] !!! W = " << W << ", crossSectionForMC = " << crossSectionForMC);

    double trial = gRandom->Uniform(0.0, limit);
    double crossSection = getCrossSection(W);

    if (trial < crossSection)
      return W;

  }

  return 0;
}


void TrepsInputModule::initializeGenerator()
{
  // Set parameter files
  m_generator.setParameterFile(m_parameterFile);
  m_generator.setDiffcrosssectionFile(m_differentialCrossSectionFile);
  m_generator.setWlistFile(m_wListTableFile);

  // Set cut parameters
  m_generator.setMaximalQ2(m_maximalQ2);
  m_generator.setMaximalAbsCosTheta(m_maximalAbsCosTheta);
  m_generator.applyCosThetaCutCharged(m_applyCosThetaCutCharged);
  m_generator.setMinimalTransverseMomentum(m_minimalTransverseMomentum);
  m_generator.applyTransverseMomentumCutCharged(m_applyTransverseMomentumCutCharged);

  // Initialize the initial particle information
  TVector3 p3;
  const BeamParameters& nominalBeam = m_initial.getBeamParameters();
  m_generator.setBeamEnergy(nominalBeam.getMass() / 2.);
  p3 = nominalBeam.getHER().Vect();
  m_generator.setElectronMomentum(p3);
  p3 = nominalBeam.getLER().Vect();
  m_generator.setPositronMomentum(p3);

  // Initialize generator;
  m_generator.initp();

  m_generator.create_hist();
  m_generator.initg();

  if (m_useDiscreteAndSortedW) {
    // Initialize wtable with WListTable
    B2INFO("Discrete W-list is used !!!");

    m_generator.wtcount = 0;
    m_generator.wtable(0);

    m_generator.w = (double)m_generator.wf;

    m_generator.updateW();
  } else {
    // Initialize wtable with DifferentialCrossSection
    m_generator.wtable();
  }

  m_initialized = true;
}
