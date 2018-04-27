/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//This module
#include <ecl/modules/eclElectronId/ECLElectronIdModule.h>

//MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

//Framework
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Unit.h>

//ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/electronId/ECLAbsPdf.h>
#include <ecl/electronId/ECLElectronPdf.h>
#include <ecl/electronId/ECLMuonPdf.h>
#include <ecl/electronId/ECLPionPdf.h>
#include <ecl/electronId/ECLKaonPdf.h>
#include <ecl/electronId/ECLProtonPdf.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLElectronId)

ECLElectronIdModule::ECLElectronIdModule() : Module()
{
  setDescription("E/p-based ECL charged particle ID. Likelihood values for each particle hypothesis are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("useAntiParticleHypo", m_useAntiParticleHypo,
           "Set true if you want to use different PDF hypotheses for (charged) anti-particles", bool(false));

  for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) m_pdf[i] = 0;
}

ECLElectronIdModule::~ECLElectronIdModule() {}

void ECLElectronIdModule::initialize()
{
  m_eclPidLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_eclPidLikelihoods);

  std::list<const string> paramList;
  const string eParams      = FileSystem::findFile("/data/ecl/electrons_N1.dat"); paramList.push_back(eParams);
  const string muParams     = FileSystem::findFile("/data/ecl/muons_N1.dat"); paramList.push_back(muParams);
  const string piParams     = FileSystem::findFile("/data/ecl/pions_N1.dat"); paramList.push_back(piParams);
  const string kaonParams   = FileSystem::findFile("/data/ecl/kaons_N2.dat"); paramList.push_back(kaonParams);
  const string protonParams = FileSystem::findFile("/data/ecl/protons_N1.dat"); paramList.push_back(protonParams);
  const string eAntiParams      = FileSystem::findFile("/data/ecl/electronsanti_N1.dat"); paramList.push_back(eAntiParams);
  const string muAntiParams     = FileSystem::findFile("/data/ecl/muonsanti_N1.dat"); paramList.push_back(muAntiParams);
  const string piAntiParams     = FileSystem::findFile("/data/ecl/pionsanti_N1.dat"); paramList.push_back(piAntiParams);
  const string kaonAntiParams   = FileSystem::findFile("/data/ecl/kaonsanti_N2.dat"); paramList.push_back(kaonAntiParams);
  const string protonAntiParams = FileSystem::findFile("/data/ecl/protonsanti_N1.dat"); paramList.push_back(protonAntiParams);

  for (const auto& p : paramList) {
    if (p.empty()) B2FATAL(p << " pdfs parameter files not found.");
  }

  ECL::ECLAbsPdf::setEnergyUnit(Unit::MeV); // The energy unit in the .dat files
  ECL::ECLAbsPdf::setAngularUnit(Unit::deg); // The angular unit in the .dat files

  (m_pdf[Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init(eParams.c_str());
  (m_pdf[Const::muon.getIndex()]     = new ECL::ECLMuonPdf)->init(muParams.c_str());
  (m_pdf[Const::pion.getIndex()]     = new ECL::ECLPionPdf)->init(piParams.c_str());
  (m_pdf[Const::kaon.getIndex()]     = new ECL::ECLKaonPdf)->init(kaonParams.c_str());
  (m_pdf[Const::proton.getIndex()]   = new ECL::ECLProtonPdf)->init(protonParams.c_str());
  if (m_useAntiParticleHypo) {
    (m_pdf[Const::antielectron.getIndex()] = new ECL::ECLElectronPdf)->init(eAntiParams.c_str());
    (m_pdf[Const::antimuon.getIndex()]     = new ECL::ECLMuonPdf)->init(muAntiParams.c_str());
    (m_pdf[Const::antipion.getIndex()]     = new ECL::ECLPionPdf)->init(piAntiParams.c_str());
    (m_pdf[Const::antikaon.getIndex()]     = new ECL::ECLKaonPdf)->init(kaonAntiParams.c_str());
    (m_pdf[Const::antiproton.getIndex()]   = new ECL::ECLProtonPdf)->init(protonAntiParams.c_str());
  } else {
    (m_pdf[Const::antielectron.getIndex()] = new ECL::ECLElectronPdf)->init(eParams.c_str());
    (m_pdf[Const::antimuon.getIndex()]     = new ECL::ECLMuonPdf)->init(muParams.c_str());
    (m_pdf[Const::antipion.getIndex()]     = new ECL::ECLPionPdf)->init(piParams.c_str());
    (m_pdf[Const::antikaon.getIndex()]     = new ECL::ECLKaonPdf)->init(kaonParams.c_str());
    (m_pdf[Const::antiproton.getIndex()]   = new ECL::ECLProtonPdf)->init(protonParams.c_str());
  }
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

    float likelihoods[Const::ChargedStable::c_SetSize];
    double eop = energy / p;

    for (const auto& hypo : Const::chargedStableSet) {
      ECL::ECLAbsPdf* currentpdf = m_pdf[hypo.getIndex()];
      if (currentpdf == 0) {
        currentpdf = m_pdf[Const::pion.getIndex()]; // use pion pdf when specialized pdf is not assigned.
      }
      double pdfval = currentpdf->pdf(eop, p, theta);

      if (hypo.getIndex() == Const::pion.getIndex()) {
        ECL::ECLPionPdf* pipdf = dynamic_cast<ECL::ECLPionPdf*>(currentpdf);
        pipdf = pipdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is PION (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::antipion.getIndex()) {
        ECL::ECLPionPdf* antipipdf = dynamic_cast<ECL::ECLPionPdf*>(currentpdf);
        antipipdf = antipipdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ANTI-PION (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::kaon.getIndex()) {
        ECL::ECLKaonPdf* kaonpdf = dynamic_cast<ECL::ECLKaonPdf*>(currentpdf);
        kaonpdf = kaonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is KAON (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::antikaon.getIndex()) {
        ECL::ECLKaonPdf* antikaonpdf = dynamic_cast<ECL::ECLKaonPdf*>(currentpdf);
        antikaonpdf = antikaonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ANTI-KAON (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::proton.getIndex()) {
        ECL::ECLProtonPdf* protonpdf = dynamic_cast<ECL::ECLProtonPdf*>(currentpdf);
        protonpdf = protonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is PROTON (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::antiproton.getIndex()) {
        ECL::ECLProtonPdf* antiprotonpdf = dynamic_cast<ECL::ECLProtonPdf*>(currentpdf);
        antiprotonpdf = antiprotonpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ANTI-PROTON (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::muon.getIndex()) {
        ECL::ECLMuonPdf* mupdf = dynamic_cast<ECL::ECLMuonPdf*>(currentpdf);
        mupdf = mupdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is MUON (" << hypo.getIndex() << ")");
        B2DEBUG(20, "p = " << p / mupdf->getEnergyUnit() << ", theta = " << theta / mupdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << mupdf->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << mupdf->pdfParams(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << mupdf->pdfParams(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << mupdf->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << mupdf->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "fraction = " << mupdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (hypo.getIndex() == Const::antimuon.getIndex()) {
        ECL::ECLMuonPdf* antimu = dynamic_cast<ECL::ECLMuonPdf*>(currentpdf);
        antimu = antimu; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ANTI-MUON (" << hypo.getIndex() << ")");
        B2DEBUG(20, "p = " << p / antimu->getEnergyUnit() << ", theta = " << theta / antimu->getAngularUnit());
        B2DEBUG(20, "mu1 = " << antimu->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << antimu->pdfParams(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << antimu->pdfParams(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << antimu->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << antimu->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "fraction = " << antimu->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (hypo.getIndex() == Const::electron.getIndex()) {
        ECL::ECLElectronPdf* elpdf = dynamic_cast<ECL::ECLElectronPdf*>(currentpdf);
        elpdf = elpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ELECTRON (" << hypo.getIndex() << ")");
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

      if (hypo.getIndex() == Const::antielectron.getIndex()) {
        ECL::ECLElectronPdf* antielpdf = dynamic_cast<ECL::ECLElectronPdf*>(currentpdf);
        antielpdf = antielpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ANTI-ELECTRON (" << hypo.getIndex() << ")");
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

      if (isnormal(pdfval) && pdfval > 0) likelihoods[hypo.getIndex()] = log(pdfval);
      else likelihoods[hypo.getIndex()] = m_minLogLike;
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
  delete m_pdf[Const::electron.getIndex()];
  delete m_pdf[Const::muon.getIndex()];
  delete m_pdf[Const::pion.getIndex()];
  delete m_pdf[Const::kaon.getIndex()];
  delete m_pdf[Const::proton.getIndex()];
  delete m_pdf[Const::antielectron.getIndex()];
  delete m_pdf[Const::antimuon.getIndex()];
  delete m_pdf[Const::antipion.getIndex()];
  delete m_pdf[Const::antikaon.getIndex()];
  delete m_pdf[Const::antiproton.getIndex()];
}
