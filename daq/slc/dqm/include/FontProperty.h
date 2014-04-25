#ifndef _Belle2_FontProperty_h
#define _Belle2_FontProperty_h

#include "daq/slc/dqm/Property.h"

#include <string>

namespace Belle2 {

  class FontProperty : public Property {

  public:
    FontProperty(const MonColor& color, float size = -1,
                 const std::string& weight = "",
                 const std::string& family = "") throw();
    FontProperty(float size = -1,
                 const std::string& weight = "",
                 const std::string& family = "") throw();
    virtual ~FontProperty() throw() {}

  public:
    float getSize() const throw() { return m_size; }
    const std::string& getFamily() const throw() { return m_family; }
    void setSize(float size) throw() { m_size = size; }
    void setFamily(const std::string& family) throw() { m_family = family; }
    void setAlign(const std::string& align) throw() { m_align = align; }
    void setAngle(const std::string& angle) throw() { m_angle = angle; }
    std::string toString() const throw();

  private:
    float m_size;
    std::string m_weight;
    std::string m_family;
    std::string m_align;
    std::string m_angle;

  };

};

#endif
