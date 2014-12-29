#pragma once

#include "macros.h"
#include "leaf_double.h"
#include "leaf_int.h"
#include "leaf_string.h"
#include "tree.h"
#include "types.h"

#include <stdexcept>

namespace Belle2 {
  namespace xmldb {

    class Node;

    /** A pointer to a datum read from the database, for it to be used like
     *  a POD type. */
    template<typename T> class Value {
    public:
      /** Initialize the value with data read from the database.
       *  @param _t The tree loaded from the database.
       *  @param _xpath The XPath pointing to the requested datum.
       */
      Value(const Tree* _t, const std::string& _xpath)
        : node_(_t->getNodeData(_t->findNode(_xpath))) {
      }

      /** Assign a new value. */
      T operator=(const T& _newValue) throw (std::logic_error) {
        throw std::logic_error("Generic = is not implemented.");
      }

      /** Get the current value. */
      T operator()() throw (std::logic_error) {
        throw std::logic_error("Generic () is not implemented.");
      }

    private:
      /** Pointer the the Node containing the value within the XML tree. */
      Node* const node_;
    }; // class Value

    template<> ConfigDouble Value<ConfigDouble>::operator=(const ConfigDouble& _newValue)
    throw (std::logic_error)
    {
      if (!node_->isLeaf() || static_cast<Leaf*>(node_)->type() != Leaf::DataType::Double)
        throw std::logic_error("");
      static_cast<LeafDouble*>(node_)->setValue(_newValue);
      return _newValue;
    } // Value<ConfigDouble>::operator=

    template<> ConfigDouble Value<ConfigDouble>::operator()() throw (std::logic_error)
    {
      if (!node_->isLeaf() || static_cast<Leaf*>(node_)->type() != Leaf::DataType::Double)
        throw std::logic_error("");
      return static_cast<LeafDouble*>(node_)->value();
    } // Value<ConfigDouble>::operator()

    // TOCO: const & ??

    template<> ConfigInt Value<ConfigInt>::operator=(const ConfigInt& _newValue)
    throw (std::logic_error)
    {
      if (!node_->isLeaf() || static_cast<Leaf*>(node_)->type() != Leaf::DataType::Int)
        throw std::logic_error("");
      static_cast<LeafInt*>(node_)->setValue(_newValue);
      return _newValue;
    } // Value<ConfigInt>::operator=

    template<> ConfigInt Value<ConfigInt>::operator()() throw (std::logic_error)
    {
      if (!node_->isLeaf() || static_cast<Leaf*>(node_)->type() != Leaf::DataType::Int)
        throw std::logic_error("");
      return static_cast<LeafInt*>(node_)->value();
    } // Value<ConfigInt>::operator()

    template<> std::string Value<std::string>::operator=(const std::string& _newValue)
    throw (std::logic_error)
    {
      if (!node_->isLeaf() || static_cast<Leaf*>(node_)->type() != Leaf::DataType::String)
        throw std::logic_error("");
      static_cast<LeafString*>(node_)->setValue(_newValue);
      return _newValue;
    } // Value<ConfigInt>::operator=

    template<> std::string Value<std::string>::operator()() throw (std::logic_error)
    {
      if (!node_->isLeaf() || static_cast<Leaf*>(node_)->type() != Leaf::DataType::String)
        throw std::logic_error("");
      return static_cast<LeafString*>(node_)->value();
    } // Value<ConfigInt>::operator()
  } // namespace xmldb
} // namespace Belle2

