/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <sqlite3.h>

#include <optional>
#include <string>
#include <vector>
#include <tuple>
#include <stdexcept>

/** C++ wrapper around the sqlite C Api for convenient use of SQLite statements in C++
 *
 * This namespace contains a header only interface to the
 * <a href="https://www.sqlite.org/">sqlite library</a>. It mainly consists of
 * of a Connection class to wrap the underlying C api sqlite object in a safe way.
 * It is intended for comfortable read only access to sqlite files.
 *
 * It also contains three classes to handle SQL statements in sqlite files in a type safe way:
 *
 * * Statement: execute statement and return rows as std::tuple<Columns...>
 * * ObjectStatement: execute statement and return rows as any object with a
 *   constructor accepting all elements of the std::tuple<Columns...>
 * * SimpleStatement: execute a statement returning a single column and return
 *   that column without any wrapper.
 *
 */
namespace sqlite {
  /** Simple error class to be thrown if there is any sqlite error on any of the operations */
  class SQLiteError: public std::runtime_error {
  public:
    /** Construct an instance from a prefix string and an sqlite error code */
    explicit SQLiteError(int code, const std::string& prefix = ""): std::runtime_error(prefix + sqlite3_errstr(code)), m_code(code) {}
    /** Return the sqlite error code */
    int code() const { return m_code; }
  private:
    int m_code; /**< SQLite error code */
  };

  namespace detail {
    /** Throw a std::runtime_error with the sqlite error message if the given return code is not SQLITE_OK
     * @param code return code from a sqlite api call
     * @param prefix text to prepend in front of the sqlite error message
     */
    inline void checkSQLiteError(int code, const std::string& prefix = "")
    {
      if (code != SQLITE_OK) throw SQLiteError(code, prefix);
    }

    /** Struct to fill the different columns in a sqlite result row into a
     * std::tuple.
     *
     * So given a sqlite statement with result values and a std::tuple to adopt
     * all these values this class can be used to extract the values of the row
     * into the given references with the correct types.
     */
    class ColumnFiller {
    public:
      /** Create a new instance for the given statement */
      explicit ColumnFiller(sqlite3_stmt* statement): m_stmt(statement) {}
      /** Fill integer column */
      void operator()(int index, int& col) { col = sqlite3_column_int(m_stmt, index); }
      /** Fill 64bit integer column */
      void operator()(int index, int64_t& col) { col = sqlite3_column_int(m_stmt, index); }
      /** Fill double column */
      void operator()(int index, double& col) { col = sqlite3_column_double(m_stmt, index); }
      /** Fill string column */
      void operator()(int index, std::string& col)
      {
        // ptr is owned by sqlite, no need to free but we need to copy
        if (auto ptr = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, index)); ptr != nullptr) {
          col = ptr;
        }
      }
      /** Fill blob column */
      void operator()(int index, std::vector<std::byte>& col)
      {
        // ptr is owned by sqlite, no need to free but we need to copy
        if (auto ptr = reinterpret_cast<const std::byte*>(sqlite3_column_blob(m_stmt, index)); ptr != nullptr) {
          size_t bytes = sqlite3_column_bytes(m_stmt, index);
          col = std::vector<std::byte>(ptr, ptr + bytes);
        }
      }
      /** Fill an possibly null column in an optional */
      template<class T>
      void operator()(int index, std::optional<T>& col)
      {
        if (sqlite3_column_type(m_stmt, index) != SQLITE_NULL) {
          col.emplace();
          (*this)(index, *col);
        }
      }
    private:
      sqlite3_stmt* m_stmt; /**< statement on which to work on */
    };

    /** Bind the given parameter to the sqlite statement
     *
     * This functor object only serves the purpose to bind the values of the
     * given parameters to a sqlite statement. So given a tuple of parameters it
     * can be used to bind all the values in the tuple to the statement with the
     * correct type.
    */
    class ParameterBinder {
    public:
      /** Create a new object for the given statement*/
      explicit ParameterBinder(sqlite3_stmt* statement): m_stmt(statement) {}

      /** Bind the parameter with the given index to the statement */
      template<class T>
      void operator()(int index, T&& param)
      {
        // For some reason bind parameters start at one ...
        checkSQLiteError(bind(index + 1, std::forward<T>(param)));
      }
    private:
      /** bind an integer parameter */
      int bind(int index, int param) { return sqlite3_bind_int(m_stmt, index, param);}
      /** bind a 64bit integer parameter */
      int bind(int index, int64_t param) { return sqlite3_bind_int64(m_stmt, index, param); }
      /** bind a double parameter */
      int bind(int index, double param) { return sqlite3_bind_double(m_stmt, index, param);}
      /** bind a string parameter */
      int bind(int index, const std::string& param)
      {
        return sqlite3_bind_text(m_stmt, index, param.data(), param.size(), SQLITE_TRANSIENT);
      }
      /** bind a bytestream */
      int bind(int index, const std::vector<std::byte>& param)
      {
        return sqlite3_bind_blob(m_stmt, index, param.data(), param.size(), SQLITE_TRANSIENT);
      }
      /** bind an optional: if it doesn't have a value we bind NULL */
      template<class T>
      int bind(int index, const std::optional<T>& param)
      {
        if (param.has_value()) return bind(*param);
        return sqlite3_bind_null(m_stmt, index);
      }
      sqlite3_stmt* m_stmt; /**< statement on which to work on */
    };

    /** Implementation function to call a functor for each element in a tuple with the index and a reference to the value. */
    template<class Tuple, class Func, std::size_t ...Is>
    void visitTupleWithIndexImpl(Tuple&& t, Func&& f, std::index_sequence<Is...>)
    {
      // C++17 fold expression to just do all the calls in one  separated by "," operator ...
      (f(std::integral_constant<std::size_t, Is> {}, std::get<Is>(t)), ...);
    }

    /** Call a functor for each element in a tuple with the index and a reference to the value
     *
     * - for the first element of the tuple we call f(0, std::get<0>(tuple))
     * - for the second element we call f(1, std::get<1>(tuple))
     * - and so forth
     */
    template<class ... T, class Func>
    void visitTupleWithIndex(std::tuple<T...>& t, Func&& f)
    {
      visitTupleWithIndexImpl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)> {});
    }
  }

  /** SQLite prepared statement wrapper.
   *
   * This class is meant to prepare a SQLite statement where each row of the
   * result is supposed to fill one object of type ObjectType.
   *
   * Columns is the type of each of the columns in the returned data. When
   * calling getRow() the selected columns from the current result row will
   * be converted to the typesspecified by Columns and passed to the constructor
   * of ObjectType and the resulting object is returned.
   *
   * After calling execute one can iterate over the statement to get all rows.
   *
   * \code{.cc}
     ObjectStatement<TVector3, double, double, double> vectors(connection, "SELECT x,y,z from vectors");
     for(auto&& tvec3: vectors.execute()) {
       tvec3.Print();
     }
     \endcode
   *
   * \warning Be aware that you cannot iterate over this statement multiple times and
   *   should not have multiple independent iterators as the underlying sqlite statement
   *   object will change its state. One iteration over the rows per execute() call.
   */
  template<class ObjectType, class ... Columns>
  class ObjectStatement {
  public:
    /** each row gets converted to a instance of this type */
    using value_type = ObjectType;

    /** Iterator class to allow iterating over the rows */
    class iterator: public std::iterator<std::input_iterator_tag, value_type, size_t> {
    public:
      /** with a default constructor */
      iterator() = default;
      /** and a real constructor to a statement */
      explicit iterator(ObjectStatement* instance): m_instance(instance), m_row{0}
      {
        (*this)++;
      }
      /** increment to next row, stop in case we are done */
      iterator& operator++()
      {
        if (m_row < 0) return *this;
        ++m_row;
        if (!m_instance->step()) m_row = -1;
        return *this;
      }
      /** also postfix increment */
      iterator operator++(int) {iterator retval = *this; ++(*this); return retval;}
      /** and equality check */
      bool operator==(const iterator& other) const { return m_row == other.m_row; }
      /** as well as unequality check */
      bool operator!=(const iterator& other) const { return m_row != other.m_row; }
      /** and a dereference operator */
      value_type operator*() const { return m_instance->getRow(); }
    private:
      /** pointer to the statement */
      ObjectStatement* m_instance{nullptr};
      /** current row index */
      int64_t m_row{ -1};
    };

    /** Iterator to the beginning */
    iterator begin() { return iterator(this); }
    /** Iterator to the end */
    iterator end() const { return iterator(); }

    /** Create a statement for an existing database object */
    ObjectStatement(sqlite3* db, const std::string& query, bool persistent)
    {
      detail::checkSQLiteError(sqlite3_prepare_v3(db, query.data(), query.size(), persistent ? SQLITE_PREPARE_PERSISTENT : 0,
                                                  &m_statement, nullptr), "Cannot prepare database statement: ");
      if (auto cols = sqlite3_column_count(m_statement); cols != sizeof...(Columns)) {
        throw std::runtime_error("Number of column (" + std::to_string(cols) + ") doesn't match number of template parameters (" +
                                 std::to_string(sizeof...(Columns)) + ')');
      }
    }
    /** Clean up the statement */
    ~ObjectStatement() { if (m_statement) sqlite3_finalize(m_statement); }

    /** Execute the statement, providing all necessary parameters */
    template<class ... Parameters>
    ObjectStatement& execute(Parameters... parameters)
    {
      if (auto params = sqlite3_bind_parameter_count(m_statement); params != sizeof...(Parameters)) {
        throw std::runtime_error("Number of arguments (" + std::to_string(sizeof...(Parameters)) +
                                 ") doesn't match number of statement parameters (" + std::to_string(params) + ")");
      }
      sqlite3_reset(m_statement);
      auto parameter_tuple = std::make_tuple(parameters...);
      detail::visitTupleWithIndex(parameter_tuple, detail::ParameterBinder{m_statement});
      return *this;
    }

    /** Step to the next row in the result.
     * @returns true if a new row is loaded, false if there a no more rows.
     */
    bool step()
    {
      if (auto ret = sqlite3_step(m_statement); ret != SQLITE_ROW) {
        if (ret == SQLITE_DONE) return false;
        detail::checkSQLiteError(ret);
      }
      return true;
    }

    /** Return the current row */
    value_type getRow() const
    {
      std::tuple<Columns...> result;
      detail::visitTupleWithIndex(result, detail::ColumnFiller{m_statement});
      return std::make_from_tuple<value_type>(std::move(result));
    }
  private:
    /** pointer to the statement object */
    sqlite3_stmt* m_statement{nullptr};
  };

  /** Basic Statement returning a tuple of columns. In this case the result for each row is a std::tuple with all columns */
  template<class ... Columns> using Statement = ObjectStatement<std::tuple<Columns...>, Columns...>;
  /** Simple statement for only one column where the value is returned as is */
  template<class T> using SimpleStatement = ObjectStatement<T, T>;

  /** Simple wrapper for a SQLite database connection */
  class Connection {
  public:
    /** Create from filename */
    explicit Connection(const std::string& filename)
    {
      detail::checkSQLiteError(sqlite3_open_v2(filename.c_str(), &m_connection, SQLITE_OPEN_READONLY, nullptr));
    }
    /** And clean up */
    ~Connection() { if (m_connection) sqlite3_close_v2(m_connection); }
    /** Return a prepared statement for execution */
    template<class ... Columns>
    Statement<Columns...> prepare(const std::string& query, bool persistent = false) { return Statement<Columns...>(m_connection, query, persistent); }
    /** Convert to raw sqlite3 pointer to allow initialization of statements without calling prepare */
    operator sqlite3* () const { return m_connection; }
  private:
    /** Pointer to the sqlite database object */
    sqlite3* m_connection;
  };
}
