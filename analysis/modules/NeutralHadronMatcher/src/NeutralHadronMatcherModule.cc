/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/NeutralHadronMatcher/NeutralHadronMatcherModule.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TRandom.h>
#include <analysis/variables/MCTruthVariables.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(NeutralHadronMatcher);

NeutralHadronMatcherModule::NeutralHadronMatcherModule() : Module()
{
  setDescription("Perform geometrical match between MC neutral hadron (given by mcPDG) and ECL clusters from the particleLists");
  addParam("particleLists", m_ParticleLists, "Input particle list");
  addParam("efficiencyCorrection", m_effcorr, "data/mc efficiency ratio", 0.83);
  addParam("distanceCut", m_distance, "Matching distance", 15.0);
  addParam("mcPDGcode", m_mcPDG, "MC PDG code of the neutral hadron", 130);
  addParam("ignoreClustersWithPDGcodes", m_PDGignore, "Do not attempt to match clusters that are already matched with specific codes",
           std::vector<int> {22});
}

void NeutralHadronMatcherModule::initialize()
{
  if (m_mcPDG == 130) {
    m_infoName = "mcdistanceKL";
    m_matchedId = "mdstIndexTruthKL";
  } else if (abs(m_mcPDG) == 2112) {
    m_infoName = "mcdistanceNeutron";
    m_matchedId = "mdstIndexTruthNeutron";
  } else {
    B2FATAL("Unsupported mcPDG value: " << m_mcPDG);
  }

  for (auto& iList : m_ParticleLists)
    StoreObjPtr<ParticleList>().isRequired(iList);

  if (! StoreArray<MCParticle>().isValid())
    B2WARNING("No MCParticles array found. Do you call the NeutralHadronMatcher with real data?");

}

void NeutralHadronMatcherModule::event()
{

  StoreArray<MCParticle> mcparticles;
  if (!mcparticles.isValid())
    return;
  if (mcparticles.getEntries() < 1)
    B2FATAL("Missing mcparticles list for MC");

  // Initialize extra info
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);

    //check particle List has particles
    const size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);
      particle->addExtraInfo(m_infoName, 1.e10);
      particle->addExtraInfo(m_matchedId, -1);
    }
  }

  // collect only primary mcparticles that match m_mcPDG
  std::vector< std::pair<MCParticle*, bool> > primaryMCParticles;
  for (int i = 0; i < mcparticles.getEntries(); i++) {
    auto mcPart = mcparticles[i];
    if (mcPart->isPrimaryParticle() && abs(mcPart->getPDG()) == m_mcPDG) {
      bool AddInefficiency = (gRandom->Uniform() > m_effcorr);
      primaryMCParticles.push_back(std::pair(mcPart, AddInefficiency));
    }
  }

  const size_t nMCPart = primaryMCParticles.size();

  // if no primary MCParticles matching m_mcPDG exists, nothing to do.
  if (nMCPart == 0)
    return;

  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);

    // loop over particles first
    const size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {

      auto particle = particleList->getParticle(iPart);
      auto bestMatch = Variable::particleClusterBestMCPDGCode(particle);

      if ((bestMatch) && (std::find(m_PDGignore.begin(), m_PDGignore.end(), int(bestMatch)) != m_PDGignore.end())) {
        continue;
      }

      const ECLCluster* eclcluster = particle->getECLCluster();
      if (!eclcluster) continue;

      // create a vector of distances from MCParticles
      std::vector< std::pair <double, bool> > distances;
      distances.resize(nMCPart);

      // loop over mcparticles
      for (size_t iMCPart = 0; iMCPart < nMCPart; iMCPart++) {

        auto mcPart = primaryMCParticles[iMCPart].first;

        auto vtx = mcPart->getProductionVertex();
        auto momentum = mcPart->getMomentum();

        // got potential KL candidate
        double R = eclcluster->getR();
        double phi = eclcluster->getPhi();
        double theta = eclcluster->getTheta();
        double zcl = R / tan(theta);
        double xcl = R * cos(phi);
        double ycl = R * sin(phi);

        double rECL = R; //cm
        double z = vtx.Z() + rECL / tan(momentum.Theta());
        double x = vtx.X() + rECL * cos(momentum.Phi());
        double y = vtx.Y() + rECL * sin(momentum.Phi());

        double dist = sqrt((x - xcl) * (x - xcl) + (y - ycl) * (y - ycl) + (z - zcl) * (z - zcl));
        distances[iMCPart] = std::pair(dist, primaryMCParticles[iMCPart].second);
      }

      auto it_distMin = std::min_element(distances.begin(), distances.end(),
      [](const auto & a, const auto & b) { return a.first < b.first;});
      double distMin = (*it_distMin).first;
      bool AddInefficiency = (*it_distMin).second;
      if ((distMin < m_distance) && AddInefficiency)
        distMin = -distMin;

      particle->setExtraInfo(m_infoName, distMin);

      if ((distMin < m_distance) && (distMin > 0))
        particle->setExtraInfo(m_matchedId, primaryMCParticles[std::distance(distances.begin(), it_distMin)].first->getArrayIndex());

    }
  }

}


