/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej, Sviatoslav Bilokin                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace Belle2 {

  /**
   * Creates a mask (vector of boolean values) for tracks and clusters in RestOfEvent regarding their usage.
   */
  class RestOfEventInterpreterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    RestOfEventInterpreterModule();

    /** Overriden initialize method */
    virtual void initialize() override;

    /** Overriden event method */
    virtual void event() override;


  private:

    StoreObjPtr<ParticleList> m_plist; /**< input particle list */
    std::string m_particleListName;  /**< Name of the ParticleList */
    std::vector<std::tuple<std::string, std::string, std::string, std::string>>
        m_ROEMasks; /**< Container for tuples */
    typedef std::map<std::string, std::shared_ptr<Variable::Cut>>
                                                               stringAndCutMap; /**< Type definition helper for a map of string and cuts */
    typedef std::map<std::string, std::vector<double>>
                                                    stringAndVectorMap; /**< Type definition helper for a map of string and vector of doubles */
    typedef std::map<unsigned int, bool> intAndBoolMap; /**< Type definition helper for a map of integer and boolean */
    typedef std::map<std::string, std::map<unsigned int, bool>>
                                                             stringAndMapOfIntAndBoolMap; /**< Type definition helper for a map of string and nested map of integer and boolean */


    std::vector<std::string> m_maskNames; /**< Container for added mask names of ROE interpretations */

    stringAndCutMap m_trackCuts; /**< Cut object which performs the cuts on the remaining tracks for a single ROE interpretation */
    stringAndCutMap
    m_eclClusterCuts; /**< Cut object which performs the cuts on the remaining ECL clusters for a single ROE interpretation */
    stringAndCutMap
    m_klmClusterCuts; /**< Cut object which performs the cuts on the remaining KLM clusters for a single ROE interpretation */

    bool m_update; /**< Set true for updating a-priori charged stable fractions. */

  };
}

