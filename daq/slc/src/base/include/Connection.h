#ifndef _Belle2_Connection_hh
#define _Belle2_Connection_hh

#include <string>

namespace Belle2 {

  class Connection {

  public:
    static const Connection UNKNOWN;
    static const Connection ONLINE;
    static const Connection OFFLINE;

  public:
    Connection() throw();
    Connection(const Connection& st)  throw();
    Connection(const std::string& st) throw() { *this = st; }
    Connection(int id) throw() { *this = id; }
    ~Connection()  throw();

  private:
    Connection(int id, const char* text)  throw();

  public:
    int getId() const throw() { return _id; }
    const char* getLabel() const throw() { return _label.c_str(); }

  public:
    bool operator==(const Connection& st) const throw();
    bool operator!=(const Connection& st) const throw();
    const Connection& operator=(const Connection& st) throw();
    const Connection& operator=(const std::string& msg) throw();
    const Connection& operator=(const char* msg) throw();
    const Connection& operator=(int i) throw();

  private:
    int _id;
    std::string _label;

  };

  inline bool Connection::operator==(const Connection& st) const throw()
  {
    return (_id == st._id);
  }
  inline bool Connection::operator!=(const Connection& st) const throw()
  {
    return (_id != st._id);
  }
  inline const Connection& Connection::operator=(const Connection& st) throw()
  {
    _id = st._id;
    _label = st._label;
    return *this;
  }

}

#endif
