/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATORMANAGER_H
#define CREATORMANAGER_H

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
      /** constructor */
      CreatorManager() {}
      CreatorManager(const CreatorManager&);  /** Parameter of the creator manager */
      void operator=(const CreatorManager&);  /** Parameter of the operator */
      static CreatorManager& getInstance();   /** Static map to hold all registered factories */
      std::map<std::string, CreatorFactory*> m_creatorFactories;
    };

  }
} //end of namespace Belle2

#endif /* CREATORMANAGER_H_ */
