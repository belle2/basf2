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
#include <generators/dataobjects/MCParticleGraph.h>

#include <evtgen/EvtGenBase/EvtPDL.hh>


#include <string>
#include <fstream>

#include <TLorentzRotation.h>

#define nmxhep 4000
struct hepevt {
  int nevhep;               // serial number
  int nhep;                 // number of particles
  int isthep[nmxhep];       // status code
  int idhep[nmxhep];        // particle ident KF
  int jmohep[nmxhep][2];    // parent particles
  int jdahep[nmxhep][2];    // childreen particles
  double phep[nmxhep][5];   // four-momentum, mass [GeV]
  double vhep[nmxhep][4];   // vertex [mm]
};
struct pydat2_type {
  int KCHG[4][500];
  double PMAS[4][500];
  double PARF[2000];
  double VCKM[4][4];
};

extern hepevt hepevt_;
extern pydat2_type pydat2_;

extern "C" {

  void kk_init_(const char*, const char*, const char*, int*);
  void kk_init_seed_();
  void kk_term_(double*, double*);
  void kk_event_(int*);

  void kk_getbeam_(float*, float*, float*, float*,
                   float*, float*, float*, float*);
  void kk_putbeam_(float*, float*, float*, float*,
                   float*, float*, float*, float*);

  int pycomp_(int&);

}


namespace Belle2 {

  class KKGenInterface {

  public:

    //Define exceptions

    /**
     * Constructor.
     */
    KKGenInterface() {}

    /**
     * Destructor.
     */
    ~KKGenInterface() {
    }

    int setup(const std::string& KKdefaultFileName, const std::string& tauinputFileName, const std::string& taudecaytableFileName, const std::string& EvtPDLFileName, TLorentzVector P4_LER, TLorentzVector P4_HER);

    int simulateEvent(MCParticleGraph& graph);

  private:
    int addParticles2Graph(MCParticleGraph& graph);
    void updateGraphParticle(int, MCParticleGraph::GraphParticle* gParticle);
    bool getPythiaCharge(int, double&);
    int getPythiaSpinType(int);

  protected:
    EvtPDL* myevtpdl;
  };

}

#endif //KKGENINTERFACE_H
