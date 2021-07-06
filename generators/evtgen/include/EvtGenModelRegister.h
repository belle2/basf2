/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <list>
#include <vector>

class EvtDecayBase;

namespace Belle2 {

  /** Class to keep a register of all Belle2 EvtDecayBases */
  class EvtGenModelRegister {
  public:
    /** Callback type to return a new model instance */
    typedef EvtDecayBase* ModelFactory();

    /** Helper Class to easily register new EvtDecayBases for Belle2 */
    template<class MODEL> class Factory {
    public:
      /** Constructor to register a model with a given name */
      Factory()
      {
        EvtGenModelRegister::registerModel(factory);
      }
      /** Create a new instance of the EvtDecayBase and return the pointer */
      static EvtDecayBase* factory()
      {
        return new MODEL();
      }
    };

    /** Register a new Model, called by the EvtGenModelRegister::Factory class */
    static void registerModel(ModelFactory* factory)
    {
      getInstance().m_models.push_back(factory);
    }

    /** Return a list of models. Caller takes responsibility to free the instances when no longer needed
     * @return list with pointers to instances of all registered models.
     */
    static std::list<EvtDecayBase*> getModels();

  private:

    /** Singleton: private constructor */
    EvtGenModelRegister() {};

    /** Singleton: private copy constructor */
    EvtGenModelRegister(const EvtGenModelRegister& other) = delete;
    /** Singleton: private assignment operator */
    EvtGenModelRegister& operator=(const EvtGenModelRegister&) = delete;

    /** Return reference to the instance.
     * This class behaves like a purely static class but we need a singleton
     * pattern to avoid initialisation hell.
     */
    static EvtGenModelRegister& getInstance();

    /** List of all registered EvtGenModels */
    std::vector<ModelFactory*> m_models;
  };

  /** Class to register B2_EVTGEN_REGISTER_MODEL */
#define B2_EVTGEN_REGISTER_MODEL(classname) namespace {\
    Belle2::EvtGenModelRegister::Factory<classname> EvtGenModelFactory_##classname; \
  }

} //Belle2 namespace
