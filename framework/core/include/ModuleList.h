/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULELIST_H_
#define MODULELIST_H_

#include <framework/core/Module.h>
#include <framework/core/FwExceptions.h>

#include <list>


namespace Belle2 {

  //! The ModuleList Class
  /*!
    This class manages the user created module instances. It provides methods to created
    new module instances.
  */
  class ModuleList : public std::list<ModulePtr> {

  public:

    //! Constructor
    ModuleList();

    //! Destructor
    virtual ~ModuleList();

    //! Creates a new module and adds it to the list of available modules.
    /*!
        This method creates a new Module based on the given type and adds it to the list.

        If the module could not be created, an exception of type FwExcModuleNotCreated is thrown.

        \param type The type of the module which should be created.
        \return A reference to the newly created module.
    */
    ModulePtr createModule(const std::string& type) throw(FwExcModuleNotCreated);


  protected:


  private:

  };

} // end namespace Belle2

#endif /* MODULELIST_H_ */
