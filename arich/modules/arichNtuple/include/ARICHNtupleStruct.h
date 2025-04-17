/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "arich/dataobjects/ARICHPhoton.h"

//#define ALIGNMENT_USING_BHABHA

namespace Belle2 {
  namespace ARICH {

    /**
     * Structure for particle hypothesis dependent arrays
     */
    struct ParticlesArray {
      Float_t e{0};   /**< for electron */
      Float_t mu{0};  /**< for muon */
      Float_t pi{0};  /**< for pion */
      Float_t K{0};   /**< for kaon */
      Float_t p{0};   /**< for proton */
      Float_t d{0};   /**< for deuteron */

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        *this = ParticlesArray();
      }
    };


    /**
     * Structure for track parameters at ARICH
     */
    struct TrackHit {
      Int_t PDG{0};    /**< PDG code */
      Float_t x{0};    /**< impact point, x component */
      Float_t y{0};    /**< impact point, y component */
      Float_t z{0};    /**< impact point, z component */
      Float_t p{0};     /**< momentum magnitude */
      Float_t theta{0}; /**< momentum polar angle */
      Float_t phi{0};   /**< momentum azimuthal angle */

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        *this = TrackHit();
      }

    };


    /**
     * Structure of a flat ntuple.
     */
    struct ARICHTree {

      /** Event number. */
      Int_t evt{0};

      /** Run number. */
      Int_t run{0};

      /** Experiment number. */
      Int_t exp{0};

      /** Charge. */
      Short_t charge{0};

      /** p-value of Track fit. */
      Float_t pValue{0};

      /** Track z0. */
      Float_t z0{0};

      /** Track d0. */
      Float_t d0{0};

#ifdef ALIGNMENT_USING_BHABHA

      /** E/p for bhabha. */
      Float_t eop {0};

      /** E9/E21 for bhabha. */
      Float_t e9e21{0};

      /** Total energy of ECL clusters. */
      Float_t etot{0};

#endif

      /** PDG code of related MCParticle. */
      Int_t PDG{0};

      /** PDG code of related mother MCParticle. */
      Int_t motherPDG{0};

      /** Track status (add proper description). */
      Int_t status{0};

      /** Is a primary particle (from related MCParticle). */
      Short_t primary{0};

      /** Is seen in ARICH (from related MCParticle). */
      Short_t seen{0};

      /** Production vertex (cylindrical coordinate r) of MCParticle. */
      Float_t rhoProd{0};

      /** Production vertex (cylindrical coordinate z) of MCParticle. */
      Float_t zProd{0};

      /** Production vertex (cylindrical coordinate phi) of MCParticle. */
      Float_t phiProd{0};

      /** Decay vertex (cylindrical coordinate r) of MCParticle. */
      Float_t rhoDec{0};

      /** Decay vertex (cylindrical coordinate z) of MCParticle. */
      Float_t zDec{0};

      /** Decay vertex (cylindrical coordinate phi) of MCParticle. */
      Float_t phiDec{0};

      /** 1 if particle scattered (i.e. has daughter with same PDG). */
      Int_t scatter{0};

      /** Number of detected photons. */
      Int_t detPhot{0};

      /** Number of track CDC hits. */
      Int_t nCDC{0};

      /**
       * Track in detector acceptance, i.e. > 0 expected photons
       * for electron ring.
       */
      Bool_t inAcc{0};

      /** Number of expected background photons. */
      ParticlesArray numBkg{0};

      /** Number of expected photons (signal + bkg). */
      ParticlesArray expPhot{0};

      /** Log likelihoods. */
      ParticlesArray logL{0};


      /** Extrapolated Track hit. */
      TrackHit recHit{0};

      /** Related MC particle hit. */
      TrackHit mcHit{0};

      /** Number of reconstructed photons. */
      Int_t nRec{0};

      /** Vector of reconstructed photons. */
      std::vector<Belle2::ARICHPhoton> photons;

      /** Track hit on hapd window (x,y coordinates). */
      Float_t winHit[2] {0};

      /** Event trigger type. */
      Int_t trgtype{0};

      /**
       * Clear the structure: set elements to zero.
       */
      void clear()
      {
        *this = ARICHTree();
      }

    };

  } // ARICH namestace
} // Belle2 namespace
