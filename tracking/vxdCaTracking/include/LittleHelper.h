/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef LITTLEHELPER_H
#define LITTLEHELPER_H

#include <TVector3.h>



namespace Belle2 {

  /** bundles small but often used functions for smearing and others.  */
  class LittleHelper {
  public:

    /** Empty constructor. For initialisation only, an object generated this way is useless unless reset() is called at least once */
    LittleHelper():
      m_meanValue(0.),
      m_sigmaValue(1.) {}

    /** Constructor. first parameter is the mean, second is the sigma of the distribution */
    LittleHelper(double mean, double sigma):
      m_meanValue(mean),
      m_sigmaValue(sigma) {}

    /** Destructor. */
    ~LittleHelper() {}


    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one */
    void resetValues(double mean, double sigma);

    /** use this member if you want to smear your value using gauss within a certain area (generalized). */
    double smearValueGauss(double low, double high, double mean, double sigma);
    /** use this member if you want to smear your value using gauss within a certain area using preset for mean and sigma. */
    double smearValueGauss(double low, double high);

    /** use this member if you want to smear your value using gauss resulting in a positive value (generalized). */
    double smearPositiveGauss(double high, double mean, double sigma);
    /** use this member if you want to smear your value using gauss resulting in a positive value using preset for mean and sigma. */
    double smearPositiveGauss(double high);

    /** use this member if you want to smear your value using gauss resulting in a value between 0-1 (generalized). */
    double smearNormalizedGauss(double mean, double sigma);
    /** use this member if you want to smear your value using gauss resulting in a value between 0-1 using preset for sigma. */
    double smearNormalizedGauss(double mean);
    /** use this member if you want to smear your value using gauss resulting in a value between 0-1 using preset for mean and sigma. */
    double smearNormalizedGauss();

    /** use this member if you want to smear your value using uniform within a certain area (generalized). */
    double smearValueUniform(double low, double high, double mean, double sigma);
    /** use this member if you want to smear your value using uniform within a certain area using preset for mean and sigma. */
    double smearValueUniform(double low, double high);

    /** use this member if you want to smear your value using uniform resulting in a positive value (generalized). */
    double smearPositiveUniform(double high, double mean, double sigma);
    /** use this member if you want to smear your value using uniform resulting in a positive value using preset for mean and sigma. */
    double smearPositiveUniform(double high);

    /** use this member if you want to smear your value using uniform resulting in a value between 0 and 1 (generalized). */
    double smearNormalizedUniform(double mean, double sigma);
    /** use this member if you want to smear your value using uniform resulting in a value between 0-1 using preset for mean and sigma. */
    double smearNormalizedUniform();

  protected:
    /** safety check whether incoming values make sense for smearing */
    bool checkSanity(double low, double high, double mean, double sigma);

    double m_meanValue; /**< delivers preset for a mean value to be smeared */
    double m_sigmaValue; /**< delivers preset for a sigma value to determine the degree of smearing */
  }; //end class LittleHelper
} //end namespace Belle2

#endif //LITTLEHELPER


