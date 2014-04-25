#ifndef _Belle2_Property_h
#define _Belle2_Property_h

#include "daq/slc/dqm/MonColor.h"

#include <string>

namespace Belle2 {

  class Property {

  public:
    Property(const MonColor& color) throw();
    Property() throw();
    virtual ~Property() throw() {}

  public:
    const MonColor& getColor() const throw() { return m_color; }
    void setColor(int r, int g, int b) throw() {
      m_use_color = true;
      m_color.setRGB(r, g, b);
    }
    void setColor(const MonColor& color) throw() {
      m_use_color = true;
      m_color = color;
    }
    void setLabel(const std::string& label) throw() {
      m_label = label;
    }

  public:
    virtual std::string toString() const throw() = 0;

  protected:
    MonColor m_color;
    std::string m_label;
    bool m_use_color;

  };

};

#endif
