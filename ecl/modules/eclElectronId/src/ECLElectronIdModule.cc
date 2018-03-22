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

  // const string eParams = FileSystem::findFile("/data/ecl/electrons.dat");
  // const string muParams = FileSystem::findFile("/data/ecl/muons.dat");
  // const string piParams = FileSystem::findFile("/data/ecl/pions.dat");
  const string eParams  = FileSystem::findFile("/data/ecl/electrons_N1.dat");
  const string muParams = FileSystem::findFile("/data/ecl/muons_N1.dat");
  const string piParams = FileSystem::findFile("/data/ecl/pions_N1.dat");

  if (eParams.empty() || muParams.empty() || piParams.empty()) {
    B2FATAL("Electron ID pdfs parameter files not found.");
  }

  (m_pdf[Const::electron.getIndex()] = new ECL::ECLElectronPdf)->init(eParams.c_str());
  (m_pdf[Const::muon.getIndex()]     = new ECL::ECLMuonPdf)->init(muParams.c_str());
  (m_pdf[Const::proton.getIndex()]   =
     m_pdf[Const::kaon.getIndex()]     =
       m_pdf[Const::pion.getIndex()]     = new ECL::ECLPionPdf)->init(piParams.c_str());

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
      Belle2::ECL::ECLAbsPdf* currentpdf = m_pdf[hypo.getIndex()];
      if (currentpdf == 0) {
        currentpdf = m_pdf[Const::pion.getIndex()]; // use pion pdf when specialized pdf is not assigned.
      }
      double pdfval = currentpdf->pdf(eop, p, theta);

      // DEBUG
      if (hypo.getIndex() == Const::electron.getIndex()) {
        std::cout << "" << std::endl;
        std::cout << "Current hypothesis is ELECTRON (" << hypo.getIndex() << ")" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "p = " << p << ", theta = " << theta << std::endl;
        std::cout << "" << std::endl;
        std::cout << "mu1 = " << currentpdf->pdfParams(p, theta).mu1;
        std::cout << "sigma1 = " << currentpdf->pdfParams(p, theta).sigma1;
        std::cout << "mu2 = " << currentpdf->pdfParams(p, theta).mu2;
        std::cout << "sigma2 = " << currentpdf->pdfParams(p, theta).sigma2;
        std::cout << "alpha = " << currentpdf->pdfParams(p, theta).alpha;
        std::cout << "nn = " << currentpdf->pdfParams(p, theta).nn;
        std::cout << "fraction = " << currentpdf->pdfParams(p, theta).fraction;
        std::cout << "" << std::endl;
        std::cout << "pdfval: " << pdfval << std::endl;
        std::cout << "" << std::endl;
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
