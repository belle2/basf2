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

#include <boost/python.hpp>

#include <list>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/python/list.hpp>

#include <framework/core/Module.h>
#include <framework/core/ModuleList.h>

namespace Belle2 {

  //! The path class
  /*! Implements a path consisting of modules. The modules are arranged
      as a list of modules.
  */
  class Path {

  public:

    //! Constructor
    Path();

    //! Destructor
    ~Path();

    //! Adds a new module to the path.
    /*!
        A new module is added to the path by inserting it to the end
        of the list of modules.
        \param module Reference to the module that should be added to the path.
    */
    void addModule(ModulePtr module);

    //! Returns a list of the modules in this path.
    /*!
        \return A list of all modules of this path.
    */
    const std::list<ModulePtr>& getModules() const {return m_modules; };


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    //! Returns a list of the modules in this path (as python list).
    /*!
        \return A list of all modules of this path (as python list).
    */
    boost::python::list getModulesPython() const;

    //! Exposes methods of the Path class to Python.
    static void exposePythonAPI();


  protected:


  private:

    std::list<ModulePtr> m_modules; /*!< The list of modules (the module objects are owned by ModuleList). */

  };

  typedef boost::shared_ptr<Path> PathPtr;

} // end namespace Belle2

#endif /* PATH_H_ */
