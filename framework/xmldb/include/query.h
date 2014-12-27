#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include <stdint.h>
#include <sys/time.h>

#include <libpq-fe.h>

#include "framework/xmldb/macros.h"

namespace Belle2 {
  namespace xmldb {

    class Connection;
    class CopyQuery;
    class QueryBuilder;

//! Execute a query to a PostgreSQL database and read the returned data.
    class Query {
    public:
      //! Start a query over the given connection.
      explicit Query(Connection* _conn);
      ~Query();

      //! Find the index of a returned column by name.
      int columnIndex(const std::string& _name) throw(std::logic_error)
      MUST_USE_RESULT;

      //! Execute a plain-text query.
      bool execute(const std::string& _query);
      //! Execute a query with parameters.
      bool executeGenerated(const QueryBuilder& _gen);

      //! Get the data from a BYTEA column.
      std::vector<uint8_t> getValueAsBlob(int _row, int _col,
                                          bool* _is_null = nullptr)
      throw(std::logic_error) MUST_USE_RESULT;
      //! Get the data from a floating point column.
      double getValueAsFloat(int _row, int _col, bool* _is_null = nullptr)
      throw(std::logic_error) MUST_USE_RESULT;
      //! Get the data from an integer column.
      int64_t getValueAsInt(int _row, int _col, bool* _is_null = nullptr)
      throw(std::logic_error) MUST_USE_RESULT;
      //! Get the data from a string column.
      std::string getValueAsString(int _row, int _col, bool* _is_null = nullptr)
      throw(std::logic_error) MUST_USE_RESULT;
      //! Get the data from a timestamp column.
      timeval getValueAsTimestamp(int _row, int _col, bool* _is_null = nullptr)
      throw(std::logic_error) MUST_USE_RESULT;

      //! Get the number of columns returned by the query.
      int numColumns() const throw(std::logic_error) MUST_USE_RESULT;
      //! Get the number of rows returned by the query.
      int numRows() const throw(std::logic_error) MUST_USE_RESULT;

    private:
      bool checkResult(const std::string& _query);

      const char* getValue(int _row, int _col, bool* _is_null)
      throw(std::logic_error) MUST_USE_RESULT;

      Connection* const conn_;
      bool data_returned_;
      bool executed_;
      int numColumns_;
      int numRows_;
      ::PGresult* result_;
    };  // class Query

  }  // namespace xmldb
}  // namespace Belle2

