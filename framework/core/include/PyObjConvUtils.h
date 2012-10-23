#ifndef PYOBJCONVUTILS_H_
#define PYOBJCONVUTILS_H_

#include <boost/python.hpp>
#include <boost/python/object.hpp>
#include <boost/python/list.hpp>

#include <vector>


namespace Belle2 {

  /**
   * Python object converter utilities class.
   * Provides various static template methods to convert a STL variable to a python object.
   */
  class PyObjConvUtils {

  public:
    /**
     * Adds a single variable value to a python list.
     *
     * @param value The single value which should be added to the python list.
     * @param outputList The python list to which the converted value is added.
     */
    template<typename T>
    static void addSingleValueToList(const T& value, boost::python::list& outputList);

    /**
     * Adds the content of a STL vector to a python list.
     *
     * @param value The STL vector whose items should be added to a python list.
     * @param outputList The python list to which the content of the STL vector is added.
     */
    template<typename T>
    static void addSTLVectorToList(const T& value, boost::python::list& outputList);

  };


  //------------------------------------------------------
  //       Implementation of template based methods
  //------------------------------------------------------

  template<typename T>
  void PyObjConvUtils::addSingleValueToList(const T& value, boost::python::list& outputList)
  {
    outputList.append(boost::python::object(value));
  }


  template<typename T>
  void PyObjConvUtils::addSTLVectorToList(const T& value, boost::python::list& outputList)
  {
    typename T::const_iterator listIter;
    for (listIter = value.begin(); listIter != value.end(); ++listIter) {
      outputList.append(boost::python::object(*listIter));
    }
  }

}

#endif /* PYOBJCONVUTILS_H_ */
