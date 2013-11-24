#include "ArichHVMonitor.h"

extern "C" {
  //#include <nsm/nsm2.h>
  //#include "ArichHVStatus.h"
}

#include <dqm/Histo1F.h>
#include <dqm/TimedGraph1F.h>
#include <dqm/TablePanel.h>
#include <dqm/TabbedPanel.h>
#include <dqm/CanvasPanel.h>
#include <dqm/Text.h>

#include <system/Time.h>

#include <base/StringUtil.h>
#include <base/Debugger.h>

#include <iostream>

using namespace Belle2;

ArichHVMonitor::ArichHVMonitor()
  : AbstractMonitor("ArichHVMonitor")
{
}

void ArichHVMonitor::init() throw()
{
  HistoPackage* package = getPackage();
  _label_state = (MonLabel*)package->addMonObject(new MonLabel("label_state", 20));
  _label_state->setUpdated(true);
  _label_state->setBounds(0.3, 0.12, 0.4, 0.06);
  FontProperty* font = new FontProperty(MonColor::WHITE, 1.6);
  font->setAlign("center middle");
  _label_state->setFont(font);

  _label_voltage_mon = (MonLabel*)package->addMonObject(new MonLabel("label_voltage_mon", 25));
  _label_voltage_mon->setUpdated(true);
  _label_voltage_mon->setLineColor(MonColor::NON);
  _label_voltage_mon->setFillColor(MonColor::NON);
  _label_voltage_mon->setBounds(0.5, 0.25, 0.4, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  _label_voltage_mon->setFont(font);

  _label_current_mon = (MonLabel*)package->addMonObject(new MonLabel("label_current_mon", 25));
  _label_current_mon->setUpdated(true);
  _label_current_mon->setLineColor(MonColor::NON);
  _label_current_mon->setFillColor(MonColor::NON);
  _label_current_mon->setBounds(0.5, 0.4, 0.4, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  _label_current_mon->setFont(font);

  _gr_voltage = (TimedGraph1*)package->addHisto(new TimedGraph1F("gr_voltage", "Monitored voltage;;Voltage [digit]", 602, 0, 600 * 5));
  _gr_voltage->fixMinimum(0);
  _gr_voltage->fixMaximum(10000);
  _gr_voltage->setUpdated(true);
  _gr_voltage->setLine(new LineProperty(MonColor::RED));
  _gr_voltage->setFill(new FillProperty(MonColor::PINK));
  _gr_voltage->setDrawOption("AL");

  _gr_current = (TimedGraph1*)package->addHisto(new TimedGraph1F("gr_current", "Monitored current;;Current [digit]", 602, 0, 600 * 5));
  _gr_current->fixMinimum(0);
  _gr_current->fixMaximum(500);
  _gr_current->setUpdated(true);
  _gr_current->setLine(new LineProperty(MonColor::RED));
  _gr_current->setFill(new FillProperty(MonColor::PINK));
  _gr_current->setDrawOption("AL");

  RootPanel* root_panel = getRootPanel();
  TablePanel* table = new TablePanel("table_main", 2, 1);
  root_panel->add(table);
  CanvasPanel* canvas = new CanvasPanel("c_labels", "States");
  Text* text = new Text("", "Status summary", 0.5, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.8);
  font->setAlign("center middle");
  text->setFont(font);
  canvas->add(text);
  text = new Text("", "Voltage : ", 0.1, 0.275);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  text = new Text("", "Current : ", 0.1, 0.425);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  canvas->add(_label_state);
  canvas->add(_label_voltage_mon);
  canvas->add(_label_current_mon);
  table->add(canvas);
  TabbedPanel* tab_graph = new TabbedPanel("tab_graph");
  canvas = new CanvasPanel("c_voltage_gr", "Monitored voltage");
  canvas->add(_gr_voltage);
  tab_graph->add("Voltage", canvas);
  canvas = new CanvasPanel("c_current_gr", "Monitored current");
  canvas->add(_gr_current);
  tab_graph->add("Current", canvas);
  table->add(tab_graph);
}

void ArichHVMonitor::update(NSMData* data) throw()
{
  try {
    if (!data->isAvailable()) return;
    //std::cout << data->toSQLValues() << std::endl;
    //return;
    _gr_voltage->addPoint(data->getInt32("voltage_mon"));
    _gr_current->addPoint(data->getInt32("current_mon"));
    _label_state->setFontColor(MonColor::WHITE);
    int state = data->getByte("status");
    if (state == 201) {
      _label_state->setLineColor(MonColor::GREEN);
      _label_state->setFillColor(MonColor::RUNNING_GREEN);
      _label_state->setText("RUNNING");
    } else if (state == 221) {
      _label_state->setLineColor(MonColor::YELLOW);
      _label_state->setFillColor(MonColor::ORANGE);
      _label_state->setText("RAMP UP");
    } else if (state == 222) {
      _label_state->setLineColor(MonColor::YELLOW);
      _label_state->setFillColor(MonColor::ORANGE);
      _label_state->setText("RAMP DOWN");
    } else if (state == 101) {
      _label_state->setLineColor(MonColor::LIGHT_GRAY);
      _label_state->setFillColor(MonColor::GRAY);
      _label_state->setText("OFF");
    } else {
      _label_state->setLineColor(MonColor::PINK);
      _label_state->setFillColor(MonColor::RED);
      if (state == 111) _label_state->setText("ERROR");
      else if (state == 211) _label_state->setText("TRIP");
      else if (state == 212) _label_state->setText("OVER CURRENT");
      else if (state == 213) _label_state->setText("OVER VOLTAGE");
      else _label_state->setText("UNKNOWN");
    }
    _label_voltage_mon->setText(Belle2::form("%d [digit]", data->getInt32("voltage_mon")));
    _label_current_mon->setText(Belle2::form("%d [digit]", data->getInt32("current_mon")));
  } catch (const IOException& e) {
    Belle2::debug("NSM error:%s", e.what());
  }
}

