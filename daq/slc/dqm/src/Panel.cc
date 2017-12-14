#include "daq/slc/dqm/Panel.h"

using namespace Belle2;

Panel::~Panel() throw()
{
  for (std::vector<Panel*>::iterator it = m_sub_panel_v.begin();
       it != m_sub_panel_v.end(); it++) {
    if ((*it) != NULL) delete(*it);
  }
}
