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

    //! Parameter : Defines the Cut Off values for each charged particle. The Cut Off values in the list for
    //        the  different pdgs have to be in the following order  [11,13,211,2212,321,1000010020]
    std::vector<double> m_param_CutOffs;

    std::string param_linalg;

    // Performance test Parameter
    //
    // Total Track Count
    int totalCount = 0;
    // Hit Relation counter: counts how many Relations where set by the Hit Matching
    int hitRelationCounter = 0;
    // Chi2 Relation Counter: counts how many Relations where set by the chi2 Matching
    int chi2RelationCounter = 0;
    // both Relation Counter: counts how many Tracks got a relation by Hit Matching and chi2 Matching
    int bothRelationCounter = 0;
    // both Relations and Same MC Counter
    int bothRelationAndSameMCCounter = 0;
    // Not both Relation Counter: opposite of previous
    int notBothRelationCounter = 0;
    // No Relation Counter
    int noRelationCounter = 0;
  };
}
