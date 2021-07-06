/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TTree.h>

namespace Belle2 {
  namespace PID {

    /**
     * Structure for the output of PID log likelihoods to a flat ntuple
     */
    struct LogLikelihoods {
      Float_t le;   /**< log likelihood for electron */
      Float_t lmu;  /**< log likelihood for muon */
      Float_t lpi;  /**< log likelihood for pion */
      Float_t lk;   /**< log likelihood for kaon */
      Float_t lp;   /**< log likelihood for proton */
      Short_t flag; /**< flag: information is available (1) or not (0) */
      Short_t seen; /**< is seen in this component (from related MCParticle) */

      /**
       * Default constructor
       */
      LogLikelihoods(): le(0), lmu(0), lpi(0), lk(0), lp(0), flag(0), seen(0)
      {}

      /**
       * Clear the structure: set elements to zero
       */
      void clear()
      {
        le = 0;
        lmu = 0;
        lpi = 0;
        lk = 0;
        lp = 0;
        flag = 0;
        seen = 0;
      }
    }; // struct LogLikelihoods

    /**
     * Structure for the output of PID log likelihoods + tracking info to a flat ntuple
     */
    struct PIDTree {
      Int_t evt; /**< event number */
      Int_t run; /**< run number */

      Float_t p;   /**< momentum magnitude of Track */
      Float_t cth; /**< cosine of polar angle of Track */
      Float_t phi; /**< azimuthal angle of Track */
      Float_t pValue; /**< p-value of Track fit */

      Int_t PDG;        /**< PDG code of related MCParticle */
      Int_t motherPDG;  /**< PDG code of related mother MCParticle */
      Short_t primary; /**< is a primary particle (from related MCParticle) */
      Float_t rhoProd; /**< production vertex (cylindrical coordinate r) of MCParticle */
      Float_t zProd;   /**< production vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiProd; /**< production vertex (cylindrical coordinate phi) of MCParticle */
      Float_t rhoDec;  /**< decay vertex (cylindrical coordinate r) of MCParticle */
      Float_t zDec;    /**< decay vertex (cylindrical coordinate z) of MCParticle */
      Float_t phiDec;  /**< decay vertex (cylindrical coordinate phi) of MCParticle */

      LogLikelihoods cdcdedx;  /**< log likelihoods from CDC dE/dx */
      LogLikelihoods svddedx;  /**< log likelihoods from SVD dE/dx */
      LogLikelihoods top;   /**< log likelihoods from TOP */
      LogLikelihoods arich; /**< log likelihoods from ARICH */
      LogLikelihoods ecl;   /**< log likelihoods from ECL */
      LogLikelihoods klm;   /**< log likelihoods from KLM */

      /**
       * Default constructor
       */
      PIDTree(): evt(0), run(0), p(0), cth(0), phi(0), pValue(0), PDG(0), motherPDG(0),
        primary(0), rhoProd(0), zProd(0), phiProd(0), rhoDec(0), zDec(0), phiDec(0)
      {}

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
        rhoProd = 0;
        zProd = 0;
        phiProd = 0;
        rhoDec = 0;
        zDec = 0;
        phiDec = 0;
        primary = 0;
        cdcdedx.clear();
        svddedx.clear();
        top.clear();
        arich.clear();
        ecl.clear();
        klm.clear();
      }
    }; // struct PIDTree
  } // PID namespace
} // Belle2 namespace

