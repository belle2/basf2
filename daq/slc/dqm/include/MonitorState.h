#ifndef _Belle2_MonitorState_h
#define _Belle2_MonitorState_h

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
    int getValue() const throw() { return m_value; }
    const std::string& getText() const throw() { return m_text; }
    void setValue(int value) throw() { m_value = value; }
    void setText(const std::string& text) throw() { m_text = text; }

  private:
    int m_value;
    std::string m_text;

  };

  bool operator==(int value, const MonitorState& state) throw();
  bool operator!=(int value, const MonitorState& state) throw();

};

#endif
