/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/teegg/Teegg.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <framework/datastore/StoreObjPtr.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TRandom3.h>

using namespace std;
using namespace Belle2;

extern "C" {

  //same convention as BHWIDE
  extern struct {
    double tp1[4];        /**< 4-momenta of incoming positron. */
    double tq1[4];        /**< 4-momenta of incoming electron. */
    double tp2[4];        /**< 4-momenta of outgoing positron. */
    double tq2[4];        /**< 4-momenta of outgoing electron. */
    double tphot[4][2];   /**< 4-momenta of real photons. */
    int tnphot;           /**< Number of photons. */
  } momset_;

//   results for cross section and photon multiplicity
  extern struct {
    int resngen;
    int resntrials;
    double reseff;
    double rescross;
    double rescrosserr;
    double avgq2;
  } teeggresults_;

//   extrainfo
  extern struct {
    double tt;
    double tw2;
    double tweight;
    double tvp2;
  } teeggextra_;


  /** Replace internal random generator with the framework random generator */
  void teegg_rndmarr_(double* drvec, int* lengt)
  {
    for (int i = 0; i < *lengt; ++i) {
      double rr = gRandom->Rndm();
      drvec[i] = rr;
    }
  }

  double teegg_rndm_(int*)
  {
    double r = gRandom->Rndm();
    return r;
  }

  /** FORTRAN routine based on main.f */
  void teeggm_(int* mode, double* xpar, int* npar);

  /** Callback to show warning */
  void teegg_warning_generic_(double* weight, double* max)
  {
    B2WARNING("TEEGG: Maximum weight " << *max  << " to small, increase fmax to at least " << *weight);
  }

  /** just defined to avoid undefined symbol in fortran code, should not be called */
  void report_() { B2FATAL("Teegg: report_() is not implemented"); }
  /** just defined to avoid undefined symbol in fortran code, should not be called */
  void golife_() { B2FATAL("Teegg: golife_() is not implemented"); }
  /** just defined to avoid undefined symbol in fortran code, should not be called */
  void golint_() { B2FATAL("Teegg: golint_() is not implemented"); }
}

Teegg::Teegg()
{
  for (int i = 0; i < 100; ++i) {
    m_npar[i] = 0;
    m_xpar[i] = 0.0;
  }

  setDefaultSettings();
}

Teegg::~Teegg()
{

}

void Teegg::setDefaultSettings()
{
  m_cmsEnergy = 5.28695 * 2.0 * Unit::GeV;

  m_pi              = 3.1415926535897932384626433832795029;
  m_conversionFactor = 0.389379660e6;
  m_alphaQED0        = 1.0 / 137.0359895;
  m_massElectron     = 0.51099906 * Unit::MeV;

  m_sVACPOL = "HLMNT";
  m_sRADCOR = "NONE";
  m_sCONFIG = "EGAMMA";
  m_sMATRIX = "BKM2";
  m_sMTRXGG = "EPADC";

  m_TEVETO = 0.1;
  m_TEMIN  = 20.0;// TMath::ACos(0.750);
  m_TGMIN  = 20.0;// TMath::ACos(0.750);
  m_TGVETO = 0.05;
  m_EEMIN  = 2.00;
  m_EGMIN  = 2.00;
  m_PEGMIN = m_pi / 4.00;
  m_EEVETO = 0.00;
  m_EGVETO = 0.00;
  m_PHVETO = m_pi / 4.00;
  m_CUTOFF = 0.25;
  m_EPS    = 0.01;
  m_FRAPHI = 0.00;
  m_EPSPHI = 1.0e-4;
  m_WGHT1M = 1.001;
  m_WGHTMX = 1.000;

  m_RADCOR    = -1;
  m_CONFIG    = -1;
  m_MATRIX    = -1;
  m_MTRXGG    = -1;
  m_VACPOL    = -1;
  m_UNWGHT    = 1;
  m_t         = 0.0;
  m_w2        = 1.0;
  m_weight    = 1.0;
  m_vp2 = 1.0;
}

void Teegg::initExtraInfo()
{
  StoreObjPtr<EventExtraInfo> eventextrainfo;
  eventextrainfo.registerInDataStore();
}

void Teegg::init()
{
  applySettings();
}


void Teegg::generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, TLorentzRotation boost)
{
  //Generate event
  int mode = 1;
  teeggm_(&mode, m_xpar, m_npar);

  //Store the final state fermions as real particle into the MCParticleGraph
  storeParticle(mcGraph, momset_.tq2, 11, vertex, boost, false, false);
  storeParticle(mcGraph, momset_.tp2, -11, vertex, boost, false, false);

  //Store the real photon(s) into the MCParticleGraph
  for (int iPhot = 0; iPhot <  momset_.tnphot; ++iPhot) {
    double photMom[4] = {momset_.tphot[0][iPhot], momset_.tphot[1][iPhot], momset_.tphot[2][iPhot], momset_.tphot[3][iPhot]};
    storeParticle(mcGraph, photMom, 22, vertex, boost, false, false);
  }

  // Get extra information
  m_t      = teeggextra_.tt;
  m_w2     = teeggextra_.tw2;
  m_weight = teeggextra_.tweight;
  m_vp2    = teeggextra_.tvp2;

  // Fill event weight
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  eventMetaDataPtr->setGeneratedWeight(m_weight);

  // Fill vacuum polarization correction
  StoreObjPtr<EventExtraInfo> eventExtraInfo;
  if (not eventExtraInfo.isValid())
    eventExtraInfo.create();
  if (eventExtraInfo->hasExtraInfo("GeneratorVP2")) {
    B2WARNING("EventExtraInfo with given name is already set! I won't set it again!");
  } else {
    eventExtraInfo->addExtraInfo("GeneratorVP2", m_vp2);
  }

}

void Teegg::term()
{
  int mode = 2;
  teeggm_(&mode, m_xpar, m_npar);

  B2RESULT("Cross-section (nb)  = " << teeggresults_.rescross / 1.e3 << " +/- " << teeggresults_.rescrosserr / 1.e3 << "");
  B2RESULT("Events (unweighted) = " << teeggresults_.resngen);
  B2RESULT("Trials              = " << teeggresults_.resntrials);
  B2RESULT("Efficiency          = " << 100.*teeggresults_.reseff << " %");
  B2RESULT("Average Q2          = " << teeggresults_.avgq2);

}

//=========================================================================
//                       Protected methods
//=========================================================================

void Teegg::applySettings()
{
  //--------------------
  // Integer parameters
  //--------------------
  m_npar[0] = m_UNWGHT; //producing strange results...

  if (m_sRADCOR == "NONE") m_RADCOR = 3; //DEFAULT
  else if (m_sRADCOR == "SOFT") m_RADCOR = 2;
  else if (m_sRADCOR == "HARD") m_RADCOR = 1;
  else m_sRADCOR = 3;
  m_npar[1] = m_RADCOR;

  if (m_sCONFIG == "EGAMMA") m_CONFIG = 11; //DEFAULT
  else if (m_sCONFIG == "ETRON") m_CONFIG = 12;
  else if (m_sCONFIG == "GAMMA") m_CONFIG = 13;
  else if (m_sCONFIG == "GAMMAE") m_CONFIG = 14;
  else m_CONFIG = 11;
  m_npar[2] = m_CONFIG;

  if (m_sMATRIX == "BK") m_MATRIX = 21;
  else if (m_sMATRIX == "BKM2") m_MATRIX = 22; //DEFAULT
  else if (m_sMATRIX == "TCHAN") m_MATRIX = 23;
  else if (m_sMATRIX == "EPA") m_MATRIX = 24;
  else m_MATRIX = 22;
  m_npar[3] = m_MATRIX;

  if (m_sMTRXGG == "EPADC") m_MTRXGG = 31; //DEFAULT
  else if (m_sMTRXGG == "BEEGG") m_MTRXGG = 32;
  else if (m_sMTRXGG == "MEEGG") m_MTRXGG = 33;
  else if (m_sMTRXGG == "HEEGG") m_MTRXGG = 34;
  else m_MTRXGG = 31;
  m_npar[4] = m_MTRXGG;

  if (m_sVACPOL == "OFF") m_VACPOL = 41;
  else if (m_sVACPOL == "HLMNT") m_VACPOL = 42; //DEFAULT
  else if (m_sVACPOL == "NSK") m_VACPOL = 43;
  else m_VACPOL = 42;
  m_npar[5] = m_VACPOL;

  //--------------------
  // Double parameters
  //--------------------
  double toRad = TMath::DegToRad();
  m_xpar[0]  = m_TEVETO * toRad;
  m_xpar[1]  = m_TEMIN * toRad;
  m_xpar[2]  = m_TGMIN * toRad;
  m_xpar[3]  = m_TGVETO * toRad;
  m_xpar[4]  = m_EEMIN;
  m_xpar[5]  = m_EGMIN;
  m_xpar[6]  = m_PEGMIN * toRad;
  m_xpar[7]  = m_EEVETO;
  m_xpar[8]  = m_EGVETO;
  m_xpar[9]  = m_PHVETO * toRad;
  m_xpar[10] = m_CUTOFF;
  m_xpar[11] = m_EPS;
  m_xpar[12] = m_FRAPHI;
  m_xpar[13] = m_EPSPHI;
  m_xpar[14] = m_WGHT1M;
  m_xpar[15] = m_WGHTMX;
  m_xpar[30] = m_cmsEnergy;

  int mode = -1; //use mode to control init/generation/finalize in FORTRAN code
  teeggm_(&mode, m_xpar, m_npar);
}


void Teegg::storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                          bool isVirtual, bool isInitial)
{
  // Create particle
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  if (isVirtual) {
    part.addStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.addStatus(MCParticle::c_Initial);
  }

  // All particles from a generator are primary
  part.addStatus(MCParticle::c_PrimaryParticle);

  // All particles from TEEGG are stable
  part.addStatus(MCParticle::c_StableInGenerator);

  // All gammas from TEEGG are ISR or FSR (impossible to distinguish due to IFI)
  if (pdg == 22) {
    part.addStatus(MCParticleGraph::GraphParticle::c_IsISRPhoton);
    part.addStatus(MCParticleGraph::GraphParticle::c_IsFSRPhoton);
  }

  part.setPDG(pdg);
  part.setFirstDaughter(0);
  part.setLastDaughter(0);
  part.setMomentum(TVector3(mom[0], mom[1], mom[2]));
  part.setMass(TDatabasePDG::Instance()->GetParticle(pdg)->Mass());
  part.setEnergy(mom[3]);

  //boost
  TLorentzVector p4 = part.get4Vector();
  p4.SetPz(-1.0 * p4.Pz()); //TEEGG uses other direction convention
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
