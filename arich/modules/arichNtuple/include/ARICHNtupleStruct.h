/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      Float_t e;   /**< for electron */
      Float_t mu;  /**< for muon */
      Float_t pi;  /**< for pion */
      Float_t K;   /**< for kaon */
      Float_t p;   /**< for proton */
      Float_t d;   /**< for deuteron */

      /**
       * Default constructor
       */
      ParticlesArray(): e(0), mu(0), pi(0), K(0), p(0), d(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        e = mu = pi = K = p = d = 0;
      }
    };


    /**
     * Structure for track parameters at ARICH
     */
    struct TrackHit {
      Int_t PDG;    /**< PDG code */
      Float_t x;    /**< impact point, x component */
      Float_t y;    /**< impact point, y component */
      Float_t z;    /**< impact point, z component */
      Float_t p;     /**< momentum magnitude */
      Float_t theta; /**< momentum polar angle */
      Float_t phi;   /**< momentum azimuthal angle */

      /**
       * Default constructor
       */
      TrackHit(): PDG(0), x(0), y(0), z(0), p(0), theta(0), phi(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        PDG = 0;
        x = y = z = p = theta = phi = 0;
      }

    };


    /**
     * Structure of a flat ntuple
     */
    struct ARICHTree {
      Int_t evt; /**< event number */
      Int_t run; /**< run number */
      Int_t exp; /**< exp number */

      Short_t charge;    /**< charge */
      Float_t pValue; /**< p-value of Track fit */
      Float_t z0;     /**< track z0 */
      Float_t d0;     /**< track d0 */
#ifdef ALIGNMENT_USING_BHABHA
      Float_t eop;    /**< E/p for bhabha */
      Float_t e9e21;    /**< E9/E21 for bhabha */
      Float_t etot;    /**< total energy of ECL clusters */
#endif

      Int_t PDG;       /**< PDG code of related MCParticle */
      Int_t motherPDG; /**< PDG code of related mother MCParticle */
      Int_t status;    /**< track status (add proper description)*/
      Short_t primary; /**< is a primary particle (from related MCParticle) */
      Short_t seen;    /**< is seen in ARICH (from related MCParticle) */
      Float_t rhoProd; /**< production vertex (cylindrical coordinate r) of MCParticle */
      Float_t zProd;   /**< production vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiProd; /**< production vertex (cylindrical coordinate phi) of MCParticle */
      Float_t rhoDec;  /**< decay vertex (cylindrical coordinate r) of MCParticle */
      Float_t zDec;    /**< decay vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiDec;  /**< decay vertex (cylindrical coordinate phi) of MCParticle */
      Int_t scatter;     /**< 1 if particle scattered (i.e. has daughter with same PDG) */

      Int_t detPhot;      /**< number of detected photons */
      Int_t nCDC;      /**< number of track CDC hits */
      Bool_t inAcc;     /**< track in detector acceptance, i.e. > 0 expected photons for electron ring */
      ParticlesArray numBkg;       /**< number of expected background photons */
      ParticlesArray expPhot;      /**< number of expected photons (signal + bkg) */
      ParticlesArray logL;   /**< log likelihoods */


      TrackHit recHit;  /**< extrapolated Track hit */
      TrackHit mcHit;  /**< related MC particle hit */
      Int_t nRec; /**< number of reconstructed photons */
      std::vector<Belle2::ARICHPhoton>  photons; /** vector of reconstructed photons */
      Float_t winHit[2];                         /** track hit on hapd window (x,y coordinates) */
      Int_t trgtype; /**< event trigger type */



      /**
       * Default constructor
       */
      ARICHTree(): evt(0), run(0), exp(0), charge(0), pValue(0), z0(0), d0(0), PDG(0), motherPDG(0),
#ifdef ALIGNMENT_USING_BHABHA
        eop(0), e9e21(0), etot(0),
#endif
        status(0), primary(0), seen(0), rhoProd(0), zProd(0), phiProd(0), rhoDec(0), zDec(0),
        phiDec(0), scatter(0), nRec(0), nCDC(0), inAcc(0)
      {

      }

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        evt = 0;
        run = 0;
        exp = 0;
        trgtype = 0;

        pValue = 0;
        z0 = 0;
        d0 = 0;
        charge = 0;

#ifdef ALIGNMENT_USING_BHABHA
        eop = 0;
        e9e21 = 0;
        etot = 0;
#endif

        PDG = 0;
        motherPDG = 0;
        status = 0;
        primary = 0;
        seen = 0;
        rhoProd = 0;
        zProd = 0;
        phiProd = 0;
        rhoDec = 0;
        zDec = 0;
        phiDec = 0;
        scatter = 0;
        nRec = 0;
        detPhot = 0;
        nCDC = 0;
        inAcc = 0;

        numBkg.clear();
        expPhot.clear();
        logL.clear();

        recHit.clear();
        mcHit.clear();
        photons.clear();

        winHit[0] = 0.; winHit[1] = 0.;
      }
    };

  } // ARICH namestace
} // Belle2 namespace


