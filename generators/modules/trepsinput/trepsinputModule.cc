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

#include <random>
#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(trepsinput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trepsinputModule::trepsinputModule() : Module(), UtrepsB()
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
  strncpy(TrepsB::parameterFile, m_parameterFile.c_str(), 130);
  strncpy(TrepsB::diffcrosssectionFile, m_differentialCrossSectionFile.c_str(), 130);
  strncpy(TrepsB::wlistFile, m_wListTableFile.c_str(), 130);

  TrepsB::initp();

  TrepsB::create_hist();
  UtrepsB::initg();

  if (m_useDiscreteAndSortedW) {
    // Initialize wtable with WListTable
    B2INFO("Discrete W-list is used !!!");
    TrepsB::wtcount = 0;
    TrepsB::wtable(0);

    TrepsB::w = (double)TrepsB::wf;
    trepsinputModule::updateW();
  } else {
    // Initialize wtable with DifferentialCrossSection
    TrepsB::wtable();
  }

  m_mcparticles.registerInDataStore();

}

void trepsinputModule::event()
{
  if (m_useDiscreteAndSortedW) {
    if (TrepsB::inmode != 0) return;

    TrepsB::wtcount++;
    TrepsB::wf = (float)(wtable(1));

    if (abs((double)TrepsB::wf - TrepsB::w) >= 0.001 && TrepsB::wf > 0.01) {
      B2INFO(" W value changed. " << TrepsB::w << " to " << TrepsB::wf);
      TrepsB::w = (double)TrepsB::wf;
      trepsinputModule::updateW();
    }
  } else {
    TrepsB::w = simulateW();
    trepsinputModule::updateW();
  }

  int idummy = 0;
  int iret = TrepsB::event_gen(idummy);
  m_mpg.clear();

  if (iret >= 1) {

    const Part_gen* part = TrepsB::partgen;

    // fill data of the final-state particles
    for (int i = 0; i < npart ; i++) {
      auto& p = m_mpg.addParticle();
      p.setPDG(part[i].part_prop.icode);
      p.set4Vector(part[i].p);
      p.setMass(part[i].part_prop.pmass);
      p.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
    }
    // fill data of the recoil electron and positron
    auto& p1 = m_mpg.addParticle();
    p1.setPDG(11);
    p1.set4Vector(TrepsB::pe);
    p1.setMass(TrepsB::me);
    p1.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);

    auto& p2 = m_mpg.addParticle();
    p2.setPDG(-11);
    p2.set4Vector(TrepsB::pp);
    p2.setMass(TrepsB::me);
    p2.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);


  }
  //Fill MCParticle List
  m_mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);
}

void trepsinputModule::terminate()
{
  TrepsB::terminate();
}

double trepsinputModule::getCrossSection(double W)
{
  if (TrepsB::diffCrossSectionOfW.size() == 0) {
    B2FATAL("Cross Section Table is empty !!!");
    return 0.;
  }

  // lower_bound returns first iterator which meets >=W condition. --> upper side
  auto it_upper = TrepsB::diffCrossSectionOfW.lower_bound(W);
  auto it_lower = it_upper;
  it_lower--;

  return (it_upper->second - it_lower->second) / (it_upper->first - it_lower->first) * (W - it_lower->first) + it_lower->second;
}

double trepsinputModule::simulateW()
{
  std::random_device rnd;
  std::mt19937 mt(rnd());

  std::uniform_real_distribution<double> getCrossSectionForMC(0.0, TrepsB::totalCrossSectionForMC);

  while (1) {
    double crossSectionForMC = getCrossSectionForMC(mt);

    auto it_upper = TrepsB::WOfCrossSectionForMC.lower_bound(crossSectionForMC);
    auto it_lower = it_upper;
    it_lower--;

    double diffCrossSectionAtUpper = TrepsB::diffCrossSectionOfW.at(it_upper->second);
    double diffCrossSectionAtLower = TrepsB::diffCrossSectionOfW.at(it_lower->second);
    double limit = (diffCrossSectionAtUpper > diffCrossSectionAtLower) ? diffCrossSectionAtUpper * 1.01 : diffCrossSectionAtLower *
                   1.01;
    // Higher diffCrossSection * 1.01 is set as limit. Same definition of limit is used in TrepsB::wtable().

    double W = (crossSectionForMC - it_lower->first) / limit + it_lower->second ;
    if (W < TrepsB::diffCrossSectionOfW.begin()->first or W > TrepsB::diffCrossSectionOfW.rbegin()->first)
      B2FATAL("W has to be in [" << TrepsB::diffCrossSectionOfW.begin()->first << ", " << TrepsB::diffCrossSectionOfW.rbegin()->first
              << "] !!! W = " << W << ", crossSectionForMC = " << crossSectionForMC);

    std::uniform_real_distribution<double> getTrial(0.0, limit);
    double trial = getTrial(mt);
    double crossSection = getCrossSection(W);

    if (trial < crossSection)
      return W;

  }

  return 0;
}

