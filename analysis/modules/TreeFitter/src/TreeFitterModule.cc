/* BASF2 (Belle Analysis Framework 2)                                    *
*                                                                        *
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
#include <analysis/VertexFitting/TreeFitter/FitManager.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/utility/ParticleCopy.h>

#include <framework/geometry/BFieldManager.h>

#include <analysis/VertexFitting/TreeFitter/ConstraintConfig.h>

#include <framework/particledb/EvtGenDatabasePDG.h>
using namespace Belle2;

REG_MODULE(TreeFitter)

TreeFitterModule::TreeFitterModule() : Module(), m_nCandidatesBeforeFit(-1), m_nCandidatesAfter(-1)
{
  setDescription("Tree Fitter module. Performs simultaneous fit of all vertices in a decay chain. Can also be used to just fit a single vertex.");
  addParam("particleList", m_particleList, "Input mother of the decay tree to fit. For example 'B0:myB0particleList'.");
  addParam("confidenceLevel", m_confidenceLevel,
           "Confidence level to accept fitted decay tree. Candidates with < confidenceLevel will be removed from the particle list! Typical Values: -1: keep all particle candidates, 0: remove all that fail the fit, 0.001: standard cut, 0.1: (too) tight cut. Optimise using a figure of merit (for example S/(sqrt{S+B}) ) for your analysis.",
           0.0);
  addParam("convergencePrecision", m_precision,
           "Upper limit for chi2 fluctuations to accept result. Larger value = less signal rejection but also less background rejection. Optimized for FOM on different topologies - don't touch unless you REALLY want this.",
           1.);
  addParam("massConstraintList", m_massConstraintList,
           "Type::[int]. List of particles to mass constrain with int = pdg code. Note that the variables 'M': fit result for the particle and 'InvM': calculated from the daughter momenta, will look different (especially if you don't update the daughters!).");
  addParam("massConstraintList_particlename", m_massConstraintList_particlename, "");
  addParam("customOriginVertex", m_customOriginVertex,
           "Type::[double]. List of vertex coordinates to be used in the custom origin constraint.", {0.001, 0, 0.0116});
  addParam("customOriginCovariance", m_customOriginCovariance,
           "Type::[double]. List vertex covariance elements used in the custom origin constraint (as a vector). Default is ment for B0 decays and is taken from 100k generated B0 to mumu events.",
  {
    0.0048, 0,        0,
    0,      0.003567, 0,
    0,      0,        0.0400
  }
          );
  addParam("customOriginConstraint", m_customOrigin,
           "Use a custom vertex as the production point of the highest hierachy particle (register this as the mother of the list you specify). Like the beam constraint but you can specify the position its covariance yourself. ",
           false);
  addParam("ipConstraint", m_ipConstraint,
           "use the IP as the origin of the tree. This registers an internal IP particle as the mother of the list you give. Or in other words forces the PRODUCTION vertex of your particle to be the IP and its covariance as specified in the database.",
           false);
  addParam("updateAllDaughters", m_updateDaughters,
           "Update all daughters (vertex position and momenta) in the tree. If not set only the 4-momenta for the head of the tree will be updated. We also update the vertex position of the daughters regardless of what you put here, because otherwise the default when the particle list is created is {0,0,0}.",
           false);
  //
  addParam("expertMassConstraintType", m_massConstraintType,
           "Type::[int]. False(0): use particles parameters in mass constraint; True: use sum of daughter parameters for mass constraint. WAARNING not even guranteed that it works.",
           0);
  addParam("expertRemoveConstraintList", m_removeConstraintList,
           "Type::[string]. List of constraints that you do not want to be used in the fit. WARNING don't use if you don't know exactly what it does.", {});
  addParam("expertUseReferencing", m_useReferencing,
           "Use the Extended Kalman Fitler. This implementation linearises around the previous state vector which gives smoother convergence.",
           true);
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
  const unsigned int n = plist->getListSize();
  m_nCandidatesBeforeFit += n;

  for (unsigned i = 0; i < n; i++) {
    Belle2::Particle* particle = plist->getParticle(i);

    if (m_updateDaughters == true) {
      ParticleCopy::copyDaughters(particle);
    }

    bool ok = fitTree(particle);

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
    B2WARNING("Not a single candidate survived the fit. Candidates before fit: " << m_nCandidatesBeforeFit << " after: " <<
              m_nCandidatesAfter);
  }
}

bool TreeFitterModule::fitTree(Belle2::Particle* head)
{
  std::unique_ptr<TreeFitter::FitManager> TreeFitter(
    new TreeFitter::FitManager(
      head,
      m_precision,
      m_ipConstraint,
      m_customOrigin,
      m_updateDaughters,
      m_customOriginVertex,
      m_customOriginCovariance,
      m_useReferencing
    )
  );
  /** TODO this is a bit of a hack. Make a config struct or so. */
  //  TreeFitter::massConstraintListPDG = m_massConstraintList;
  if ((m_massConstraintList.size()) == 0 && (m_massConstraintList_particlename.size()) > 0) {
    for (unsigned int i = 0; i < m_massConstraintList_particlename.size(); i++) {
      TParticlePDG* particletemp = TDatabasePDG::Instance()->GetParticle((m_massConstraintList_particlename[i]).c_str());
      m_massConstraintList.push_back(particletemp->PdgCode());
    }
    TreeFitter::massConstraintListPDG = m_massConstraintList;
  } else {
    TreeFitter::massConstraintListPDG = m_massConstraintList;
  }

  TreeFitter::massConstraintType = m_massConstraintType;
  TreeFitter::removeConstraintList = m_removeConstraintList;

  bool rc = TreeFitter->fit();
  return rc;
}

void TreeFitterModule::plotFancyASCII()
{
  B2INFO("\033[1;35m================================================================================\033[0m");
  B2INFO("\033[40;97m            ,.,                                                                 \033[0m");
  B2INFO("\033[40;97m           ;%&M%;_   ,..,                                                       \033[0m");
  B2INFO("\033[40;97m             \"_ “__” % M % M %;          , ..., ,                               \033[0m");
  B2INFO("\033[40;97m      , ..., __.\" --\"    , .,     _ - “ % &W % WM %;                            \033[0m");
  B2INFO("\033[40;97m     ; % M&$M % ”___ \"_._   %M%”_.”” _ \"\"\"\"\"\"                                   \033[0m");
  B2INFO("\033[40;97m       \"\"\"\"\"    \"\" , \\_.   \"_. .\"                                               \033[0m");
  B2INFO("\033[40;97m              , ., _\"__ \\__./ .\"                                                \033[0m");
  B2INFO("\033[40;97m          ___       __ |  y     , ..,     \033[97;40mThank you for using TreeFitter.       \033[0m");
  B2INFO("\033[40;97m         /)'\\    ''''| u  \\ %W%W%%;                                             \033[0m");
  B2INFO("\033[40;97m     ___)/   \"---\\_ \\   |____”            \033[97;40mCite:                                 \033[0m");
  B2INFO("\033[40;97m   ;&&%%;           (|__.|)./  ,..,           \033[97;40m(paper here)                      \033[0m");
  B2INFO("\033[40;97m             ,.., ___\\    |/     &&\"                                            \033[0m");
  B2INFO("\033[40;97m           &&%%&    (| Uo /        '\"     \033[97;40mEmail:                                \033[0m");
  B2INFO("\033[40;97m            ''''     \\ 7 \\                   \033[97;40mfrancesco.tenchini@desy.de         \033[0m");
  B2INFO("\033[40;97m  ._______________.-‘____””—.____.           \033[97;40mjo-frederik.krohn@desy.de          \033[0m");
  B2INFO("\033[40;97m   \\                           /                                                \033[0m");
  B2INFO("\033[40;97m    \\       \033[0m\033[32;40mTREEFITTER\033[0m\033[40;97m        /                                                 \033[0m");
  B2INFO("\033[40;97m     \\_______________________/                                                  \033[0m");
  B2INFO("\033[40;97m      (_)                 (_)                                                   \033[0m");
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
