#include "daq/slc/dqm/Property.h"

using namespace Belle2;

Property::Property(const MonColor& color) throw()
  : m_color(color), m_label(""), m_use_color(true) {}

Property::Property() throw()
  : m_color(), m_label(""), m_use_color(false) {}
