/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/babayaganlo/BabayagaNLO.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TRandom3.h>

#include <generators/utilities/InitialParticleGeneration.h>

using namespace std;
using namespace Belle2;

extern "C" {

  extern struct {
    double bweight;       /**< Event weight. */
    double bp1[4];        /**< 4-momenta of incoming positron. */
    double bq1[4];        /**< 4-momenta of incoming electron. */
    double bp2[4];        /**< 4-momenta of outgoing positron. */
    double bq2[4];        /**< 4-momenta of outgoing electron. */
    double bphot[4][100]; /**< 4-momenta of real photons. */
    int bnphot;           /**< Number of photons. */
  } momset_;

  extern struct {
    double bprescale;      /**< Event prescale. */
  } prescale_;

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

  /** Add this because it's undefined in babayaga otherwise */
  double babayaganlo_getrandomcmsenergy_()
  {
    B2FATAL("babayaganlo_getrandomcmsenergy() is not implmented");
  }

  /** FORTRAN routine based on main.f */
//   void main_belle2_(int* mode, double* xpar, int* npar, double *ecm);
  void main_belle2_(int* mode, double* ecm, double* xpar, int* npar);

  /** VP data for novosibirsk routine */
  void babayaga_setvpolnsk_(const char* vpolnskfilename, size_t* length);

  /** Callback to show warning if weights are not sufficient --> bias*/
  void babayaganlo_warning_overweight_(double* weight, double* max)
  {
    B2WARNING("Babayaga.NLO: Maximum weight " << *max  << " to small, increase fmax to at least " << *weight);
  }

  /** Callback to show error if event is rejected*/
  void babayaganlo_error_rejection_(double* ratio)
  {
    B2ERROR("Babayaga.NLO: Event rejected! Ratio of cross section error increase too large: " << *ratio);
  }

  /** Callback to show error if weight is negative*/
  void babayaganlo_error_negative_(double* weight)
  {
    B2ERROR("Babayaga.NLO: Event has negative weight: " << *weight);
  }

  /** Callback results */
  void babayaganlo_result_nominalcmsenergy_(double* energy)
  {
    B2RESULT("Babayaga.NLO: Nominal CMS energy (GeV): " << *energy);
  }

  void babayaganlo_result_weightedxsec_(double* xsec, double* xsecerr)
  {
    B2RESULT("Babayaga.NLO: Weighted cross section (nb): " << *xsec << " +/- " << *xsecerr);
  }

  void babayaganlo_result_unweightedxsec_(double* xsec, double* xsecerr)
  {
    B2RESULT("Babayaga.NLO: Unweighted cross section (nb): " << *xsec << " +/- " << *xsecerr);
  }

  void babayaganlo_result_unweightedxsec_biascorrected_(double* xsec, double* xsecerr)
  {
    B2RESULT("Babayaga.NLO: Unweighted cross section, bias corrected (nb): " << *xsec << " +/- " << *xsecerr);
  }

  void babayaganlo_result_unweightedxsec_overweight_(double* xsec, double* xsecerr)
  {
    B2RESULT("Babayaga.NLO: Unweighted cross section overweight (nb): " << *xsec << " +/- " << *xsecerr);
  }

  void babayaganlo_result_unweightedxsec_underweight_(double* xsec, double* xsecerr)
  {
    B2RESULT("Babayaga.NLO: Unweighted cross section underweight (nb): " << -*xsec << " +/- " << *xsecerr);
  }

  void babayaganlo_result_hitormisseff_(double* eff)
  {
    B2RESULT("Babayaga.NLO: Hit or miss efficiency: " << *eff * 100. << " % ");
  }

  void babayaganlo_result_nover_(int* nover)
  {
    B2RESULT("Babayaga.NLO: Points with w > fmax (bias): " << *nover);
  }

  void babayaganlo_result_biashit_(double* biashit)
  {
    B2RESULT("Babayaga.NLO: Bias/hit: " << *biashit * 100. << " % ");
  }

  void babayaganlo_result_biashitpmiss_(double* biashitpmiss)
  {
    B2RESULT("Babayaga.NLO: Bias/(hit+missed): " << *biashitpmiss * 100. << " % ");
  }

  void babayaganlo_result_nneg_(int* nneg)
  {
    B2RESULT("Babayaga.NLO: Points with w > fmax (bias): " << *nneg);
  }

  void babayaganlo_result_biasneghit_(double* biasneghit)
  {
    B2RESULT("Babayaga.NLO: Neg. bias/hit: " << *biasneghit * 100. << " % ");
  }

  void babayaganlo_result_biasneghitmiss_(double* biasneghitmiss)
  {
    B2RESULT("Babayaga.NLO: Neg. bias/(hit+missed): " << *biasneghitmiss * 100. << " % ");
  }

  void babayaganlo_result_maxweight_(double* sdifmax, double* fmax)
  {
    B2RESULT("Babayaga.NLO: Maximum weight (sdifmax): " << *sdifmax << ", user maximum weight(fmax): " << *fmax);
  }

  void babayaganlo_result_vpmin_(double* xsec, double* xsecerr, double* frac)
  {
    B2RESULT("Babayaga.NLO: VP Uncertainty, minimum (nb): " << *xsec << " +/- " << *xsecerr << " (" << *frac << " %)");
  }

  void babayaganlo_result_vpcentral_(double* xsec, double* xsecerr)
  {
    B2RESULT("Babayaga.NLO: VP Uncertainty, central (nb): " << *xsec << " +/- " << *xsecerr);
  }

  void babayaganlo_result_vpmax_(double* xsec, double* xsecerr, double* frac)
  {
    B2RESULT("Babayaga.NLO: VP Uncertainty, maximum (nb): " << *xsec << " +/- " << *xsecerr << " ( " << *frac << " %)");
  }

  void babayaganlo_finishedmaxsearch_(double* fmax)
  {
    B2INFO("Babayaga.NLO: Finished maximum search: " << *fmax << ", starting now unweighted generation");
  }

  void babayaganlo_fatal_usercuts_()
  {
    B2FATAL("Babayaga.NLO: UserMode is selected but wrong user cuts are provided!");
  }

  void babayaganlo_fatal_usercutsfs_()
  {
    B2FATAL("Babayaga.NLO: UserMode is only possible for ee final states!");
  }

  void babayaganlo_fatal_usercutsprescale_()
  {
    B2FATAL("Babayaga.NLO: Prescale value must be larger than 0 (zero), hint: ~100 is reasonable!");
  }

  void babayaganlo_fatal_weightedprescale_()
  {
    B2FATAL("Babayaga.NLO: Generator prescale is not allowed with usermode = weighted");
  }

  void babayaganlo_result_weightsum_(double* wsum)
  {
    B2RESULT("Babayaga.NLO: Sum of weights: " << *wsum);
  }

  void babayaganlo_result_intlum_(double* lum, double* lumerr)
  {
    B2RESULT("Babayaga.NLO: Luminosity equivalent (using calc. xsec) (fb-1): " << *lum << " +/- " << *lumerr);
  }

  void babayaganlo_warning_prescaleweight_(double* weight)
  {
    B2WARNING("Babayaga.NLO: Prescale of less than one, increase prescale safety margin " << *weight);
  }

  void babayaganlo_error_isnan1_(double* phsp, double* w)
  {
    B2ERROR("Babayaga.NLO: phsp (" << *phsp << ") or w (" << *w << ") are NAN, skipping event (nan1 type)");
  }

  void babayaganlo_error_isnan2_(double* sdif)
  {
    B2ERROR("Babayaga.NLO: sdif (" << *sdif << ") is NAN, skipping event (nan2 type)");
  }



}

BabayagaNLO::BabayagaNLO()
{
  for (int i = 0; i < 100; ++i) {
    m_npar[i] = 0;
    m_xpar[i] = 0.0;
  }

  m_sDif = 0.;

  setDefaultSettings();
}

BabayagaNLO::~BabayagaNLO()
{

}

void BabayagaNLO::setDefaultSettings()
{
//   m_cmsEnergy = -1.;
  m_cmsEnergyNominal = -1.;

  m_applyBoost = true;

  m_finalState = "ee";
  m_vacPol     = "hlmnt";
  m_order      = "exp";
  m_model      = "matched";
  m_mode       = "unweighted";
  m_userMode   = "NONE";

  m_pi               = 3.1415926535897932384626433832795029;
  m_conversionFactor = 0.389379660e6;
  m_alphaQED0        = 1.0 / 137.0359895;
  m_massElectron     = 0.51099906 * Unit::MeV;
  m_massMuon         = 105.65836900 * Unit::MeV;
  m_massW            = 80.385 * Unit::GeV;
  m_massZ            = 91.1882 * Unit::GeV;
  m_widthZ           = 2.4952 * Unit::GeV;
  m_eMin             = 0.1 * Unit::GeV;
  m_epsilon          = 5.e-7;
  m_maxAcollinearity = 180.0;// * Unit::Deg;

  m_ScatteringAngleRange = make_pair(15.0, 165.0); //in [deg]
  m_ScatteringAngleRangePhoton = make_pair(15.0, 165.0); //in [deg] (unused in original babayaga)

  m_nPhot      = -1;
  m_nSearchMax = 50000;
  m_fMax       = -1.;
  m_EnergySpread = 5e-3;
  m_VPUncertainty = false;
  m_eemin = 0.0;
  m_temin = 0.0;
  m_egmin = 0.0;
  m_tgmin = 0.0;
  m_eeveto = 0.0;
  m_teveto = 0.0;
  m_egveto = 0.0;
  m_tgveto = 0.0;
  m_maxprescale = 1.0;
}

void BabayagaNLO::initExtraInfo()
{
  StoreObjPtr<EventExtraInfo> extrainfo;
  extrainfo.registerInDataStore();
}

void BabayagaNLO::init()
{
  applySettings();
}


void BabayagaNLO::generateEvent(MCParticleGraph& mcGraph, double ecm, TVector3 vertex, TLorentzRotation boost)
{
  //Generate event
  int mode = 1;
  main_belle2_(&mode, &ecm, m_xpar, m_npar);

  //Store the initial particles as virtual particles into the MCParticleGraph
  storeParticle(mcGraph, momset_.bq1, 11, vertex, boost, true);
  storeParticle(mcGraph, momset_.bp1, -11, vertex, boost, true);

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

  storeParticle(mcGraph, momset_.bq2, pdg, vertex, boost, false, false, false);
  storeParticle(mcGraph, momset_.bp2, antipdg, vertex, boost, false, false, false);

  //Store the real ISR and FSR photons into the MCParticleGraph
  for (int iPhot = 0; iPhot <  momset_.bnphot; ++iPhot) {
    double photMom[4] = {momset_.bphot[0][iPhot], momset_.bphot[1][iPhot], momset_.bphot[2][iPhot], momset_.bphot[3][iPhot]};
    storeParticle(mcGraph, photMom, 22, vertex, boost, false, false, true);
  }

  //set event weight
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  eventMetaDataPtr->setGeneratedWeight(momset_.bweight);

  //set event prescale
  StoreObjPtr<EventExtraInfo> eventExtraInfo;
  if (not eventExtraInfo.isValid())
    eventExtraInfo.create();
  if (eventExtraInfo->hasExtraInfo("GeneratedPrescale")) {
    B2WARNING("EventExtraInfo with given name is already set! I won't set it again!");
  } else {
    float targetValue = prescale_.bprescale;
    eventExtraInfo->addExtraInfo("GeneratedPrescale", targetValue);
  }

}


void BabayagaNLO::term()
{

  B2RESULT("Babayaga.NLO: Final state:    " << m_finalState);
  B2RESULT("Babayaga.NLO: Mode:           " << m_mode);
  B2RESULT("Babayaga.NLO: Order:          " << m_order);
  B2RESULT("Babayaga.NLO: Model:          " << m_model);
  B2RESULT("Babayaga.NLO: Vac. pol. (VP): " << m_vacPol);
  B2RESULT("Babayaga.NLO: Usercuts:       " << m_userMode);

  // all other results are displayed from fortran using a callback to C extern logging

  int mode = 2;
  double ecm = -1.;
  main_belle2_(&mode, &ecm, m_xpar, m_npar);

}

//=========================================================================
//                       Protected methods
//=========================================================================

void BabayagaNLO::applySettings()
{
  //--------------------
  // Integer parameters
  //--------------------
  m_npar[0] = m_nPhot;
  m_npar[1] = m_nSearchMax;
  if (m_VPUncertainty) m_npar[5] = 1;
  else m_npar[5] = 0;

  //--------------------
  // Double parameters
  //--------------------
  m_xpar[0]  = m_cmsEnergyNominal;
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
  m_xpar[40] = m_EnergySpread; // only for some internal checks, not the actual smaering!

  //user cuts
  m_xpar[50] = m_eemin;
  m_xpar[51] = m_temin;
  m_xpar[52] = m_egmin;
  m_xpar[53] = m_tgmin;
  m_xpar[54] = m_eeveto;
  m_xpar[55] = m_teveto;
  m_xpar[56] = m_egveto;
  m_xpar[57] = m_tgveto;
  m_xpar[58] = m_maxprescale;

  //--------------------
  // string parameters, wrapped to integers
  //--------------------
  if (m_finalState == "ee") m_npar[20] = 1;
  else if (m_finalState == "gg") m_npar[20] = 2;
  else if (m_finalState == "mm") m_npar[20] = 3;
  else B2FATAL("Invalid final state: " << m_finalState);

  if (m_model == "matched") m_npar[21] = 1;
  else if (m_model == "ps") m_npar[21] = 2;
  else B2FATAL("Invalid matching model: " << m_model);

  if (m_order == "born") m_npar[22] = 1;
  else if (m_order == "alpha") m_npar[22] = 2;
  else if (m_order == "exp") m_npar[22] = 3;
  else B2FATAL("Invalid QED order: " << m_order);

  if (m_vacPol == "off") m_npar[23] = 1;
  else if (m_vacPol == "hadr5") m_npar[23] = 2;
  else if (m_vacPol == "hlmnt") m_npar[23] = 3;
  else B2FATAL("Invalid vacuum polarization code: " << m_vacPol);

  if (m_mode == "unweighted" || m_mode == "uw") m_npar[24] = 1;
  else if (m_mode == "weighted" || m_mode == "w") m_npar[24] = 2;
  else B2FATAL("Invalid mode: " << m_mode);

  if (m_userMode == "NONE") m_npar[25] = 1;
  else if (m_userMode == "GAMMA") m_npar[25] = 2;
  else if (m_userMode == "EGAMMA") m_npar[25] = 3;
  else if (m_userMode == "ETRON") m_npar[25] = 4;
  else if (m_userMode == "PRESCALE") m_npar[25] = 5;
  else B2FATAL("Invalid user mode: " << m_userMode);

  // set the data file for the novosibirsk routine (not supported yet, but needs to be set)
  size_t fileLength = m_NSKDataFile.size();
  babayaga_setvpolnsk_(m_NSKDataFile.c_str(), &fileLength);

  //use mode to control init/generation/finalize in FORTRAN code
  int mode = -1;
  double ecm = -1.;
  main_belle2_(&mode, &ecm, m_xpar, m_npar);
}


void BabayagaNLO::storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                                bool isVirtual, bool isInitial, bool isISRFSR)
{

//   Create particle
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  if (isVirtual) {
    part.setStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.setStatus(MCParticle::c_Initial);
  }

  // all particle of a generator are primary
  part.addStatus(MCParticle::c_PrimaryParticle);

  // all particles produced by BABAYAGA are stable
  part.addStatus(MCParticle::c_StableInGenerator);

  // add ISR/FSR flags to all photons that are not the primary gg pair
  if (isISRFSR) {
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
