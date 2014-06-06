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
  protected:

  private:

    std::list<PathPtr> m_createdPathList; /**< List of all created paths. */

  };

}

#endif /* PATHMANAGER_H_ */
