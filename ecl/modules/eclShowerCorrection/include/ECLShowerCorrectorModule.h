/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower (mainly longitudinal *
 * leakage): corr = (Reconstructed / Truth).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Gulgilemo De Nardo (denardo@na.infn.it) (GDN)            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERCORRECTORMODULE_H_
#define ECLSHOWERCORRECTORMODULE_H_

// ECL
#include <ecl/dataobjects/ECLShower.h>

// FRAMEWORK
#include <framework/core/Module.h>

// OTHER
#include <string>
#include <vector>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLShowerCorrectorModule : public Module {

    public:
      /** Constructor. */
      ECLShowerCorrectorModule();

      /** Destructor. */
      ~ECLShowerCorrectorModule();

      /** Initialize. */
      virtual void initialize();

      /** Begin run. */
      virtual void beginRun();

      /** Event. */
      virtual void event();

      /** End run. */
      virtual void endRun();

      /** Terminate. */
      virtual void terminate();

      // OLD METHOD (TF)
      /**calculate correction factor of energy depending on Energy and Theta  */
      double correctionFactor(double energy, double theta);

      /** read correction accounting shower leakage to get unbiased photon energy */
      void ReadCorrection();

      /**  theta ranges for the correction */
      std::vector<double> m_ranges;

      /** correction polynomial coefficients storage */
      std::vector<double> m_ecorr;

      /** Temp fix to correct energy bias (to be removed ASAP with a proper calibration) */
      class TmpClusterCorrection {
      public:
        void init(const std::string& filename); /**< initialize*/
//        void scale(Belle2::ECLCluster& c) const;
        void scale(Belle2::ECLShower& c) const;  /**< scale the shower energies */
      private:
        double m_deltaE;   /**< energ bin */
        std::size_t m_npointsE;   /**< number of bins */
        std::vector<double> m_maxTheta;   /**< theta bins */
        std::vector<double> m_tmpCorrection;   /**< correction factor */
      };

      TmpClusterCorrection m_tmpClusterCorrection; /**< correction class instance */


    private:

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/

      /** Default name ECLShowers */
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      /** Default name ECLConnectedRegion */
      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }
    }; // end of ECLCRFinderAndSplitterModule


    /** The very same module but for PureCsI */
    class ECLShowerCorrectorPureCsIModule : public ECLShowerCorrectorModule {
    public:

      /** PureCsI name ECLShowersPureCsI */
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      /** PureCsI name ECLConnectedRegionPureCsI */
      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }
    }; // end of ECLCRFinderAndSplitterPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
