#include "daq/slc/apps/dqmviewd/template/TemplateDQMPackage.h"

#include <daq/slc/dqm/Histo1F.h>
#include <daq/slc/dqm/Histo2F.h>
#include <daq/slc/dqm/TimedGraph1F.h>
#include <daq/slc/dqm/TablePanel.h>
#include <daq/slc/dqm/TabbedPanel.h>
#include <daq/slc/dqm/CanvasPanel.h>
#include <daq/slc/dqm/Text.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

TemplateDQMPackage::TemplateDQMPackage(const std::string& name)
  : DQMPackage(name)
{

}

void TemplateDQMPackage::init()
{
  // getting histogram preloaded from mapped file
  HistoPackage* pack = getPackage();
  _h_ncpr = pack->getHisto("h_ncpr");
  _h_nevt = pack->getHisto("h_nevt");
  _h_size = pack->getHisto("h_size");
  _h_size2d = pack->getHisto("h_size2d");
  _h_ncpr_cpy = pack->getHisto("h_ncpr_cpy");
  _h_nevt_cpy = pack->getHisto("h_nevt_cpy");
  _h_size_cpy = pack->getHisto("h_size_cpy");

  // creating monitored objects
  _label_state = (MonLabel*)pack->addMonObject(new MonLabel("label_state", 20));
  _label_state->setUpdated(true);
  _label_state->setBounds(0.3, 0.12, 0.4, 0.06);
  FontProperty* font = new FontProperty(MonColor::WHITE, 1.6);
  font->setAlign("center middle");
  _label_state->setFont(font);
  _label_nevt = (MonLabel*)pack->addMonObject(new MonLabel("label_nevt", 25));
  _label_nevt->setUpdated(true);
  _label_nevt->setLineColor(MonColor::NON);
  _label_nevt->setFillColor(MonColor::NON);
  _label_nevt->setBounds(0.5, 0.25, 0.4, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  _label_nevt->setFont(font);
  _label_nevt_rate = (MonLabel*)pack->addMonObject(new MonLabel("label_nevt_rate", 25));
  _label_nevt_rate->setUpdated(true);
  _label_nevt_rate->setLineColor(MonColor::NON);
  _label_nevt_rate->setFillColor(MonColor::NON);
  _label_nevt_rate->setBounds(0.5, 0.4, 0.4, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  _label_nevt_rate->setFont(font);
  _gr_nevt = (TimedGraph1*)pack->addHisto(new TimedGraph1F("gr_nevt",
                                                           "Event rate;;event rate [Hz]",
                                                           62, 0, 60 * 5));
  _gr_nevt->fixMinimum(0);
  _gr_nevt->fixMaximum(30);
  _gr_nevt->setUpdated(true);
  _gr_nevt->setLine(new LineProperty(MonColor::RED));
  _gr_nevt->setFill(new FillProperty(MonColor::PINK));
  _gr_nevt->setDrawOption("AL");

  // creating panels
  TabbedPanel* tabpanel = new TabbedPanel("tab_cdc");
  TablePanel* table = new TablePanel("table_ncpr", 1, 1);
  tabpanel->add("Number of COPPERs", table);
  CanvasPanel* canvas = new CanvasPanel("c_ncpr", "Number of COPPERs");
  canvas->add(_h_ncpr);
  table->add(canvas);
  canvas = new CanvasPanel("c_size2d", "Data Size vs. Copper");
  canvas->add(_h_size2d);
  table->add(canvas);

  table = new TablePanel("table_size", 2, 1);
  tabpanel->add("Number of COPPERs", table);
  canvas = new CanvasPanel("c_size", "Data Size");
  canvas->add(_h_size);
  table->add(canvas);
  canvas = new CanvasPanel("c_nevt", "Number of Events");
  canvas->add(_h_nevt);
  table->add(canvas);

  table = new TablePanel("table_nevt", 2, 1);
  tabpanel->add("Number of COPPERs", table);
  canvas = new CanvasPanel("c_labels", "States");
  Text* text = new Text("", "Status summary", 0.5, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.8);
  font->setAlign("center middle");
  text->setFont(font);
  canvas->add(text);
  text = new Text("", "Event rate : ", 0.1, 0.275);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  text = new Text("", "Total Events : ", 0.1, 0.425);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  canvas->add(_label_state);
  canvas->add(_label_nevt);
  canvas->add(_label_nevt_rate);
  table->add(canvas);
  canvas = new CanvasPanel("c_graphs", "Rate stability");
  GAxis* axis_rate = new GAxis("axis_rate", "Event rate [Hz]");
  axis_rate->setTickLine(new LineProperty(MonColor::BLACK, 1));
  axis_rate->setLabelFont(new FontProperty(MonColor::BLACK, 1.2));
  axis_rate->setFont(new FontProperty(MonColor::BLACK, 1.2));
  axis_rate->setMin(0);
  axis_rate->fixMin(true);
  _gr_nevt->setLinkedAxis(axis_rate);
  canvas->setYAxis(axis_rate);
  canvas->add(_gr_nevt);
  table->add(canvas);

  // registering panel contents
  getRootPanel()->add(tabpanel);

  _time = Belle2::Time().getSecond();
}

void TemplateDQMPackage::update()
{
  double time = Belle2::Time().getSecond();
  double rate = 0;
  for (int n = 0; n < _h_nevt->getAxisX().getNbins(); n++) {
    rate += _h_nevt->getBinContent(n) - _h_nevt_cpy->getBinContent(n);
  }
  rate /= (time - _time);
  _h_ncpr_cpy->getData().copy(_h_ncpr->getData());
  _h_nevt_cpy->getData().copy(_h_nevt->getData());
  _h_size_cpy->getData().copy(_h_size->getData());
  _gr_nevt->addPoint(rate);
  _label_nevt_rate->setText(Belle2::form("%2.2f [Hz]", rate));
  _label_nevt->setText(Belle2::form("%d events", (int)(_h_nevt->getEntries())));
  _label_state->setFontColor(MonColor::WHITE);
  if (rate > 16.5 && rate < 20.5) {
    _label_state->setLineColor(MonColor::GREEN);
    _label_state->setFillColor(MonColor::RUNNING_GREEN);
    _label_state->setText("STABLE");
  } else {
    _label_state->setLineColor(MonColor::PINK);
    _label_state->setFillColor(MonColor::RED);
    if (rate > 20.5) {
      _label_state->setText("HIGH RATE");
    } else {
      _label_state->setText("LOW RATE");
    }
  }
  _time = time;
}
