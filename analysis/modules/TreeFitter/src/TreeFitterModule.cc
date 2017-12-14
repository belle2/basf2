/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Implementation of Decay Tree Fitter based on arXiv:physics/0503191
//Main module implementation

#include <analysis/modules/TreeFitter/TreeFitterModule.h>
#include <analysis/modules/TreeFitter/Fitter.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

REG_MODULE(TreeFitter)

TreeFitterModule::TreeFitterModule() : Module()
{
  setDescription("Tree Fitter module. Performs simultaneous fit of all vertices in a decay chain.");
  addParam("particleList", m_particleList, "Input mother of the decay tree to fit");
  addParam("confidenceLevel", m_confidenceLevel,
           "Confidence level to accept fitted decay tree. -1.0 for failed fits. Candidates with < confidenceLevel will be removed from the particle list! ",
           0.0);
  addParam("convergencePrecision", m_precision, "Upper limit for chi2 fluctuations to accept result.", 1.); //large value for now
  addParam("verbose", m_verbose, "BaBar verbosity (to be phased out in the future)", 5);
  addParam("massConstraintList", m_massConstraintList, "Type::[int]. List of particles to mass constrain with int = pdg code.");
  addParam("ipConstraintDimension", m_ipConstraintDimension,
           "Type::Int. Use the x-y-z-beamspot or x-y-beamtube constraint. Zero means no cosntraint which is the default. The Beamspot will be treated as the mother of the particlelist you feed.",
           0);
}

void TreeFitterModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_particleList);
  StoreArray<Belle2::Particle> particles;
  particles.isRequired();
  m_nCandidatesBeforeFit = 0;
  m_nCandidatesAfter = 0;
}

void TreeFitterModule::beginRun()
{
}

void TreeFitterModule::event()
{
  StoreObjPtr<ParticleList> plist(m_particleList);
  if (!plist) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  }

  std::vector<unsigned int> toRemove;
  unsigned int n = plist->getListSize();
  m_nCandidatesBeforeFit += n;
  for (unsigned i = 0; i < n; i++) {
    Belle2::Particle* particle = plist->getParticle(i);
    bool ok = doTreeFit(particle);
    if (!ok) {
      particle->setPValue(-1);
    }
    if (particle->getPValue() < m_confidenceLevel) {
      toRemove.push_back(particle->getArrayIndex());
    }
  }
  plist->removeParticles(toRemove);
  m_nCandidatesAfter += plist->getListSize();
}


void TreeFitterModule::terminate()
{
  if (m_nCandidatesAfter > 0) {
    plotFancyASCII();
  } else {
    B2ERROR("Not a single candidate survived the fit.");
  }
}

bool TreeFitterModule::doTreeFit(Belle2::Particle* head)
{
  std::unique_ptr<TreeFitter::Fitter> TreeFitObject(new TreeFitter::Fitter(head, m_precision, m_ipConstraintDimension));
  TreeFitObject->setVerbose(m_verbose);
  TreeFitObject->setMassConstraintList(m_massConstraintList);
  bool rc = TreeFitObject->fitUseEigen();
  return rc;
}

void TreeFitterModule::plotFancyASCII()
{
  //JFK: colors depend on your shell settings...
  //blinking is unfortunenately just supported by a few terminal species...
  //TODO write paper to cite...2017-10-27
  B2INFO("\033[1;35m================================================================================\033[0m");
  B2INFO("\033[40;97m            ,.,                                                                 \033[0m");
  B2INFO("\033[40;97m           ;%&M%;_   ,..,                                                       \033[0m");
  B2INFO("\033[40;97m             \"_ “__” % M % M %;          , ..., ,                               \033[0m");
  B2INFO("\033[40; 97m      , ..., __.\" --\"    , .,     _ - “ % &W % WM %;                            \033[0m");
  B2INFO("\033[40; 97m     ; % M&$M % ”___ \"_._   %M%”_.”” _ \"\"\"\"\"\"                                   \033[0m");
  B2INFO("\033[40;97m       \"\"\"\"\"    \"\" , \\_.   \"_. .\"                                               \033[0m");
  B2INFO("\033[40; 97m              , ., _\"__ \\__./ .\"                                                \033[0m");
  B2INFO("\033[40; 97m          ___       __ |  y     , ..,     \033[97;40mThank you for using TreeFitter.       \033[0m");
  B2INFO("\033[40; 97m         /)'\\    ''''| u  \\ %W%W%%;                                             \033[0m");
  B2INFO("\033[40;97m     ___)/   \"---\\_ \\   |____”            \033[97;40mCite:                                 \033[0m");
  B2INFO("\033[40;97m   ;&&%%;           (|__.|)./  ,..,           \033[97;40m(paper here)                      \033[0m");
  B2INFO("\033[40;97m             ,.., ___\\    |/     &&\"                                            \033[0m");
  B2INFO("\033[40;97m           &&%%&    (| Uo /        '\"     \033[97;40mEmail:                                \033[0m");
  B2INFO("\033[40;97m            ''''     \\ 7 \\                   \033[97;40mfrancesco.tenchini@unimelb.edu.au  \033[0m");
  B2INFO("\033[40;97m  ._______________.-‘____””—.____.           \033[97;40mjo-frederik.krohn@desy.de          \033[0m");
  B2INFO("\033[40;97m   \\                           /                                                \033[0m");
  B2INFO("\033[40;97m    \\       \033[0m\033[32;40mTREEFITTER\033[0m\033[40;97m        /                                                 \033[0m");
  B2INFO("\033[40;97m     \\_______________________/                                                  \033[0m");
  B2INFO("\033[40;97m      (_)                   (_)                                                 \033[0m");
  B2INFO("\033[40;97m                                                                                \033[0m");
  B2INFO("\033[1;35m============= TREEFIT STATISTICS ===============================================\033[0m");
  B2INFO("\033[1;39mCandidates before fit: " << m_nCandidatesBeforeFit << "\033[0m");
  B2INFO("\033[1;39mCandidates after fit:  " << m_nCandidatesAfter << "\033[0m");
  B2INFO("\033[1;39mA total of " << m_nCandidatesBeforeFit - m_nCandidatesAfter << " candidates was removed during the fit.\033[0m");
  B2INFO("\033[1;39m" << (double)m_nCandidatesAfter / (double)m_nCandidatesBeforeFit * 100.0 <<
         "% of candidates survived the fit.\033[0m");
  B2INFO("\033[1;39m" << 100. - (double)m_nCandidatesAfter / (double)m_nCandidatesBeforeFit * 100.0 <<
         "% of candidates did not.\033[0m");
  B2INFO("\033[1;39mYou choose to drop all candidates with pValue < " << m_confidenceLevel << ".\033[0m");
  B2INFO("\033[1;35m================================================================================\033[0m");
}
