#include "Panel.hh"

using namespace B2DQM;

Panel::~Panel() throw()
{
  for (std::vector<Panel*>::iterator it = _sub_panel_v.begin();
       it != _sub_panel_v.end(); it++) {
    if ((*it) != NULL) delete(*it);
  }
}
