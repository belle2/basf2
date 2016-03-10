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

#include <boost/shared_ptr.hpp>

#include <list>


namespace Belle2 {

  class Path;
  class Module;

  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  /** Defines a pointer to a path object as a boost shared pointer. */
  typedef boost::shared_ptr<Path> PathPtr;

  /** Implements a path consisting of Module and/or Path objects. The modules are arranged in a linear order.
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
     * Adds a module to the path.
     *
     * The module is added to the path by inserting it to the end
     * of the list of modules.
     *
     * @param module Module that should be added to the path.
     */
    void addModule(boost::shared_ptr<Module> module);


    /** Insert another path at the end of this one.
     *
     * E.g.
     *
       \code
       main.add_module(a)
       main.add_path(otherPath)
       main.add_module(b)
       \endcode
     *
     * would create a path [ A -> [ contents of otherPath ] -> B ].
     */
    void addPath(PathPtr path);

    /** Returns true if this Path doesn't contain any elements. */
    bool isEmpty() const;

    /**
     * Returns a list of the modules in this path. (Recursively searches sub-paths)
     */
    std::list<boost::shared_ptr<Module> > getModules() const override;

    /**
     * Builds a list of all modules which could be executed during the data processing.
     *
     * The method starts with the current path, iterates over the modules in the path and
     * follows recursively module conditions to make sure the final list contains all
     * modules which could be executed while preserving their correct order.
     *
     * @param unique If true, the list will be unique.
     * @return A list containing all modules which could be executed during the data processing.
     */
    std::list<boost::shared_ptr<Module> > buildModulePathList(bool unique = true) const;

    /**
     * Replaces all Modules and sub-Paths with the specified Module list
     */
    void putModules(const std::list<boost::shared_ptr<Module> >& mlist);

    /**
     * Does this Path contain a module of the given type?
     *
     * Useable in Python via '"ModuleType" in path' syntax.
     */
    bool contains(std::string moduleType) const;

    /**
     * Similar to addPath()/add_path(), this will execute path at the current position, but
     * will run it once for each object in the given array 'arrayName', and set the loop variable
     * 'loopObjectName' (a StoreObjPtr of same type as array) to the current object.
     *
     * Main use case is after using the RestOfEventBuilder on a ParticeList, where
     * you can use this feature to perform actions on only a part of the event
     * for a given list of candidates:
     *
       \code
       #read: for each  $objName   in $arrayName   run over $path
       path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
       \endcode
     *
     * If 'RestOfEvents' contains two elements, during the execution of roe_path a StoreObjectPtr 'RestOfEvent'
     * will be available, which will point to the first element in the first run, and the second element
     * in the second run. You can use the variable 'isInRestOfEvent' to select Particles that
     * originate from this part of the event.
     *
     * Changes to existing arrays / objects will be available to all modules after the for_each(),
     * including those made to the loop variable (it will simply modify the i'th item in the array looped over.)
     * Arrays / objects of event durability created inside the loop will however be limited to the validity of the loop variable. That is,
     * creating a list of Particles matching the current MCParticle (loop object) will no longer exist after switching
     * to the next MCParticle or exiting the loop.
     */
    void forEach(std::string loopObjectName, std::string arrayName, PathPtr path);

    /**
     * Add given path at the end of this path and ensure all modules there
     * do not influence the main DataStore. You can thus use modules in
     * skim_path to clean up e.g. the list of particles, save a skimmed uDST file,
     * and continue working with the unmodified DataStore contents outside of
     * skim_path.
     * ds_ID can be specified to give a defined ID to the temporary DataStore,
     * otherwise, a random name will be generated.
     */
    void addSkimPath(PathPtr skim_path, std::string ds_ID);

    /** Create an independent copy of this path, recreating all contained modules with the same parameters.
     *
     * Note that parameters are shared, so changing them on a module in the cloned path will also affect
     * the module in the original path.
     */
    boost::shared_ptr<PathElement> clone() const override;


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /** return a string of the form [module a -> module b -> [another path]]
     *
     *  can be used to 'print' a path in a steering file.
     */
    std::string getPathString() const override;

    /** Exposes methods of the Path class to Python. */
    static void exposePythonAPI();


  private:

    std::list<boost::shared_ptr<PathElement> > m_elements; /**< The list of path elements (Modules and sub-Paths) */

    friend class PathIterator;
  };

} // end namespace Belle2

#endif /* PATH_H_ */
