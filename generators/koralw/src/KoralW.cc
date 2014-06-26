/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/koralw/KoralW.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

using namespace std;
using namespace Belle2;

//Declaration of the KoralW FORTRAN methods and common blocks.
extern"C" {

  extern struct {
    double q1[4];
    double q2[4];
    double p1[4];
    double p2[4];
    double p3[4];
    double p4[4];
  } momdec_;

  extern struct {
    double qeff1[4];
    double qeff2[4];
    double sphum[4];
    double sphot[4][100];
    int nphot;
  } momset_;

  extern struct {
    int nevhep;          /**< serial number. */
    int nhep;            /**< number of particles. */
    int isthep[2000];    /**< status code. */
    int idhep[2000];     /**< particle ident KF. */
    int jmohep[2000][2]; /**< parent particles. */
    int jdahep[2000][2]; /**< children particles. */
    float phep[2000][5]; /**< four-momentum, mass [GeV]. */
    float vhep[2000][4]; /**< vertex [mm]. */
  } hepevt_;


  void kw_setdatapath_(const char* filename, size_t* length);
  void kw_readatax_(const char* filename, size_t* length, int* reset, int* max, double* xpar);
  void kw_initialize_(double* xpar);
  void marini_(int* mar1, int* mar2, int* mar3);
  void rmarin_(int* mar1, int* mar2, int* mar3);
  void kw_make_();
  void kw_finalize_();
  void kw_getmomdec_(double* p1, double* p2, double* p3, double* p4);
  void kw_getxsecmc_(double* xSecMC, double* xErrMC);
}


void KoralW::init(const std::string& dataPath, const std::string& userDataFile, int randomSeed)
{
  if (dataPath.empty()) B2FATAL("KoralW: The specified data path is empty !")
    if (userDataFile.empty()) B2FATAL("KoralW: The specified user data file is empty !")
      if ((randomSeed < 0) || (randomSeed > 900000000)) B2FATAL("The random seed has to be in the range 0 .. 900 OOO OOO !")

        //Make sure the dataPath ends with an "/"
        string dataPathNew = dataPath;
  if (dataPath[dataPath.length() - 1] != '/') dataPathNew += "/";

  //Set the path to the data files
  size_t pathLength = dataPathNew.size();
  kw_setdatapath_(dataPathNew.c_str(), &pathLength);

  //Load the default KoralW input data
  int numXPar = NUM_XPAR;
  int reset = 1;
  const string defaultDataFile = dataPathNew + "KoralW_Default.data";
  pathLength = defaultDataFile.size();
  kw_readatax_(defaultDataFile.c_str(), &pathLength, &reset, &numXPar, m_xpar);

  //Load the user KoralW input data
  reset = 0;
  pathLength = userDataFile.size();
  kw_readatax_(userDataFile.c_str(), &pathLength, &reset, &numXPar, m_xpar);

  //Initialize KoralW
  kw_initialize_(m_xpar);

  //Initialize random number generator
  int mar1 = randomSeed;
  int mar2 = 7345;
  int mar3 = 239;
  marini_(&mar1, &mar2, &mar3);
  rmarin_(&mar1, &mar2, &mar3);
}


void KoralW::generateEvent(MCParticleGraph& mcGraph)
{
  kw_make_();

  //Store the particles with status id 3 and 2 as virtual particles, the particles with status id 1 as real particles
  for (int iPart = 0; iPart < hepevt_.nhep; ++iPart) {
    if (hepevt_.isthep[iPart] > 1) {
      storeParticle(mcGraph, hepevt_.phep[iPart], hepevt_.vhep[iPart], hepevt_.idhep[iPart], true);
    } else {
      storeParticle(mcGraph, hepevt_.phep[iPart], hepevt_.vhep[iPart], hepevt_.idhep[iPart]);
    }
  }
}


void KoralW::term()
{
  kw_finalize_();

  //Get the cross section
  kw_getxsecmc_(&m_crossSection, &m_crossSectionError);
}



//=========================================================================
//                       Protected methods
//=========================================================================

void KoralW::storeParticle(MCParticleGraph& mcGraph, const float* mom, const float* vtx, int pdg, bool isVirtual, bool isInitial)
{
  // //Create particle
  //MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  //if (!isVirtual) {
  //  part.setStatus(MCParticle::c_PrimaryParticle);
  //} else {
  //  part.setStatus(MCParticle::c_IsVirtual);
  //}

  //  RG 6/25/14 Add new flag for ISR "c_Initial"
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
  part.setEnergy(mom[3]);
  part.setMass(mom[4]);
  part.setDecayVertex(vtx[0]*Unit::mm, vtx[1]*Unit::mm, vtx[2]*Unit::mm);

  //If boosting is enable boost the particles to the lab frame
  TLorentzVector p4 = part.get4Vector();
  //p4.SetPz(-1.0*p4.Pz());
  if (m_applyBoost) p4 = m_boostVector * p4;
  part.set4Vector(p4);
}
