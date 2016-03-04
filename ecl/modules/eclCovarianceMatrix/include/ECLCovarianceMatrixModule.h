/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates the covariance matrix for a shower.             *
 * The matrix will depend on the shower region (FWD, Bartel, BWD) and     *
 * possibly on the hypothesis.                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCOVARIANCEMATRIXMODULE_H_
#define ECLCOVARIANCEMATRIXMODULE_H_

#include <framework/core/Module.h>
#include <TMatrixFSym.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLCovarianceMatrixModule : public Module {

    public:
      /** Constructor. */
      ECLCovarianceMatrixModule();

      /** Destructor. */
      ~ECLCovarianceMatrixModule();

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

      /** function of square */
      double squ(double x) { return x * x; }

      /**calculate error of Energy with Energy  */
      double errorEnergy(double energy);

      /**calculate error of Theta with Energy and Theta  */
      double errorTheta(double energy, double theta);

      /**calculate error of Phi with Energy and Theta  */
      double errorPhi(double energy, double theta);

      /** Fill error matrix for Px, Py Pz and E */
      void readErrorMatrix(double energy, double theta,
                           double phi, TMatrixFSym& errorMatrix);

      /** Fill error matrix for Px, Py Pz and E */
      void readErrorMatrix7x7(double energy, double theta,
                              double phi, TMatrixFSym& errorMatrix);

      /**calculate Px using Energy, Theta and Phi */
      double Px(double energy, double theta, double phi);

      /** Calculate Py using Energy, Theta and Phi */
      double Py(double energy, double theta, double phi);

      /** Calculate Pz using Energy, Theta  */
      double Pz(double energy, double theta);


    private:

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }
    }; // end of ECLCovarianceMatrixModule


    /** The very same module but for PureCsI */
    class ECLCovarianceMatrixPureCsIModule : public ECLCovarianceMatrixModule {
    public:
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }
    }; // end of ECLCovarianceMatrixPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
