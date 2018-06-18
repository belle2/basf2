/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//This module
#include <ecl/modules/eclChargedPID/ECLChargedPIDModule.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLChargedPID)

ECLChargedPIDModule::ECLChargedPIDModule() : Module()
{
  setDescription("E/p-based ECL charged particle ID. Likelihood values for each particle hypothesis are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useUnsignedParticleHypo", m_useUnsignedParticleHypo,
           "Set true if you want to use PDF hypotheses that do not distinguish between +/- charge.", bool(false));

  for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
    m_pdf[0][i] = 0;
    m_pdf[1][i] = 0;
  }
}

ECLChargedPIDModule::~ECLChargedPIDModule() {}

void ECLChargedPIDModule::initialize()
{
  m_eclPidLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_eclPidLikelihoods);

  std::list<const string> paramList;
  const string eParams          = FileSystem::findFile("/data/ecl/electrons_N1.dat"); paramList.push_back(eParams);
  const string muParams         = FileSystem::findFile("/data/ecl/muons_N1.dat"); paramList.push_back(muParams);
  const string piParams         = FileSystem::findFile("/data/ecl/pions_N1.dat"); paramList.push_back(piParams);
  const string kaonParams       = FileSystem::findFile("/data/ecl/kaons_N1.dat"); paramList.push_back(kaonParams);
  const string protonParams     = FileSystem::findFile("/data/ecl/protons_N1.dat"); paramList.push_back(protonParams);
  const string eAntiParams      = FileSystem::findFile("/data/ecl/electronsanti_N1.dat"); paramList.push_back(eAntiParams);
  const string muAntiParams     = FileSystem::findFile("/data/ecl/muonsanti_N1.dat"); paramList.push_back(muAntiParams);
  const string piAntiParams     = FileSystem::findFile("/data/ecl/pionsanti_N1.dat"); paramList.push_back(piAntiParams);
  const string kaonAntiParams   = FileSystem::findFile("/data/ecl/kaonsanti_N1.dat"); paramList.push_back(kaonAntiParams);
  const string protonAntiParams = FileSystem::findFile("/data/ecl/protonsanti_N1.dat"); paramList.push_back(protonAntiParams);

  for (const auto& p : paramList) {
    if (p.empty()) B2FATAL(p << " pdfs parameter files not found.");
  }

  ECL::ECLAbsPdf::setEnergyUnit(Unit::MeV); // The energy unit in the .dat files
  ECL::ECLAbsPdf::setAngularUnit(Unit::deg); // The angular unit in the .dat files

  // Initialise the PDFs.
  // Here the order matters.
  // The positively charged particle hypotheses must go first (in the same order as they appear in the Const::chargedStableSet), then the negatively charged ones.

  (m_pdf[0][Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init((!m_useUnsignedParticleHypo) ? eAntiParams.c_str() :
      eParams.c_str());  // e+
  (m_pdf[0][Const::muon.getIndex()] = new ECL::ECLMuonPdf)->init((!m_useUnsignedParticleHypo) ? muAntiParams.c_str() :
      muParams.c_str());  // mu+
  (m_pdf[0][Const::pion.getIndex()] = new ECL::ECLPionPdf)->init(piParams.c_str()); // pi+
  (m_pdf[0][Const::kaon.getIndex()] = new ECL::ECLKaonPdf)->init(kaonParams.c_str()); // K+
  (m_pdf[0][Const::proton.getIndex()] = new ECL::ECLProtonPdf)->init(protonParams.c_str()); // p+
  (m_pdf[1][Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init(eParams.c_str()); // e-
  (m_pdf[1][Const::muon.getIndex()] = new ECL::ECLMuonPdf)->init(muParams.c_str()); // mu-
  (m_pdf[1][Const::pion.getIndex()] = new ECL::ECLPionPdf)->init((!m_useUnsignedParticleHypo) ? piAntiParams.c_str() :
      piParams.c_str());  // pi-
  (m_pdf[1][Const::kaon.getIndex()] = new ECL::ECLKaonPdf)->init((!m_useUnsignedParticleHypo) ? kaonAntiParams.c_str() :
      kaonParams.c_str());  // K-
  (m_pdf[1][Const::proton.getIndex()] = new ECL::ECLProtonPdf)->init((!m_useUnsignedParticleHypo) ? protonAntiParams.c_str() :
      protonParams.c_str());  // p-

}

void ECLChargedPIDModule::beginRun() {}

void ECLChargedPIDModule::event()
{

  for (const auto& track : m_tracks) {

    // load the pion fit hypothesis or the hypothesis which is the closest in mass to a pion
    // the tracking will not always successfully fit with a pion hypothesis
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;
    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    const double p     = fitRes->getMomentum().Mag();
    const double theta = fitRes->getMomentum().Theta();

    double energy(0), maxEnergy(0), e9e21(0);
    double lat(0), dist(0), trkdepth(0), shdepth(0);
    double nCrystals = 0;
    int nClusters = relShowers.size();

    for (const auto& eclShower : relShowers) {

      if (eclShower.getHypothesisId() != ECLCluster::c_nPhotons) continue;
      if (abs(eclShower.getTime()) > eclShower.getDeltaTime99()) continue;

      const double shEnergy = eclShower.getEnergy();
      energy += shEnergy;
      if (shEnergy > maxEnergy) {
        maxEnergy = shEnergy;
        e9e21 = eclShower.getE9oE21();
        lat = eclShower.getLateralEnergy();
        dist = eclShower.getMinTrkDistance();
        trkdepth = eclShower.getTrkDepth();
        shdepth = eclShower.getShowerDepth();
      }
      nCrystals += int(eclShower.getNumberOfCrystals());
    }

    float likelihoods[Const::ChargedStable::c_SetSize];
    double eop = energy / p;

    const unsigned int charge_idx = (fitRes->getChargeSign() > 0) ? 0 : 1;

    // Store the right PDFs depending on charge of the particle's track.
    for (unsigned int index(0); index < Const::ChargedStable::c_SetSize; ++index) {

      ECL::ECLAbsPdf* currentpdf = m_pdf[charge_idx][index];

      if (currentpdf == 0) {
        currentpdf = m_pdf[charge_idx][Const::pion.getIndex()]; // use pion pdf when specialized pdf is not assigned.
      }
      double pdfval = currentpdf->pdf(eop, p, theta);

      if (isnormal(pdfval) && pdfval > 0) likelihoods[index] = log(pdfval);
      else likelihoods[index] = m_minLogLike;
    } // end loop on hypo

    const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(likelihoods, energy, eop, e9e21, lat, dist, trkdepth, shdepth,
                                                                (int) nCrystals,
                                                                nClusters);
    track.addRelationTo(eclPidLikelihood);

  } // end loop on tracks
}

void ECLChargedPIDModule::endRun()
{
}

void ECLChargedPIDModule::terminate()
{
  for (unsigned int index(0); index < Const::ChargedStable::c_SetSize; ++index) {
    delete m_pdf[0][index];
    delete m_pdf[1][index];
  }
}
