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
/* --------------- WARNING ---------------------------------------------- *
   If you have more complex parameter types in your class then simple int,
   double or std::vector of those you might need to uncomment the following
   include directive to avoid an undefined reference on compilation.
   * ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



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

  // setDescription(R"DOC("Input from TREPS generator (No-tag)

  //     Input from TREPS generator for ee->ee hadrons
  //     )DOC");

  // Parameter definitions
  std::string dfname("treps_par.dat");
  std::string fname;
  addParam("InputFileName", fname, "filename for TREPS input", dfname);
  strncpy(filename, fname.c_str(), 130);

  std::string dfname2("pipidcs.dat");
  std::string fname2;
  addParam("InputFileName2", fname2, "filename for W-List input", dfname2);
  strncpy(filename2, fname2.c_str(), 130);

  std::string nocheck("");
  addParam("RootFileNameForCheck", rfnfc, "filename for TREPS W-Listbehavior check", nocheck);

  TrepsB::initp();
  TrepsB::create_hist();

  //Initialize generator;
  UtrepsB::initg();

  //201903
  B2DEBUG(10, "wtable started !!!");
  TrepsB::wtable();
  //201903E

}

void trepsinputModule::initialize()
{
  B2DEBUG(10, "TrepsInputModule is initialized !!!");

  m_mcparticles.registerInDataStore();
}

void trepsinputModule::event()
{
  B2DEBUG(10, "Event started in TrepsInputModule !!!");

  TrepsB::w = simulateW();
  trepsinputModule::updateW();

  int idummy = 0;
  int iret = TrepsB::event_gen(idummy);
  mpg.clear();

  if (iret >= 1) {

    const Part_gen* part = TrepsB::partgen;

    // fill data of the final-state particles
    for (int i = 0; i < npart ; i++) {
      auto& p = mpg.addParticle();
      p.setPDG(part[i].part_prop.icode);
      p.set4Vector(part[i].p);
      p.setMass(part[i].part_prop.pmass);
      p.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
    }
    // fill data of the recoil electron and positron
    auto& p1 = mpg.addParticle();
    p1.setPDG(11);
    p1.set4Vector(TrepsB::pe);
    p1.setMass(TrepsB::me);
    p1.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);

    auto& p2 = mpg.addParticle();
    p2.setPDG(-11);
    p2.set4Vector(TrepsB::pp);
    p2.setMass(TrepsB::me);
    p2.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);


  }
  //Fill MCParticle List
  mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);
}
void trepsinputModule::terminate()
{
  if (boost::filesystem::path(rfnfc).extension().string() == std::string(".root")) {
    TFile* f = new TFile(rfnfc.c_str(), "recreate");
    if (f) {
      B2DEBUG(10, "Write histograms for check into " << rfnfc);
      f->cd();
      treh1->Write();
      treh2->Write();
      treh3->Write();
      treh4->Write();
      treh5->Write();
      treh6->Write();
      f->Flush();
      f->Close();
    }
  }
  TrepsB::terminate();
}

double trepsinputModule::simulateW()
{
  std::random_device rnd;
  std::mt19937 mt(rnd());

  const std::map<double, double> upperLimit_pipi = {
    {0.5, 4.5},
    {1.5, 0.2},
    {2.0, 0.025},
    {2.5, 0.006},
    {3.0, 0.0013},
    {4.0, 0.}
  };

  std::map<double, double> areaUpToBin;
  double areaOfRandom = 0.;
  double lowerEdge = 0.;
  double upperEdge = 0.;
  double currentLimit = 0.;
  for (auto x : upperLimit_pipi) {
    upperEdge = x.first;
    areaOfRandom += (upperEdge - lowerEdge) * currentLimit;

    lowerEdge = upperEdge;
    currentLimit = x.second;

    areaUpToBin[lowerEdge] = areaOfRandom;
  }

  std::uniform_real_distribution<double> getScaledW(0.0, areaOfRandom);

  while (1) {
    double scaledW = getScaledW(mt);

    double W = 0.;

    double edge = 0.;
    double limit = 0.;
    double area = 0.;
    for (auto x : areaUpToBin) {

      if (scaledW < x.second) {
        W = (scaledW - area) / limit + edge;
        break;
      }

      edge = x.first;
      area = x.second;
      limit = upperLimit_pipi.at(edge);
    }

    if (W < 0.5 or W > 4.0)
      B2FATAL("W has to be in [0.5, 4.0] !!! W = " << W << ", scaledW = " << scaledW);

    std::uniform_real_distribution<double> getTrial(0.0, limit);
    double trial = getTrial(mt);
    double crossSection = getCrossSection(W);

    if (trial < crossSection)
      return W;

  }

  return 0;
}

double trepsinputModule::getCrossSection(double W)
{
  if (TrepsB::crossSectionOfW.size() == 0) {
    B2ERROR("Cross Section Table is empty !!!");
    return 0.;
  }

  auto it_upper = crossSectionOfW.lower_bound(W); // This lower_bound returns first iterator which meets >=w condition. --> upper side
  auto it_lower = it_upper;
  it_lower--;

  return (it_upper->second - it_lower->second) / (it_upper->first - it_lower->first) * (W - it_lower->first) + it_lower->second;
}
