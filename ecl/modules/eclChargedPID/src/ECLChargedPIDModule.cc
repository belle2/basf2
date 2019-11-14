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

#include <ecl/dataobjects/ECLShower.h>

using namespace Belle2;

REG_MODULE(ECLChargedPID)

ECLChargedPIDModule::ECLChargedPIDModule() : Module()
{
  setDescription("ECL charged particle PID module. Likelihood values for each signed particle hypothesis (sign chosen will depend on the reco track charge) are stored in an ECLPidLikelihood object.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("applyClusterTimingSel", m_applyClusterTimingSel,
           "Set true if you want to apply a abs(clusterTiming)/clusterTimingError<1 cut on clusters. This cut is optimised to achieve 99% timing efficiency for true photons from the IP.",
           bool(false));
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

    // Load the pion fit hypothesis or the hypothesis which is the closest in mass to a pion
    // (the tracking will not always successfully fit with a pion hypothesis).
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;

    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    const double p     = fitRes->getMomentum().Mag();
    const double theta = fitRes->getMomentum().Theta();

    double maxEnergy(0), e9e21(0);
    double lat(0), dist(0), trkdepth(0), shdepth(0);
    double nCrystals = 0;
    int nClusters = relShowers.size();

    for (const auto& eclShower : relShowers) {

      if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) continue;
      if (m_applyClusterTimingSel) {
        if (abs(eclShower.getTime()) > eclShower.getDeltaTime99()) continue;
      }

      const double shEnergy = eclShower.getEnergy();
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

    double eop = maxEnergy / p;
    const auto charge = fitRes->getChargeSign();

    B2DEBUG(20, "P = " << p << " [GeV]");
    B2DEBUG(20, "Theta = " << theta << " [rad]");
    B2DEBUG(20, "E/P = " << eop);
    B2DEBUG(20, "charge = " << charge);

    // Store the right PDF depending on the charge of the particle's track.
    double pdfval = -1;
    for (const auto& hypo : Const::chargedStableSet) {

      auto signedhypo = hypo.getPDGCode() * charge;

      B2DEBUG(20, "\n\thypo[" << hypo_idx << "] = " << hypo.getPDGCode()
              << ", signedhypo[" << hypo_idx << "] = " << signedhypo);

      // For the moment, do not split by charge
      signedhypo = fabs(signedhypo);

      // For tracks w/:
      // -) A matched shower
      // -) Shower is in the tracker acceptance (reg != 0)
      // -) Well defined charge (+/-1),
      // get the pdf value.
      if (mostEnergeticShower && showerReg && std::abs(charge)) {

        // Get the transformed input variables.
        // If no transformation parameters are stored in the payload, this will be just a copy of the input variable list.
        auto variables_transfo = transfoGaussDecorr(signedhypo, p, showerTheta);

        // Get the PDF templates for the various observables, and multiply the PDF values for this candidate.
        // This assumes observables aren't independent, or at least linear correlations have been removed by suitably transforming the inputs....
        double prod(1.0);
        double ipdfval;
        for (unsigned int idx(0); idx < variables_transfo.size(); idx++) {

          auto var   = variables_transfo.at(idx);
          auto varid = m_pdfs->getVars(signedhypo, p, showerTheta)->at(idx);

          const TF1* pdf = m_pdfs->getPdf(signedhypo, p, showerTheta, varid);
          if (pdf) {

            ipdfval = getPdfVal(var, pdf);

            B2DEBUG(30, "\t\tL(" << hypo.getPDGCode() << ") = " << prod
                    << " * pdf(varid: " << static_cast<unsigned int>(varid) << ") = "
                    << prod << " * " << ipdfval
                    << " = " << prod * ipdfval);

            prod *= ipdfval;
          }
        }

        if (prod != 1.0) {
          pdfval = prod;
        }

        B2DEBUG(20, "\tL(" << hypo.getPDGCode() << ") = " << pdfval);

      }

      B2DEBUG(20, "hypo = " << hypo.getPDGCode() << ", signedhypo = " << signedhypo << ", pdf(E/P=" << eop << ") = " << pdfval);

      likelihoods[hypo.getIndex()] = (std::isnormal(pdfval) && pdfval > 0) ? log(pdfval) : m_minLogLike;

    }

    const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(likelihoods,
                                                                maxEnergy,
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

double ECLChargedPIDModule::getPdfVal(const double& x, const TF1* pdf)
{

  double y, xmin, xmax;
  pdf->GetRange(xmin, xmax);

  if (x <= xmin) { y = pdf->Eval(xmin + 1e-9); }
  else if (x >= xmax) { y = pdf->Eval(xmax - 1e-9); }
  else                { y = pdf->Eval(x); }

  return (y) ? y : 1e-9; // Do not return exactly 0, otherwise deltaLogL might be biased...
}

std::vector<double> ECLChargedPIDModule::transfoGaussDecorr(const int pdg, const double& p, const double& theta)
{

  // Retrieve the list of enum ids for the input variables from the payload.
  auto varids = m_pdfs->getVars(pdg, p, theta);

  // Fill the vector w/ the values taken from the module map.
  std::vector<double> vtransfo_gauss;
  for (const auto& varid : *varids) {
    vtransfo_gauss.push_back(m_variables.at(varid));
  }

  unsigned int nvars = varids->size();

  // Get the variable transformation settings for this (hypo, p, theta).
  auto vts = m_pdfs->getVTS(pdg, p, theta);

  // Transform the input variables only if necessary.
  if (!vts->doTransfo) {
    return vtransfo_gauss; // Just a copy of the original vars at this stage!
  }

  B2DEBUG(30, "");
  B2DEBUG(30, "\tclass path: " << vts->classPath);
  B2DEBUG(30, "\tgbin = " << vts->gbin << ", (theta,p) = (" << vts->jth << "," << vts->ip << ")");
  B2DEBUG(30, "\tnvars: " << nvars);

  for (unsigned int ivar(0); ivar < nvars; ivar++) {
    unsigned int ndivs = vts->nDivisions[ivar];
    B2DEBUG(30, "\tvarid: " << static_cast<unsigned int>(varids->at(ivar)) << " = " << vtransfo_gauss.at(
              ivar) << ", nsteps = " << ndivs);
    for (unsigned int jdiv(0); jdiv < ndivs; jdiv++) {
      auto ij = linIndex(ivar, jdiv, vts->nDivisionsMax);
      B2DEBUG(30, "\t\tx[" << ivar << "][" << jdiv << "] = x[" << ij << "] = " << vts->x[ij]);
      B2DEBUG(30, "\t\tcumulDist[" << ivar << "][" << jdiv << "] = cumulDist[" << ij << "] = " << vts->cumulDist[ij]);
    }
  }

  double cumulant;
  for (unsigned int ivar(0); ivar < nvars; ivar++) {

    int ndivs = vts->nDivisions[ivar];

    int jdiv = 0;
    auto ij = linIndex(ivar, jdiv, vts->nDivisionsMax);
    while (vtransfo_gauss.at(ivar) > vts->x[ij]) {
      jdiv++;
      ij = linIndex(ivar, jdiv, vts->nDivisionsMax);
    }

    if (jdiv < 0) { jdiv = 0; }
    if (jdiv >= ndivs) { jdiv = ndivs - 1; }
    int jnextdiv = jdiv;
    if ((vtransfo_gauss.at(ivar) > vts->x[ij] && jdiv != ndivs - 1) || jdiv == 0) {
      jnextdiv++;
    } else {
      jnextdiv--;
    }
    auto ijnext = linIndex(ivar, jnextdiv, vts->nDivisionsMax);

    double dx = vts->x[ij] - vts->x[ijnext];
    double dy = vts->cumulDist[ij] - vts->cumulDist[ijnext];
    cumulant  = vts->cumulDist[ij] + (vtransfo_gauss.at(ivar) - vts->x[ijnext]) * dy / dx;

    cumulant = std::min(cumulant, 1.0 - 10e-10);
    cumulant = std::max(cumulant, 10e-10);

    double maxErfInvArgRange = 0.99999999;
    double arg = 2.0 * cumulant - 1.0;

    arg = std::min(maxErfInvArgRange, arg);
    arg = std::max(-maxErfInvArgRange, arg);

    vtransfo_gauss.at(ivar) = c_sqrt2 * TMath::ErfInverse(arg);

  }

  B2DEBUG(30, "\tSHOWER properties (Gaussian-transformed):");
  for (unsigned int idx(0); idx < nvars; idx++) {
    B2DEBUG(30, "\tvarid: " << static_cast<unsigned int>(varids->at(idx)) << " = " << vtransfo_gauss.at(idx));
  }
  B2DEBUG(30,  "\t-------------------------------");

  std::vector<double> vtransfo_decorr;
  vtransfo_decorr.reserve(nvars);

  for (unsigned int i(0); i < nvars; i++) {
    double vartransfo(0);
    for (unsigned int j(0); j < nvars; j++) {
      auto ij = linIndex(i, j, nvars);
      vartransfo += vtransfo_gauss[j] * vts->covMatrix[ij];
    }
    vtransfo_decorr.push_back(vartransfo);
  }

  B2DEBUG(30, "\tSHOWER properties (Decorrelation-transformed):");
  for (unsigned int idx(0); idx < nvars; idx++) {
    B2DEBUG(30, "\tvarid: " << static_cast<unsigned int>(varids->at(idx)) << " = " << vtransfo_decorr.at(idx));
  }
  B2DEBUG(30,  "\t-------------------------------");

  return vtransfo_decorr;

}

void ECLChargedPIDModule::endRun()
{
}

void ECLChargedPIDModule::terminate()
{
}
