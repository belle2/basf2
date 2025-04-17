/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Rtypes.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Structure for TOPLikelihood members
     */
    struct Likelihoods {
      Float_t e = 0;   /**< for electron */
      Float_t mu = 0;  /**< for muon */
      Float_t pi = 0;  /**< for pion */
      Float_t K = 0;   /**< for kaon */
      Float_t p = 0;   /**< for proton */
      Float_t d = 0;   /**< for deuteron */

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        e = mu = pi = K = p = d = 0;
      }
    };


    /**
     * Structure for track parameters at TOP
     */
    struct TrackHit {
      Int_t moduleID;  /**< module ID */
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
      TrackHit(): moduleID(0), PDG(0), x(0), y(0), z(0), p(0), theta(0), phi(0), time(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        moduleID = PDG = 0;
        x = y = z = p = theta = phi = time = 0;
      }

    };


    /**
     * Structure of a flat ntuple
     */
    struct TOPTree {
      Int_t evt = 0; /**< event number */
      Int_t run = 0; /**< run number */

      Float_t p = 0;      /**< momentum magnitude of Track */
      Float_t cth = 0;    /**< cosine of polar angle of Track */
      Float_t phi = 0;    /**< azimuthal angle of Track */
      Float_t pValue = 0; /**< p-value of Track fit */

      Int_t PDG = 0;       /**< PDG code of related MCParticle */
      Int_t motherPDG = 0; /**< PDG code of related mother MCParticle */
      Short_t primary = 0; /**< is a primary particle (from related MCParticle) */
      Short_t seen = 0;    /**< is seen in TOP (from related MCParticle) */
      Float_t rhoProd = 0; /**< production vertex (cylindrical coordinate r) of MCParticle */
      Float_t zProd = 0;   /**< production vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiProd = 0; /**< production vertex (cylindrical coordinate phi) of MCParticle */
      Float_t rhoDec = 0;  /**< decay vertex (cylindrical coordinate r) of MCParticle */
      Float_t zDec = 0;    /**< decay vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiDec = 0;  /**< decay vertex (cylindrical coordinate phi) of MCParticle */
      Int_t yieldMC = 0;   /**< signal yield MC truth */

      Int_t numPhot = 0;      /**< number of detected photons */
      Float_t numBkg = 0;     /**< number of expected background photons */
      Int_t moduleID = 0;     /**< module ID from TOPLikelihoods */
      Likelihoods phot;   /**< number of expected photons (signal + bkg) */
      Likelihoods yield;   /**< effective signal yields by sPlot */
      Likelihoods logL;   /**< log likelihoods */

      TrackHit extHit;  /**< extrapolated Track hit (in local module frame) */
      TrackHit barHit;  /**< related MC particle hit (in local module frame) */

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
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
        yieldMC = 0;

        numPhot = 0;
        numBkg = 0;
        moduleID = 0;
        phot.clear();
        yield.clear();
        logL.clear();

        extHit.clear();
        barHit.clear();
      }
    };

  } // TOP namestace
} // Belle2 namespace


