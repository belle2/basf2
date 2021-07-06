/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <math.h>
#include <algorithm>
#include "TMath.h"

#include <ecl/modules/eclChargedPID/ECLChargedPIDModule.h>

using namespace Belle2;

REG_MODULE(ECLChargedPID)


ECLChargedPIDModule::ECLChargedPIDModule() : Module()
{
  setDescription("The module implements charged particle identification using ECL-related observables. For each Track matched with a suitable ECLShower, likelihoods for each particle hypothesis are obtained from pdfs stored in a conditions database payload, and then get stored in an ECLPidLikelihood object. The dimensionality of the likelihood depends on how many variables are stored in the payload. The baseline method could be a simple univariate likelihood based on E/p PDFs, but it could be extended to include more ECL quantitites (e.g. shower shape variables, w/ proper decorrelation).");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("applyClusterTimingSel",
           m_applyClusterTimingSel,
           "Set true if you want to apply a abs(clusterTiming)/clusterTimingError<1 cut on clusters. This cut is optimised to achieve 99% timing efficiency for true photons from the IP.",
           bool(false));
}


ECLChargedPIDModule::~ECLChargedPIDModule() {}


void ECLChargedPIDModule::initialize()
{
  m_eventMetaData.isRequired();

  m_eclPidLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_eclPidLikelihoods);
}


void ECLChargedPIDModule::checkPdfsDB()
{
  if (!m_pdfs) { B2FATAL("No ECL charged PID PDFs payload found in database!"); }
}


void ECLChargedPIDModule::beginRun()
{
  m_pdfs.addCallback([this]() { checkPdfsDB(); });
  checkPdfsDB();
}


void ECLChargedPIDModule::event()
{

  for (const auto& track : m_tracks) {

    // Don't forget to clear variable map before a track gets processed!
    m_variables.clear();

    // Load the pion fit hypothesis or the hypothesis which is the closest in mass to a pion
    // (the tracking will not always successfully fit with a pion hypothesis).
    const TrackFitResult* fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
    if (fitRes == nullptr) continue;
    const auto relShowers = track.getRelationsTo<ECLShower>();
    if (relShowers.size() == 0) continue;

    const double p     = fitRes->getMomentum().Mag();
    const double theta = fitRes->getMomentum().Theta();
    const auto charge  = fitRes->getChargeSign();

    double shEnergy(0.0), maxEnergy(0.0);

    const ECLShower* mostEnergeticShower = nullptr;

    for (const auto& eclShower : relShowers) {

      if (eclShower.getHypothesisId() != ECLShower::c_nPhotons) continue;
      if (m_applyClusterTimingSel) {
        if (abs(eclShower.getTime()) > eclShower.getDeltaTime99()) continue;
      }

      shEnergy = eclShower.getEnergy();
      if (shEnergy > maxEnergy) {
        maxEnergy = shEnergy;
        mostEnergeticShower = &eclShower;
      }

    }

    double showerTheta = (mostEnergeticShower) ? mostEnergeticShower->getTheta() : -999.0;
    int showerReg = (mostEnergeticShower) ? mostEnergeticShower->getDetectorRegion() : -1;

    // These are the variables that can be used to extract PDF templates for the likelihood / for the MVA training.
    m_variables[ECLChargedPidPDFs::InputVar::c_E1E9] = (mostEnergeticShower) ? mostEnergeticShower->getE1oE9() : -1.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_E9E21] = (mostEnergeticShower) ? mostEnergeticShower->getE9oE21() : -1.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_E] = (mostEnergeticShower) ? maxEnergy : -1.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_EoP] = (mostEnergeticShower) ? maxEnergy / p : -1.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_Z40] = (mostEnergeticShower) ? mostEnergeticShower->getAbsZernike40() : -999.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_Z51] = (mostEnergeticShower) ? mostEnergeticShower->getAbsZernike51() : -999.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_ZMVA] = (mostEnergeticShower) ? mostEnergeticShower->getZernikeMVA() : -999.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_PSDMVA] = (mostEnergeticShower) ? mostEnergeticShower->getPulseShapeDiscriminationMVA() :
                                                         -999.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_DeltaL] = (mostEnergeticShower) ? mostEnergeticShower->getTrkDepth() : -1.0;
    m_variables[ECLChargedPidPDFs::InputVar::c_LAT] = (mostEnergeticShower) ? mostEnergeticShower->getLateralEnergy() : -999.0;

    B2DEBUG(20, "EVENT: " << m_eventMetaData->getEvent());
    B2DEBUG(20,  "-------------------------------");
    B2DEBUG(20, "TRACK properties:");
    B2DEBUG(20, "p = " << p << " [GeV/c]");
    B2DEBUG(30, "theta = " << theta << " [rad]");
    B2DEBUG(20, "charge = " << charge);
    B2DEBUG(20,  "-------------------------------");
    B2DEBUG(20, "SHOWER properties:");
    B2DEBUG(20, "showerTheta = " << showerTheta << " [rad], showerReg = " << showerReg);
    for (const auto& [varid, var] : m_variables) {
      B2DEBUG(30, "varid: " << static_cast<unsigned int>(varid) << " = " << var);
    }
    B2DEBUG(20,  "-------------------------------");

    // For tracks w/:
    // -) A matched shower
    // -) Shower is in the tracker acceptance (reg != 0)
    // -) Well defined charge (+/-1),
    // get the pdf value for each charged particle hypothesis,
    // and store a ECLPidLikelihood data object.
    if (mostEnergeticShower && showerReg && std::abs(charge)) {

      float likelihoods[Const::ChargedStable::c_SetSize];
      // Initialise PDF value.
      // This will correspond to a LogL = 0.0 if unchanged.
      double pdfval(0.0);

      // Order of loop is defined in UnitConst.cc: e, mu, pi, K, p, d
      for (const auto& hypo : Const::chargedStableSet) {

        unsigned int hypo_idx = hypo.getIndex();

        auto pdgId = hypo.getPDGCode();

        B2DEBUG(20, "\n\thypo[" << hypo_idx << "] = " << pdgId << ", hypo[" << hypo_idx << "] * reco_charge = " << pdgId * charge);

        // Retrieve the list of enum ids for the input variables from the payload.
        auto varids = m_pdfs->getVars(pdgId, charge, p, showerTheta);

        // Fill the vector w/ the values taken from the module map.
        std::vector<double> variables;
        for (const auto& varid : *varids) {
          variables.push_back(m_variables.at(varid));
        }

        // Transform the input variables via gaussianisation+decorrelation only if necessary.
        if (m_pdfs->doVarsTransfo()) {
          transfoGaussDecorr(pdgId, charge, p, showerTheta, variables);
        }

        // Get the PDF templates for the various observables, and multiply the PDF values for this candidate.
        // If more than 1 observable is used, this assumes they are independent
        // (or at least linear correlations have been removed by suitably transforming the inputs...).
        double prod(1.0);
        double ipdfval;
        for (unsigned int idx(0); idx < variables.size(); idx++) {

          auto var   = variables.at(idx);
          auto varid = varids->at(idx);

          const TF1* pdf = m_pdfs->getPdf(pdgId, charge, p, showerTheta, varid);
          if (pdf) {

            ipdfval = getPdfVal(var, pdf);

            B2DEBUG(30, "\t\tL(" << pdgId * charge << ") = " << prod
                    << " * pdf(varid: " << static_cast<unsigned int>(varid) << ") = "
                    << prod << " * " << ipdfval
                    << " = " << prod * ipdfval);

            prod *= ipdfval;
          }
        }

        if (prod != 1.0) {
          pdfval = prod;
        }

        B2DEBUG(20, "\tL(" << pdgId * charge << ") = " << pdfval);

        likelihoods[hypo_idx] = (std::isnormal(pdfval) && pdfval > 0) ? log(pdfval) : c_dummyLogL;

        B2DEBUG(20, "\tlog(L(" << pdgId * charge << ")) = " << likelihoods[hypo_idx]);

      } // Loop over hypotheses

      const auto eclPidLikelihood = m_eclPidLikelihoods.appendNew(likelihoods);

      track.addRelationTo(eclPidLikelihood);

    } // Check on good tracks.

  } // End loop on tracks
}

double ECLChargedPIDModule::getPdfVal(const double& x, const TF1* pdf)
{

  double y, xmin, xmax;
  pdf->GetRange(xmin, xmax);

  if (x <= xmin) { y = pdf->Eval(xmin + 1e-9); }
  else if (x >= xmax) { y = pdf->Eval(xmax - 1e-9); }
  else                { y = pdf->Eval(x); }

  return (y) ? y : 1e-9; // Shall we allow for 0? That translates in LogL = NaN...
}

void ECLChargedPIDModule::transfoGaussDecorr(const unsigned int pdg, const int charge, const double& p, const double& theta,
                                             std::vector<double>& variables)
{

  unsigned int nvars = variables.size();

  // Get the variable transformation settings for this hypo pdg, p, theta.
  auto vts = m_pdfs->getVTS(pdg, charge, p, theta);

  B2DEBUG(30, "");
  B2DEBUG(30, "\tclass path: " << vts->classPath);
  B2DEBUG(30, "\tgbin = " << vts->gbin << ", (theta,p) = (" << vts->jth << "," << vts->ip << ")");
  B2DEBUG(30, "\tnvars: " << nvars);

  auto varids = m_pdfs->getVars(pdg, charge, p, theta);

  for (unsigned int ivar(0); ivar < nvars; ivar++) {
    unsigned int ndivs = vts->nDivisions[ivar];
    B2DEBUG(30, "\tvarid: " << static_cast<unsigned int>(varids->at(ivar)) << " = " << variables.at(ivar) << ", nsteps = " << ndivs);
    for (unsigned int jdiv(0); jdiv < ndivs; jdiv++) {
      auto ij = linIndex(ivar, jdiv, vts->nDivisionsMax);
      B2DEBUG(30, "\t\tx[" << ivar << "][" << jdiv << "] = x[" << ij << "] = " << vts->x[ij]);
      B2DEBUG(30, "\t\tcumulDist[" << ivar << "][" << jdiv << "] = cumulDist[" << ij << "] = " << vts->cumulDist[ij]);
    }
  }

  std::vector<double> vtransfo_gauss;
  vtransfo_gauss.reserve(nvars);

  for (unsigned int ivar(0); ivar < nvars; ivar++) {

    int ndivs = vts->nDivisions[ivar];

    int jdiv = 0;
    auto ij = linIndex(ivar, jdiv, vts->nDivisionsMax);
    while (variables.at(ivar) > vts->x[ij]) {
      jdiv++;
      ij = linIndex(ivar, jdiv, vts->nDivisionsMax);
    }

    if (jdiv < 0) { jdiv = 0; }
    if (jdiv >= ndivs) { jdiv = ndivs - 1; }
    int jnextdiv = jdiv;
    if ((variables.at(ivar) > vts->x[ij] && jdiv != ndivs - 1) || jdiv == 0) {
      jnextdiv++;
    } else {
      jnextdiv--;
    }
    auto ijnext = linIndex(ivar, jnextdiv, vts->nDivisionsMax);

    double dx = vts->x[ij] - vts->x[ijnext];
    double dy = vts->cumulDist[ij] - vts->cumulDist[ijnext];
    double cumulant  = vts->cumulDist[ij] + (variables.at(ivar) - vts->x[ijnext]) * dy / dx;

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

  // Now modify the input variables vector.
  for (unsigned int i(0); i < nvars; i++) {
    variables[i] = vtransfo_decorr.at(i);
  }

}

void ECLChargedPIDModule::endRun()
{
}

void ECLChargedPIDModule::terminate()
{
}
