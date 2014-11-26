/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/babayaganlo/BabayagaNLO.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TRandom3.h>

using namespace std;
using namespace Belle2;

extern "C" {

  //same convention as BHWIDE
  extern struct {
    double bp1[4];        /**< 4-momenta of incoming positron. */
    double bq1[4];        /**< 4-momenta of incoming electron. */
    double bp2[4];        /**< 4-momenta of outgoing positron. */
    double bq2[4];        /**< 4-momenta of outgoing electron. */
    double bphot[4][100]; /**< 4-momenta of real photons. */
    int bnphot;           /**< Number of photons. */
  } momset_;

  //results for cross section and photon multiplicity
  extern struct {
    double rescross;
    double rescrosserr;
    double rescrossphot[40];
    double rescrossphoterr[40];
    double rescrossphotfrac[40];
    double resnphmax;
  } bresults_;

  //results and statistics for hit/miss
  extern struct {
    double hnmcross;
    double hnmcrosserr;
    double hnmeff;
    double hnmsdifmax;
    double hnmfmax;
    double hnmmaxtriallimit;
    double hnmmaxtrial;
  } bhitnmiss_;

  //bias for hit/miss
  extern struct {
    double biashit;
    double biashitpmiss;
    double biasneghit;
    double biasneghitmiss;
    double sezover;
    double errsezover;
    double sezneg;
    double errsezneg;
    double nover;
    double nneg;
  } bbias_;

  //monitoring
  extern struct {
    double monsdif;
  } bmonitoring_;

  /** Replace internal random generator with the framework random generator */
  double babayaganlo_rndm_(int*)
  {
    double r = gRandom->Rndm();
    return r;
  }

  /** FORTRAN routine based on main.f */
  void belle_(int* mode, double* xpar, int* npar);

  /** Callback to show warning if weights are not sufficient --> bias*/
  void babayaganlo_warning_overweight_(double* weight, double* max)
  {
    B2WARNING("BABAYAGA.NLO: Maximum weight " << *max  << " to small, increase fmax to at least " << *weight);
  }

  /** Callback to show error if event is rejected*/
  void babayaganlo_error_rejection_(double* ratio)
  {
    B2ERROR("BABAYAGA.NLO: Event rejected! Ratio of cross section error increase:  " << *ratio);
  }

  /** Callback to show error if weight is negative*/
  void babayaganlo_error_negative_(double* weight)
  {
    B2ERROR("BABAYAGA.NLO: Event has negative weight! " << *weight);
  }

}

BabayagaNLO::BabayagaNLO()
{
  for (int i = 0; i < 100; ++i) {
    m_npar[i] = 0;
    m_xpar[i] = 0.0;
  }

  setDefaultSettings();
}

BabayagaNLO::~BabayagaNLO()
{

}

void BabayagaNLO::setDefaultSettings()
{
  m_cmsEnergy = 10.58 * Unit::GeV;

  m_applyBoost = true;

  m_finalState = "ee";

  m_pi               = 3.1415926535897932384626433832795029;
  m_conversionFactor = 0.389379660e6;
  m_alphaQED0        = 1.0 / 137.0359895;
  m_massElectron     = 0.51099906 * Unit::MeV;
  m_massMuon         = 105.65836900 * Unit::MeV;
  m_massW            = 80.385 * Unit::GeV;
  m_massZ            = 91.1882 * Unit::GeV;
  m_widthZ           = 2.4952 * Unit::GeV;
  m_eMin             = 0.15 * Unit::GeV;
  m_epsilon          = 5.e-4;
  m_maxAcollinearity = 180.0;// * Unit::Deg;

  m_ScatteringAngleRange = make_pair(15.0, 165.0); //in [deg]
  m_ScatteringAngleRangePhoton = make_pair(15.0, 165.0); //in [deg] (unused in original babayaga)

  m_nPhotMode = -1;

  m_nSearchMax = 50000;
  m_fMax = -1.;

}

void BabayagaNLO::init()
{

  applySettings();
}


void BabayagaNLO::generateEvent(MCParticleGraph& mcGraph)
{
  //Generate event
  int mode = 1;
  belle_(&mode, m_xpar, m_npar);

  //Store the initial particles as virtual particles into the MCParticleGraph
  storeParticle(mcGraph, momset_.bq1, 11, true);
  storeParticle(mcGraph, momset_.bp1, -11, true);

  //Store the final state fermions or photons (for 'gg') as real particle into the MCParticleGraph
  int pdg = 11;
  int antipdg = -11;

  if (m_finalState == "gg") {
    pdg = 22;
    antipdg = 22;
  } else  if (m_finalState == "mm") {
    pdg = 13;
    antipdg = -13;
  }

  storeParticle(mcGraph, momset_.bq2, pdg);
  storeParticle(mcGraph, momset_.bp2, antipdg);

  //Store the real photons into the MCParticleGraph
  for (int iPhot = 0; iPhot <  momset_.bnphot; ++iPhot) {
    double photMom[4] = {momset_.bphot[0][iPhot], momset_.bphot[1][iPhot], momset_.bphot[2][iPhot], momset_.bphot[3][iPhot]};
    storeParticle(mcGraph, photMom, 22);
  }

  //Fill monitoring
  m_sDif = bmonitoring_.monsdif;

}


void BabayagaNLO::term()
{
  int mode = 2;
  belle_(&mode, m_xpar, m_npar);

  B2INFO(">>> xsec (weighted)   = (" << bresults_.rescross << " +/- " << bresults_.rescrosserr << ") nb")
  for (int i = 0; i < bresults_.resnphmax; i++) {
    B2INFO(">>> " << i << " photon(s), xsec (weighted) = (" << bresults_.rescrossphot[i] << " +/- " << bresults_.rescrossphoterr[i] << ") nb")
  }

  B2INFO(">>> xsec (unweighted) = (" << bhitnmiss_.hnmcross << " +/- " << bhitnmiss_.hnmcrosserr << ") nb")
  B2INFO(">>> unweighted eff.   = " << bhitnmiss_.hnmeff * 100. << "%")
  B2INFO(">>> fmax              = " << bhitnmiss_.hnmfmax)
  B2INFO(">>> sdifmax           = " << bhitnmiss_.hnmsdifmax)
  B2INFO(">>> max. trials       = " << bhitnmiss_.hnmmaxtrial)
  B2INFO(">>> trial limit       = " << bhitnmiss_.hnmmaxtriallimit)

  B2INFO(">>> points with w > fmax (bias): = " << bbias_.nover)
  B2INFO(">>> points with w <0:            = " << bbias_.nneg)
  B2INFO(">>> bias over fmax               = (" << bbias_.sezover << " +/- " << bbias_.errsezover << ") nb")
  B2INFO(">>> negative bias                = (" << -bbias_.sezneg << " +/- " << bbias_.errsezneg << ") nb")
  B2INFO(">>> xsec incl. biases            = (" << bhitnmiss_.hnmcross + bbias_.sezover - bbias_.sezneg << " +/- " << bhitnmiss_.hnmcrosserr + bbias_.errsezover + bbias_.errsezneg << ") nb")
  B2INFO(">>> bias (w>fmax)/hit            = (" << bbias_.biashit * 100. << ") %")
  B2INFO(">>> bias (w>fmax)/(hit+missed)   = (" << bbias_.biashitpmiss * 100. << ") %")
  B2INFO(">>> bias (w<0)/hit               = (" << bbias_.biasneghit * 100. << ") %")
  B2INFO(">>> bias (w<0)/(hit+missed)      = (" << bbias_.biasneghitmiss * 100. << ") %")

}

//=========================================================================
//                       Protected methods
//=========================================================================

void BabayagaNLO::applySettings()
{
  //--------------------
  // Integer parameters
  //--------------------
  m_npar[0] = m_nPhotMode;
  m_npar[1] = m_nSearchMax;

  //--------------------
  // Double parameters
  //--------------------
  m_xpar[0]  = m_cmsEnergy;
  m_xpar[1]  = m_pi;
  m_xpar[2]  = m_conversionFactor;
  m_xpar[3]  = m_alphaQED0;
  m_xpar[4]  = m_massElectron;
  m_xpar[5]  = m_massMuon;
  m_xpar[6]  = m_massW;
  m_xpar[7]  = m_massZ;
  m_xpar[8]  = m_widthZ;
  m_xpar[9]  = m_eMin;
  m_xpar[10] = m_maxAcollinearity;
  m_xpar[11] = m_epsilon;

  m_xpar[20] = m_ScatteringAngleRange.first;
  m_xpar[21] = m_ScatteringAngleRange.second;

  m_xpar[30] = m_fMax;

  //--------------------
  // string parameters, wrapped to integers (fortran/C with characters is weird)
  //--------------------
  if (m_finalState == "ee") m_npar[20] = 1;
  else if (m_finalState == "gg") m_npar[20] = 2;
  else if (m_finalState == "mm") m_npar[20] = 3;

  if (m_model == "matched") m_npar[21] = 1;
  else if (m_model == "ps") m_npar[21] = 2;

  if (m_finalState == "exp") m_npar[22] = 1;

  int mode = -1; //use mode to control init/generation/finalize in FORTRAN code
  belle_(&mode, m_xpar, m_npar);
}


void BabayagaNLO::storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, bool isVirtual, bool isInitial)
{

//   Create particle
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  if (isVirtual) {
    part.setStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.setStatus(MCParticle::c_Initial);
  } else {
    part.setStatus(MCParticle::c_PrimaryParticle);
  }

  part.setPDG(pdg);
  part.setFirstDaughter(0);
  part.setLastDaughter(0);
  part.setMomentum(TVector3(mom[0], mom[1], mom[2]));
  part.setMass(TDatabasePDG::Instance()->GetParticle(pdg)->Mass());
  part.setEnergy(mom[3]);

  //Mirror Pz and if boosting is enable boost the particles to the lab frame - ALSO FOR BABAYAGA!??
  TLorentzVector p4 = part.get4Vector();
//   p4.SetPz(-1.0 * p4.Pz());
  if (m_applyBoost) p4 = m_boostVector * p4;
  part.set4Vector(p4);
}
