#pragma once

#include <stdint.h>
#include <sys/time.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <libpq-fe.h>

namespace Belle2 {
  namespace xmldb {

//! Build PostgreSQL queries using binary transfer of parameters.
    /*!
        Note: In function calls, parameters are counted 1..num_params, as they
        are given in the request string.
    */
    class QueryBuilder {
    public:
      ~QueryBuilder();

      //! Bind a parameter as a raw binary value (for a BYTEA column).
      /*! The std::vector object passed must not be altered between the call to
          setParameterString and execution of the query.
      */
      bool setParameterBlob(int _param, const std::vector<uint8_t>& _value)
      throw(std::logic_error);
      //! Bind a parameter as a floating point value.
      bool setParameterFloat(int _param, double _value) throw(std::logic_error);
      //! Bind a parameter as an integer value.
      bool setParameterInt(int _param, int64_t _value) throw(std::logic_error);
      //! Bind a parameter as a string value.
      /*! The std::string object passed must not be altered between the call to
          setParameterString and execution of the query.
      */
      bool setParameterString(int _param, const std::string& _value)
      throw(std::logic_error);
      bool setParameterTimestamp(int _param, long _value)
      throw(std::logic_error);
      //! Bind a parameter as a timestamp value.
      /*! It is recommended to use the TIMESTAMPTZ SQL type.
      */
      bool setParameterTimestamp(int _param, const timeval& _value)
      throw(std::logic_error);
      //! Bind a parameter as an XML document.
      /*! The std::string object passed must not be altered between the call to
          setParameterString and execution of the query.
      */
      bool setParameterXML(int _param, const std::string& _value)
      throw(std::logic_error);

      /*! Initialize the object with the query string.

          Values to be passed to the database must be replaced with $1, $2, ... .
          It is assumed that the parameter numbers are consecutive, starting at 1.

          \param _stmt The query string.
          \param _num_params The number of parameters in the query.
          \return true, iff the call completed successfully.
      */
      bool setStatement(const std::string& _stmt, int _num_params);

    private:
      friend class Query;

      void verifyParameterId(int _param) throw(std::out_of_range);

      // prepare a union to easily support more data types later.
      typedef union {
        int64_t int_value;
      } Parameter;

      std::vector<int> param_formats_;
      std::vector<int> param_lengths_;
      std::vector< ::Oid > param_types_;
      std::vector<const char*> param_values_;
      std::vector<Parameter> parameters_;

      std::string query_;
    }; // class QueryBuilder

  } // namespace xmldb
} // namespace Belle2

