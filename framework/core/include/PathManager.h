/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PATHMANAGER_H_
#define PATHMANAGER_H_

#include <framework/core/Module.h>
#include <framework/core/Path.h>

#include <list>


namespace Belle2 {

  /**
   * The PathManager Class.
   *
   * This class is responsible for creating and managing new paths.
   * A path is a container for modules. The user can create an arbitrary
   * number of paths and connect them using conditions. This class allows
   * the creation of paths and provides convenient utility methods.
   */
  class PathManager {

  public:
    /**
     * Constructor.
     */
    PathManager();

    /**
     * Destructor.
     */
    ~PathManager();

    /**
     * Creates a new path and adds it to the list of available paths.
     *
     * This method creates a new path and adds it to the list.
     *
     * @return Shared pointer to the newly created path.
     */
    PathPtr createPath();

    /**
     * Builds a list of all modules which could be executed during the data processing.
     *
     * The method starts with the given path, iterates over the modules in the path and
     * follows recursively module conditions to make sure the final list contains all
     * modules which could be executed while preserving their correct order.
     * Special care is taken to avoid that a module is added more than once to the list.
     *
     * @param startPath The path with which the search should start.
     * @return A list containing all modules which could be executed during the data processing.
     */
    ModulePtrList buildModulePathList(const PathPtr startPath) const;


  protected:

  private:

    std::list<PathPtr> m_createdPathList; /**< List of all created paths. */

    /**
     * Fills the module list with the modules of the given path.
     *
     * Calls itself recursively for modules having a condition
     *
     * @param path The path whose modules should be added to the module list.
     * @param modList The list of modules.
     */
    static void fillModulePathList(const PathPtr path, ModulePtrList& modList);

  };

}

#endif /* PATHMANAGER_H_ */
