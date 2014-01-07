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

TemplateDQMPackage::TemplateDQMPackage(const std::string& name,
                                       const std::string& filename)
  : DQMPackage(name, filename)
{

}

void TemplateDQMPackage::init()
{
  // getting histogram preloaded from mapped file
  HistoPackage* pack = getPackage();
  h1 = pack->getHisto("h1");
  h1_2 = pack->getHisto("h1_2");
  h2 = pack->getHisto("h2");
  h3 = pack->addHisto(new Histo1F("h3", "Histogram 3;X value;Entires", 100, -20, 20));
  h3->setUpdated(true);
  h3->setLine(new LineProperty(MonColor::RED, 2));

  h3_2 = pack->addHisto(new Histo1F("h3_2", "Histogram 3-2;X value;Entires", 100, -20, 20));
  h3_2->setLine(new LineProperty(MonColor::BLACK, 2));

  h4 = pack->addHisto(new Histo2F("h4", "2D-Histogram;X value;Y value", 40, -40, 40, 30, -30, 30));
  h4->setLine(new LineProperty(MonColor::WHITE, 2));

  label_state = (MonLabel*)pack->addMonObject(new MonLabel("label_state", 20));
  label_state->setUpdated(true);
  label_state->setBounds(0.3, 0.12, 0.4, 0.06);
  FontProperty* font = new FontProperty(MonColor::WHITE, 1.6);
  font->setAlign("center middle");
  label_state->setFont(font);

  label_rate1 = (MonLabel*)pack->addMonObject(new MonLabel("label_rate1", 25));
  label_rate1->setUpdated(true);
  label_rate1->setLineColor(MonColor::NON);
  label_rate1->setFillColor(MonColor::NON);
  label_rate1->setBounds(0.5, 0.25, 0.4, 0.05);
  font = new FontProperty(MonColor::ORANGE, 1.4);
  font->setAlign("left middle");
  label_rate1->setFont(font);

  label_rate1_2 = (MonLabel*)pack->addMonObject(new MonLabel("label_rate1_2", 25));
  label_rate1_2->setUpdated(true);
  label_rate1_2->setLineColor(MonColor::NON);
  label_rate1_2->setFillColor(MonColor::NON);
  label_rate1_2->setBounds(0.5, 0.4, 0.4, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  label_rate1_2->setFont(font);

  label_rate1_2 = (MonLabel*)pack->addMonObject(new MonLabel("label_rate1_2", 25));
  label_rate1_2->setUpdated(true);
  label_rate1_2->setLineColor(MonColor::NON);
  label_rate1_2->setFillColor(MonColor::NON);
  label_rate1_2->setBounds(0.5, 0.4, 0.4, 0.05);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  label_rate1_2->setFont(font);

  g1 = (TimedGraph1*)pack->addHisto(new TimedGraph1F("g1", "Event rate 1;;event rate [Hz]", 62, 0, 60 * 5));
  g1->fixMinimum(0);
  g1->fixMaximum(30);
  g1->setUpdated(true);
  g1->setLine(new LineProperty(MonColor::BLACK));
  g1_2 = (TimedGraph1*)pack->addHisto(new TimedGraph1F("g1_2", "Event rate 2;;event rate [Hz]", 62, 0, 60 * 5));
  g1_2->setLine(new LineProperty(MonColor::RED));
  g1_2->setFill(new FillProperty(MonColor::PINK));
  g1_2->setUpdated(true);
  g1_2->setDrawOption("AL");

  // creating panels
  TabbedPanel* tabpanel = new TabbedPanel("gause_tab");
  TablePanel* table = new TablePanel("gaus_table", 2, 1);
  tabpanel->add("Test1", table);
  CanvasPanel* canvas = new CanvasPanel("c_test1", "Gaus distribution");
  canvas->setTitlePosition("left");
  canvas->add(h1);
  canvas->add(h1_2);
  Legend* legend = new Legend("legend", 0.6, 0.02, 0.35, 0.8);
  legend->add(h1);
  legend->add(h1_2);
  canvas->setLegend(legend);
  table->add(canvas);
  canvas = new CanvasPanel("c_test2", "2D Gaus distribution");
  canvas->add(h2);
  table->add(canvas);
  table = new TablePanel("gaus_table2", 2, 1);
  tabpanel->add("Test2", table);
  canvas = new CanvasPanel("c_test3", "Gaus distribution");
  canvas->add(h3);
  canvas->add(h3_2);
  table->add(canvas);
  canvas = new CanvasPanel("c_test4", "2D Gaus distribution");
  canvas->add(h4);
  table->add(canvas);
  table = new TablePanel("gaus_analysis", 2, 1);
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
  text = new Text("", "Total Entries : ", 0.1, 0.425);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  canvas->add(label_state);
  canvas->add(label_rate1);
  canvas->add(label_rate1_2);
  table->add(canvas);
  canvas = new CanvasPanel("c_graphs", "Rate stability");
  canvas->add(g1_2);
  GAxis* axis_entries = new GAxis("axis_entries", "Integrated entries");
  axis_entries->setTickLine(new LineProperty(MonColor::RED, 2));
  axis_entries->setTickFont(new FontProperty(MonColor::RED, 0.9));
  axis_entries->setLabelFont(new FontProperty(MonColor::RED, 0.9));
  axis_entries->setFont(new FontProperty(MonColor::RED, 1.2));
  axis_entries->setMin(0);
  axis_entries->fixMin(true);
  g1_2->setLinkedAxis(axis_entries);
  canvas->setY2Axis(axis_entries);
  GAxis* axis_rate = new GAxis("axis_rate", "Event rate [Hz]");
  axis_rate->setTickLine(new LineProperty(MonColor::BLACK, 1));
  axis_rate->setLabelFont(new FontProperty(MonColor::BLACK, 1.2));
  axis_rate->setFont(new FontProperty(MonColor::BLACK, 1.2));
  axis_rate->setMin(0);
  axis_rate->fixMin(true);
  g1->setLinkedAxis(axis_rate);
  canvas->setYAxis(axis_rate);
  canvas->add(g1);
  table->add(canvas);
  tabpanel->add("Status", table);

  // registering panel contents
  getRootPanel()->add(tabpanel);

  _time = Belle2::Time().get();
}

void TemplateDQMPackage::update()
{
  double time = Belle2::Time().get();
  double rate = 0;
  for (int n = 0; n < h1->getAxisX().getNbins(); n++) {
    rate += h1->getBinContent(n) - h3->getBinContent(n);
  }
  rate /= (time - _time);
  g1->addPoint(rate);
  g1_2->addPoint(h1->getEntries());
  h3->getData().copy(h1->getData());
  label_rate1->setText(Belle2::form("%2.2f [Hz]", rate));
  label_rate1_2->setText(Belle2::form("%d", (int)(h1->getEntries())));
  label_state->setFontColor(MonColor::WHITE);
  if (rate > 16.5 && rate < 20.5) {
    label_state->setLineColor(MonColor::GREEN);
    label_state->setFillColor(MonColor::RUNNING_GREEN);
    label_state->setText("STABLE");
  } else {
    label_state->setLineColor(MonColor::PINK);
    label_state->setFillColor(MonColor::RED);
    if (rate > 20.5) {
      label_state->setText("HIGH RATE");
    } else {
      label_state->setText("LOW RATE");
    }
  }
  _time = time;
}
