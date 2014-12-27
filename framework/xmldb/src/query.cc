#include "framework/xmldb/query.h"
#include "framework/xmldb/htobe.h"

#include <netinet/in.h>
#include <stdint.h>

#include <cassert>
#include <iostream>

// PostgreSQL
#include <libpq-fe.h>
#include "catalog/pg_type.h"

#include "framework/xmldb/connection.h"
#include "framework/xmldb/query_builder.h"

namespace Belle2 {
  namespace xmldb {

    Query::Query(Connection* _conn)
      : conn_(_conn), data_returned_(false), executed_(false), result_(nullptr)
    {
      assert(_conn);
    } // Query constructor

    Query::~Query()
    {
      if (result_) {
        ::PQclear(result_);
      }
    } // Query destructor

    bool Query::checkResult(const std::string& _query)
    {
      ::ExecStatusType execstatus = ::PQresultStatus(result_);
      if (execstatus == ::PGRES_TUPLES_OK) {
        numColumns_ = ::PQnfields(result_);
        numRows_ = ::PQntuples(result_);
      } // if

      if (execstatus == ::PGRES_COMMAND_OK) {
        executed_ = true;
        return true;
      } else if (execstatus == ::PGRES_TUPLES_OK) {
        data_returned_ = true;
        executed_ = true;
        return true;
      }
      // bad return code. report the error
      std::cerr << "Query: " << _query << std::endl;
      std::cerr << "Query status: " << execstatus << " (" <<
                ::PQresStatus(execstatus) << ") " << std::endl
                << "Error: " << ::PQresultErrorMessage(result_) << std::endl << std::flush;
      return false;
    } // Query::checkResult

    int Query::columnIndex(const std::string& _name)
    throw (std::logic_error)
    {
      if (!executed_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query has not been executed yet.");
      } // if
      if (!data_returned_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query returned no data.");
      } // if

      const int index = ::PQfnumber(result_, _name.c_str());
      if (index == -1) {
        std::string error("No such column: '");
        error.append(_name);
        error.append("'.");
        throw std::invalid_argument(error);
      } // if
      return index;
    } // Query::columnIndex

    bool Query::execute(const std::string& _query)
    {
      // use the more complex PQexecParams call to be able to specify "obtain the
      // results in binary format".
      result_ = ::PQexecParams(conn_->conn(), _query.c_str(),
                               0 /* no params follow */, nullptr, nullptr,
                               nullptr, nullptr,
                               1 /* return values in binary */);
      return checkResult(_query);
    } // Query::execute

    bool Query::executeGenerated(const QueryBuilder& _gen)
    {
      result_ = ::PQexecParams(conn_->conn(), _gen.query_.c_str(),
                               _gen.parameters_.size(), _gen.param_types_.data(),
                               _gen.param_values_.data(), _gen.param_lengths_.data(),
                               _gen.param_formats_.data(),
                               1 /* return values in binary */);
      return checkResult(_gen.query_);
    } // Query::executeGenerated

    const char* Query::getValue(int _row, int _col, bool* _is_null)
    throw (std::logic_error)
    {
      // verify state of the query
      if (!executed_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query has not been executed yet.");
      } // if
      if (!data_returned_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query returned no data.");
      } // if

      // verify input arguments
      if ((_col < 0) || (_col >= numColumns_)) {
        throw std::out_of_range("Column index out of range.");
      } // if
      if ((_row < 0) || (_row >= numRows_)) {
        throw std::out_of_range("Row index out of range.");
      } // if

      const bool null_data = ::PQgetisnull(result_, _row, _col);
      if (_is_null) {
        *_is_null = null_data;
        if (null_data) {
          return nullptr;
        } // if
      } // if
      if (null_data) {
        throw std::logic_error("NULL received, but not expected.");
      } // if

      const char* ptr = ::PQgetvalue(result_, _row, _col);
      assert(ptr);
      return ptr;
    } // Query::getValue

    std::vector<uint8_t> Query::getValueAsBlob(int _row, int _col,
                                               bool* _is_null)
    throw (std::logic_error)
    {
      const char* ptr = getValue(_row, _col, _is_null);
      if (!ptr) {
        return std::vector<uint8_t>();
      } // if

      ::Oid type = ::PQftype(result_, _col);
      if (type == BYTEAOID) {
        const uint8_t* val = reinterpret_cast<const uint8_t*>(ptr);
        const int length = ::PQgetlength(result_, _row, _col);
        return std::vector<uint8_t>(val, val + length);
      } else {
        throw std::logic_error("Type conversion not (yet) supported.");
      } // else
    } // Query::getValueAsBlob

    double Query::getValueAsFloat(int _row, int _col, bool* _is_null)
    throw (std::logic_error)
    {
      const char* ptr = getValue(_row, _col, _is_null);
      if (!ptr) {
        return 0;
      } // if

      ::Oid type = ::PQftype(result_, _col);
      if (type == FLOAT4OID) {
        const int32_t as_int = ntohl(*reinterpret_cast<const int32_t*>(ptr));
        return *reinterpret_cast<const float*>(&as_int);
      } else if (type == FLOAT8OID) {
        const int64_t as_int = ntohll(*reinterpret_cast<const int64_t*>(ptr));
        return *reinterpret_cast<const double*>(&as_int);
      } else {
        throw std::logic_error("Type conversion not (yet) supported.");
      } // else
    } // Query::getValueAsFloat

    int64_t Query::getValueAsInt(int _row, int _col, bool* _is_null)
    throw (std::logic_error)
    {
      const char* ptr = getValue(_row, _col, _is_null);
      if (!ptr) {
        return 0;
      } // if

      ::Oid type = ::PQftype(result_, _col);
      if (type == INT2OID) {
        const int16_t* val = reinterpret_cast<const int16_t*>(ptr);
        return ntohs(*val);
      } else if (type == INT4OID) {
        const int32_t* val = reinterpret_cast<const int32_t*>(ptr);
        return ntohl(*val);
      } else if (type == INT8OID) {
        const int64_t* val = reinterpret_cast<const int64_t*>(ptr);
        return ntohll(*val);
      } else {
        throw std::logic_error("Type conversion not (yet) supported.");
      } // else
      // just mutes a compiler warning
      return 0;
    } // Query::getValueAsInt

    std::string Query::getValueAsString(int _row, int _col, bool* _is_null)
    throw (std::logic_error)
    {
      const char* ptr = getValue(_row, _col, _is_null);
      if (!ptr) {
        return "(NULL)";
      } // if

      ::Oid type = ::PQftype(result_, _col);
      if (type == TEXTOID) {
        return std::string(ptr);
      } else if (type == CHAROID) {
        return std::string(1, *ptr);
      } else if (type == NAMEOID) {
        return std::string(ptr);
      } else if (type == VARCHAROID) {
        return std::string(ptr);
      } else if (type == XMLOID) {
        return std::string(ptr);
      } else {
        throw std::logic_error("Type conversion not (yet) supported.");
      } // else
    } // Query::getValueAsString

    timeval Query::getValueAsTimestamp(int _row, int _col, bool* _is_null)
    throw (std::logic_error)
    {
      if (!conn_->integer_datetime()) {
        throw std::logic_error("Only integer timestamps supported at the moment.");
      } // if

      const char* ptr = getValue(_row, _col, _is_null);
      if (!ptr) {
        return timeval();
      } // if

      ::Oid type = ::PQftype(result_, _col);
      if ((type == TIMESTAMPOID) || (type == TIMESTAMPTZOID)) {
        // usec since 1.1.2000
        const int64_t usec2000 = ntohll(*reinterpret_cast<const int64_t*>(ptr));

        /* # select EXTRACT(EPOCH FROM TIMESTAMPTZ('2000-01-01 00:00+00'));
           date_part
          -----------
           946684800
         */
        timeval ret;
        ret.tv_sec = (usec2000 / 1000000) + 946684800;
        ret.tv_usec = usec2000 % 1000000;
        return ret;
      } else {
        throw std::logic_error("Type conversion not (yet) supported.");
      } // else
    } // Query::getValueAsTimestamp

    int Query::numColumns() const throw (std::logic_error)
    {
      if (!executed_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query has not been executed yet.");
      } // if
      if (!data_returned_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query returned no data.");
      } // if
      return numColumns_;
    } // Query::numColumns

    int Query::numRows() const throw (std::logic_error)
    {
      if (!executed_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query has not been executed yet.");
      } // if
      if (!data_returned_) {
        // This was an INSERT, UPDATE or similar statement.
        throw std::logic_error("Query returned no data.");
      } // if
      return numRows_;
    } // Query::numRows

  } // namespace xmldb
} // namespace Belle2

