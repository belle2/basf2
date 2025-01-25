/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/klm/modules/klmtrigger/KLMTriggerModule.h>


#include "trg/klm/modules/klmtrigger/KLMAxis.h"

#include "trg/klm/modules/klmtrigger/KLM_Trig.h"
#include <trg/klm/modules/klmtrigger/klm_trig_linear_fit.h>

#include <trg/klm/modules/klmtrigger/IO_csv.h>


#include <klm/bklm/geometry/GeometryPar.h>
// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// event data
#include <framework/dataobjects/EventMetaData.h>

// digits
#include <klm/dataobjects/KLMDigit.h>

#include <trg/klm/dataobjects/KLMTriggerHit.h>
#include <trg/klm/dataobjects/KLMTriggerTrack.h>
#include <trg/klm/dataobjects/KLMTrgSummary.h>
#include <trg/klm/dataobjects/KLMTrgFittedTrack.h>






#include <trg/klm/dbobjects/KLMTriggerParameters.h>
#include <framework/database/DBObjPtr.h>


#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <iostream>

//#include <trg/klm/modules/klmtrigger/geometry.h>


using namespace std;
using namespace Belle2;









using namespace Belle2;
using namespace Belle2::KLM_TRG_definitions;



// part of unused old Trigger collection
const std::string m_klmtrackCollectionName = "TRGKLMTracks";
const std::string m_klmhitCollectionName = "TRGKLMHits";
// end






//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KLMTrigger);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


vector<string> split(const string& str, const string& delim)
{
  vector<string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delim, prev);
    if (pos == string::npos) pos = str.length();
    string token = str.substr(prev, pos - prev);
    if (!token.empty()) tokens.push_back(token);
    prev = pos + delim.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}

std::vector<int> layer_string_list_to_integer_range(const std::string& instr)
{
  std::vector<int> ret;
  auto str_spl = split(instr, ":");

  int start = std::stoi(str_spl[0]);
  int stop = std::stoi(str_spl[1]);
  for (int i = start; i < stop ; ++i) {
    ret.push_back(i);
  }

  return ret;
}
std::vector<int> layer_string_list_to_integer_list(const std::string& instr)
{
  std::vector<int> ret;
  auto str_spl = split(instr, ",");

  for (const auto& e : str_spl) {
    ret.push_back(std::stoi(e));
  }

  return ret;
}
std::vector<int> layer_string_list_to_integer(const std::string& instr)
{
  if (instr.find(":") != string::npos) {
    return layer_string_list_to_integer_range(instr);
  }
  if (instr.find(",") != string::npos)  {
    return layer_string_list_to_integer_list(instr);
  }
  std::vector<int> ret;
  return ret;
}
KLMTriggerModule::KLMTriggerModule() : Module()
{
  setDescription("KLM trigger simulation");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("y_cutoff", y_cutoff, "", 500);
  addParam("intercept_cutoff", m_intercept_cutoff, "", 500);
  addParam("CSV_Dump_Path", m_dump_Path, "", m_dump_Path);

}













void KLMTriggerModule::initialize()
{
  m_event_nr = 0;
  StoreArray<KLMDigit> klmDigits;
  klmDigits.isRequired();
  if (!klmDigits.isValid())
    return;

  StoreObjPtr<KLMTrgSummary> KLMTrgSummary;
  KLMTrgSummary.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);



  StoreArray<KLMTrgFittedTrack> KLMTrgFittedTrack_("KLMTrgFittedTrack");
  KLMTrgFittedTrack_.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);





// unused

  StoreArray<KLMTriggerHit> klmTriggerHits(m_klmhitCollectionName);
  klmTriggerHits.registerInDataStore();
  klmTriggerHits.registerRelationTo(klmDigits);

  StoreArray<KLMTriggerTrack> klmTriggerTracks(m_klmtrackCollectionName);
  klmTriggerTracks.registerInDataStore();
  klmTriggerTracks.registerRelationTo(klmTriggerHits);
// end unused

  if (!m_dump_Path.empty()) {
    get_IO_csv_handle().dump_path  = m_dump_Path;
    get_IO_csv_handle().do_dump    = true;
  }

}

void KLMTriggerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  DBObjPtr<KLMTriggerParameters> KLMTriggerParameters;
  B2DEBUG(100, "KLMTrigger: Experiment " << evtMetaData->getExperiment() << ", run " << evtMetaData->getRun());
  if (not KLMTriggerParameters.isValid())
    B2FATAL("KLM trigger parameters are not available.");

  try {
    m_layerUsed = layer_string_list_to_integer(KLMTriggerParameters->getWhichLayers());
    B2DEBUG(20, "KLMTrigger: m_layerUsed " << KLMTriggerParameters->getWhichLayers());
    for (auto e : m_layerUsed) {
      B2DEBUG(20, "KLMTrigger: layer " << e << " used.");
    }
  } catch (const std::exception& e) {
    B2FATAL("Something went wrong when parsing the 'm_whichLayers' string"
            << LogVar("string", KLMTriggerParameters->getWhichLayers())
            << LogVar("exception", e.what()));
  }
  try {
    m_klmtrg_layer_counter = std::make_shared< Belle2::klmtrg_layer_counter_t>();
    m_klm_trig_linear_fit = std::make_shared< Belle2::klm_trig_linear_fit_t>();
    m_klm_trig_linear_fit->set_y_cutoff(y_cutoff);
    m_klm_trig_linear_fit->set_intercept_cutoff(m_intercept_cutoff);


    m_klmtrg_layer_counter->set_NLayerTrigger(KLMTriggerParameters->getNLayers());

    for (auto e : m_layerUsed) {
      m_klmtrg_layer_counter->add_layersUsed(e);
    }


    Belle2::KLM_TRG_definitions::KLM_geo_fit_t e{};


    for (size_t i = 0 ; i < KLMTriggerParameters->getGeometryDataSize() ; ++i) {
      e.subdetector  = KLMTriggerParameters->getSubdetector(i);
      e.section      = KLMTriggerParameters->getSection(i);
      e.sector       = KLMTriggerParameters->getSector(i);
      e.layer        = KLMTriggerParameters->getLayer(i);
      e.plane        = KLMTriggerParameters->getPlane(i);
      e.slopeX       = KLMTriggerParameters->getSlopeX(i);
      e.offsetX      = KLMTriggerParameters->getOffsetX(i);
      e.slopeY       = KLMTriggerParameters->getSlopeY(i);
      e.offsetY      = KLMTriggerParameters->getOffsetY(i);
      m_klm_trig_linear_fit->add_geometry(e);

    }



  } catch (const std::exception& er) {
    B2FATAL(er.what());
  }

}


void KLMTriggerModule::endRun()
{


}









template <typename T1, typename T2>
auto push_linear_fit_to_KLMTrgFittedTrack(T1&& linear_fited,  T2& KLMTrgFittedTrack_)
{
  for (const auto& e : linear_fited) {
    //if (e.slopeXY >= 1e100) {     continue;    }
    auto FittedTrack =  KLMTrgFittedTrack_.appendNew();
    FittedTrack->setSlopeXY(e.slopeXY) ;
    FittedTrack->setInterceptXY(e.interceptXY);
    FittedTrack->setIpXY(e.ipXY) ;
    FittedTrack->setPlane(e.plane);
    FittedTrack->setChisqXY(e.chisqXY);
    FittedTrack->setSubdetector(e.subdetector);
    FittedTrack->setSection(e.section);
    FittedTrack->setSector(e.sector);
    FittedTrack->setNhits(e.Nhits);
    FittedTrack->setTrack_id(e.track_id);

  }

}







void KLMTriggerModule::event()
{

  try {
    ++m_event_nr;
    StoreArray<KLMDigit> klmDigits;
    StoreArray<KLMTrgFittedTrack> KLMTrgFittedTrack_("KLMTrgFittedTrack");
    StoreObjPtr<KLMTrgSummary> KLMTrgSummary;
    KLMTrgSummary.create();


    get_IO_csv_handle().event_nr = m_event_nr;
    Belle2::KLM_TRG_definitions::KLM_Digit_compact_t dummy {};
    auto hits = nt::algorithms::fill_vector(klmDigits.getEntries(),
    [&](auto Index) {
      const auto& digit = klmDigits[Index];


      dummy.event_nr    = m_event_nr;
      dummy.subdetector = digit->getSubdetector();
      dummy.section     = digit->getSection();
      dummy.sector      = digit->getSector();
      dummy.plane       = digit->getPlane();
      dummy.layer       = digit->getLayer();
      dummy.strip       = digit->getStrip();
      return dummy;

    });

    m_klmtrg_layer_counter->run(hits);
    m_klm_trig_linear_fit->run(hits);


    push_linear_fit_to_KLMTrgFittedTrack(m_klm_trig_linear_fit->get_result(), KLMTrgFittedTrack_);

    KLMTrgSummary->setBKLM_back_to_back_flag(
      m_klmtrg_layer_counter->get_BKLM_back_to_back_flag(KLMElementNumbers::c_BKLM)
    );

    KLMTrgSummary->setEKLM_back_to_back_flag(
      m_klmtrg_layer_counter->get_BKLM_back_to_back_flag(KLMElementNumbers::c_EKLM)
    );

    KLMTrgSummary->setBKLM_n_trg_sectors(
      m_klmtrg_layer_counter->get_n_sections_trig(KLMElementNumbers::c_BKLM)
    );
    KLMTrgSummary->setEKLM_n_trg_sectors(
      m_klmtrg_layer_counter->get_n_sections_trig(KLMElementNumbers::c_EKLM)
    );
    KLMTrgSummary->setSector_mask_Backward_Barrel(
      m_klmtrg_layer_counter->get_triggermask(KLMElementNumbers::c_BKLM, BKLMElementNumbers::c_BackwardSection)
    );

    KLMTrgSummary->setSector_mask_Forward_Barrel(
      m_klmtrg_layer_counter->get_triggermask(KLMElementNumbers::c_BKLM, BKLMElementNumbers::c_ForwardSection)
    );
    KLMTrgSummary->setSector_mask_Backward_Endcap(
      m_klmtrg_layer_counter->get_triggermask(KLMElementNumbers::c_EKLM, EKLMElementNumbers::c_BackwardSection)
    );

    KLMTrgSummary->setSector_mask_Forward_Endcap(
      m_klmtrg_layer_counter->get_triggermask(KLMElementNumbers::c_EKLM, EKLMElementNumbers::c_ForwardSection)
    );







    KLMTrgSummary->setSector_mask_SLF_Backward_Barrel(
      m_klm_trig_linear_fit->get_triggermask(KLMElementNumbers::c_BKLM, BKLMElementNumbers::c_BackwardSection)
    );

    KLMTrgSummary->setSector_mask_SLF_Forward_Barrel(
      m_klm_trig_linear_fit->get_triggermask(KLMElementNumbers::c_BKLM, BKLMElementNumbers::c_ForwardSection)
    );
    KLMTrgSummary->setSector_mask_SLF_Backward_Endcap(
      m_klm_trig_linear_fit->get_triggermask(KLMElementNumbers::c_EKLM, EKLMElementNumbers::c_BackwardSection)
    );

    KLMTrgSummary->setSector_mask_SLF_Forward_Endcap(
      m_klm_trig_linear_fit->get_triggermask(KLMElementNumbers::c_EKLM, EKLMElementNumbers::c_ForwardSection)
    );


    KLMTrgSummary->setSector_mask_SLF_OR_Backward_Barrel(
      m_klm_trig_linear_fit->get_triggermask_or(KLMElementNumbers::c_BKLM, BKLMElementNumbers::c_BackwardSection)
    );

    KLMTrgSummary->setSector_mask_SLF_OR_Forward_Barrel(
      m_klm_trig_linear_fit->get_triggermask_or(KLMElementNumbers::c_BKLM, BKLMElementNumbers::c_ForwardSection)
    );
    KLMTrgSummary->setSector_mask_SLF_OR_Backward_Endcap(
      m_klm_trig_linear_fit->get_triggermask_or(KLMElementNumbers::c_EKLM, EKLMElementNumbers::c_BackwardSection)
    );

    KLMTrgSummary->setSector_mask_SLF_OR_Forward_Endcap(
      m_klm_trig_linear_fit->get_triggermask_or(KLMElementNumbers::c_EKLM, EKLMElementNumbers::c_ForwardSection)
    );


  } catch (const std::exception& er) {
    B2FATAL(er.what());
  }
}









