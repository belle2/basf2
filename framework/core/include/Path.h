/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PATH_H_
#define PATH_H_

#include <framework/core/PathElement.h>
#include <framework/core/Module.h>

#include <boost/shared_ptr.hpp>

#include <list>


namespace Belle2 {

  class Path;

  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  /** Defines a pointer to a path object as a boost shared pointer. */
  typedef boost::shared_ptr<Path> PathPtr;

  /** The path class.
   * Implements a path consisting of modules. The modules are arranged in a linear order.
   */
  class Path : public PathElement {

  public:

    /**
     * Constructor.
     */
    Path();

    /**
     * Destructor.
     */
    ~Path();

    /**
     * Adds a new module to the path.
     *
     * A new module is added to the path by inserting it to the end
     * of the list of modules.
     *
     * @param module Reference to the module that should be added to the path.
     */
    void addModule(boost::shared_ptr<Module> module);


    /** Connect another path with this one.
     *
     * Modules in the other path will be executed after all modules in the
     * curren path.
     */
    void addPath(PathPtr path);

    /**
     * Returns a list of the modules in this path.
     *
     * @return A list of all modules of this path.
     */
    std::list<boost::shared_ptr<Module> > getModules() const;

    /**
     * Builds a list of all modules which could be executed during the data processing.
     *
     * The method starts with the current path, iterates over the modules in the path and
     * follows recursively module conditions to make sure the final list contains all
     * modules which could be executed while preserving their correct order.
     * Special care is taken to avoid that a module is added more than once to the list.
     *
     * @return A list containing all modules which could be executed during the data processing.
     */
    std::list<boost::shared_ptr<Module> > buildModulePathList() const;

    /**
     * Replaces all Modules and sub-Paths with the specified Module list
     */
    void putModules(const std::list<boost::shared_ptr<Module> >& mlist) { m_elements.assign(mlist.begin(), mlist.end()); }


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /** return a string of the form [module a, module b, [another path]]
     *
     *  can be used to 'print' a path in a steering file.
     */
    virtual std::string getPathString() const;

    /** Exposes methods of the Path class to Python. */
    static void exposePythonAPI();


  private:
    /**
     * Fills the module list with the modules of this path.
     *
     * Calls itself recursively for modules having a condition
     *
     * @param modList The list of modules.
     */
    void fillModulePathList(std::list<boost::shared_ptr<Module> >& modList) const;

    std::list<boost::shared_ptr<PathElement> > m_elements; /**< The list of path elements (Modules and sub-Paths) */

    friend class PathIterator;
  };

} // end namespace Belle2

#endif /* PATH_H_ */
