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

#include <ecl/modules/eclChargedPID/ECLChargedPIDModule.h>

using namespace Belle2;

REG_MODULE(ECLChargedPID)

ECLChargedPIDModule::ECLChargedPIDModule() : Module()
{
  setDescription("ECL charged particle PID. Likelihood values for each particle hypothesis (dependent on the reco track charge) are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLChargedPIDModule::~ECLChargedPIDModule() {}

void ECLChargedPIDModule::checkDB()
{
  if (!m_pdfs) { B2FATAL("No ECL charged PID PDFs found in database!"); }
}

void ECLChargedPIDModule::initialize()
{
  m_eclPidLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_eclPidLikelihoods);

  m_pdfs.addCallback([this]() { checkDB(); });
  checkDB();
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
    const auto charge = fitRes->getChargeSign();

    // Store the right PDF depending on charge of the particle's track.
    for (const auto& hypo : Const::chargedStableSet) {

      auto chargedpdg = hypo.getPDGCode() * charge;

      const TF1* currentpdf = m_pdfs->getPdf(chargedpdg, theta, p);
      double pdfval = currentpdf->Eval(eop);

      likelihoods[hypo.getIndex()] = (std::isnormal(pdfval) && pdfval > 0) ? log(pdfval) : m_minLogLike;

    }

    const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(likelihoods,
                                                                energy,
                                                                eop,
                                                                e9e21,
                                                                lat,
                                                                dist,
                                                                trkdepth,
                                                                shdepth,
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
}
