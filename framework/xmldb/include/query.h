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
      /** Check if the query was executed correctly. */
      bool checkResult(const std::string& _query);

      /** Get the binary representation of a returned datum. */
      const char* getValue(int _row, int _col, bool* _is_null)
      throw(std::logic_error) MUST_USE_RESULT;

      Connection* const conn_; /**< The connection used for this query. */
      bool data_returned_; /**< True, iff data has been returned by the query. */
      bool executed_; /**< True, iff the query has already been executed. */
      /** The number of columns in the query result.
       *  Only valid if data_returned_ is true.
       */
      int numColumns_;
      /** The number of rows in the query result.
       *  Only valid if data_returned_ is true.
       */
      int numRows_;
      ::PGresult* result_; /**< Handle for the result. */
    };  // class Query

  }  // namespace xmldb
}  // namespace Belle2

