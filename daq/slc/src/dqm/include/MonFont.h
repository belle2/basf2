#ifndef _Belle2_MonFont_hh
#define _Belle2_MonFont_hh

#include "dqm/MonColor.h"

#include <string>

namespace Belle2 {

  class MonFont {

  public:
    MonFont(const MonColor& color = MonColor::BLACK, int size = -1,
            const std::string& family = "") throw();
    virtual ~MonFont() throw() {}

  public:
    const MonColor& getColor() const throw() { return _color; }
    int getSize() const throw() { return _size; }
    const std::string& getFamily() const throw() { return _family; }
    void setColor(int r, int g, int b) throw() { _color.setRGB(r, g, b); }
    void setColor(const MonColor& color) throw() { _color = color; }
    void setSize(int size) throw() { _size = size; }
    void setFamily(const std::string& family) throw() { _family = family; }

  public:
    const MonFont& operator=(const MonFont& font) throw();

  private:
    MonColor _color;
    int _size;
    std::string _family;

  };

};

#endif
