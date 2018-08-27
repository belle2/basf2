/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENTUPDATERMODULE_H
#define RESTOFEVENTUPDATERMODULE_H

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace Belle2 {

  /**
   * Updates an existing mask (map of boolean values) for tracks or eclClusters in RestOfEvent with an available property (e.g. after performing training)
   */
  class RestOfEventUpdaterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    RestOfEventUpdaterModule();

    /**  */
    virtual void initialize() override;

    /** n */
    virtual void event() override;
    // TODO: copy all functionality and delete this!
    virtual void deprecatedEvent();


  private:

    typedef std::map<unsigned int, bool> intAndBoolMap; /**< Type definition helper for a map of integer and boolean */
    typedef std::map<unsigned int, unsigned int> intAndIntMap; /**< Type definition helper for a map of integer and integer */

    std::string m_inputListName; /**< Name of the ParticleList which contains information that will be used for updating */
    StoreObjPtr<ParticleList> m_inputList; /**< ParticleList which contains information that will be used for updating */

    std::vector<std::string> m_maskNamesForUpdating; /**< Container for all mask names which will be updated */

    std::string m_selection; /**< Cut string which will be used for updating masks */
    std::shared_ptr<Variable::Cut> m_cut; /**< Cut object which performs the cuts */

    bool m_discard; /**< Update the ROE mask by passing or discarding particles in the provided particle list. Default is to pass. */

    std::vector<double>
    m_fractions; /**< A priori fractions used for newly added masks. Fractions of existing masks will not be changed (default: pion always) */

    // Methods
    bool isInParticleList(const Particle* roeParticle, std::vector<const Particle*>& particlesToUpdate);
    Particle::EParticleType getListType(const int& pdgCode);
  };
}

#endif /* RESTOFEVENTUPDATERMODULE_H_H */
