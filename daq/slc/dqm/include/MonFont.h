#ifndef _Belle2_MonFont_h
#define _Belle2_MonFont_h

#include "daq/slc/dqm/MonColor.h"

#include <string>

namespace Belle2 {

  class MonFont {

  public:
    MonFont(const MonColor& color = MonColor::BLACK, int size = -1,
            const std::string& family = "") throw();
    virtual ~MonFont() throw() {}

  public:
    const MonColor& getColor() const throw() { return m_color; }
    int getSize() const throw() { return m_size; }
    const std::string& getFamily() const throw() { return m_family; }
    void setColor(int r, int g, int b) throw() { m_color.setRGB(r, g, b); }
    void setColor(const MonColor& color) throw() { m_color = color; }
    void setSize(int size) throw() { m_size = size; }
    void setFamily(const std::string& family) throw() { m_family = family; }

  public:
    const MonFont& operator=(const MonFont& font) throw();

  private:
    MonColor m_color;
    int m_size;
    std::string m_family;

  };

};

#endif
