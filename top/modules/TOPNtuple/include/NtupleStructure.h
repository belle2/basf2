/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPNTUPLESTRUCTURE_H
#define TOPNTUPLESTRUCTURE_H

#include <TTree.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Structure for TOPLikelihood members
     */
    struct Likelihoods {
      Float_t e;   /**< for electron */
      Float_t mu;  /**< for muon */
      Float_t pi;  /**< for pion */
      Float_t K;   /**< for kaon */
      Float_t p;   /**< for proton */

      /**
       * Default constructor
       */
      Likelihoods(): e(0), mu(0), pi(0), K(0), p(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear() {
        e = mu = pi = K = p = 0;
      }
    };


    /**
     * Structure for track parameters at TOP
     */
    struct TrackHit {
      Int_t barID;  /**< bar ID */
      Int_t PDG;    /**< PDG code */
      Float_t x;    /**< impact point, x component */
      Float_t y;    /**< impact point, y component */
      Float_t z;    /**< impact point, z component */
      Float_t p;     /**< momentum magnitude */
      Float_t theta; /**< momentum polar angle */
      Float_t phi;   /**< momentum azimuthal angle */
      Float_t time;  /**< impact time */

      /**
       * Default constructor
       */
      TrackHit(): barID(0), PDG(0), x(0), y(0), z(0), p(0), theta(0), phi(0), time(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear() {
        barID = PDG = 0;
        x = y = z = p = theta = phi = time = 0;
      }

    };


    /**
     * Structure of a flat ntuple
     */
    struct TOPTree {
      Int_t evt; /**< event number */
      Int_t run; /**< run number */

      Float_t p;      /**< momentum magnitude of Track */
      Float_t cth;    /**< cosine of polar angle of Track */
      Float_t phi;    /**< azimuthal angle of Track */
      Float_t pValue; /**< p-value of Track fit */

      Int_t PDG;       /**< PDG code of related MCParticle */
      Int_t motherPDG; /**< PDG code of related mother MCParticle */
      Short_t primary; /**< is a primary particle (from related MCParticle) */
      Short_t seen;    /**< is seen in TOP (from related MCParticle) */
      Float_t rhoProd; /**< production vertex (cylindrical coordinate r) of MCParticle */
      Float_t zProd;   /**< production vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiProd; /**< production vertex (cylindrical coordinate phi) of MCParticle */
      Float_t rhoDec;  /**< decay vertex (cylindrical coordinate r) of MCParticle */
      Float_t zDec;    /**< decay vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiDec;  /**< decay vertex (cylindrical coordinate phi) of MCParticle */

      Int_t numPhot;      /**< number of detected photons */
      Likelihoods phot;   /**< number of expected photons */
      Likelihoods logL;   /**< log likelihoods */

      TrackHit extHit;  /**< extrapolated Track hit (in local bar frame) */
      TrackHit barHit;  /**< related MC particle hit (in local bar frame) */

      /**
       * Default constructor
       */
      TOPTree(): evt(0), run(0), p(0), cth(0), phi(0), pValue(0), PDG(0), motherPDG(0),
        primary(0), seen(0), rhoProd(0), zProd(0), phiProd(0), rhoDec(0), zDec(0),
        phiDec(0), numPhot(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear() {
        evt = 0;
        run = 0;

        p = 0;
        cth = 0;
        phi = 0;
        pValue = 0;

        PDG = 0;
        motherPDG = 0;
        primary = 0;
        seen = 0;
        rhoProd = 0;
        zProd = 0;
        phiProd = 0;
        rhoDec = 0;
        zDec = 0;
        phiDec = 0;

        numPhot = 0;
        phot.clear();
        logL.clear();

        extHit.clear();
        barHit.clear();
      }
    };

  } // TOP namestace
} // Belle2 namespace

#endif

