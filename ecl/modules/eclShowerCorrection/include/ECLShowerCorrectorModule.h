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

      // read correction accounting shower leakage to get unbiased photon energy
      void ReadCorrection();

      // m_ranges -- theta ranges for the correction
      // m_ecorr  -- correction polynomial coefficients storage
      std::vector<double> m_ranges, m_ecorr;

      //Temp fix to correct energy bias
      //to be removed ASAP with a proper calibration

      class TmpClusterCorrection {
      public:
        void init(const std::string& filename);
//        void scale(Belle2::ECLCluster& c) const;
        void scale(Belle2::ECLShower& c)
        const; // we cannot correct clusters at this stage since they dont exist in the refactored here at this very moment (TF)
      private:
        double m_deltaE;
        std::size_t m_npointsE;
        std::vector<double> m_maxTheta;
        std::vector<double> m_tmpCorrection;
      };

      TmpClusterCorrection m_tmpClusterCorrection;


    private:

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }
    }; // end of ECLCRFinderAndSplitterModule


    /** The very same module but for PureCsI */
    class ECLShowerCorrectorPureCsIModule : public ECLShowerCorrectorModule {
    public:
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }
    }; // end of ECLCRFinderAndSplitterPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
