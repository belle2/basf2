#include "daq/slc/apps/dqmviewd/SimpleDQMPackage.h"

#include "daq/slc/dqm/TabbedPanel.h"
#include "daq/slc/dqm/CanvasPanel.h"

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

SimpleDQMPackage::SimpleDQMPackage(const std::string& name,
                                   const std::string& filename)
  : DQMPackage(name, filename)
{

}

SimpleDQMPackage:: ~SimpleDQMPackage() throw()
{

}

void SimpleDQMPackage::init()
{

}

void SimpleDQMPackage::update()
{
  TabbedPanel* tabpanel = new TabbedPanel("tab_main");
  getRootPanel()->add(tabpanel);
  for (int i = 0; i < getPackage()->getNHistos(); i++) {
    Histo* histo = getPackage()->getHisto(i);
    std::string c_name = Belle2::form("c_%s", histo->getName().c_str());
    CanvasPanel* canvas = new CanvasPanel(c_name, histo->getTitle());
    canvas->add(histo);
    tabpanel->add(histo->getTitle(), canvas);
  }
}
