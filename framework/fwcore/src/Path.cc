/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <fwcore/Path.h>
#include <logging/Logger.h>

#include <boost/python/register_ptr_to_python.hpp>

using namespace std;
using namespace Belle2;
using namespace boost::python;


Path::Path()
{

}


Path::~Path()
{

}


void Path::addModule(ModulePtr module)
{
  //Only add modules to the path if they are
  //were registered by the user to the framework
  if (module->isRegisteredToFramework()) {
    m_modules.push_back(module);
  } else {
    string errorString = "Only user registered modules can be added to a path.";
    errorString += "Please use the method 'register_module()' to create such a module.";
    ERROR(errorString);
  }
}


//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Path::getModulesPython() const
{
  boost::python::list returnList;

  for (std::list<ModulePtr>::const_iterator listIter = m_modules.begin(); listIter != m_modules.end(); listIter++)
    returnList.append(boost::python::object(ModulePtr(*listIter)));

  return returnList;
}


void Path::exposePythonAPI()
{
  class_<Path>("Path")
  .def("add_module", &Path::addModule)
  .def("modules", &Path::getModulesPython)
  ;

  register_ptr_to_python<PathPtr>();
}
