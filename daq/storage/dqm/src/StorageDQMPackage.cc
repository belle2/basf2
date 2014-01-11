#include "daq/storage/dqm/StorageDQMPackage.h"

#include <daq/slc/dqm/Histo1F.h>
#include <daq/slc/dqm/Histo2F.h>
#include <daq/slc/dqm/TimedGraph1F.h>
#include <daq/slc/dqm/TablePanel.h>
#include <daq/slc/dqm/TabbedPanel.h>
#include <daq/slc/dqm/CanvasPanel.h>
#include <daq/slc/dqm/Text.h>
#include <daq/slc/dqm/GAxis.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

using namespace Belle2;

StorageDQMPackage::StorageDQMPackage(const std::string& name,
                                     const std::string& filename)
  : DQMPackage(name, filename)
{

}

void StorageDQMPackage::init()
{
  // get histogram preloaded from mapped file
  HistoPackage* pack = getPackage();
  m_h_runinfo = pack->getHisto("h_runinfo");

  // create and register extra objects
  m_label_expno = (MonLabel*)pack->addMonObject(new MonLabel("m_label_expno", 20));
  m_label_expno->setUpdated(true);
  FontProperty* font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_expno->setFont(font);

  m_label_runno = (MonLabel*)pack->addMonObject(new MonLabel("m_label_runno", 20));
  m_label_runno->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_runno->setFont(font);

  m_label_starttime = (MonLabel*)pack->addMonObject(new MonLabel("m_label_starttime", 20));
  m_label_starttime->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_starttime->setFont(font);

  m_label_runlength = (MonLabel*)pack->addMonObject(new MonLabel("m_label_runlength", 20));
  m_label_runlength->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_runlength->setFont(font);

  m_label_event_rate = (MonLabel*)pack->addMonObject(new MonLabel("m_label_event_rate", 20));
  m_label_event_rate->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_event_rate->setFont(font);

  m_label_nevts = (MonLabel*)pack->addMonObject(new MonLabel("m_label_nevts", 20));
  m_label_nevts->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_nevts->setFont(font);

  m_label_data_rate = (MonLabel*)pack->addMonObject(new MonLabel("m_label_data_rate", 20));
  m_label_data_rate->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_data_rate->setFont(font);

  m_label_data_size = (MonLabel*)pack->addMonObject(new MonLabel("m_label_data_size", 20));
  m_label_data_size->setUpdated(true);
  font = new FontProperty(MonColor::WHITE, 1.4);
  font->setAlign("right middle");
  m_label_data_size->setFont(font);

  m_g_event_rate = (TimedGraph1*)pack->addHisto(new TimedGraph1F("m_g_event_rate",
                                                "Event rate 1;;Event rate [kHz]", 300, 0, 60 * 5));
  m_g_event_rate->fixMinimum(0);
  m_g_event_rate->fixMaximum(15);
  m_g_event_rate->setUpdated(true);
  m_g_event_rate->setLine(new LineProperty(MonColor::BLACK));

  m_g_data_rate = (TimedGraph1*)pack->addHisto(new TimedGraph1F("m_g_data_rate",
                                               "Data rate 1;;Data rate [MB/s]", 300, 0, 60 * 5));
  m_g_data_rate->fixMinimum(0);
  m_g_data_rate->fixMaximum(100);
  m_g_data_rate->setUpdated(true);
  m_g_data_rate->setLine(new LineProperty(MonColor::RED));

  // creating panels
  TablePanel* table = new TablePanel("summary_table", 2, 1);
  CanvasPanel* canvas = new CanvasPanel("c_test1", "Rate stability");
  canvas->setTitlePosition("left");
  canvas->add(m_g_event_rate);
  GAxis* axis_data_rate = new GAxis("axis_data_rate", "Data rate (MB/s)");
  axis_data_rate->setTickLine(new LineProperty(MonColor::RED, 2));
  axis_data_rate->setTickFont(new FontProperty(MonColor::RED, 0.9));
  axis_data_rate->setLabelFont(new FontProperty(MonColor::RED, 0.9));
  axis_data_rate->setFont(new FontProperty(MonColor::RED, 0.9));
  axis_data_rate->setMax(50);
  axis_data_rate->setMin(0);
  axis_data_rate->fixMax(true);
  axis_data_rate->fixMin(true);
  m_g_data_rate->setLinkedAxis(axis_data_rate);
  canvas->setY2Axis(axis_data_rate);
  canvas->add(m_g_data_rate);
  Legend* legend = new Legend("legend", 0.6, 0.02, 0.35, 0.8);
  legend->add(m_g_event_rate);
  legend->add(m_g_data_rate);
  canvas->setLegend(legend);
  table->add(canvas);

  canvas = new CanvasPanel("c_labels", "States");
  Text* text = new Text("", "Storage status", 0.5, 0.1);
  font = new FontProperty(MonColor::BLACK, 1.8);
  font->setAlign("center middle");
  text->setFont(font);
  canvas->add(text);

  int step = 1;
  m_label_runno->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_runno);
  text = new Text("", "Run # : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  m_label_starttime->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_starttime);
  text = new Text("", "Start time : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  m_label_runlength->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_runlength);
  text = new Text("", "Run length : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  m_label_event_rate->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_event_rate);
  text = new Text("", "Event rate : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  m_label_nevts->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_nevts);
  text = new Text("", "Total Events : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  m_label_data_rate->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_data_rate);
  text = new Text("", "Data rate : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  m_label_data_size->setBounds(0.5, 0.135 + step * 0.1, 0.4, 0.05);
  canvas->add(m_label_data_size);
  text = new Text("", "Data size : ", 0.05, 0.16 + step * 0.1);
  font = new FontProperty(MonColor::BLACK, 1.4);
  font->setAlign("left middle");
  text->setFont(font);
  canvas->add(text);
  step++;

  table->add(canvas);

  // registering panel contents
  getRootPanel()->add(table);

}

void StorageDQMPackage::update()
{
  TH1* h_runinfo = getHistMap()->getHist("h_runinfo");
  int expno = (int)h_runinfo->GetBinContent(1);
  int runno = (int)h_runinfo->GetBinContent(2);
  int subno = (int)h_runinfo->GetBinContent(3);
  //int evtno = h_runinfo->GetBinContent(4);
  double nevts = h_runinfo->GetBinContent(5);
  long long starttime = (long long)h_runinfo->GetBinContent(6);
  long long curtime = (long long)h_runinfo->GetBinContent(7) + starttime;
  int runlength = (int)(curtime - starttime);
  double data_size = h_runinfo->GetBinContent(8);
  double freq = h_runinfo->GetBinContent(9);
  double rate = h_runinfo->GetBinContent(10);
  long long record_time = Time(curtime).getSecond();
  m_g_event_rate->addPoint(record_time, freq);
  m_g_data_rate->addPoint(record_time, rate);
  m_label_event_rate->setText(Belle2::form("%2.2f [kHz]", freq));
  m_label_nevts->setText(Belle2::form("%d", (int)nevts));
  m_label_runno->setText(Belle2::form("%04d.%04d.%04d", expno, runno, subno));
  m_label_starttime->setText(Date(starttime).toString());
  m_label_runlength->setText(Belle2::form("%02d:%02d:%02d",
                                          (runlength / 60 / 60),
                                          ((runlength / 60) % 60),
                                          (runlength % 60)));
  m_label_data_rate->setText(Belle2::form("%2.2f [MB/s]", data_size / runlength / 1000. / 1000.));
  m_label_data_size->setText(Belle2::form("%2.2f [kB/event]", data_size / nevts / 1000.));
  getPackage()->setUpdateTime(record_time);
}
