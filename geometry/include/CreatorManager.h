/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <map>

namespace Belle2 {
  namespace geometry {

    class CreatorBase;

    /** Class to manage all creators and provide factory access */
    class CreatorManager {
    public:
      /** Typedef for a factory function */
      typedef CreatorBase* CreatorFactory();

      /**
       * Register a new creator by providing a name and a pointer to a factory
       * for this kind of creator.
       * @param name Name of the creator to be registered
       * @param factory Pointer to a function returning new instances of the creator
       */
      static void registerCreatorFactory(const std::string& name, CreatorFactory* factory);

      /**
       * Return a new instance of a creator with the given name. If library is
       * not empty, the specified library will be loaded before attempting to
       * create the creator to allow for on-demand loading of libraries.
       *
       * Returns 0 if no creator with the given name is registered. Ownership
       * of the creator is transferred to the caller who is responsible of
       * freeing the creator.
       *
       * @param name Name of the Creator to be created
       * @param library Short name of the library which should be loaded before
       *        trying to create the creator. When library is "foo", libfoo.so
       *        will be loaded
       */
      static CreatorBase* getCreator(const std::string& name, const std::string& library = "");

    protected:
      /** singleton, hide constructor */
      CreatorManager() {}
      /** singleton, hide copy constructor */
      CreatorManager(const CreatorManager&) = delete;
      /** singleton, hide assignment operator */
      void operator=(const CreatorManager&) = delete;
      /** getter for the singleton instance */
      static CreatorManager& getInstance();
      /** Static map to hold all registered factories */
      std::map<std::string, CreatorFactory*> m_creatorFactories;
    };

  }
} //end of namespace Belle2
