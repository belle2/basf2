#ifndef _Belle2_MonitorConnection_hh
#define _Belle2_MonitorConnection_hh

#include <string>

namespace Belle2 {

  class MonitorConnection {

  public:
    static const MonitorConnection UNKNOWN;
    static const MonitorConnection ON;
    static const MonitorConnection OFF;

  private:
    int _value;
    std::string _text;

  private:
    MonitorConnection(int value, const std::string& text);

  public:
    MonitorConnection();
    ~MonitorConnection() throw();

  public:
    const MonitorConnection& operator=(int value) throw();
    const MonitorConnection& operator=(const MonitorConnection& connection) throw();
    bool operator==(int value) const throw();
    bool operator==(const MonitorConnection& connection) const throw();
    bool operator!=(int value) const throw() {
      return !(*this == value);
    }
    bool operator!=(const MonitorConnection& connection) const throw() {
      return !(*this == connection);
    }

  public:
    int getValue() const throw() { return _value; }
    const std::string& getText() const throw() { return _text; }
    void setValue(int value) throw() { _value = value; }
    void setText(const std::string& text) throw() { _text = text; }

  };

  bool operator==(int value, const MonitorConnection& connection) throw();
  bool operator!=(int value, const MonitorConnection& connection) throw();

};

#endif
