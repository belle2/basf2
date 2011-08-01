/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATORFACTORY_H
#define CREATORFACTORY_H

#include <geometry/CreatorManager.h>

namespace Belle2 {
  namespace geometry {

    class CreatorBase;

    /**
     * Very simple class to provide an easy way to register creators with the
     * CreatorManager. When defining a new creator, add a
     *
     * CreatorFactory<Classname> Classname_factory("CreatorName");
     *
     * or similiar to the source file to automatically provide the needed
     * factory function and register the creator with the CreatorManager
     */
    template<class T> struct CreatorFactory {
      /** Constructor to register the Creator with the CreatorManager */
      CreatorFactory(const std::string& name) {
        CreatorManager::registerCreatorFactory(name, factory);
      }
      /**
       * Static factory function to return a new instance of the given Creator
       * class. Ownership of the object goes to the caller who is responsible
       * of freeing the creator once it is done
       */
      static CreatorBase* factory() {
        return new T();
      }
    };

  }
} //end of namespace Belle2

#endif /* CREATORFACTORY_H */
