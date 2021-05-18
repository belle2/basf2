/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Florian Schweiger                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {
  /**
   * Monte Carlo matcher using the helix parameters for matching by chi2-method
   */
  class Chi2McMatcherModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    Chi2McMatcherModule();

    /**  */
    //~Chi2McMatcherModule() override;

    /** Register input and output data */
    void initialize() override;


    // Do matching for each event
    void event() override;
  private: //Parameters

    //! Parameter : String to define Cut Off Type
    //
    // ->individual: each charged particle gets its own CutOff
    //         defined by IndividualCutOffs parameter
    // ->general:    all charged particle get the same CutOff
    //         defined by GeneralCutOff parameter
    std::string m_param_CutOffType;

    //! Parameter : Defines the Cut Off value for general CutOffType Default->electron 99 percent border (128024)
    double m_param_GeneralCutOff;

    //! Parameter : Defines the Cut Off values for each charged particle. The Cut Off values in the list for
    //        the  different pdgs have to be in the following order  [11,13,211,2212,321,1000010020]
    std::vector<double> m_param_IndividualCutOffs;
  };
}
