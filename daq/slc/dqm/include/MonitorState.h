#ifndef _Belle2_MonitorState_hh
#define _Belle2_MonitorState_hh

#include <string>

namespace Belle2 {

  class MonitorState {

  public:
    static const MonitorState UNKNOWN;
    static const MonitorState NOTUSED;
    static const MonitorState BOOTED;
    static const MonitorState READY;
    static const MonitorState RUNNING;
    static const MonitorState ERROR;

  private:
    int _value;
    std::string _text;

  private:
    MonitorState(int value, const std::string& text);
  public:
    ~MonitorState() throw();

  public:
    MonitorState();

  public:
    const MonitorState& operator=(int value) throw();
    const MonitorState& operator=(const MonitorState& state) throw();
    bool operator==(int value) const throw();
    bool operator==(const MonitorState& state) const throw();
    bool operator!=(int value) const throw() {
      return !(*this == value);
    }
    bool operator!=(const MonitorState& state) const throw() {
      return !(*this == state);
    }

  public:
    int getValue() const throw() { return _value; }
    const std::string& getText() const throw() { return _text; }
    void setValue(int value) throw() { _value = value; }
    void setText(const std::string& text) throw() { _text = text; }

  };

  bool operator==(int value, const MonitorState& state) throw();
  bool operator!=(int value, const MonitorState& state) throw();

};

#endif
