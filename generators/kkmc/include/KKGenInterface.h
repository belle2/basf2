/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KKGENINTERFACE_H
#define KKGENINTERFACE_H

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <evtgen/EvtGenBase/EvtPDL.hh>

#include <string>
#include <fstream>
#include <string.h>

#include <TLorentzRotation.h>

#define nmxhep 4000
struct hepevt_type {
  int nevhep;               /**< serial number. */
  int nhep;                 /**< number of particles. */
  int isthep[nmxhep];       /**< status code. */
  int idhep[nmxhep];        /**< particle ident KF. */
  int jmohep[nmxhep][2];    /**< parent particles. */
  int jdahep[nmxhep][2];    /**< childreen particles. */
  double phep[nmxhep][5];   /**< four-momentum, mass [GeV]. */
  double vhep[nmxhep][4];   /**<  vertex [mm]. */
}; /**< /HEPEVT/ common block of PYTHIA6. */

struct pydat2_type {
  int KCHG[4][500];  /**< particle information such as spin, charge... */
  double PMAS[4][500]; /**< particle information such as mass, width... */
  double PARF[2000]; /**< parametrization of dd-uu-ss flavor mixing. */
  double VCKM[4][4]; /**< squared CKM matrix elements. */
}; /**< /PYDAT2/ common block of PYTHIA6. */

extern hepevt_type hepevt_;
extern pydat2_type pydat2_;

extern "C" {

  void kk_init_(const char*, const char*, const char*, int*, const char*);
  void kk_begin_run_(double*);
  void kk_init_seed_();
  void kk_term_(double*, double*);
  void kk_event_(int*);
  void kk_getbeam_(double*, double*, double*, double*,
                   double*, double*, double*, double*);
  void kk_putbeam_(double*, double*, double*, double*,
                   double*, double*, double*, double*);
  int pycomp_(int&);

}

namespace Belle2 {

  //! Module for using KKMC generator
  class KKGenInterface {

  public:

    //Define exceptions

    /**
     * Constructor.
     */
    KKGenInterface();

    /**
     * Destructor.
     */
    ~KKGenInterface() {}

    /**
     * Copy constructor (for cppcheck).
     */
    KKGenInterface(const KKGenInterface& m);

    /**
     * Assignment operator (for cppcheck).
     */
    KKGenInterface& operator= (const KKGenInterface& m);

    int setup(const std::string& KKdefaultFileName, const std::string& tauinputFileName, const std::string& taudecaytableFileName,
              const std::string& EvtPDLFileName, const std::string& KKMCOutputFileName); /**< Setup for KKMC/TAUOLA  */

    void set_beam_info(TLorentzVector P4_LER, double Espread_LER, TLorentzVector P4_HER,
                       double Espread_HER); /**< Setup for beam inforamtion to KKMC */

    int simulateEvent(MCParticleGraph& graph, TVector3 vertex); /**< MC simulation function */

    /**
    * Terminates the generator.
    */
    void term();


  private:
    int addParticles2Graph(MCParticleGraph& graph, TVector3 vertex);  /**< Function to add particle decays */
    void updateGraphParticle(int, MCParticleGraph::GraphParticle* gParticle,
                             TVector3 vertex);  /**< Function to update particle decays */
    // bool getPythiaCharge(int, double&); /**< Function to get charge from Pythia ID. */
    // int getPythiaSpinType(int); /**< Function to get SpinType from Pythia ID. */

  protected:
    EvtPDL* myevtpdl; /**<Variable needed for EvtPDL (just for translation from PYTHIA6 KF to PDG id). */
  };

}

#endif //KKGENINTERFACE_H
