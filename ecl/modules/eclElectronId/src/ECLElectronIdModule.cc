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

#include <ecl/modules/eclElectronId/ECLElectronIdModule.h>
#include <ecl/dbobjects/ECLEoParams.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLElectronId)

ECLElectronIdModule::ECLElectronIdModule() : Module()
{
  setDescription("E/p-based ECL charged particle ID. Likelihood values for each particle hypothesis are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useUnsignedParticleHypo", m_useUnsignedParticleHypo,
           "Set true if you want to use PDF hypotheses that do not distinguish between +/- charge.", bool(false));

  for (unsigned int i = 0; i < ECLPidLikelihood::c_noOfHypotheses; i++) {
    m_pdf[i] = 0;
  }
}

ECLElectronIdModule::~ECLElectronIdModule() {}

void ECLElectronIdModule::initialize()
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

  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::electron,
                                               1)]  = new ECL::ECLElectronPdf)->init((!m_useUnsignedParticleHypo) ? eAntiParams.c_str() : eParams.c_str());  // e+
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::muon,
                                               1)]      = new ECL::ECLMuonPdf)    ->init((!m_useUnsignedParticleHypo) ? muAntiParams.c_str() : muParams.c_str());  // mu+
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::pion, 1)]      = new ECL::ECLPionPdf)    ->init(piParams.c_str()); // pi+
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::kaon, 1)]      = new ECL::ECLKaonPdf)    ->init(kaonParams.c_str()); // K+
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::proton, 1)]    = new ECL::ECLProtonPdf)  ->init(protonParams.c_str()); // p+
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::electron, -1)] = new ECL::ECLElectronPdf)->init(eParams.c_str()); // e-
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::muon, -1)]     = new ECL::ECLMuonPdf)    ->init(muParams.c_str()); // mu-
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::pion,
                                               -1)]     = new ECL::ECLPionPdf)    ->init((!m_useUnsignedParticleHypo) ? piAntiParams.c_str() : piParams.c_str());  // pi-
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::kaon,
                                               -1)]     = new ECL::ECLKaonPdf)    ->init((!m_useUnsignedParticleHypo) ? kaonAntiParams.c_str() : kaonParams.c_str());  // K-
  (m_pdf[ECLPidLikelihood::getChargeAwareIndex(Const::proton,
                                               -1)]   = new ECL::ECLProtonPdf)  ->init((!m_useUnsignedParticleHypo) ? protonAntiParams.c_str() : protonParams.c_str());  // p-
}

void ECLElectronIdModule::beginRun() {}

void ECLElectronIdModule::event()
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

    float likelihoods[ECLPidLikelihood::c_noOfHypotheses];
    double eop = energy / p;

    // Store the PDFs for both +/- particle hypotheses

    for (int index(0); index < ECLPidLikelihood::c_noOfHypotheses; ++index) {

      ECL::ECLAbsPdf* currentpdf = m_pdf[index];

      if (currentpdf == 0) {
        currentpdf = m_pdf[Const::pion.getIndex()]; // use pion pdf when specialized pdf is not assigned.
      }
      double pdfval = currentpdf->pdf(eop, p, theta);

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::electron, 1)) {
        ECL::ECLElectronPdf* antielpdf = dynamic_cast<ECL::ECLElectronPdf*>(currentpdf);
        antielpdf = antielpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is e+ (" << index << ")");
        B2DEBUG(20, "p = " << p / antielpdf->getEnergyUnit() << ", theta = " << theta / antielpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << antielpdf->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1 = " << antielpdf->pdfParams(p, theta)->sigma1);
        B2DEBUG(20, "mu2 = " << antielpdf->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << antielpdf->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "alpha = " << antielpdf->pdfParams(p, theta)->alpha);
        B2DEBUG(20, "nn = " << antielpdf->pdfParams(p, theta)->nn);
        B2DEBUG(20, "fraction = " << antielpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::muon, 1)) {
        ECL::ECLMuonPdf* antimu = dynamic_cast<ECL::ECLMuonPdf*>(currentpdf);
        antimu = antimu; // get rid of warning
        B2DEBUG(20, "Current hypothesis is mu+ (" << index << ")");
        B2DEBUG(20, "p = " << p / antimu->getEnergyUnit() << ", theta = " << theta / antimu->getAngularUnit());
        B2DEBUG(20, "mu1 = " << antimu->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << antimu->pdfParams(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << antimu->pdfParams(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << antimu->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << antimu->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "fraction = " << antimu->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::pion, 1)) {
        ECL::ECLPionPdf* pipdf = dynamic_cast<ECL::ECLPionPdf*>(currentpdf);
        pipdf = pipdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is pi+ (" << index << ")");
        B2DEBUG(20, "p = " << p / pipdf->getEnergyUnit() << ", theta = " << theta / pipdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << pipdf->pdfParamsMu(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << pipdf->pdfParamsMu(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << pipdf->pdfParamsMu(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << pipdf->pdfParamsMu(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << pipdf->pdfParamsMu(p, theta)->sigma2);
        B2DEBUG(20, "fraction (mu) = " << pipdf->pdfParamsMu(p, theta)->fraction);
        B2DEBUG(20, "mu3 = " << pipdf->pdfParams(p, theta)->mu3);
        B2DEBUG(20, "sigma3 = " << pipdf->pdfParams(p, theta)->sigma3);
        B2DEBUG(20, "fraction = " << pipdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::kaon, 1)) {
        ECL::ECLKaonPdf* kaonpdf = dynamic_cast<ECL::ECLKaonPdf*>(currentpdf);
        kaonpdf = kaonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is K+ (" << index << ")");
        B2DEBUG(20, "p = " << p / kaonpdf->getEnergyUnit() << ", theta = " << theta / kaonpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << kaonpdf->pdfParamsMu(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << kaonpdf->pdfParamsMu(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << kaonpdf->pdfParamsMu(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << kaonpdf->pdfParamsMu(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << kaonpdf->pdfParamsMu(p, theta)->sigma2);
        B2DEBUG(20, "fraction (mu) = " << kaonpdf->pdfParamsMu(p, theta)->fraction);
        B2DEBUG(20, "mu3 = " << kaonpdf->pdfParams(p, theta)->mu3);
        B2DEBUG(20, "sigma3 = " << kaonpdf->pdfParams(p, theta)->sigma3);
        B2DEBUG(20, "fraction = " << kaonpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::proton, 1)) {
        ECL::ECLProtonPdf* protonpdf = dynamic_cast<ECL::ECLProtonPdf*>(currentpdf);
        protonpdf = protonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is p+ (" << index << ")");
        B2DEBUG(20, "p = " << p / protonpdf->getEnergyUnit() << ", theta = " << theta / protonpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << protonpdf->pdfParamsMu(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << protonpdf->pdfParamsMu(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << protonpdf->pdfParamsMu(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << protonpdf->pdfParamsMu(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << protonpdf->pdfParamsMu(p, theta)->sigma2);
        B2DEBUG(20, "fraction (mu) = " << protonpdf->pdfParamsMu(p, theta)->fraction);
        B2DEBUG(20, "mu3 = " << protonpdf->pdfParams(p, theta)->mu3);
        B2DEBUG(20, "sigma3 = " << protonpdf->pdfParams(p, theta)->sigma3);
        B2DEBUG(20, "fraction = " << protonpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::electron, -1)) {
        ECL::ECLElectronPdf* elpdf = dynamic_cast<ECL::ECLElectronPdf*>(currentpdf);
        elpdf = elpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is e- (" << index << ")");
        B2DEBUG(20, "p = " << p / elpdf->getEnergyUnit() << ", theta = " << theta / elpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << elpdf->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1 = " << elpdf->pdfParams(p, theta)->sigma1);
        B2DEBUG(20, "mu2 = " << elpdf->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << elpdf->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "alpha = " << elpdf->pdfParams(p, theta)->alpha);
        B2DEBUG(20, "nn = " << elpdf->pdfParams(p, theta)->nn);
        B2DEBUG(20, "fraction = " << elpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::muon, -1)) {
        ECL::ECLMuonPdf* mupdf = dynamic_cast<ECL::ECLMuonPdf*>(currentpdf);
        mupdf = mupdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is mu- (" << index << ")");
        B2DEBUG(20, "p = " << p / mupdf->getEnergyUnit() << ", theta = " << theta / mupdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << mupdf->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << mupdf->pdfParams(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << mupdf->pdfParams(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << mupdf->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << mupdf->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "fraction = " << mupdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::pion, -1)) {
        ECL::ECLPionPdf* antipipdf = dynamic_cast<ECL::ECLPionPdf*>(currentpdf);
        antipipdf = antipipdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is pi- (" << index << ")");
        B2DEBUG(20, "p = " << p / antipipdf->getEnergyUnit() << ", theta = " << theta / antipipdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << antipipdf->pdfParamsMu(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << antipipdf->pdfParamsMu(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << antipipdf->pdfParamsMu(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << antipipdf->pdfParamsMu(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << antipipdf->pdfParamsMu(p, theta)->sigma2);
        B2DEBUG(20, "fraction (mu) = " << antipipdf->pdfParamsMu(p, theta)->fraction);
        B2DEBUG(20, "mu3 = " << antipipdf->pdfParams(p, theta)->mu3);
        B2DEBUG(20, "sigma3 = " << antipipdf->pdfParams(p, theta)->sigma3);
        B2DEBUG(20, "fraction = " << antipipdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::kaon, -1)) {
        ECL::ECLKaonPdf* antikaonpdf = dynamic_cast<ECL::ECLKaonPdf*>(currentpdf);
        antikaonpdf = antikaonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is K- (" << index << ")");
        B2DEBUG(20, "p = " << p / antikaonpdf->getEnergyUnit() << ", theta = " << theta / antikaonpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << antikaonpdf->pdfParamsMu(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << antikaonpdf->pdfParamsMu(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << antikaonpdf->pdfParamsMu(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << antikaonpdf->pdfParamsMu(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << antikaonpdf->pdfParamsMu(p, theta)->sigma2);
        B2DEBUG(20, "fraction (mu) = " << antikaonpdf->pdfParamsMu(p, theta)->fraction);
        B2DEBUG(20, "mu3 = " << antikaonpdf->pdfParams(p, theta)->mu3);
        B2DEBUG(20, "sigma3 = " << antikaonpdf->pdfParams(p, theta)->sigma3);
        B2DEBUG(20, "fraction = " << antikaonpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (index == ECLPidLikelihood::getChargeAwareIndex(Const::proton, -1)) {
        ECL::ECLProtonPdf* antiprotonpdf = dynamic_cast<ECL::ECLProtonPdf*>(currentpdf);
        antiprotonpdf = antiprotonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is p- (" << index << ")");
        B2DEBUG(20, "p = " << p / antiprotonpdf->getEnergyUnit() << ", theta = " << theta / antiprotonpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << antiprotonpdf->pdfParamsMu(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << antiprotonpdf->pdfParamsMu(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << antiprotonpdf->pdfParamsMu(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << antiprotonpdf->pdfParamsMu(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << antiprotonpdf->pdfParamsMu(p, theta)->sigma2);
        B2DEBUG(20, "fraction (mu) = " << antiprotonpdf->pdfParamsMu(p, theta)->fraction);
        B2DEBUG(20, "mu3 = " << antiprotonpdf->pdfParams(p, theta)->mu3);
        B2DEBUG(20, "sigma3 = " << antiprotonpdf->pdfParams(p, theta)->sigma3);
        B2DEBUG(20, "fraction = " << antiprotonpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (isnormal(pdfval) && pdfval > 0) likelihoods[index] = log(pdfval);
      else likelihoods[index] = m_minLogLike;
    } // end loop on hypo

    const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(likelihoods, energy, eop, e9e21, lat, dist, trkdepth, shdepth,
                                                                (int) nCrystals,
                                                                nClusters);
    track.addRelationTo(eclPidLikelihood);

  } // end loop on tracks
}

void ECLElectronIdModule::endRun()
{
}

void ECLElectronIdModule::terminate()
{
  for (int index(0); index < ECLPidLikelihood::c_noOfHypotheses; ++index) {
    delete m_pdf[index];
  }
}
