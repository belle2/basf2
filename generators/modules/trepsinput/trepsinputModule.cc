/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Yo Sato                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/trepsinput/trepsinputModule.h>
#include <generators/treps/Sutool.h>
#include <generators/treps/UtrepsB.h>
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
REG_MODULE(trepsinput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trepsinputModule::trepsinputModule() : Module()
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

}

void trepsinputModule::initialize()
{
  //Initialize generator;
  m_generator.setParameterFile(m_parameterFile);
  m_generator.setDiffcrosssectionFile(m_differentialCrossSectionFile);
  m_generator.setWlistFile(m_wListTableFile);

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

  m_mcparticles.registerInDataStore();

}

void trepsinputModule::event()
{
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

  if (iret >= 1) {

    const Part_gen* part = m_generator.partgen;

    // fill data of the final-state particles
    for (int i = 0; i < m_generator.npart ; i++) {
      auto& p = m_mpg.addParticle();
      p.setPDG(part[i].part_prop.icode);
      p.set4Vector(part[i].p);
      p.setMass(part[i].part_prop.pmass);
      p.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
    }
    // fill data of the recoil electron and positron
    auto& p1 = m_mpg.addParticle();
    p1.setPDG(11);
    p1.set4Vector(m_generator.pe);
    p1.setMass(m_generator.me);
    p1.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);

    auto& p2 = m_mpg.addParticle();
    p2.setPDG(-11);
    p2.set4Vector(m_generator.pp);
    p2.setMass(m_generator.me);
    p2.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);


  }
  //Fill MCParticle List
  m_mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);
}

void trepsinputModule::terminate()
{
  // m_generator.terminate();
}

double trepsinputModule::getCrossSection(double W)
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

double trepsinputModule::simulateW()
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

