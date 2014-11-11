/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <generators/aafh/AAFHInterface.h>
#include <TRandom3.h>
#include <TDatabasePDG.h>
#include <algorithm>
#include <array>
#include <iomanip>

using namespace Belle2;

extern "C" {
  /** Replace internal random generator with the framework random generator */
  double aafhdiag36_rndm_(int*)
  {
    return gRandom->Rndm();
  }
  /** Replace internal random generator with the framework random generator */
  double aafhdiag36_rnf100_(int*)
  {
    return gRandom->Rndm();
  }
  /** Callback to show error if weights are not sufficient */
  void aafhdiag36_esft_maxed_(double* weight)
  {
    B2ERROR("AAFH: Maximum weight to small, increase maxFinalWeight to at least "
            << *weight);
  }
  /** Callback to show error if weights are not sufficient */
  void aafhdiag36_eswe_maxed_(double* weight)
  {
    B2ERROR("AAFH: Maximum weight to small, increase maxSubGeneratorWeight to at least "
            << *weight);
  }

  /** Initialization function */
  extern void aafhdiag36_init_();
  /** Event function, will create one event */
  extern void aafhdiag36_event_(int*);
  /** Calculates cross section */
  extern void aafhdiag36_finish_();

  /** Common block containing beam energy, max weights and sub generator
   * weights */
  extern struct {
    double eb; /**< beam energy in GeV */
    double eswe; /**< maximum sub generator weight */
    double esft; /**< maximum event weight */
    double wap[4]; /**< sub generator weights */
    double wbp[4]; /**< sub generator scaling */
    double vap[4]; /**< internal weights, do not modify */
  } input_;

  /** Common block containing minimum and maximum invariant mass. maximum is
   * set automatically, minimum can be adjusted */
  extern struct {
    double w2min; /**< minimum invariant mass squared in units of beam energy */
    double w2max; /**< maximum invariant mass squared in units of beam energy */
  } bound_;

  /** common block for the input parameters */
  extern struct {
    int iproc; /**< process to generate */
    int irejec; /**< reject method */
    int idump[4]; /**< dump sub generator info */
    int info; /**< verbosity */
    int itot; /**< maximum number of tries */
  } input2_;

  /** common block for the accuracy parameters. See aafh.F for explanation */
  extern struct {
    double face; /**< accuracy for electron matrix element calculation */
    double facl; /**< accuracy for muon matrix element calculation */
    double facm; /**< accuracy for particle matrix element calculation */
    double proc; /**< accuracy for feynman graph calculation */
  } factor_;

  /** common block for the generated particles including initial particles.
   * Elements are x,y,z,E,m in units of beam energy
   */
  extern struct {
    double q1[5]; /**< initial electron */
    double q2[5]; /**< initial positron */
    double q3[5]; /**< pep */
    double q4[5]; /**< pem */
    double q5[5]; /**< qep */
    double q6[5]; /**< qem */
  } momenz_;

  /** common block for the results, that is the total cross section and its
   * error */
  extern struct {
    double crosec; /**< total cross section in nanobarn */
    double ercros; /**< error on the cross section in nanobarn */
  } output_;

  /** common block for the masses */
  extern struct {
    double xm;  /**< electron mass */
    double xmu; /**< muon mass */
    double xml; /**< particle mass for c_MuonParticle and c_ElectronParticle */
    double xm2;  /**< squared electron mass */
    double xmu2; /**< squared muon mass */
    double xml2; /**< squared particle mass for c_MuonParticle and c_ElectronParticle */
  } masses_;

  /** common block for the charge of the created particle pair */
  extern struct {
    double qcharg; /**< negative charge of the generated particle */
    double qchrg2; /**< negative charge^2 of the generated particle */
    double qchrg3; /**< negative charge^3 of the generated particle */
    double qchrg4; /**< negative charge^4 of the generated particle */
  } charge_;

  /** common block for the statistics of the 4 sub generators */
  extern struct {
    double swe[4]; /**< sum of weights */
    double swek[4]; /**< sum of weights squared */
    double mwe[4]; /**< maximum subgenerator weight */
    double sum; /**< total sum of weights */
    double sumk; /**< total sum of weights squared */
    double maxwe; /**< total max weight */
    int iwe[4]; /**< number of generated events */
    int igen; /**< total number of generated events */
  } westat_;

  /** comon block for final event statistics */
  extern struct {
    double sumft; /**< sum of weights */
    double sumft2; /**< sum of weights squared */
    double ftmax; /**< maximum final event weight */
    int ieen; /**< number of accepted events */
  } ftstat_;
};


void AAFHInterface::setMaxTries(int tries)
{
  input2_.itot = tries;
}

void AAFHInterface::setGeneratorWeights(const std::vector<double>& weights)
{
  if (weights.size() > 8) {
    B2WARNING("AAFH: more than 8 generator weights supplied, "
              "ignoring the extra weights");
  } else if (weights.size() > 4 && weights.size() < 8) {
    B2WARNING("AAFH: more than 4 but less than 8 generator weights supplied, "
              "ignoring everything afer the first four");
  } else if (weights.size() < 4) {
    B2ERROR("AAFH: not enough generator weights, need exactly four or eight values");
  }
  std::copy_n(weights.begin(), 4, input_.wap);
  if (weights.size() >= 8) {
    std::copy_n(weights.begin() + 4, 4, input_.wbp);
  }
}

void AAFHInterface::setMaxWeights(double subgeneratorWeight, double finalWeight)
{
  input_.eswe = subgeneratorWeight;
  input_.esft = finalWeight;
}

void AAFHInterface::setSupressionLimits(std::vector<double> limits)
{
  if (limits.size() == 1) {
    B2WARNING("AAFH: Only one suppression limit supplied, using same value for all limits");
    limits.resize(4, limits[0]);
  } else if (limits.size() != 4) {
    B2ERROR("AAFH: Wrong number of suppression limits: supply either one or 4 values");
  }
  factor_.face = limits[0];
  factor_.facm = limits[1];
  factor_.facl = limits[2];
  factor_.proc = limits[3];
}

int AAFHInterface::getMaxTries() const
{
  return input2_.itot;
}

std::vector<double> AAFHInterface::getGeneratorWeights() const
{
  std::vector<double> weights(8);
  std::copy_n(input_.wap, 4, weights.begin());
  std::copy_n(input_.wbp, 4, weights.begin() + 4);
  return weights;
}

double AAFHInterface::getMaxSubGeneratorWeight() const
{
  return input_.eswe;
}

double AAFHInterface::getMaxFinalWeight() const
{
  return input_.esft;
}

std::vector<double> AAFHInterface::getSuppressionLimits() const
{
  return {factor_.face, factor_.facm, factor_.facl, factor_.proc};
}

double AAFHInterface::getTotalCrossSection() const
{
  return output_.crosec * Unit::nb;
}

double AAFHInterface::getTotalCrossSectionError() const
{
  return output_.ercros * Unit::nb;
}

void AAFHInterface::initialize(double beamEnergy, EMode mode, ERejection rejection)
{
  input_.eb = beamEnergy;
  //minimum invariant mass is in units of beam energy and needs to be squared
  bound_.w2min = (m_minMass / beamEnergy) * (m_minMass / beamEnergy);
  //set mode and rejection scheme
  input2_.iproc = mode;
  input2_.irejec = rejection;

  //initialize the L
  auto* part = TDatabasePDG::Instance()->GetParticle(m_particle.c_str());
  if (!part) {
    B2ERROR("AAFH: Particle '" << m_particle << "' does not exist");
    return;
  }
  //set the mass in units of beam energy
  masses_.xml = part->Mass() / beamEnergy;
  //set the charge but with opposite sign
  charge_.qcharg = -part->Charge() / 3.0;
  //and remember pdg code
  m_particlePDG = part->PdgCode();

  aafhdiag36_init_();
}

void AAFHInterface::updateParticle(MCParticleGraph::GraphParticle& p, double* q, int pdg)
{
  // all values in q are in units of beam energy
  TLorentzVector vec(q[0]*input_.eb, q[1]*input_.eb, q[2]*input_.eb, q[3]*input_.eb);
  p.set4Vector(vec);
  // mass is multiplied by charge sign so take the absolute
  p.setMass(fabs(q[4])*input_.eb);
  p.setPDG(pdg);
  p.setStatus(MCParticle::c_PrimaryParticle | MCParticle::c_StableInGenerator);
}

void AAFHInterface::generateEvent(MCParticleGraph& mpg)
{
  int status {0};
  aafhdiag36_event_(&status);
  if (status != 0) {
    B2ERROR("Failed to generate event after " << status << " tries, giving up");
    return;
  }
  //initial beam particles, ignore
  //auto &p1 = mpg.addParticle();
  //auto &p2 = mpg.addParticle();
  //generated particles
  auto& p3 = mpg.addParticle();
  auto& p4 = mpg.addParticle();
  auto& p5 = mpg.addParticle();
  auto& p6 = mpg.addParticle();

  //array of generated particle pdg codes depends on generated mode
  std::array<int, 4> pdg {{0}};
  switch (input2_.iproc) {
    case c_MuonParticle:
      pdg = {{m_particlePDG, -m_particlePDG, 13, -13}};
      break;
    case c_MuonMuon:
      pdg = {{13, -13, 13, -13}};
      break;
    case c_ElectonMuon:
      pdg = {{11, -11, 13, -13}};
      break;
    case c_ElectronParticle:
      pdg = {{11, -11, m_particlePDG, -m_particlePDG}};
      break;
    case c_ElectonElectron:
      pdg = {{11, -11, 11, -11}};
      break;
  }

  updateParticle(p3, momenz_.q3, pdg[0]);
  updateParticle(p4, momenz_.q4, pdg[1]);
  updateParticle(p5, momenz_.q5, pdg[2]);
  updateParticle(p6, momenz_.q6, pdg[3]);
}

void AAFHInterface::finish()
{
  aafhdiag36_finish_();
  B2INFO("Final cross section:         " << std::setprecision(3) << output_.crosec
         << "+-" << std::setprecision(3) << output_.ercros << " nb");
  B2INFO("Minimum invariant mass:      " << sqrt(bound_.w2min)*input_.eb << " GeV");
  B2INFO("Maximum invariant mass:      " << sqrt(bound_.w2max)*input_.eb << " GeV");
  B2INFO("Maximum subgenerator weight: " << westat_.maxwe << " ("
         << westat_.mwe[0] << ", "
         << westat_.mwe[1] << ", "
         << westat_.mwe[2] << ", "
         << westat_.mwe[3] << ")"
        );
  B2INFO("Maximum final weight:        " << ftstat_.ftmax);
  //The fortran source says that it's advisable if all subgenerators have the
  //same probability to account for all peaks in the differental cross section
  //and that it runs with the highest efficiency when the maximum weights in
  //all sub generators are equal. So let's calculate appropriate subgenerator
  //weights by looking how many events were generated per sub generator or the
  //maximum weights
  std::array<double, 4> idealWAP;
  std::array<double, 4> idealWBP;
  //find the first non-zero weight
  int reference = 0;
  for (int i = 0; i < 4; ++i) {
    if (input_.wap[i] != 0) {
      reference = i;
      break;
    }
  }
  //and use that es reference
  const double w0 = westat_.mwe[reference] * input_.wbp[reference];
  const double n0 = westat_.iwe[reference] / input_.wap[reference];
  //now calculate the relative weights we need to get
  //1) same maximum weight which is inversely proportional to to wbp
  //2) same number of generated events which is proportional to wap
  for (int i = 0; i < 4; ++i) {
    if (input_.wap[i] == 0) {
      idealWBP[i] = input_.wbp[i];
      idealWAP[i] = 0;
    } else {
      idealWBP[i] = (westat_.mwe[i] * input_.wbp[i]) / w0;
      idealWAP[i] = n0 * input_.wap[i] / westat_.iwe[i];
      //and normalize to 1.0
      idealWAP[i] /= idealWAP[reference];
    }
  }

  B2INFO("To get the same maximum event weight and chance for each sub generator, try:");
  B2INFO("   'subgeneratorWeights': [" << std::scientific
         << std::setprecision(3) << idealWAP[0] << ", "
         << std::setprecision(3) << idealWAP[1] << ", "
         << std::setprecision(3) << idealWAP[2] << ", "
         << std::setprecision(3) << idealWAP[3] << ", "
         << "\n                                  "
         << std::setprecision(3) << idealWBP[0] << ", "
         << std::setprecision(3) << idealWBP[1] << ", "
         << std::setprecision(3) << idealWBP[2] << ", "
         << std::setprecision(3) << idealWBP[3] << "]"
        );
}
