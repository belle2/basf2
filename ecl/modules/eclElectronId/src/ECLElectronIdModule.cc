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


#include <ecl/modules/eclElectronId/ECLElectronIdModule.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/electronId/ECLMuonPdf.h>
#include <ecl/electronId/ECLElectronPdf.h>
#include <ecl/electronId/ECLPionPdf.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLElectronId)

ECLElectronIdModule::ECLElectronIdModule() : Module()
{
  setDescription("E/p based Electron ID. Likelihood values for each particle hypothesis are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
  for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) m_pdf[i] = 0;
}

ECLElectronIdModule::~ECLElectronIdModule() {}

void ECLElectronIdModule::initialize()
{
  StoreArray<Track> tracks;
  StoreArray<ECLPidLikelihood> eclPidLikelihoods;
  eclPidLikelihoods.registerInDataStore();
  tracks.registerRelationTo(eclPidLikelihoods);

  const string eParams  = FileSystem::findFile("/data/ecl/electrons_N1.dat");
  const string muParams = FileSystem::findFile("/data/ecl/muons_N1.dat");
  const string piParams = FileSystem::findFile("/data/ecl/pions_N1.dat");
  const string piAntiParams = FileSystem::findFile("/data/ecl/pionsanti_N1.dat");

  if (eParams.empty() || muParams.empty() || piParams.empty()) {
    B2FATAL("Electron ID pdfs parameter files not found.");
  }

  ECL::ECLAbsPdf::setEnergyUnit("MeV"); // The energy unit in the .dat files
  ECL::ECLAbsPdf::setAngularUnit("deg"); // The angular unit in the .dat files

  (m_pdf[Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init(eParams.c_str());
  (m_pdf[Const::muon.getIndex()]     = new ECL::ECLMuonPdf)->init(muParams.c_str());
  (m_pdf[Const::proton.getIndex()]   = m_pdf[Const::kaon.getIndex()] = m_pdf[Const::pion.getIndex()] = new ECL::ECLPionPdf)->init(
    piParams.c_str());

  // Shall we add anti-charged particle defs in framework/gearbox/Const.h?
  //(m_pdf[Const::antipion.getIndex()] = new ECL::ECLPionPdf)->init(piAntiParams.c_str());

}

void ECLElectronIdModule::beginRun() {}

void ECLElectronIdModule::event()
{

  StoreArray<Track> tracks;
  StoreArray<ECLPidLikelihood> eclPidLikelihoods;

  for (const auto& track : tracks) {

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
        B2DEBUG(20, "p = " << p * pipdf->getEnergyUnit() << ", theta = " << theta * pipdf->getAngularUnit());
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

      if (hypo.getIndex() == Const::muon.getIndex()) {
        ECL::ECLMuonPdf* mupdf = dynamic_cast<ECL::ECLMuonPdf*>(currentpdf);
        mupdf = mupdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is MUON (" << hypo.getIndex() << ")");
        B2DEBUG(20, "p = " << p * mupdf->getEnergyUnit() << ", theta = " << theta * mupdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << mupdf->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1l = " << mupdf->pdfParams(p, theta)->sigma1l);
        B2DEBUG(20, "sigma1r = " << mupdf->pdfParams(p, theta)->sigma1r);
        B2DEBUG(20, "mu2 = " << mupdf->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << mupdf->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "fraction = " << mupdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (hypo.getIndex() == Const::electron.getIndex()) {
        ECL::ECLElectronPdf* elpdf = dynamic_cast<ECL::ECLElectronPdf*>(currentpdf);
        elpdf = elpdf; // get rid of warning
        B2DEBUG(20, "Current hypothesis is ELECTRON (" << hypo.getIndex() << ")");
        B2DEBUG(20, "p = " << p * elpdf->getEnergyUnit() << ", theta = " << theta * elpdf->getAngularUnit());
        B2DEBUG(20, "mu1 = " << elpdf->pdfParams(p, theta)->mu1);
        B2DEBUG(20, "sigma1 = " << elpdf->pdfParams(p, theta)->sigma1);
        B2DEBUG(20, "mu2 = " << elpdf->pdfParams(p, theta)->mu2);
        B2DEBUG(20, "sigma2 = " << elpdf->pdfParams(p, theta)->sigma2);
        B2DEBUG(20, "alpha = " << elpdf->pdfParams(p, theta)->alpha);
        B2DEBUG(20, "nn = " << elpdf->pdfParams(p, theta)->nn);
        B2DEBUG(20, "fraction = " << elpdf->pdfParams(p, theta)->fraction);
        B2DEBUG(20, "pdfval: " << pdfval);
      }

      if (isnormal(pdfval) && pdfval > 0) likelihoods[hypo.getIndex()] = log(pdfval);
      else likelihoods[hypo.getIndex()] = m_minLogLike;
    } // end loop on hypo

    const auto eclPidLikelihood = eclPidLikelihoods.appendNew(likelihoods, energy, eop, e9e21, lat, dist, trkdepth, shdepth,
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
  if (m_pdf[Const::proton.getIndex()] != m_pdf[Const::pion.getIndex()]) delete m_pdf[Const::proton.getIndex()];
  if (m_pdf[Const::kaon.getIndex()] != m_pdf[Const::pion.getIndex()]) delete m_pdf[Const::kaon.getIndex()];
}
