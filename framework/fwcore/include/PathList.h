/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PATHLIST_H_
#define PATHLIST_H_

#include <fwcore/Module.h>
#include <fwcore/Path.h>

#include <list>
#include <set>

namespace Belle2 {


  //! The PathList Class
  /*!
    This class manages the created path instances. It provides methods to created
    new path instances.
  */
  class PathList : public std::list<PathPtr> {

  public:

    //! Constructor
    PathList();

    //! Destructor
    virtual ~PathList();

    //! Creates a new path and adds it to the list of available paths.
    /*!
        This method creates a new path and adds it to the list.

        If the path could not be created, an exception of type FwExcPathNotCreated is thrown.

        \return A reference to the newly created path.
    */
    PathPtr createPath() throw(FwExcPathNotCreated);

    //! Builds a list of all modules which could be executed during the data processing.
    /*!
        The method starts with the given path, iterates over the modules in the path and
        follows recursively module conditions to make sure the final list contains all
        modules which could be executed while preserving their correct order.
        Special care is taken to avoid that a module is added more than once to the list.

        \param startPath The path with which the search should start.
        \return A list containing all modules which could be executed during the data processing.
    */
    ModulePtrList buildModulePathList(PathPtr startPath) const;


  protected:


  private:

    //! Fills the module list with the modules of the given path.
    /*!
        Calls itself recursively for modules having a condition

        \param path The path whose modules should be added to the module list.
        \param modList The list of modules.
    */
    void fillModulePathList(PathPtr path, ModulePtrList& modList) const;

  };

} // end namespace Belle2

#endif /* PATHLIST_H_ */
