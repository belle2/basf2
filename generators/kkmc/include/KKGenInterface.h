/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <mdst/dataobjects/MCParticleGraph.h>

/* C++ headers. */
#include <string>
#include <unordered_map>

#define nmxhep 4000

/**
 *HEPEVT common block of PYTHIA6.
 */
struct hepevt_type {
  int nevhep;               /**< serial number. */
  int nhep;                 /**< number of particles. */
  int isthep[nmxhep];       /**< status code. */
  int idhep[nmxhep];        /**< particle ident KF. */
  int jmohep[nmxhep][2];    /**< parent particles. */
  int jdahep[nmxhep][2];    /**< childreen particles. */
  double phep[nmxhep][5];   /**< four-momentum, mass [GeV]. */
  double vhep[nmxhep][4];   /**<  vertex [mm]. */
};

/**
 * PYDAT2 common block of PYTHIA6.
 */
struct pydat2_type {
  int KCHG[4][500];  /**< particle information such as spin, charge... */
  double PMAS[4][500]; /**< particle information such as mass, width... */
  double PARF[2000]; /**< parametrization of dd-uu-ss flavor mixing. */
  double VCKM[4][4]; /**< squared CKM matrix elements. */
};

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

  /**
   * Interface class for using the KKMC generator.
   */
  class KKGenInterface {

  public:

    /**
     * Constructor.
     */
    KKGenInterface() {}

    /**
     * Destructor.
     */
    ~KKGenInterface() {}

    /**
     * Copy constructor, explicitly deleted.
     */
    KKGenInterface(const KKGenInterface& m) = delete;

    /**
     * Assignment operator, explicitly deleted.
     */
    KKGenInterface& operator= (const KKGenInterface& m) = delete;

    /**
     * Setup for KKMC and TAUOLA.
     */
    int setup(const std::string& KKdefaultFileName, const std::string& tauinputFileName, const std::string& taudecaytableFileName,
              const std::string& KKMCOutputFileName);

    /**
     * Setup for beams information.
     */
    void set_beam_info(ROOT::Math::PxPyPzEVector P4_LER, double Espread_LER, ROOT::Math::PxPyPzEVector P4_HER,
                       double Espread_HER);

    /**
     * Simulate the events.
     */
    int simulateEvent(MCParticleGraph& graph, TVector3 vertex);

    /**
     * Terminate the generator.
     */
    void term();

  private:

    /**
     * Add particles to the MCParticleGraph.
     */
    int addParticles2Graph(MCParticleGraph& graph, TVector3 vertex);

    /**
     * Update the MCParticleGraph.
     */
    void updateGraphParticle(int, MCParticleGraph::GraphParticle* gParticle,
                             TVector3 vertex);

    /**
     * Map between PYTHIA id and PDG codes.
     */
    std::unordered_map<int, int> m_mapPythiaIDtoPDG;

  };

}
