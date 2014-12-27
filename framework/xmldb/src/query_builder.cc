#include "framework/xmldb/query_builder.h"
#include "framework/xmldb/htobe.h"
#include "framework/xmldb/query.h"

#include <netinet/in.h>
#include <string.h>

#include <libpq-fe.h>
#include "catalog/pg_type.h"

namespace Belle2 {
  namespace xmldb {
    QueryBuilder::~QueryBuilder()
    {
      std::vector< ::Oid >::size_type n = 0;
      for (std::vector< ::Oid >::const_iterator i = param_types_.begin();
           i != param_types_.end(); ++i, ++n) {
        if (*i == TEXTOID) {
          delete[] param_values_[n];
        }
      }
    }
    bool QueryBuilder::setParameterBlob(int _param,
                                        const std::vector<uint8_t>& _value)
    throw(std::logic_error)
    {
      verifyParameterId(_param);
      param_formats_[_param - 1] = 1 /* binary */;
      param_lengths_[_param - 1] = _value.size();
      // need not set parameters
      param_types_[_param - 1] = BYTEAOID;
      param_values_[_param - 1] = reinterpret_cast<const char*>(_value.data());
      return true;
    }  // QueryBuilder::setParameterBlob


    bool QueryBuilder::setParameterFloat(int _param, double _value)
    throw(std::logic_error)
    {
      verifyParameterId(_param);
      param_formats_[_param - 1] = 1 /* binary */;
      param_lengths_[_param - 1] = 8;
      int64_t as_int = *reinterpret_cast<int64_t*>(&_value);
      parameters_[_param - 1].int_value = htonll(as_int);
      param_types_[_param - 1] = FLOAT8OID;
      param_values_[_param - 1] = reinterpret_cast<char*>(
                                    &parameters_[_param - 1].int_value);
      return true;
    }  // QueryBuilder::setParameterFloat


    bool QueryBuilder::setParameterInt(int _param, int64_t _value)
    throw(std::logic_error)
    {
      verifyParameterId(_param);
      param_formats_[_param - 1] = 1 /* binary */;
      param_lengths_[_param - 1] = 8 /* bytes */;
      parameters_[_param - 1].int_value = htonll(_value);
      param_types_[_param - 1] = INT8OID;
      param_values_[_param - 1] = reinterpret_cast<char*>(
                                    &parameters_[_param - 1].int_value);
      return true;
    }  // QueryBuilder::setParameterInt


    bool QueryBuilder::setParameterString(int _param, const std::string& _value)
    throw(std::logic_error)
    {
      // play it safe: don't keep the pointer to the user-supplied string.
      // there is potential for optimization, here!
      char* x = new char[_value.size() + 1];
      memcpy(x, _value.c_str(), _value.size() + 1);
      verifyParameterId(_param);
      param_formats_[_param - 1] = 1 /* binary */;
      param_lengths_[_param - 1] = _value.size();
      // need not set parameters_
      param_types_[_param - 1] = TEXTOID;
      param_values_[_param - 1] = x; //&_value[0];
      return true;
    }  // QueryBuilder::setParameterString


    bool QueryBuilder::setParameterTimestamp(int _param, long _value)
    throw(std::logic_error)
    {
      timeval tv;
      tv.tv_sec = _value;
      tv.tv_usec = 0;
      return setParameterTimestamp(_param, tv);
    }  // QueryBuilder::setParameterTimestamp


    bool QueryBuilder::setParameterTimestamp(int _param, const timeval& _value)
    throw(std::logic_error)
    {
      verifyParameterId(_param);
      param_formats_[_param - 1] = 1 /* binary */;
      param_lengths_[_param - 1] = 8 /* bytes */;
      int64_t secs2000 = _value.tv_sec - 946684800;
      int64_t usecs2000 = secs2000 * 1000000 + _value.tv_usec;
      parameters_[_param - 1].int_value = htonll(usecs2000);
      param_types_[_param - 1] = TIMESTAMPTZOID;
      param_values_[_param - 1] = reinterpret_cast<char*>(
                                    &parameters_[_param - 1].int_value);
      return true;
    }  // QueryBuilder::setParameterTimestamp


    bool QueryBuilder::setParameterXML(int _param, const std::string& _value)
    throw(std::logic_error)
    {
      verifyParameterId(_param);
      param_formats_[_param - 1] = 1 /* binary */;
      param_lengths_[_param - 1] = _value.size();
      // need not set parameters_
      param_types_[_param - 1] = XMLOID;
      param_values_[_param - 1] = &_value[0];
      return true;
    }  // QueryBuilder::setParameterXML


    bool QueryBuilder::setStatement(const std::string& _stmt, int _num_params)
    {
      query_ = _stmt;

      param_formats_.clear();
      param_formats_.resize(_num_params);
      param_lengths_.clear();
      param_lengths_.resize(_num_params);
      param_types_.clear();
      param_types_.resize(_num_params);
      param_values_.clear();
      param_values_.resize(_num_params);
      parameters_.clear();
      parameters_.resize(_num_params);

      return true;
    }  // QueryBuilder::setStatement


    void QueryBuilder::verifyParameterId(int _param) throw(std::out_of_range)
    {
      if (_param < 0 || static_cast<std::vector<Parameter>::size_type>(_param)
          > parameters_.size()) {
        throw std::out_of_range("Parameter Id out-of-range.");
      }  // if
    }  // QueryBuilder::verifyParameterId


  } // namespace xmldb
} // namespace Belle2

