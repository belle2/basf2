/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInputTest.cc
// Description : Input module for offline testing of histogram analysis code.
//-


#include <dqm/analysis/modules/DQMHistAnalysisInputTest.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <TROOT.h>

//#include <iostream>
using namespace Belle2;
using namespace boost::property_tree;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisInputTest);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisInputTestModule::DQMHistAnalysisInputTestModule()
  : DQMHistAnalysisModule()
{
  setDescription("Testing input file functionality for DQMHistAnalysisModules");
  addParam("Experiment", m_expno, "Experiment Nr", 26u);
  addParam("Run", m_runno, "Run Number List", 1u);
  addParam("RunType", m_runtype, "Run Type", std::string("physics"));
  addParam("Events", m_events, "Number of events for each run", 100u);
  addParam("NonFillMod", m_nonfillmod, "Non-Fill event Modulo, 0 disable", 7u);
  addParam("ConfigFiles", m_configs, "List of config files");
  B2DEBUG(1, "DQMHistAnalysisInputTest: Constructor done.");
}

void DQMHistAnalysisInputTestModule::initialize()
{
  m_eventMetaDataPtr.registerInDataStore();
  B2DEBUG(20, "DQMHistAnalysisInputTest: initialized.");

  // do config file parsing
  for (auto cf : m_configs) {
    ptree tree;
    json_parser::read_json(cf, tree);
    // just name it, the list is called "HistogramList"
    ptree pt = tree.get_child("HistogramList");
    for (auto pair : pt) {
      // the array entries do not have a name ...
      std::string name;
      std::string dir;
      unsigned int dtype = 0;
      unsigned int parameter = 0;
      unsigned int amount = 0;
      unsigned int fill = 0;
      unsigned int underflow = 0;
      std::string fitfunc;
      std::vector<double> fitparm;
      std::vector<double> histparm;
      for (auto pair2 : pair.second) {
        if (pair2.first == "Name") {
          name = pair2.second.get_value<std::string>();
        } else if (pair2.first == "Dir") {
          dir = pair2.second.get_value<std::string>();
        } else if (pair2.first == "Delta") {
          for (auto pair3 : pair2.second) {
            if (pair3.first == "Type") {
              dtype = pair3.second.get_value<unsigned int>();
            } else if (pair3.first == "Parameter") {
              parameter = pair3.second.get_value<unsigned int>();
            } else if (pair3.first == "Amount") {
              amount = pair3.second.get_value<unsigned int>();
            } else {
              std::cout << pair3.first << " is unknown parameter -> parse error"  << std::endl;
            }
          }// end Delta loop
        } else if (pair2.first == "Simulation") {
          for (auto pair3 : pair2.second) {
            if (pair3.first == "Function") {
              fitfunc = pair3.second.get_value<std::string>();
            } else if (pair3.first == "Fill") {
              fill = pair3.second.get_value<unsigned int>();
            } else if (pair3.first == "Underflow") {
              underflow = pair3.second.get_value<unsigned int>();
            } else if (pair3.first == "FPar") {
              for (auto pair4 : pair3.second) {
                fitparm.push_back(pair4.second.get_value<double>());
              }
            } else if (pair3.first == "HPar") {
              for (auto pair4 : pair3.second) {
                histparm.push_back(pair4.second.get_value<double>());
              }
            }
          }// end Simulation loop
        } else if (pair2.first == "Analysis") {
          // ignore
        } else if (pair2.first == "EPICS") {
          // ignore
        } else {
          std::cout << pair2.first << " is unknown parameter -> parse error"  << std::endl;
        }
      }

      if (dir == "" or name == "") {
        std::cout << "Both Dir and Name must be given. -> parse error"  << std::endl;
      } else {
        // as we are already on analyis side, we dont
        // need to put it in the directory
        // (even so it may be a cleaner solution)
        m_testHisto.push_back((TH1*) new TH1F(name.data(), "test Histo", histparm.at(0), histparm.at(1), histparm.at(2)));
        m_myNames.push_back(name);
        auto func = new TF1(TString(name) + "_func", fitfunc.data(), fitparm.at(0), fitparm.at(1));
        /*        func->SetParameters(100, 100, 30); // scale, mean, sigma
                func->SetParLimits(0, 100, 100);
                func->SetParLimits(1, 100, 900); // mean
                func->SetParLimits(2, 10, 100); // sigma*/
        m_func.push_back(func);
        m_fpar.push_back(fitparm);
        m_fill.push_back(fill);
        m_underflow.push_back(underflow);

        if (dtype > 0) addDeltaPar(dir, name, HistDelta::EDeltaType(dtype), parameter, amount); // only if we want delta for this one
      }
    }
  }

}

void DQMHistAnalysisInputTestModule::beginRun()
{
  B2INFO("DQMHistAnalysisInputTest: beginRun called. Run: " << m_runno);
  clearHistList();

  m_count = 0;
  for (auto h : m_testHisto) h->Reset();

}

void DQMHistAnalysisInputTestModule::event()
{
  // attention, the first call to event is BEFORE the first call to begin run!

  B2INFO("DQMHistAnalysisInputTest: event called.");

  initHistListBeforeEvent();

  if (m_count > m_events) {
    m_eventMetaDataPtr.create();
    m_eventMetaDataPtr->setEndOfData();
    B2INFO("DQMHistAnalysisInputTest: max event number reached, set EndOfData");
    return;
  }

  if (m_count == 0 || m_nonfillmod == 0 || (m_count % m_nonfillmod) != 0) {
    int i = 0;// index
    for (auto h : m_testHisto) {
      for (int p = 0; p < m_fpar[i].at(2); p++) {
        m_func[i]->SetParameter(p, m_fpar[i].at(3 + 2 * p) + m_count * (m_fpar[i].at(4 + 2 * p) - m_fpar[i].at(3 + 2 * p)) / m_events);
      }
      h->FillRandom(TString(m_myNames[i]) + "_func", m_fill[i]);
      if (m_underflow[i]) {
        h->SetBinContent(0, h->GetBinContent(0) + m_underflow[i]);
      }
      i++;
    }
  }

  for (auto h : m_testHisto) {
    addHist("test", h->GetName(), h);
  }

  /*{
    auto c=new TCanvas();
    c->cd();
    m_testHisto->Draw("hist");
    TString fn;
    fn.Form("testHist_%d.png",m_count);
    c->Print(fn);
  }*/

  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(m_expno);
  m_eventMetaDataPtr->setRun(m_runno);
  m_eventMetaDataPtr->setEvent(m_count);
  m_eventMetaDataPtr->setTime(m_count * 1e9);
  //setExpNr(m_expno); // redundant access from MetaData
  //setRunNr(m_runno); // redundant access from MetaData
  setRunType(m_runtype);
  setEventProcessed(m_count * 10000); // just fix it for now.
  B2INFO("DQMHistAnalysisInputTest: event finished. count: " << m_count);

  PlotDelta();
  m_count++;
}

void DQMHistAnalysisInputTestModule::PlotDelta(void)
{
  B2INFO("Delta");
  for (auto a : getDeltaList()) {
    B2INFO(a.first << " " << a.second->m_type << " " << a.second->m_parameter
           << " " << a.second->m_amountDeltas << " " << a.second->m_deltaHists.size());
  }

  for (auto n : m_myNames) {
    // we use fixed dir here as example
    std::string name = "test/" + n;
    auto c = new TCanvas();
    c->Divide(3, 3);
    c->cd(1);
    auto h = findHist(name);
    if (h) h->Draw("hist");

    // for testing, we use low level access to histograms here
    // for analysis, better use getDelta(name,index) base class function
    c->cd(2);
    auto it = getDeltaList().find(name);
    if (it != getDeltaList().end()) {
      h = it->second->m_lastHist;
      if (h) h->Draw("hist");

      for (unsigned int i = 0; i < it->second->m_amountDeltas; i++) {
        c->cd(i + 4);
        h = it->second->getDelta(i);
        if (h) {
          h->Draw("hist");
          c->cd(3);
          if (i == 0) {
            h->Draw();
          } else {
            h->SetLineColor(i + 2);
            h->Draw("same");
          }
        }
        if (i + 4 == 9) break;
      }
    }

    c->cd(0);

    TString fn;
    fn.Form("%s_Delta_%d.png", n.data(), m_count);
    c->Print(fn);
    delete c; // remove temporary canvas, otherwise it may end up in output
  }
}

void DQMHistAnalysisInputTestModule::endRun()
{
  B2INFO("DQMHistAnalysisInputTest: endRun called. Run: " << m_runno);

  B2INFO("DQMHistAnalysisInputTest: endRun: Histos");
  for (auto a : getHistList()) {
    B2INFO(a.first);
  }
  // The following will produce errors, as the histograms may not exist in endRun
  PlotDelta();
  B2INFO("DQMHistAnalysisInputTest: endRun: done");
}


void DQMHistAnalysisInputTestModule::terminate()
{
  B2INFO("DQMHistAnalysisInputTest: terminate called. Run: " << m_runno);
}
