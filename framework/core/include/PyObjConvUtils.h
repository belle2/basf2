/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <boost/python/object.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/extract.hpp>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>
#include <map>
#include <string>
#include <vector>


namespace Belle2 {
  class Path;

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
    /** check if the python object can be converted to the given type. */
    bool checkPythonObject(const boost::python::object& pyObject, bool);
    /** check if the python object can be converted to the given type. */
    bool checkPythonObject(const boost::python::object& pyObject, float);
    /** check if the python object can be converted to the given type. */
    bool checkPythonObject(const boost::python::object& pyObject, double);
    /** check if the python object can be converted to the given an integral number */
    bool checkPythonObject(const boost::python::object& pyObject, int);
    /** check if the python object can be converted to the given an integral number */
    bool checkPythonObject(const boost::python::object& pyObject, unsigned int);
    /** check if the python object can be converted to the given an integral number */
    bool checkPythonObject(const boost::python::object& pyObject, unsigned long int);
    /** check if the python object can be converted to the given a string */
    bool checkPythonObject(const boost::python::object& pyObject, const std::string&);
    template<typename Key, typename Value>
    bool checkPythonObject(const boost::python::object& pyObject, const std::map<Key, Value>&);
    /** check if the python object can be converted to the given std::vector type */
    template<typename Value>
    bool checkPythonObject(const boost::python::object& pyObject, const std::vector<Value>&);
    /** check if the python object can be converted to the given std::set type */
    template<typename Value>
    bool checkPythonObject(const boost::python::object& pyObject, const std::set<Value>&);
    /** check if the python object can be converted to the given std::tuple type */
    template<typename... Types>
    bool checkPythonObject(const boost::python::object& pyObject, const std::tuple<Types...>&);
    /** check if the python object can be converted to the given boost::variant type */
    template<typename... Types>
    bool checkPythonObject(const boost::python::object& pyObject, const boost::variant<Types...>&);
    /** check if the python object can be converted to the given boost::optional<T> type */
    template<typename Type>
    bool checkPythonObject(const boost::python::object& pyObject, const boost::optional<Type>&);

    /** Convert from Python to given type. */
    template<typename Scalar>
    Scalar convertPythonObject(const boost::python::object& pyObject, Scalar);
    /** Convert from Python to given type. */
    template<typename Key, typename Value>
    std::map<Key, Value> convertPythonObject(const boost::python::object& pyObject, const std::map<Key, Value>&);
    /** Convert from Python to given type. */
    template<typename Value>
    std::vector<Value> convertPythonObject(const boost::python::object& pyObject, const std::vector<Value>&);
    /** Convert from Python to given type */
    template<typename Value>
    std::set<Value> convertPythonObject(const boost::python::object& pyObject, const std::set<Value>&);
    /** Convert from Python to given type. */
    template<typename... Types>
    std::tuple<Types...> convertPythonObject(const boost::python::object& pyObject, const std::tuple<Types...>&);
    /** Convert from Python to given type. */
    template<typename... Types>
    boost::variant<Types...> convertPythonObject(const boost::python::object& pyObject, const boost::variant<Types...>&);
    /** Convert from Python to the given boost::optional type */
    template<typename Type>
    boost::optional<Type> convertPythonObject(const boost::python::object& pyObject, const boost::optional<Type>&);

    template<typename Scalar>
    boost::python::object convertToPythonObject(const Scalar& value);
    template<typename Value>
    boost::python::list convertToPythonObject(const std::vector<Value>& vector);
    /** Convert std::set to a python object */
    template<typename Value>
    boost::python::object convertToPythonObject(const std::set<Value>& set);
    template<typename Key, typename Value>
    boost::python::dict convertToPythonObject(const std::map<Key, Value>& map);
    template<typename... Types>
    boost::python::tuple convertToPythonObject(const std::tuple<Types...>& tuple);
    template<typename... Types>
    boost::python::object convertToPythonObject(const boost::variant<Types...>& tuple);
    /** Convert from the given boost::optional type to Python */
    template<typename Type>
    boost::python::object convertToPythonObject(const boost::optional<Type>& optional);

    template<typename T> struct Type;
    template<typename T> struct Type<std::vector<T> >;
    template<typename T> struct Type<std::set<T>>;
    template<typename A, typename B> struct Type<std::map<A, B> >;
    template<typename... Types> struct Type<std::tuple<Types...> >;
    template<typename... Types> struct Type<boost::variant<Types...> >;
    template<> struct Type<int>;
    template<> struct Type<bool>;
    template<> struct Type<float>;
    template<> struct Type<double>;
    template<> struct Type<std::string>;

    template< typename T, typename... Types> struct VariadicType;
    /** Recursively convert multiple types to type names (used for tuples). */
    template< typename T> struct VariadicType<T> { /** type name. */ static std::string name() { return Type<T>::name(); } };
    /** Recursively convert multiple types to type names (used for tuples). */
    template< typename T, typename... Types> struct VariadicType {
      /** type name. */
      static std::string name()
      {
        return Type<T>::name() + ", " + VariadicType<Types...>::name();
      }
    };

    /**
     * Converts a template argument into a string for corresponding Python type.
     * Partial template specialisations are not allowed for functions, so this needs to be a class.
     */
    template<typename T> struct Type { /** type name. */ static std::string name() { return "???";} };
    /** Converts a template argument into a string for corresponding Python type. */
    template<typename T> struct Type<std::vector<T> > { /** type name. */ static std::string name() { return std::string("list(") + Type<T>::name() + ")"; } };
    /** Convert a templeate arugment into a string for corresponding Python type */
    template<typename T> struct Type<std::set<T>> { /** type name */ static std::string name() { return std::string("set(") + Type<T>::name() + ")"; }};
    /** Converts a template argument into a string for corresponding Python type. */
    template<typename A, typename B> struct Type<std::map<A, B> > { /** type name. */ static std::string name() { return std::string("dict(") + Type<A>::name() + " -> " + Type<B>::name() + ")"; } };

    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<unsigned int> { /** type name. */ static std::string name() { return "unsigned int"; } };
    /** Converts a template argument into a string for corresponding Python type. */
    template<> struct Type<unsigned long int> { /** type name. */ static std::string name() { return "unsigned long int"; } };
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
    template<typename T> struct Type<boost::optional<T>> { /** type name. */ static std::string name() { return Type<T>::name() + " or None"; } };

    /** Converts a template argument into a string for corresponding Python type. */
    template<typename... Types> struct Type<std::tuple<Types...> > { /** type name. */ static std::string name() { return std::string("tuple(") + VariadicType<Types...>::name() + ")"; } };

    /** Converts a template argument into a string for corresponding Python type. */
    template<typename... Types> struct Type<boost::variant<Types...> > { /** type name. */ static std::string name() { return std::string("variant(") + VariadicType<Types...>::name() + ")"; } };

    /** Some modules take a path as argument so let's add this as well */
    template<> struct Type<std::shared_ptr<Path>> {
      /** type name */
      static std::string name() { return std::string("Path"); }
    };


    /// Helper construct for TMP that provides an index at compile time to recurse through type lists.
    template < size_t > struct SizeT { };

    /**
     * --------------- Check python objects for exact type match ------------------------
     */

    /// Check if the python object is a boolean
    inline bool checkPythonObject(const boost::python::object& pyObject, bool /*dispatch tag*/)
    {
      return PyBool_Check(pyObject.ptr());
    }

    /// Check if the python object is an integral number
    inline bool checkPythonObject(const boost::python::object& pyObject, int /*dispatch tag*/)
    {
      return PyLong_CheckExact(pyObject.ptr());
    }
    /// Check if the python object is an integral number
    inline bool checkPythonObject(const boost::python::object& pyObject, unsigned int /*dispatch tag*/)
    {
      return PyLong_CheckExact(pyObject.ptr());
    }

    /// Check if the python object is an integral number
    inline bool checkPythonObject(const boost::python::object& pyObject, unsigned long int /*dispatch tag*/)
    {
      return PyLong_CheckExact(pyObject.ptr());
    }

    /// Check if the python object is a float number
    inline bool checkPythonObject(const boost::python::object& pyObject, float /*dispatch tag*/)
    {
      return PyFloat_CheckExact(pyObject.ptr());
    }

    /// Check if the python object is a float number
    inline bool checkPythonObject(const boost::python::object& pyObject, double /*dispatch tag*/)
    {
      return PyFloat_CheckExact(pyObject.ptr());
    }

    /// Check if the python object is a string
    inline bool checkPythonObject(const boost::python::object& pyObject, const std::string& /*dispatch tag*/)
    {
      return PyUnicode_Check(pyObject.ptr());
    }

    /// Check if the python object is a mapping of the correct key and value types
    template<typename Key, typename Value>
    bool checkPythonObject(const boost::python::object& pyObject, const std::map<Key, Value>& /*dispatch tag*/)
    {
      if (not PyDict_Check(pyObject.ptr())) return false;
      const boost::python::dict& pyDict = static_cast<const boost::python::dict&>(pyObject);
      boost::python::list keys = pyDict.keys();
      boost::python::list values = pyDict.values();
      for (int i = 0; i < boost::python::len(keys); ++i) {
        if (not checkPythonObject(keys[i], Key())) return false;
        if (not checkPythonObject(values[i], Value())) return false;
      }
      return true;
    }

    /** Helper function to loop over a python object that implements the iterator
     * concept and call a functor with each element as argument
     * @param pyObject the python object to iterate over
     * @param function any functor object which accepts a boost::python::object as its
     *        only argument and returns true if the iteration should be continued
     * @returns true if the iteration was completed over all elements
     */
    template<class Functor>
    bool iteratePythonObject(const boost::python::object& pyObject, Functor function)
    {
      boost::python::object iterator(boost::python::handle<>(PyObject_GetIter(pyObject.ptr())));
      PyObject* item{nullptr};
      // ok, loop over the iterator and check all elements
      while ((item = PyIter_Next(iterator.ptr()))) {
        boost::python::object obj{boost::python::handle<>(item)}; // to make sure we properly decref
        if (not function(obj)) return false;
      }
      return true;
    }

    /// Check if the python object is a list of objects of the correct value type
    template<typename Value>
    bool checkPythonObject(const boost::python::object& pyObject, const std::vector<Value>& /*dispatch tag*/)
    {
      if (not PyList_Check(pyObject.ptr())) return false;
      return iteratePythonObject(pyObject, [](const boost::python::object & element) {
        return checkPythonObject(element, Value());
      });
    }

    template<typename Value>
    bool checkPythonObject(const boost::python::object& pyObject, const std::set<Value>&)
    {
      if (not PyAnySet_Check(pyObject.ptr())) return false;
      return iteratePythonObject(pyObject, [](const boost::python::object & element) {
        return checkPythonObject(element, Value());
      });
    }

    /// Check if all tuple elements match
    template<typename... Types, std::size_t ... Is>
    bool CheckTuple(const std::tuple<Types...>& tuple, const boost::python::tuple& pyTuple,
                    std::index_sequence<Is...>)
    {
      return (... && checkPythonObject(pyTuple[Is], std::get<Is>(tuple)));
    }

    /// Check if the python object is a tuple of objects of the correct types
    template<typename... Types>
    bool checkPythonObject(const boost::python::object& pyObject, const std::tuple<Types...>& tuple)
    {
      if (not PyTuple_Check(pyObject.ptr())) return false;
      const boost::python::tuple& pyTuple = static_cast<const boost::python::tuple&>(pyObject);
      return CheckTuple(tuple, pyTuple, std::index_sequence_for<Types ...>());
    }

    /// Recursion sentinal for the case that all former type checks failed for the variant.
    template<typename VariantType>
    bool CheckVariant(const VariantType&,
                      const boost::python::object&,
                      SizeT<0>)
    {
      return false;
    }

    /// Recursion through the variant types checking the python object for the type at position N - 1.
    template<typename... Types, size_t N>
    bool CheckVariant(const boost::variant<Types...>& variant,
                      const boost::python::object& pyObject,
                      SizeT<N>)
    {
      using Scalar = typename std::tuple_element < N - 1, std::tuple<Types...> >::type;
      if (checkPythonObject(pyObject , Scalar())) {
        return true;
      }
      return CheckVariant(variant, pyObject, SizeT < N - 1 > ());
    }

    /// Check if the python object is a tuple of objects of the correct types
    template<typename... Types>
    bool checkPythonObject(const boost::python::object& pyObject, const boost::variant<Types...> variant)
    {
      return CheckVariant(variant, pyObject, SizeT<sizeof...(Types)>());
    }

    /** Check if a python object can be converted to boost::optional
     * @return true if pyObject is None or convertible to the type of the boost::optional
     */
    template<typename Type>
    bool checkPythonObject(const boost::python::object& pyObject, const boost::optional<Type>&)
    {
      return pyObject.is_none() or checkPythonObject(pyObject, Type());
    }

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
     * @param vector The vector whose items should be stored to a python list.
     * @return object python list where the vector is stored.
     */
    template<typename Value>
    boost::python::list convertToPythonObject(const std::vector<Value>& vector)
    {
      boost::python::list outputList;
      for (const auto& value : vector) {
        outputList.append(convertToPythonObject(value));
      }
      return outputList;
    }

    /** Write the content of a std::set to a python set
     * @param set the set that should be converted
     * @return python object for the set
     */
    template<typename Value>
    boost::python::object convertToPythonObject(const std::set<Value>& set)
    {
      boost::python::object result(boost::python::handle<>(PySet_New(nullptr)));
      for (const auto& value : set) {
        PySet_Add(result.ptr(), convertToPythonObject(value).ptr());
      }
      return result;
    }

    /**
     * Writes content of a std::map to a python dict.
     *
     * @param map The map whose items should be stored to a python dict.
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
     * To fill the python list from the C++ std::tuple we need again TMP methods.
     * The variadic template std::tuple is copied by the recursive defined template function GetTuple,
     * the overloaded argument (type SizeT<>) of the function serves as a counter for the recursion depth.
     */
    template < typename TupleType >
    inline void GetTuple(const TupleType& tuple, boost::python::list& pyList)
    {
      GetTuple(tuple, pyList, SizeT<std::tuple_size<TupleType>::value>());
    }

    /** Break recursion when all members are already appended to the list */
    template < typename TupleType >
    inline void GetTuple(const TupleType&, boost::python::list&, SizeT<0>) { }

    /** Append elements of the tuple to pyList by recursively calling this function with the
     * element index as argument type */
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
     * @param tuple The tuple whose items should be stored to a python tuple.
     * @return The python tuple where the content of the map is stored.
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
     * Helper function object to unpack a value from a variant to a python object
     */
    class convertToPythonObjectVisitor : public boost::static_visitor<boost::python::object> {
    public:
      /** actually convert a value. */
      template<class T>
      boost::python::object operator()(const T& value) const
      {
        return convertToPythonObject(value);
      }
    };

    /**
     * Writes content of a boost::variant to a python object.
     *
     * @param variant The variant whose content should be stored in a python object.
     * @return The python object where the the content of the map is stored.
     */
    template<typename... Types>
    boost::python::object convertToPythonObject(const boost::variant<Types...>& variant)
    {
      return boost::apply_visitor(convertToPythonObjectVisitor(), variant);
    }

    /**
     * Convert a boost::optional to python
     *
     * @param optional the optional object to convert
     * @return If the optional is not set return None, otherwise just return
     *         the converted type it contains.
     */
    template<typename Type>
    boost::python::object convertToPythonObject(const boost::optional<Type>& optional)
    {
      if (optional) {
        return convertToPythonObject(*optional);
      } else {
        return boost::python::object();
      }
    }

    /**
     * --------------- From Python TO C++ Converter ------------------------
     */

    /**
     * Reads a scalar type (int, double, string, bool) from a python object.
     *
     * @param pyObject Python object which stores the scalar type.
     * @param Scalar A scalar.
     * @return Scalar type, which holds the value from the python object
     */
    template<typename Scalar>
    Scalar convertPythonObject(const boost::python::object& pyObject, const Scalar)
    {

      Scalar tmpValue;
      boost::python::extract<Scalar> valueProxy(pyObject);
      if (valueProxy.check()) {
        tmpValue = static_cast<Scalar>(valueProxy);
      } else {
        throw std::runtime_error(std::string("Could not convert value: Expected type '") + Type<Scalar>::name() + "' instead of '" +
                                 pyObject.ptr()->ob_type->tp_name + "'.");
      }
      return tmpValue;

    }

    /**
     * Reads std::vector from a python object.
     * If the python object isn't a list, a std::vector with the given object as single entry is returned.
     *
     * @param pyObject Python object which stores the vector.
     * @return Vector, which holds the vector from the python object
     */
    template<typename Value>
    std::vector<Value> convertPythonObject(const boost::python::object& pyObject, const std::vector<Value>&)
    {

      std::vector<Value> tmpVector;
      if (PyList_Check(pyObject.ptr()) or PyGen_Check(pyObject.ptr())) {
        iteratePythonObject(pyObject, [&tmpVector](const boost::python::object & element) {
          tmpVector.emplace_back(convertPythonObject(element, Value()));
          return true;
        });
      } else {
        tmpVector.emplace_back(convertPythonObject(pyObject, Value()));
      }
      return tmpVector;
    }

    /** Convert a python set to a std::set */
    template<typename Value>
    std::set<Value> convertPythonObject(const boost::python::object& pyObject, const std::set<Value>&)
    {
      std::set<Value> result;
      if (PyAnySet_Check(pyObject.ptr())) {
        iteratePythonObject(pyObject, [&result](const boost::python::object & element) {
          result.emplace(convertPythonObject(element, Value()));
          return true;
        });
      } else {
        result.emplace(convertPythonObject(pyObject, Value()));
      }
      return result;
    }


    /**
     * Reads std::map from a python object.
     *
     * @param pyObject Python object which stores the map.
     * @return Map, which holds the map from the python object
     */
    template<typename Key, typename Value>
    std::map<Key, Value> convertPythonObject(const boost::python::object& pyObject, const std::map<Key, Value>&)
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
      if ((unsigned)boost::python::len(pyTuple) != N) {
        throw std::runtime_error(std::string("Given python tuple has length ") +
                                 std::to_string(boost::python::len(pyTuple)) +
                                 ", expected " + std::to_string(N));
      }
      SetTuple(tuple, pyTuple, SizeT<N>());
    }

    /** Break recursion once all elements have been set */
    template < typename TupleType >
    inline void SetTuple(TupleType&, const boost::python::tuple&, SizeT<0>) { }

    /** Set the elements of the tuple recursively by calling with the function
     * with the next tuple index and converting one element */
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
     * @return std::tuple<Types...>, which holds the map from the python object
     */
    template<typename... Types>
    std::tuple<Types...> convertPythonObject(const boost::python::object& pyObject, const std::tuple<Types...>&)
    {
      std::tuple<Types...> tmpTuple;
      const boost::python::tuple& pyTuple = static_cast<const boost::python::tuple&>(pyObject);
      SetTuple(tmpTuple, pyTuple);
      return tmpTuple;
    }

    /**
     * @{
     * TMP (Template Meta Programming )
     * The given python object is written into the given c++ boost variant.
     * To fill the C++ boost::variant we need again TMP methods.
     * The variadic template boost::variant is filled by the recursive defined template function SetVariant,
     * the overloaded argument (type SizeT<>) of the function serves as a counter for the recursion depth.
     */
    template <typename... Types>
    inline void SetVariant(boost::variant<Types...>&, const boost::python::object& pyObject, SizeT<0>)
    {
      throw std::runtime_error(std::string("Could not set module parameter: Expected type '") +
                               Type<boost::variant<Types...> >::name() + "' instead of '" +
                               pyObject.ptr()->ob_type->tp_name + "'.");
    }

    /** Recursively go through all possible types of the variant and check which one is a possible conversion */
    template < typename... Types, size_t N >
    inline void SetVariant(boost::variant<Types...>& variant, const boost::python::object& pyObject, SizeT<N>)
    {
      using Scalar = typename std::tuple_element < N - 1, std::tuple<Types...> >::type;
      if (checkPythonObject(pyObject, Scalar())) {
        try {
          Scalar value = convertPythonObject(pyObject, Scalar());
          variant = value;
          return;
        } catch (std::runtime_error& e) {
        }
      }
      // Conversion failed - try next type
      SetVariant(variant, pyObject, SizeT < N - 1 > ());
    }
    /** @} */

    /**
     * Reads boost::variant from a python object.
     *
     * @param pyObject Python object which stores the value.
     * @return boost::variant<Types...>, which holds the value from the python object
     */
    template<typename... Types>
    boost::variant<Types...> convertPythonObject(const boost::python::object& pyObject, const boost::variant<Types...>&)
    {
      boost::variant<Types...> tmpVariant;
      SetVariant(tmpVariant, pyObject, SizeT<sizeof...(Types)>());
      return tmpVariant;
    }

    /**
     * Convert a python object to boost::optional.
     *
     * @return boost::optional which is not set if pyObject is None. Otherwise
     *         it will be converted to the type of the boost optional
     */
    template<typename Type>
    boost::optional<Type> convertPythonObject(const boost::python::object& pyObject, const boost::optional<Type>&)
    {
      boost::optional<Type> tmpOptional = boost::none;
      if (!pyObject.is_none()) {
        tmpOptional = convertPythonObject(pyObject, Type());
      }
      return tmpOptional;
    }
  }
}
