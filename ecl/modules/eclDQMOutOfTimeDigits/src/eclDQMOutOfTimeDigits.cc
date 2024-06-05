/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDQMOutOfTimeDigits/eclDQMOutOfTimeDigits.h>

/* Basf2 headers. */
#include <analysis/VariableManager/Manager.h>
#include <framework/core/HistoModule.h>
#include <mdst/dataobjects/TRGSummary.h>

/* ROOT headers. */
#include <TDirectory.h>
#include <TH1F.h>

/* C++ headers. */
#include <stdexcept>

//NAMESPACE(S)
using namespace Belle2;

REG_MODULE(ECLDQMOutOfTimeDigits);

ECLDQMOutOfTimeDigitsModule::ECLDQMOutOfTimeDigitsModule()
  : HistoModule()
{
  //Set module properties.
  setDescription("ECL Data Quality monitoring module. See header file for the detailed description.");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify parallel processing.

  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));
}

ECLDQMOutOfTimeDigitsModule::~ECLDQMOutOfTimeDigitsModule()
{
}


void ECLDQMOutOfTimeDigitsModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.

  TDirectory* dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  // Create all necessary histograms for out-of-time ECLCalDigits counts
  for (auto& event_type : {"rand", "dphy", "physics"}) {
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string key_name = event_type + std::string("_") + ecl_part;
      TString hist_name    = "out_of_time_" + key_name;
      // Max possible value in the histogram is 8736 (=total number of crystals
      // in ECL), however the overflow over 2000 is not relevant for the
      // distribution shape.
      h_out_of_time[key_name] = new TH1F(hist_name, "", 250, 0, 2000);

      // Set titles
      h_out_of_time[key_name]->GetXaxis()->SetTitle("Out-of-time ECLCalDigits");
      TString title = "Out-of-time ECLCalDigits";
      title += " (";
      title += ecl_part;
      title += " of ECL, ";
      title += event_type;
      title += " events)";
      h_out_of_time[key_name]->SetTitle(title);
    }
  }

  //cd into parent directory.
  oldDir->cd();
}

void ECLDQMOutOfTimeDigitsModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager.
}

void ECLDQMOutOfTimeDigitsModule::beginRun()
{
  std::for_each(h_out_of_time.begin(), h_out_of_time.end(), [](auto & it) {it.second->Reset();});
}

std::string ECLDQMOutOfTimeDigitsModule::getEventType()
{
  // If TRGSummary is not available, we assume that all events are physics events
  if (!m_l1Trigger.isValid()) return "physics";

  if (isRandomTrigger()) return "rand";

  bool bhatrig = false;
  try { bhatrig = m_l1Trigger->testInput("bha_delay"); }
  catch (const std::exception&) { bhatrig = false; }

  if (bhatrig) return "dphy";

  return "physics";
}

void ECLDQMOutOfTimeDigitsModule::event()
{
  std::string event_type = getEventType();

  for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
    std::string key_name = event_type + std::string("_") + ecl_part;
    std::string var_name = "nECLOutOfTimeCrystals";
    if (std::string(ecl_part) != "All") var_name += ecl_part;
    auto var = Variable::Manager::Instance().getVariable(var_name);
    if (!var) continue;
    double value =  std::get<double>(var->function(nullptr));
    h_out_of_time[key_name]->Fill(value);
  }
}

void ECLDQMOutOfTimeDigitsModule::endRun()
{
}


void ECLDQMOutOfTimeDigitsModule::terminate()
{
}

bool ECLDQMOutOfTimeDigitsModule::isRandomTrigger()
{
  if (!m_l1Trigger.isValid()) return false;
  return m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_RAND ||
         m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_POIS;
}

