/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENTINTERPRETERMODULE_H
#define RESTOFEVENTINTERPRETERMODULE_H

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/VariableManager/Utility.h>

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

    /**  */
    virtual void initialize() override;

    /** n */
    virtual void event() override;


  private:

    typedef std::map<std::string, std::shared_ptr<Variable::Cut>> stringAndCutMap;
    typedef std::map<std::string, std::vector<double>> stringAndVectorMap;
    typedef std::map<unsigned int, bool> intAndBoolMap;
    typedef std::map<std::string, std::map<unsigned int, bool>> stringAndMapOfIntAndBoolMap;

    std::string m_particleList;  /**< Name of the ParticleList */

    std::vector<std::string> m_maskNames; /**< Container for added mask names of ROE interpretations */

    std::vector<std::tuple<std::string, std::string, std::string>> m_ROEMasks; /**< Container for tuples */
    std::vector<std::tuple<std::string, std::string, std::string, std::vector<double>>>
    m_ROEMasksWithFractions; /**< Container for tuples with fractions */

    stringAndCutMap m_trackCuts; /**< Cut object which performs the cuts on the remaining tracks for a single ROE interpretation */
    stringAndCutMap
    m_eclClusterCuts; /**< Cut object which performs the cuts on the remaining ECL clusters for a single ROE interpretation */
    stringAndVectorMap
    m_setOfFractions; /**< A set of probabilities of the ChargedStable particles to be used in a single ROE interpretation. Default is pion always.*/

    bool m_update; /**< Set true for updating a-priori charged stable fractions. */

  };
}

#endif /* RESTOFEVENTINTERPRETERMODULE_H_H */
