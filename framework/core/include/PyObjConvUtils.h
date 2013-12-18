/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PYOBJCONVUTILS_H_
#define PYOBJCONVUTILS_H_

#include <boost/python/object.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/extract.hpp>

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <framework/logging/Logger.h>


namespace Belle2 {

  /**
   * Python object converter utilities namespace.
   * Provides templates for converting python objects to c++ objects and the other way round.
   *
   * This is done by using the type of the value provided by the module as a template parameter.
   * Hence the compiler can infer which converter should be used.
   * Note: We are only interested in the type of the value, the actual value isn't used at all!
   */
  namespace PyObjConvUtils {

    /*
     * We need forward declarations here, because otherwise the compiler ends up calling
     * the wrong function!
     */
    template<typename Scalar>
    Scalar convertPythonObject(const boost::python::object& pyObject, Scalar defaultValue);
    template<typename Key, typename Value>
    std::map<Key, Value> convertPythonObject(const boost::python::object& pyObject, std::map<Key, Value> defaultValue);
    template<typename Value>
    std::vector<Value> convertPythonObject(const boost::python::object& pyObject, std::vector<Value> defaultValue);
    template<typename... Types>
    std::tuple<Types...> convertPythonObject(const boost::python::object& pyObject, std::tuple<Types...>);
    template<typename Scalar>
    boost::python::object convertToPythonObject(const Scalar& value);
    template<typename Value>
    boost::python::list convertToPythonObject(const std::vector<Value>& vector);
    template<typename Key, typename Value>
    boost::python::dict convertToPythonObject(const std::map<Key, Value>& map);
    template<typename... Types>
    boost::python::tuple convertToPythonObject(const std::tuple<Types...>& tuple);

    template<typename T> struct Type;
    template<typename T> struct Type<std::vector<T> >;
    template<typename A, typename B> struct Type<std::map<A, B> >;
    template<typename... Types> struct Type<std::tuple<Types...> >;
    template<> struct Type<int>;
    template<> struct Type<bool>;
    template<> struct Type<float>;
    template<> struct Type<double>;
    template<> struct Type<std::string>;

    /**
     * @{
     * TMP (Template Meta Programming )
     * The name of the tuple is generated with recursive definition of the template function GetType,
     * the overloaded argument (type SizeT<>) of the function serves as a counter for the recursion depth.
     */
    template < size_t > struct SizeT { };
    template < typename T>
    inline std::string GetType(SizeT<1> = SizeT<1>()) { return std::string(Type<T>::name()); }
    template < typename T, typename... Types>
    inline std::string GetType(SizeT < sizeof...(Types) + 1 > = SizeT < sizeof...(Types) + 1 > ()) { return std::string(Type<T>::name())  + std::string(", ") + GetType<Types...>(SizeT < sizeof...(Types) > ()); }
    /** @} */

    /**
     * Converts a template argument into a string for corresponding Python type.
     * Partial template specialisations are not allowed for functions, so this needs to be a class.
     */
    template<typename T> struct Type { /** type name. */ static std::string name() { return "???";} };
    /** Converts a template argument into a string for corresponding Python type. */
    template<typename T> struct Type<std::vector<T> > { /** type name. */ static std::string name() { return std::string("list(") + Type<T>::name() + ")"; } };
    /** Converts a template argument into a string for corresponding Python type. */
    template<typename A, typename B> struct Type<std::map<A, B> > { /** type name. */ static std::string name() { return std::string("dict(") + Type<A>::name() + " -> " + Type<B>::name() + ")"; } };

    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<int> { /** type name. */ static std::string name() { return "int"; } };
    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<bool> { /** type name. */ static std::string name() { return "bool"; } };
    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<float> { /** type name. */ static std::string name() { return "float"; } };
    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<double> { /** type name. */ static std::string name() { return "float"; } };
    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<std::string> { /** type name. */ static std::string name() { return "str"; } };

    /** Converts a template argument into a string for corresponding Python type. */
    template<typename... Types> struct Type<std::tuple<Types...> > { /** type name. */ static std::string name() { return std::string("tuple( ") + GetType<Types...>() + " )"; } };

    /**
     * --------------- From C++ TO Python Converter ------------------------
     */

    /**
     * Writes a scalar type (int, double, string, bool) to a python object.
     *
     * @param value The scalar who should be converted to a python object.
     * @return python object where the scalar is stored.
     */
    template<typename Scalar>
    boost::python::object convertToPythonObject(const Scalar& value)
    {
      return boost::python::object(value);
    }

    /**
     * Writes content of a std::vector to a python list.
     *
     * @param value The vector whose items should be stored to a python list.
     * @return object python list where the vector is stored.
     */
    template<typename Value>
    boost::python::list convertToPythonObject(const std::vector<Value>& vector)
    {
      boost::python::list outputList;
      for (auto value : vector) {
        outputList.append(convertToPythonObject(value));
      }
      return outputList;
    }

    /**
     * Writes content of a std::map to a python dict.
     *
     * @param value The map whose items should be stored to a python dict.
     * @return dict The python dict where the the content of the map is stored.
     */
    template<typename Key, typename Value>
    boost::python::dict convertToPythonObject(const std::map<Key, Value>& map)
    {
      boost::python::dict outputDict;
      for (auto pair : map) {
        outputDict[convertToPythonObject(pair.first)] = convertToPythonObject(pair.second);
      }
      return outputDict;
    }

    /**
     * @{
     * TMP (Template Meta Programming )
     * The given python list is filled, and later converted into a python tuple (in convertToPythonObject).
     * To fill the python list frmo the C++ std::tuple we need again TMP methods.
     * The variadic template std::tuple is copied by rthe ecursive defined template function GetTuple,
     * the overloaded argument (type SizeT<>) of the function serves as a counter for the recursion depth.
     */

    template < typename TupleType >
    inline void GetTuple(const TupleType& tuple, boost::python::list& pyList)
    {
      GetTuple(tuple, pyList, SizeT<std::tuple_size<TupleType>::value>());
    }

    template < typename TupleType >
    inline void GetTuple(const TupleType&, boost::python::list&, SizeT<0>) { }

    template < typename TupleType, size_t N >
    inline void GetTuple(const TupleType& tuple, boost::python::list& pyList, SizeT<N>)
    {
      GetTuple(tuple, pyList, SizeT < N - 1 > ());
      pyList.append(convertToPythonObject(std::get < N - 1 > (tuple)));
    }
    /** @} */

    /**
     * Writes content of a std::tuple to a python tuple.
     *
     * @param value The tuple whose items should be stored to a python tuple.
     * @return tuple The python tuple where the the content of the map is stored.
     */
    template<typename... Types>
    boost::python::tuple convertToPythonObject(const std::tuple<Types...>& tuple)
    {
      boost::python::list outputList;
      GetTuple(tuple, outputList);
      boost::python::tuple outputTuple(outputList);
      return outputTuple;
    }

    /**
     * --------------- From Python TO C++ Converter ------------------------
     */

    /**
     * Reads a scalar type (int, double, string, bool) from a python object.
     *
     * @param pyObject Python object which stores the scalar type.
     * @param Scalar dummy allows the compiler to infer the correct template.
     * @return Scalar type, which holds the value from the python object
     */
    template<typename Scalar>
    Scalar convertPythonObject(const boost::python::object& pyObject, Scalar)
    {

      Scalar tmpValue;
      boost::python::extract<Scalar> valueProxy(pyObject);
      if (valueProxy.check()) {
        tmpValue = static_cast<Scalar>(valueProxy);
      } else {
        throw std::runtime_error(std::string("Could not set module parameter: Expected type '") + Type<Scalar>::name() + "' instead.");
      }
      return tmpValue;

    }

    /**
     * Reads std::vector from a python object.
     *
     * @param pyObject Python object which stores the vector.
     * @param std::vector<Value> dummy allows the compiler to infer the correct template.
     * @return Vector, which holds the vector from the python object
     */
    template<typename Value>
    std::vector<Value> convertPythonObject(const boost::python::object& pyObject, std::vector<Value>)
    {

      std::vector<Value> tmpVector;
      const boost::python::list& pyList = static_cast<const boost::python::list&>(pyObject);
      int nList = boost::python::len(pyList);

      for (int iList = 0; iList < nList; ++iList) {
        tmpVector.push_back(convertPythonObject(pyList[iList], Value()));
      }
      return tmpVector;
    }


    /**
     * Reads std::map from a python object.
     *
     * @param pyObject Python object which stores the map.
     * @param std::map<Key,Value> dummy allows the compiler to infer the correct template.
     * @return Map, which holds the map from the python object
     */
    template<typename Key, typename Value>
    std::map<Key, Value> convertPythonObject(const boost::python::object& pyObject, std::map<Key, Value>)
    {

      std::map<Key, Value> tmpMap;
      const boost::python::dict& pyDict = static_cast<const boost::python::dict&>(pyObject);
      boost::python::list keys = pyDict.keys();

      for (int i = 0; i < boost::python::len(keys); ++i) {
        Key key = convertPythonObject(keys[boost::python::object(i)], Key());
        Value value = convertPythonObject(pyDict[key], Value());
        tmpMap.insert(std::pair<Key, Value>(key, value));
      }
      return tmpMap;
    }


    /**
     * @{
     * TMP (Template Meta Programming )
     * The given python tuple is written into the given c++ tuple.
     * To fill the C++ std::tuple we need again TMP methods.
     * The variadic template std::tuple is filled by the recursive defined template function SetTuple,
     * the overloaded argument (type SizeT<>) of the function serves as a counter for the recursion depth.
     */
    template < typename TupleType >
    inline void SetTuple(TupleType& tuple, const boost::python::tuple& pyTuple)
    {
      static const unsigned N = std::tuple_size<TupleType>::value;
      if (boost::python::len(pyTuple) != N) {
        B2ERROR("Given python tuple has wrong dimensions expected " << N << " received " << boost::python::len(pyTuple))
      }
      SetTuple(tuple, pyTuple, SizeT<N>());
    }

    template < typename TupleType >
    inline void SetTuple(TupleType&, const boost::python::tuple&, SizeT<0>) { }

    template < typename TupleType, size_t N >
    inline void SetTuple(TupleType& tuple, const boost::python::tuple& pyTuple, SizeT<N>)
    {
      SetTuple(tuple, pyTuple, SizeT < N - 1 > ());
      std::get < N - 1 > (tuple) = convertPythonObject(pyTuple[N - 1], std::get < N - 1 > (tuple));
    }
    /** @} */

    /**
     * Reads std::tuple from a python object.
     *
     * @param pyObject Python object which stores the map.
     * @param std::tuple<Types... > dummy allows the compiler to infer the correct template.
     * @return std::tuple<Types...>, which holds the map from the python object
     */
    template<typename... Types>
    std::tuple<Types...> convertPythonObject(const boost::python::object& pyObject, std::tuple<Types...>)
    {
      std::tuple<Types...> tmpTuple;
      const boost::python::tuple& pyTuple = static_cast<const boost::python::tuple&>(pyObject);
      SetTuple(tmpTuple, pyTuple);
      return tmpTuple;
    }
  }

}

#endif /* PYOBJCONVUTILS_H_ */
