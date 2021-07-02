/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/phokhara/Phokhara.h>
#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TRandom3.h>

using namespace std;
using namespace Belle2;

extern "C" {

  /** struct that holds the generated event four vectors */
  extern struct {
    double bp1[4];        /**< 4-momenta of incoming positron. */
    double bq1[4];        /**< 4-momenta of incoming electron. */
    double bp2[6][10];     /**< 4-momenta[0..3], pdg code [4] and mother [5] of outgoing hadrons/muons. */
    double bphot[4][2];   /**< 4-momenta of real photons. */
    int bnphot;           /**< Number of photons. */
    int bnhad;            /**< Number of hadrons/muons. */
  } momset_;

  /** Replace internal random generator with the framework random generator */
  double phokhara_rndm_(int*)
  {
    double r = gRandom->Rndm();
    return r;
  }

  /** Wrap Phokhara random number generator and use ROOT.
   * This method returns an array of random numbers in the range ]0,1[
   * @param drvec array to store the random numbers
   * @param lenght size of the array
   */
  void phokhara_rndmarray_(double* drvec, int* lengt)
  {
    for (int i = 0; i < *lengt; ++i) {
      drvec[i] = gRandom->Rndm();
    }
  }

  /** main FORTRAN routine*/
  void phokhara_(int* mode, double* xpar, int* npar);

  /** particle parameters and inputs used by PHOKHARA */
//   void phokhara_setinputfile_(const char* inputfilename, size_t* length);
  void phokhara_setparamfile_(const char* paramfilename, size_t* length);

  /** Callback to show error if event is rejected*/
  void phokhara_error_trials_(double* trials)
  {
    B2ERROR("PHOKHARA: Event rejected! Number of maximal trials (" << *trials << " << exceeded");
  }

  /** Callback to show warning if weight is larger than maxweight*/
  void phokhara_warning_weight_(int* photmult, double* weight, double* max)
  {
    B2WARNING("PHOKHARA: Event weight (" << *weight <<  ") exceeds limit (" << *max << "), photon multiplicity: " << *photmult);
  }

  /** Callback to show warning if weight is negative*/
  void phokhara_warning_negweight_(int* photmult, double* weight)
  {
    B2WARNING("PHOKHARA: Event weight (" << *weight <<  ") is negative, photon multiplicity: " << *photmult);
  }

}

Phokhara::Phokhara()
{
  for (int i = 0; i < 100; ++i) {
    m_npar[i] = 0;
    m_xpar[i] = 0.0;
  }

  setDefaultSettings();
}

Phokhara::~Phokhara()
{

}

void Phokhara::setDefaultSettings()
{
//   std::cout << "Phokhara::setDefaultSettings()" << std::endl;

//   these are the settings to reproduce the phokhara9.1 standalone result
//   m_cmsEnergy = 10.580 * Unit::GeV;
//   m_applyBoost = true;
//   m_finalState = 1;
//   m_nMaxTrials = 10000;
//   m_nSearchMax = 50000;
//   m_epsilon = 1.e-4;
//   m_LO = 1;
//   m_NLO = 0;
//   m_QED = 0;
//   m_NLOIFI = 0;
//   m_alpha = 1;
//   m_pionff = 0;
//   m_pionstructure = 0;
//   m_kaonff = 0;
//   m_narres = 0;
//   m_protonff = 1;
//   m_ScatteringAngleRangePhoton = make_pair(0.0, 180.0); //in [deg]
//   m_ScatteringAngleRangeFinalStates = make_pair(0.0, 180.0); //in [deg]
//   m_MinInvMassHadronsGamma = 0.;
//   m_MinInvMassHadrons = 0.2;
//   m_MaxInvMassHadrons = 0.5;
//   m_MinEnergyGamma = 4.0;

  m_cmsEnergy = 0.0;
  m_finalState = -1;
  m_replaceMuonsByVirtualPhoton = false;
  m_nMaxTrials = -1;
  m_nSearchMax = -1;
  m_epsilon = 1.e-4;
  m_LO = -1;
  m_NLO = -1;
  m_QED = -1;
  m_NLOIFI = -1;
  m_alpha = -1;
  m_pionff = -1;
  m_pionstructure = -1;
  m_kaonff = -1;
  m_narres = -1;
  m_protonff = -1;
  m_ScatteringAngleRangePhoton = make_pair(0.0, 180.0); //in [deg]
  m_ScatteringAngleRangeFinalStates = make_pair(0.0, 180.0); //in [deg]
  m_MinInvMassHadronsGamma = -1;
  m_MinInvMassHadrons = 0.;
  m_ForceMinInvMassHadronsCut = false;
  m_MaxInvMassHadrons = 0.;
  m_MinEnergyGamma = 0.;

  m_pi = 0.;
  m_conversionFactor = 0.;
  m_alphaQED0 = 0.;
  m_massElectron = 0.;
  m_massMuon = 0.;
  m_massW = 0.;
  m_massZ = 0.;
  m_widthZ = 0.;

}

void Phokhara::init(const std::string& paramFile)
{
  B2INFO("Phokhara::init, using paramater file: " << paramFile);

  if (paramFile.empty()) B2FATAL("Phokhara: The specified param file is empty!");
  size_t fileLength = paramFile.size();
  phokhara_setparamfile_(paramFile.c_str(), &fileLength);

//   if (inputFile.empty()) B2FATAL("Phokhara: The specified input file is empty!")
//   fileLength = inputFile.size();
//   phokhara_setinputfile_(inputFile.c_str(), &fileLength);

  applySettings();
}


void Phokhara::generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, TLorentzRotation boost)
{

  //Generate event
  int mode = 1;
  if (m_ForceMinInvMassHadronsCut) {
    while (1) {
      phokhara_(&mode, m_xpar, m_npar);
      double partMom[4] = {0, 0, 0, 0};
      for (int iPart = 0; iPart < momset_.bnhad; ++iPart) {
        for (int j = 0; j < 4; ++j)
          partMom[j] += momset_.bp2[j][iPart];
      }
      double m2 = partMom[0] * partMom[0] - partMom[1] * partMom[1]
                  - partMom[2] * partMom[2] - partMom[3] * partMom[3];
      if (m2 > m_MinInvMassHadrons)
        break;
    }
  } else
    phokhara_(&mode, m_xpar, m_npar);

  //Store the initial particles as virtual particles into the MCParticleGraph
  double eMom[4] = {momset_.bp1[1], momset_.bp1[2], momset_.bp1[3], momset_.bp1[0]};
  double pMom[4] = {momset_.bq1[1], momset_.bq1[2], momset_.bq1[3], momset_.bq1[0]};

  storeParticle(mcGraph, eMom, 11, vertex, boost, false, true);
  storeParticle(mcGraph, pMom, -11, vertex, boost, false, true);

  //Store the real photons into the MCParticleGraph
  for (int iPhot = 0; iPhot <  momset_.bnphot; ++iPhot) {
    double photMom[4] = {momset_.bphot[1][iPhot], momset_.bphot[2][iPhot], momset_.bphot[3][iPhot], momset_.bphot[0][iPhot]};
    storeParticle(mcGraph, photMom, 22, vertex, boost, false, false);
  }

  //Store the other final state particles into the MCParticleGraph
  if ((m_finalState == 0) && m_replaceMuonsByVirtualPhoton) {
    if (momset_.bnhad != 2)
      B2FATAL("Number of particles generated by PHOKHARA does not match the "
              "requested final state (mu+ mu-).");
    double partMom[4] = {momset_.bp2[1][0] + momset_.bp2[1][1],
                         momset_.bp2[2][0] + momset_.bp2[2][1],
                         momset_.bp2[3][0] + momset_.bp2[3][1],
                         momset_.bp2[0][0] + momset_.bp2[0][1]
                        };
    storeParticle(mcGraph, partMom, 10022, vertex, boost, false, false);
  } else {
    for (int iPart = 0; iPart <  momset_.bnhad; ++iPart) {
      double partMom[4] = {momset_.bp2[1][iPart], momset_.bp2[2][iPart], momset_.bp2[3][iPart], momset_.bp2[0][iPart]};

      storeParticle(mcGraph, partMom, momset_.bp2[4][iPart], vertex, boost, false, false);
    }
  }

  //some PHOKHARA final states contain unstable particle
  if (m_finalState == 9) { //Lambda, Lambdabar
    int id = 2 + momset_.bnphot;

    //get lambdabar -> p+ anti-p and lambda -> pi p
    MCParticleGraph::GraphParticle* lambdabar = &mcGraph[id];
    MCParticleGraph::GraphParticle* daughter1 = &mcGraph[id + 2];
    daughter1->comesFrom(*lambdabar);
    MCParticleGraph::GraphParticle* daughter2 = &mcGraph[id + 3];
    daughter2->comesFrom(*lambdabar);

    MCParticleGraph::GraphParticle* lambda = &mcGraph[id + 1];
    daughter1 = &mcGraph[id + 4];
    daughter1->comesFrom(*lambda);
    daughter2 = &mcGraph[id + 5];
    daughter2->comesFrom(*lambda);

    lambdabar->removeStatus(MCParticle::c_StableInGenerator);
    lambda->removeStatus(MCParticle::c_StableInGenerator);
  }

}


void Phokhara::term()
{

  int mode = 2;
  phokhara_(&mode, m_xpar, m_npar);

//   B2INFO("> Crosssection ")
//   B2INFO(">> xsec (weighted)   = (" << bresults_.rescross << " +/- " << bresults_.rescrosserr << ") nb")
//   for (int i = 0; i < 3; i++) {
//     B2INFO(">>> " << i << " photon(s), xsec (weighted) = (" << bresults_.rescrossphot[i] << " +/- " << bresults_.rescrossphoterr[i] << ") nb, fraction = " << bresults_.rescrossphotfrac[i]*100.<< "%")
//   }
//
//   B2INFO(">>> xsec (unweighted) = (" << bhitnmiss_.hnmcross << " +/- " << bhitnmiss_.hnmcrosserr << ") nb")
//   for (int i = 0; i < 3; i++) {
//     B2INFO(">>> " << i << " photon(s), xsec (unweighted) = (" << bhitnmiss_.hnmcrossphot[i] << " +/- " << bhitnmiss_.hnmcrossphoterr[i] << ") nb, fraction.= " << bhitnmiss_.hnmcrossphotfrac[i]*100.<< "%")
//   }
//
//   B2INFO("> hit/miss efficiency")
//   for (int i = 0; i < 3; i++) {
//     B2INFO(">>> " << i << " photon(s),eff.= " << bhitnmiss_.hnmeff[i]*100.<< "%")
//   }
}

//=========================================================================
//                       Protected methods
//=========================================================================

void Phokhara::applySettings()
{

  //--------------------
  // Integer parameters
  //--------------------
  m_npar[1]   = m_nMaxTrials;
  m_npar[2]   = m_nSearchMax;
  m_npar[20]  = m_finalState;
  m_npar[30]  = m_LO;
  m_npar[31]  = m_NLO;
  m_npar[32]  = m_QED;
  m_npar[33]  = m_NLOIFI;
  m_npar[34]  = m_alpha;
  m_npar[35]  = m_pionff;
  m_npar[36]  = m_pionstructure;
  m_npar[37]  = m_kaonff;
  m_npar[38]  = m_narres;
  m_npar[39]  = m_protonff;

  //--------------------
  // Double parameters
  //--------------------
  m_xpar[0]  = m_cmsEnergy;
  m_xpar[11] = m_epsilon;

  m_xpar[15] = m_MinInvMassHadronsGamma;
  m_xpar[16] = m_MinInvMassHadrons;
  m_xpar[17] = m_MaxInvMassHadrons;
  m_xpar[18] = m_MinEnergyGamma;

  m_xpar[20] = m_ScatteringAngleRangePhoton.first;
  m_xpar[21] = m_ScatteringAngleRangePhoton.second;
  m_xpar[22] = m_ScatteringAngleRangeFinalStates.first;
  m_xpar[23] = m_ScatteringAngleRangeFinalStates.second;

  int mode = -1; //use mode to control init/generation/finalize in FORTRAN code
  phokhara_(&mode, m_xpar, m_npar);
}


void Phokhara::storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                             bool isVirtual, bool isInitial)
{

  //   Create particle
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();

  //all particles are primary!
  part.addStatus(MCParticle::c_PrimaryParticle);

  //all particles are stable (if not, will be changed later)!
  part.addStatus(MCParticle::c_StableInGenerator);

  if (isVirtual) {
    part.addStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.addStatus(MCParticle::c_Initial);
  }

  //set photon flags
  if (pdg == 22) {
    part.addStatus(MCParticle::c_IsISRPhoton);
    part.addStatus(MCParticle::c_IsFSRPhoton);
  }

  part.setPDG(pdg);
  part.setFirstDaughter(0);
  part.setLastDaughter(0);
  part.setMomentum(TVector3(mom[0], mom[1], mom[2]));
  // part.get4Vector() uses mass, need to set invariant mass for virtual photons
  if ((m_finalState == 0) && m_replaceMuonsByVirtualPhoton && (pdg == 10022))
    part.setMass(sqrt(mom[3] * mom[3] - mom[0] * mom[0] - mom[1] * mom[1] -
                      mom[2] * mom[2]));
  else
    part.setMass(TDatabasePDG::Instance()->GetParticle(pdg)->Mass());
  part.setEnergy(mom[3]);

  //boost
  TLorentzVector p4 = part.get4Vector();
  p4 = boost * p4;
  part.set4Vector(p4);

  //set vertex
  if (!isInitial) {
    TVector3 v3 = part.getProductionVertex();
    v3 = v3 + vertex;
    part.setProductionVertex(v3);
    part.setValidVertex(true);
  }
}
