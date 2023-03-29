#include <analysis/modules/NeutralHadronMatcher/NeutralHadronMatcherModule.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TRandom.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(NeutralHadronMatcher)

NeutralHadronMatcherModule::NeutralHadronMatcherModule() : Module()
{
  setDescription("Perform geometrical match between MC neutral hadron (given by mcPDG) and ECL clusters from the particleLists");
  addParam("particleLists", m_ParticleLists, "Input particle list");
  addParam("efficiencyCorrection", m_effcorr, "1-data/mc efficiency correction factor", 0.83);
  addParam("distanceCut", m_distance, "Matching distance", 15.0);
  addParam("mcPDGcode", mcPDG, "MC PDG code of the neutral hadron", 130);
}

void NeutralHadronMatcherModule::initialize()
{
  if (mcPDG == 130) {
    m_infoName = "mcdistanceKL";
    m_matchedId = "mdstIndexTruthKL";
  } else if (abs(mcPDG) == 2112) {
    m_infoName = "mcdistanceNeutron";
    m_matchedId = "mdstIndexTruthNeutron";
  } else {
    B2FATAL("Unsupported mcPDG value: " << mcPDG);
  }
}

void NeutralHadronMatcherModule::event()
{

  // Initialize extra info
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);
    //check particle List exists and has particles
    if (!particleList) {
      B2FATAL("ParticleList " << iList << " not found");
      continue;
    }

    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);
      particle->addExtraInfo(m_infoName, 1.e10);
      particle->addExtraInfo(m_matchedId, -1);
    }
  }

  if (not Environment::Instance().isMC()) {
    return;
  }

  StoreArray<MCParticle> mcparticles;
  if (mcparticles.getEntries() < 1) {
    B2FATAL("Missing mcparticles list for MC");
  }


  // loop over MC particles first
  for (MCParticle& mcPart : mcparticles) {
    if (mcPart.isPrimaryParticle() && abs(mcPart.getPDG()) == mcPDG) {

      bool AddInefficiency = (gRandom->Uniform() > m_effcorr);
      auto vtx = mcPart.getProductionVertex();
      auto momentum = mcPart.getMomentum();

      // loop over particle lists
      for (auto& iList : m_ParticleLists) {
        StoreObjPtr<ParticleList> particleList(iList);

        size_t nPart = particleList->getListSize();
        for (size_t iPart = 0; iPart < nPart; iPart++) {
          auto particle = particleList->getParticle(iPart);
          const Belle2::MCParticle* mcPartCl = particle->getRelated<Belle2::MCParticle>();
          const ECLCluster* eclcluster = particle->getECLCluster();

          if (eclcluster) {
            if ((mcPartCl) && (mcPartCl->getPDG() == 22)) continue;

            // got potential KL candidate
            double R = eclcluster->getR();
            double phi = eclcluster->getPhi();
            double theta = eclcluster->getTheta();
            double zcl = R / tan(theta);
            double xcl = R * cos(phi);
            double ycl = R * sin(phi);

            double distMin = particle-> getExtraInfo(m_infoName);

            double rECL = R; //cm
            double z = vtx.Z() + rECL / tan(momentum.Theta());
            double x = vtx.X() + rECL * cos(momentum.Phi());
            double y = vtx.Y() + rECL * sin(momentum.Phi());

            double dist = sqrt((x - xcl) * (x - xcl) + (y - ycl) * (y - ycl) + (z - zcl) * (z - zcl));
            if (dist < distMin) {
              distMin = dist;
            }
            if ((distMin < m_distance) && AddInefficiency) {
              distMin = -distMin;
            }

            particle->setExtraInfo(m_infoName, distMin);
            if ((distMin < m_distance) & (distMin > 0)) {
              particle->setExtraInfo(m_matchedId, mcPart.getArrayIndex());
            }
          }
        }
      }
    }
  }
}
