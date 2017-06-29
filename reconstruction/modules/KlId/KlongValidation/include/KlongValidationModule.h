

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *
 **************************************************************************/
#ifndef KlongValidationModule_H
#define KlongValidationModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <TFile.h>
#include <string>
#include <TEfficiency.h>



namespace Belle2 {

  /** Module used by the validation server to generate root files for the validation. Calculates fake rates and efficiencies.*/
  class KlongValidationModule : public Module {

  public:

    /** Constructor   */
    KlongValidationModule();

    /** Destructor   */
    virtual ~KlongValidationModule();

    /** initialize   */
    virtual void initialize();

    /**  beginn run   */
    virtual void beginRun();

    /** process event   */
    virtual void event();

    /** end run   */
    virtual void endRun();

    /** terminate   */
    virtual void terminate();

  protected:

  private:

    /** momentum   */
    double m_momentum;
    /** angle in z-plane  */
    double m_theta;
    /** angle in x-y   */
    double m_phi;
    /** K0L truth   */
    double m_isKl;
    /** did cluster pass selection of algorythm?  */
    bool   m_passed;
    /** cluster wrongly reconstructed as K0L?   */
    bool   m_faked;
    /** cluster reconstructed as K0L?   */
    bool   m_reconstructedAsKl;

    // use TEfficiency histogramms to calculate efficiency
    /** efficiency in x-y plane   */
    TEfficiency* m_effPhi;
    /** efficiency in angle to z   */
    TEfficiency* m_effTheta;
    /** momentum efficiency   */
    TEfficiency* m_effMom;
    /** fake phi, angle in x-y   */
    TEfficiency* m_fakePhi;
    /**  fake theta, angle to z   */
    TEfficiency* m_fakeTheta;
    /**  fake momentum plot   */
    TEfficiency* m_fakeMom;

    /** output path   */
    std::string m_outPath = "nightlyKlongValidation.root";

    /** root tree etc. */
    TFile* m_f = nullptr;  //

  }; // end class
} // end namespace Belle2

#endif
