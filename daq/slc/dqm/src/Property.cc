#include "daq/slc/dqm/Property.h"

using namespace Belle2;

Property::Property(const MonColor& color) throw()
  : _color(color), _label(""), _use_color(true) {}

Property::Property() throw()
  : _color(), _label(""), _use_color(false) {}
