/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/bhwide/BHWide.h>
#include <framework/gearbox/Unit.h>

#include <TDatabasePDG.h>
#include <Math/Vector4D.h>
#include <TRandom.h>

using namespace std;
using namespace Belle2;

//Declaration of the BHWide FORTRAN methods and common blocks.
extern "C" {

  extern struct {
    double p1[4];        /**< 4-momenta of incoming positron. */
    double q1[4];        /**< 4-momenta of incoming electron. */
    double p2[4];        /**< 4-momenta of outgoing positron. */
    double q2[4];        /**< 4-momenta of outgoing electron. */
    double phit[4][100]; /**< 4-momenta of real photons. */
    int nphot;           /**< Number of photons. */
  } momset_;

  void bhwide_(int* mode, double* xpar, int* npar);
  void glimit_(int* number);

  /** Wrap BHwide random number generator and use ROOT.
   * This method returns an array of random numbers in the range ]0,1[
   * @param drvec array to store the random numbers
   * @param lenght size of the array
   */
  void varran_(double* drvec, int* lengt)
  {
    for (int i = 0; i < *lengt; ++i) {
      do {
        //BHWide does not want 1 for some reason, at least they are rejected in
        //the original VARRAN subroutine so we do it here as well
        drvec[i] = gRandom->Rndm();
      } while (drvec[i] >= 1.0);
    }
  }
  /** Also Wrap the random number generator in alibaba.f used for MC
   * integration. Does not seem to be called but better safe then sorry
   * @param rvec pointer to the random number to be set
   */
  void ranmar_(double* rvec)
  {
    *rvec = gRandom->Rndm();
  }
}


BHWide::BHWide()
{
  for (int i = 0; i < 100; ++i) {
    m_npar[i] = 0;
    m_xpar[i] = 0.0;
  }

  m_crossSection = 0.;
  m_crossSectionError = 0.;

  setDefaultSettings();
}


BHWide::~BHWide()
{

}


void BHWide::setDefaultSettings()
{

  m_zContribution = true;
  m_channel = CH_BOTH;
  m_weighted = false;
  m_randomGenerator = RG_RANMAR;
  m_weakCorrections = true;
  m_ewCorrectionLib = EC_ALIBABA;
  m_hardBremsModel = HM_CALKUL;
  m_photonVacPol = PP_BURKHARDT;

  m_cmsEnergy = 10.58 * Unit::GeV;
  m_ScatteringAngleRangePositron = make_pair(17.0, 150.0); //in [deg]
  m_ScatteringAngleRangeElectron = make_pair(17.0, 150.0); //in [deg]
  m_minEnergyFinalStatePos = 0.2 * Unit::GeV;
  m_minEnergyFinalStateElc = 0.2 * Unit::GeV;
  m_maxAcollinearity = 10.0;
  m_infCutCMSEnergy = 1e-5;
  m_maxRejectionWeight = 3.0;
  m_massZ = 91.1882 * Unit::GeV;
  m_widthZ = 2.4952;
  m_sinW2 = 0.22225;
  m_massTop = 174.3 * Unit::GeV;
  m_massHiggs = 115.0 * Unit::GeV;
}


void BHWide::init()
{
  int number = 50000;
  glimit_(&number);
  applySettings();
}


void BHWide::generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, ROOT::Math::LorentzRotation boost)
{
  //Generate event
  int mode = 0;
  bhwide_(&mode, m_xpar, m_npar);

  //Store the initial particles as virtual particles into the MCParticleGraph
  storeParticle(mcGraph, momset_.q1, 11, vertex, boost, true);
  storeParticle(mcGraph, momset_.p1, -11, vertex, boost, true);

  //Store the final state positron and electron into the MCParticleGraph
  storeParticle(mcGraph, momset_.q2, 11, vertex, boost);
  storeParticle(mcGraph, momset_.p2, -11, vertex, boost);

  //Store the real photons into the MCParticleGraph
  for (int iPhot = 0; iPhot <  momset_.nphot; ++iPhot) {
    double photMom[4] = {momset_.phit[0][iPhot], momset_.phit[1][iPhot], momset_.phit[2][iPhot], momset_.phit[3][iPhot]};
    storeParticle(mcGraph, photMom, 22, vertex, boost);
  }
}


void BHWide::term()
{
  int mode = 2;
  bhwide_(&mode, m_xpar, m_npar);

  //Get the cross section
  m_crossSection      = m_xpar[9];
  m_crossSectionError = m_xpar[10];
}

//=========================================================================
//                       Protected methods
//=========================================================================

void BHWide::applySettings()
{
  //--------------------
  // Integer parameters
  //--------------------
  int keyZof;
  if (m_zContribution) keyZof = 0;
  else keyZof = 1;
  m_npar[0] = (1000 * keyZof) + (100 * m_channel) + (10 * m_weighted) + m_randomGenerator;
  m_npar[1] = (1000 * m_weakCorrections) + (100 * m_ewCorrectionLib) + (10 * m_hardBremsModel) + m_photonVacPol;

  //--------------------
  // Double parameters
  //--------------------
  m_xpar[0]  = m_cmsEnergy;
  m_xpar[1]  = m_ScatteringAngleRangePositron.first;
  m_xpar[2]  = m_ScatteringAngleRangePositron.second;
  m_xpar[3]  = m_ScatteringAngleRangeElectron.first;
  m_xpar[4]  = m_ScatteringAngleRangeElectron.second;
  m_xpar[5]  = m_minEnergyFinalStatePos;
  m_xpar[6]  = m_minEnergyFinalStateElc;
  m_xpar[7]  = m_maxAcollinearity;
  m_xpar[8]  = m_infCutCMSEnergy;
  m_xpar[9]  = m_maxRejectionWeight;
  m_xpar[10] = m_massZ;
  m_xpar[11] = m_widthZ;
  m_xpar[12] = m_sinW2;
  m_xpar[13] = m_massTop;
  m_xpar[14] = m_massHiggs;

  int mode = -1;
  bhwide_(&mode, m_xpar, m_npar);
}


void BHWide::storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, ROOT::Math::LorentzRotation boost,
                           bool isVirtual, bool isInitial)
{
  //  //Create particle
  //MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  //if (!isVirtual) {
  //  part.setStatus(MCParticle::c_PrimaryParticle);
  //} else {
  //  part.setStatus(MCParticle::c_IsVirtual);
  //}
  //Create particle
  // RG 6/25/14 Add new flag for ISR "c_Initial"
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  if (isVirtual) {
    part.setStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.setStatus(MCParticle::c_Initial);
  }

  // every particle from a generator is primary
  part.addStatus(MCParticle::c_PrimaryParticle);

  // all particles produced by BHWIDE are stable
  part.addStatus(MCParticle::c_StableInGenerator);

  // all photons from BHWIDE are ISR or FSR
  if (pdg == 22 && !isVirtual) {
    part.addStatus(MCParticle::c_IsISRPhoton);
    part.addStatus(MCParticle::c_IsFSRPhoton);
  }

  part.setPDG(pdg);
  part.setFirstDaughter(0);
  part.setLastDaughter(0);
  part.setMomentum(TVector3(mom[0], mom[1], mom[2]));
  part.setMass(TDatabasePDG::Instance()->GetParticle(pdg)->Mass());
  part.setEnergy(mom[3]);

  //boost
  ROOT::Math::PxPyPzEVector p4 = part.get4Vector();
  p4.SetPz(-1.0 * p4.Pz()); //BHWIDE uses other direction convention
  p4 = boost * p4;
  part.set4Vector(p4);

  //set vertex
  if (!isInitial) {
    B2Vector3D v3 = part.getProductionVertex();
    v3 = v3 + vertex;
    part.setProductionVertex(v3);
    part.setValidVertex(true);
  }
}
