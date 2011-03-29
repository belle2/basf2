/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Peter Kvasnicka                           *
 *         Based on Geant4 code by author(s) listed below.      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
// -------------------------------------------------------------------
//
// This product includes software developed by Members of the Geant4
// Collaboration ( http://cern.ch/geant4 ).
//
// GEANT4 Classes utilized:
//
// File names:    G4UniversalFluctuation (by Vladimir Ivanchenko)
//                G4MollerBhabhaModel (by Vladimir Ivanchenko)
//                G4MuBetheBlochModel (by Vladimir Ivanchenko)
//                G4BetheBlochModel (by Vladimir Ivanchenko)
//
// -------------------------------------------------------------------

#include <pxd/modules/pxdDigitizer/SiEnergyFluct.h>
#include <pxd/modules/pxdDigitizer/PhysicalConstants.h>

#include <TParticlePDG.h>
#include <TDatabasePDG.h>

#include <algorithm>
#include <cmath>


using namespace Belle2;

// ************************************************
// Note on units - basf2 implementation
// ------------------------------------------------
// The philosophy is:
// 1. Don't touch the units in the code. Use the system of Geant4
//    / PhysicalConstants.h as set.
// 2. Convert input values from MCParticle to Geant4 system.
// 3. Convert results to basf2 units on output.
// ************************************************
//
// The constructor sets up the set of constants for
// dE/dx calculations & universal fluctuations (for
// Si only).
// PQ: correct the initialization list of the constructor.
SiEnergyFluct::SiEnergyFluct(double cutOnDeltaRays) :
    m_random(0),
    m_cutOnDeltaRays(cutOnDeltaRays)
{
  // Variables used to reduce recalculation of mean dE/dx
  m_prevMCPart   = 0;
  m_charge       = 0.;
  m_prevMeanLoss = 0.;

  // Constants related to dE/dx
  m_twoln10  = 2.0 * log(10.0);

  // Constants related to Si material & dE/dx
  m_eexc     = 173 * SiDigi::eV;          // material->GetIonisation()->GetMeanExcitationEnergy();
  m_eexc2    = m_eexc * m_eexc;
  m_Zeff     = 14;                // material->GetElectronDensity()/material->GetTotNbOfAtomsPerVolume();
  m_th       = 0.935414 * SiDigi::keV;    // 0.25*sqrt(Zeff)

  // Constants related to Si material & dEdx -> density effect
  m_aden     = 0.160077;          // material->GetIonisation()->GetAdensity();
  m_cden     = 4.43505;           // material->GetIonisation()->GetCdensity();
  m_mden     = 3;                 // material->GetIonisation()->GetMdensity();
  m_x0den    = 0.2;               // material->GetIonisation()->GetX0density();
  m_x1den    = 3;                 // material->GetIonisation()->GetX1density();
  m_KBethe   = 0.178328 * SiDigi::MeV / SiDigi::cm; // material->GetElectronDensity()*twopi_mc2_rcl2;

  // Constants related to dEdx in Si material & muons
  m_xgi[0] = 0.0199; m_xgi[1] = 0.1017; m_xgi[2] = 0.2372; m_xgi[3] = 0.4083; m_xgi[4] = 0.5917; m_xgi[5] = 0.7628; m_xgi[6] = 0.8983; m_xgi[7] = 0.9801;
  m_wgi[0] = 0.0506; m_wgi[1] = 0.1112; m_wgi[2] = 0.1569; m_wgi[3] = 0.1813; m_wgi[4] = 0.1813; m_wgi[5] = 0.1569; m_wgi[6] = 0.1112; m_wgi[7] = 0.0506;

  m_limitKinEnergy    = 100.* SiDigi::keV;
  m_logLimitKinEnergy = log(m_limitKinEnergy);
  m_alphaPrime        = SiDigi::fine_str_const / 2. / SiDigi::pi;

  // Constants related to universal fluctuations
  m_minLoss                   = 10 * SiDigi::eV;
  m_minNumberInteractionsBohr = 10.0;
  m_nmaxCont1                 = 4.;
  m_nmaxCont2                 = 16.;

  m_facwidth     = 1. / SiDigi::keV;
  m_f1Fluct      = 0.857143;      // material->GetIonisation()->GetF1fluct();
  m_f2Fluct      = 0.142857;      // material->GetIonisation()->GetF2fluct();
  m_e1Fluct      = 0.115437 * SiDigi::keV;// material->GetIonisation()->GetEnergy1fluct();
  m_e2Fluct      = 1.96 * SiDigi::keV;    // material->GetIonisation()->GetEnergy2fluct();
  m_e1LogFluct   = log(m_e1Fluct);
  m_e2LogFluct   = log(m_e2Fluct);
  m_ipotFluct    = 173. * SiDigi::eV;     // material->GetIonisation()->GetLogMeanExcEnergy();
  m_ipotLogFluct = log(m_ipotFluct);
  m_e0           = 10 * SiDigi::eV;       // material->GetIonisation()->GetEnergy0fluct();
}

//
// Destructor
//
SiEnergyFluct::~SiEnergyFluct() {}

//
// Main method providing energy loss fluctuations, the model used to get the
// fluctuations is essentially the same as in Glandz in Geant3 (Cern program
// library W5013, phys332). L. Urban et al. NIM A362, p.416 (1995) and Geant4
// Physics Reference Manual
//
double SiEnergyFluct::SampleFluctuations(const MCParticle * part, const double length)
{
  // Calculate particle related quantities
  double mass          = part->getMass() * SiDigi::GeV;
  double momentum2     = part->getMomentum().Mag2() * SiDigi::GeV * SiDigi::GeV;
  double kineticEnergy = sqrt(momentum2 + mass * mass) - mass;
  double ratio         = SiDigi::e_mass / mass;
  double tau           = kineticEnergy / mass;
  double gam           = tau + 1.0;
  double gam2          = gam * gam;
  double bg2           = tau * (tau + 2.0);
  double beta2         = bg2 / (gam2);
  int    pdg           = part->getPDG();

  double maxT          = 2.0 * SiDigi::e_mass * tau * (tau + 2.) / (1. + 2.0 * (tau + 1.) * ratio + ratio * ratio);
  maxT          = std::min(m_cutOnDeltaRays, maxT);

  // Recalculate mean loss - mean dE/dx, if necessary
  double meanLoss     = 0.;

  double chargeSquare  = 0.;

  if (part != m_prevMCPart) {

    // Recalculate charge2 - we have a different particle.
    // Get MCParticle charge in PDG database - it is not stored in the MCParticle object.

    TParticlePDG* pdgPart = TDatabasePDG::Instance()->GetParticle(pdg);
    if (pdgPart == NULL) {
      B2ERROR("SiEnergyFluct: Cannot retrieve PDG information for pdg = "
              << pdg);
    } else
      m_charge = pdgPart->Charge();

    chargeSquare = m_charge * m_charge;

    // Electron or positron
    if (pdg == 11) meanLoss = getElectronDEDX(part);

    // Muon
    else if (pdg == 13) meanLoss = getMuonDEDX(part);

    // Hadron
    else                meanLoss = getHadronDEDX(part);
  }
  // not needed - same particle
  else {
    meanLoss = m_prevMeanLoss;
    chargeSquare = m_charge * m_charge;
  }

  // Save values for next call
  m_prevMCPart   = part;
  m_prevMeanLoss = meanLoss;

  // Get mean loss in MeV
  meanLoss *= length;

  //
  // Start calculation
  double loss  = 0.;
  double sigma = 0.;

  // A trick for very very small loss ( out of model validity)
  if (meanLoss < m_minLoss) {
    return meanLoss;
  }

  //
  // Gaussian regime, only for heavy particles
  if ((mass > SiDigi::e_mass) && (meanLoss >= m_minNumberInteractionsBohr*maxT)) {

    double massrate = SiDigi::e_mass / mass ;
    double tmaxkine = 2.*SiDigi::e_mass * beta2 * gam2 / (1. + massrate * (2.*gam + massrate));

    if (tmaxkine <= 2.*maxT) {

      // Sigma
      sigma = (1.0 / beta2 - 0.5) * m_KBethe * maxT * length * chargeSquare;
      sigma = sqrt(sigma);

      double twomeanLoss = meanLoss + meanLoss;

      if (twomeanLoss < sigma) {

        double x;
        do {

          loss = twomeanLoss * m_random.Uniform();
          x = (loss - meanLoss) / sigma;

        } while (1.0 - 0.5*x*x < m_random.Uniform());

      } else {

        do {

          loss = m_random.Gaus(meanLoss, sigma);

        } while (loss < 0. || loss > twomeanLoss);

      }
      return loss;
    }
  }

  //
  // Glandz regime : initialisation

  // Trick for very small step or low-density material
  if (maxT <= m_e0) return meanLoss;

  double a1 = 0.;
  double a2 = 0.;
  double a3 = 0.;

  // Correction to get better width even using stepmax
  //  if(abs(meanLoss- oldloss) < 1.*SiDigi::eV)
  //    samestep += 1;
  //  else
  //    samestep = 1.;
  //  oldloss = meanLoss;

  double width = 1. + m_facwidth * meanLoss;
  if (width > 4.50) width = 4.50;
  double e1 = width * m_e1Fluct;
  double e2 = width * m_e2Fluct;

  // Cut and material dependent rate
  double rate = 1.0;
  if (maxT > m_ipotFluct) {

    double w2 = log(2.*SiDigi::e_mass * beta2 * gam2) - beta2;

    if (w2 > m_ipotLogFluct && w2 > m_e2LogFluct) {

      rate = 0.03 + 0.23 * log(log(maxT / m_ipotFluct));
      double C = meanLoss * (1. - rate) / (w2 - m_ipotLogFluct);

      a1 = C * m_f1Fluct * (w2 - m_e1LogFluct) / e1;
      a2 = C * m_f2Fluct * (w2 - m_e2LogFluct) / e2;

    }
  }

  double w1 = maxT / m_e0;
  if (maxT > m_e0) a3 = rate * meanLoss * (maxT - m_e0) / (m_e0 * maxT * log(w1));

  // 'Nearly' Gaussian fluctuation if a1>nmaxCont2&&a2>nmaxCont2&&a3>nmaxCont2
  double emean = 0.;
  double sig2e = 0.;
  double sige  = 0.;
  double p1    = 0.;
  double p2    = 0.;
  double p3    = 0.;

  // Excitation of type 1
  if (a1 > m_nmaxCont2) {

    emean += a1 * e1;
    sig2e += a1 * e1 * e1;
  } else if (a1 > 0.) {

    p1 = static_cast<double>(m_random.Poisson(a1));
    loss += p1 * e1;

    if (p1 > 0.) loss += (1. - 2.*m_random.Uniform()) * e1;
  }

  // Excitation of type 2
  if (a2 > m_nmaxCont2) {

    emean += a2 * e2;
    sig2e += a2 * e2 * e2;
  } else if (a2 > 0.) {

    p2 = double(m_random.Poisson(a2));
    loss += p2 * e2;

    if (p2 > 0.) loss += (1. - 2.*m_random.Uniform()) * e2;
  }

  // Ionisation
  double lossc = 0.;

  if (a3 > 0.) {

    p3 = a3;
    double alfa = 1.;

    if (a3 > m_nmaxCont2) {

      alfa          = w1 * (m_nmaxCont2 + a3) / (w1 * m_nmaxCont2 + a3);
      double alfa1  = alfa * log(alfa) / (alfa - 1.);
      double namean = a3 * w1 * (alfa - 1.) / ((w1 - 1.) * alfa);
      emean        += namean * m_e0 * alfa1;
      sig2e        += m_e0 * m_e0 * namean * (alfa - alfa1 * alfa1);
      p3            = a3 - namean;
    }

    double w2 = alfa * m_e0;
    double w  = (maxT - w2) / maxT;

    int    nb = m_random.Poisson(p3);

    if (nb > 0) for (int k = 0; k < nb; k++) lossc += w2 / (1. - w * m_random.Uniform());
  }

  if (emean > 0.) {

    sige   = sqrt(sig2e);
    loss  += std::max(0., m_random.Gaus(emean, sige));
  }

  loss += lossc;

  return loss;
}

//
// Method calculating actual dE/dx for hadrons
//
double SiEnergyFluct::getHadronDEDX(const MCParticle * part)
{
  // Calculate particle related quantities
  double mass          = part->getMass() * SiDigi::GeV;
  double momentum2     = part->getMomentum().Mag2() * SiDigi::GeV * SiDigi::GeV;
  double kineticEnergy = sqrt(momentum2 + mass * mass) - mass;
  double spin          = 0.5;
  double charge2       = m_charge * m_charge;
  double ratio         = SiDigi::e_mass / mass;
  double tau           = kineticEnergy / mass;
  double gam           = tau + 1.0;
  double bg2           = tau * (tau + 2.0);
  double beta2         = bg2 / (gam * gam);

  double maxT          = 2.0 * SiDigi::e_mass * tau * (tau + 2.) / (1. + 2.0 * (tau + 1.) * ratio + ratio * ratio);
  double cutEnergy     = std::min(m_cutOnDeltaRays, maxT);

  // Start with dE/dx
  double dedx = log(2.0 * SiDigi::e_mass * bg2 * cutEnergy / m_eexc2) - (1.0 + cutEnergy / maxT) * beta2;

  // Spin 0.5 particles
  if (0.5 == spin) {

    double del = 0.5 * cutEnergy / (kineticEnergy + mass);
    dedx += del * del;
  }

  // Density correction
  double x = log(bg2) / m_twoln10;

  if (x >= m_x0den) {

    dedx -= m_twoln10 * x - m_cden;
    if (x < m_x1den) dedx -= m_aden * pow((m_x1den - x), m_mden) ;
  }

  // Shell correction --> not used
  //dedx -= 2.0*corr->ShellCorrection(p,material,kineticEnergy);

  // Now compute the total ionization loss
  if (dedx < 0.0) dedx = 0.0 ;

  dedx *= m_KBethe * charge2 / beta2;

  // High order correction only for hadrons --> not used
  //if(!isIon) dedx += corr->HighOrderCorrections(p,material,kineticEnergy);

  return dedx;
}

//
// Method calculating actual dE/dx for muons
//
double SiEnergyFluct::getMuonDEDX(const MCParticle * part)
{
  // Calculate particle related quantities
  double mass          = part->getMass() * SiDigi::GeV;
  double momentum2     = part->getMomentum().Mag2() * SiDigi::GeV * SiDigi::GeV;
  double kineticEnergy = sqrt(momentum2 + mass * mass) - mass;
  double totEnergy     = kineticEnergy + mass;
  double ratio         = SiDigi::e_mass / mass;
  double tau           = kineticEnergy / mass;
  double gam           = tau + 1.0;
  double bg2           = tau * (tau + 2.0);
  double beta2         = bg2 / (gam * gam);

  double maxT          = 2.0 * SiDigi::e_mass * tau * (tau + 2.) / (1. + 2.0 * (tau + 1.) * ratio + ratio * ratio);
  double cutEnergy     = std::min(m_cutOnDeltaRays, maxT);

  // Start with dE/dx
  double dedx = log(2.0 * SiDigi::e_mass * bg2 * cutEnergy / m_eexc2) - (1.0 + cutEnergy / maxT) * beta2;

  double del = 0.5 * cutEnergy / totEnergy;
  dedx += del * del;

  // Density correction
  double x = log(bg2) / m_twoln10;

  if (x >= m_x0den) {

    dedx -= m_twoln10 * x - m_cden ;
    if (x < m_x1den) dedx -= m_aden * pow((m_x1den - x), m_mden) ;
  }

  // Shell correction --> not used
  //dedx -= 2.0*corr->ShellCorrection(p,material,kineticEnergy);


  // Now compute the total ionization loss
  if (dedx < 0.0) dedx = 0.0 ;

  // Use radiative corrections of R. Kokoulin
  if (cutEnergy > m_limitKinEnergy) {

    double logtmax = log(cutEnergy);
    double logstep = logtmax - m_logLimitKinEnergy;
    double dloss   = 0.0;
    double ftot2   = 0.5 / (totEnergy * totEnergy);

    for (int ll = 0; ll < 8; ll++) {
      double ep = exp(m_logLimitKinEnergy + m_xgi[ll] * logstep);
      double a1 = log(1.0 + 2.0 * ep / SiDigi::e_mass);
      double a3 = log(4.0 * totEnergy * (totEnergy - ep) / mass / mass);
      dloss += m_wgi[ll] * (1.0 - beta2 * ep / maxT + ep * ep * ftot2) * a1 * (a3 - a1);
    }
    dedx += dloss * logstep * m_alphaPrime;
  }

  dedx *= m_KBethe / beta2;

  //High order corrections --> not used
  //dedx += corr->HighOrderCorrections(p,material,kineticEnergy);

  return dedx;
}

//
// Method calculating actual dE/dx for electrons & positrons
//
double SiEnergyFluct::getElectronDEDX(const MCParticle * part)
{
  // Calculate particle related quantities
  double mass          = part->getMass() * SiDigi::GeV;
  double momentum      = part->getMomentum().Mag2() * SiDigi::GeV * SiDigi::GeV;
  double kineticEnergy = sqrt(momentum * momentum + mass * mass) - mass;
  double charge        = m_charge;
  double tau           = kineticEnergy / mass;
  double gam           = tau + 1.0;
  double gamma2        = gam * gam;
  double bg2           = tau * (tau + 2.0);
  double beta2         = bg2 / (gamma2);

  // Calculate the dE/dx due to the ionization by Seltzer-Berger formula
  bool   lowEnergy = false;
  double tkin      = kineticEnergy;

  if (kineticEnergy < m_th) {
    tkin = m_th;
    lowEnergy = true;
  }
  double lowLimit = 0.2 * SiDigi::keV;

  double maxT = kineticEnergy;

  if (charge < 0.) maxT *= 0.5;

  double eexc  = m_eexc / SiDigi::e_mass;
  double eexc2 = eexc * eexc;

  double d = std::min(m_cutOnDeltaRays, maxT) / SiDigi::e_mass;
  double dedx;

  // Electron
  if (charge < 0.) {

    dedx = log(2.0 * (tau + 2.0) / eexc2) - 1.0 - beta2 + log((tau - d) * d) + tau / (tau - d)
           + (0.5 * d * d + (2.0 * tau + 1.) * log(1. - d / tau)) / gamma2;
  }
  // Positron
  else {

    double d2 = d * d * 0.5;
    double d3 = d2 * d / 1.5;
    double d4 = d3 * d * 3.75;
    double y  = 1.0 / (1.0 + gam);
    dedx      = log(2.0 * (tau + 2.0) / eexc2) + log(tau * d) - beta2 * (tau + 2.0 * d - y * (3.0 * d2
                + y * (d - d3 + y * (d2 - tau * d3 + d4)))) / tau;
  }

  // Density correction
  double x = log(bg2) / m_twoln10;

  if (x >= m_x0den) {

    dedx -= m_twoln10 * x - m_cden;
    if (x < m_x1den) dedx -= m_aden * pow(m_x1den - x, m_mden);
  }

  // Now compute the total ionization loss
  dedx *= m_KBethe / beta2;
  if (dedx < 0.0) dedx = 0.0;

  // Lowenergy extrapolation

  if (lowEnergy) {

    if (kineticEnergy >= lowLimit) dedx *= sqrt(tkin / kineticEnergy);
    else                           dedx *= sqrt(tkin * kineticEnergy) / lowLimit;

  }

  return dedx;
}
