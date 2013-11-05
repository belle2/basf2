#ifndef _Belle2_FontProperty_hh
#define _Belle2_FontProperty_hh

#include "dqm/Property.h"

#include <string>

namespace Belle2 {

  class FontProperty : public Property {

  private:
    float _size;
    std::string _weight;
    std::string _family;
    std::string _align;
    std::string _angle;

  public:
    FontProperty(const MonColor& color, float size = -1,
                 const std::string& weight = "",
                 const std::string& family = "") throw();
    FontProperty(float size = -1,
                 const std::string& weight = "",
                 const std::string& family = "") throw();
    virtual ~FontProperty() throw() {}

  public:
    float getSize() const throw() { return _size; }
    const std::string& getFamily() const throw() { return _family; }
    void setSize(float size) throw() { _size = size; }
    void setFamily(const std::string& family) throw() { _family = family; }
    void setAlign(const std::string& align) throw() { _align = align; }
    void setAngle(const std::string& angle) throw() { _angle = angle; }
    std::string toString() const throw();

  };

};

#endif
