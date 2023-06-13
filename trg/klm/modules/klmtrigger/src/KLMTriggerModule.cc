/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/klm/modules/klmtrigger/KLMTriggerModule.h>
#include "trg/klm/modules/klmtrigger/group_helper.h"



#include "trg/klm/modules/klmtrigger/KLMAxis.h"

#include "trg/klm/modules/klmtrigger/KLM_Trig.h"
#include <trg/klm/modules/klmtrigger/klm_trig_linear_fit.h>


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




using namespace std;
using namespace Belle2;
using namespace Belle2::group_helper;








using namespace Belle2;
using namespace Belle2::group_helper::KLM_Coordinates_n;
using namespace Belle2::group_helper::KLM_Generic;
using namespace Belle2::group_helper;


// part of unused old Trigger collection
const std::string m_klmtrackCollectionName = "TRGKLMTracks";
const std::string m_klmhitCollectionName = "TRGKLMHits";
// end



struct KLMTriggerModule::geometry_data {
  std::vector<Belle2::group_helper::KLM_geo_fit_t> m_data;
};


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

  addParam("nLayerTrigger", m_nLayerTrigger, "", 8);

//  addParam("LayerUsed", m_dummy_used_layers, "List of layers used for the simulation", string("0:16"));


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




}

void KLMTriggerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  DBObjPtr<KLMTriggerParameters> KLMTriggerParameters;
  B2DEBUG(100, "KLMTrigger: Experiment " << evtMetaData->getExperiment() << ", run " << evtMetaData->getRun());
  if (not KLMTriggerParameters.isValid())
    B2FATAL("KLM trigger parameters are not available.");
  m_nLayerTrigger = KLMTriggerParameters->getNLayers();
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
    m_geometry = std::make_shared<KLMTriggerModule::geometry_data>();

    for (size_t i = 0; i < KLMTriggerParameters->getGeometryDataSize()  ; ++i) {
      m_geometry->m_data.emplace_back(
        Subdetector(KLMTriggerParameters->getSubdetector(i)),
        section(KLMTriggerParameters->getSection(i)),
        sector(KLMTriggerParameters->getSector(i)),
        layer(KLMTriggerParameters->getLayer(i)),
        plane(KLMTriggerParameters->getPlane(i)),
        geofit::slopeX(KLMTriggerParameters->getSlopeX(i)),
        geofit::offsetX(KLMTriggerParameters->getOffsetX(i)),
        geofit::slopeY(KLMTriggerParameters->getSlopeY(i)),
        geofit::offsetY(KLMTriggerParameters->getOffsetY(i))
      );
    }
  } catch (const std::exception& er) {
    B2FATAL(er.what());
  }

}


void KLMTriggerModule::endRun()
{


}


isectors_t to_i_sector(int KLM_type_, int section_)
{
  if (KLM_type_ == KLMElementNumbers::c_BKLM && section_ == BKLMElementNumbers::c_BackwardSection) {
    return isectors_t(Belle2::KLM_TRG_definitions::c_backward_bklm);
  } else if (KLM_type_ == KLMElementNumbers::c_BKLM && section_ == BKLMElementNumbers::c_ForwardSection) {
    return isectors_t(Belle2::KLM_TRG_definitions::c_forward_bklm);
  } else if (KLM_type_ == KLMElementNumbers::c_EKLM && section_ == EKLMElementNumbers::c_BackwardSection) {
    return isectors_t(Belle2::KLM_TRG_definitions::c_backward_eklm);
  } else if (KLM_type_ == KLMElementNumbers::c_EKLM && section_ == EKLMElementNumbers::c_ForwardSection) {
    return isectors_t(Belle2::KLM_TRG_definitions::c_forward_eklm);
  }

  return isectors_t(0);
}



template <typename T1, typename T2>
void fill_KLMTrgSummary(T1& KLMTrgSummary, const T2& summary)
{
  KLMTrgSummary.create();
  KLMTrgSummary->setBKLM_n_trg_sectors(BKLM_n_trg_sectors(summary));
  KLMTrgSummary->setEKLM_n_trg_sectors(EKLM_n_trg_sectors(summary));

  KLMTrgSummary->setSector_mask_Backward_Barrel(Sector_mask_Backward_Barrel(summary));
  KLMTrgSummary->setSector_mask_Forward_Barrel(Sector_mask_Forward_Barrel(summary));
  KLMTrgSummary->setSector_mask_Backward_Endcap(Sector_mask_Backward_Endcap(summary));
  KLMTrgSummary->setSector_mask_Forward_Endcap(Sector_mask_Forward_Endcap(summary));

  KLMTrgSummary->setSector_mask_OR_Backward_Barrel(Sector_mask_OR_Backward_Barrel(summary));
  KLMTrgSummary->setSector_mask_OR_Forward_Barrel(Sector_mask_OR_Forward_Barrel(summary));
  KLMTrgSummary->setSector_mask_OR_Backward_Endcap(Sector_mask_OR_Backward_Endcap(summary));
  KLMTrgSummary->setSector_mask_OR_Forward_Endcap(Sector_mask_OR_Forward_Endcap(summary));

  KLMTrgSummary->setBKLM_back_to_back_flag(BKLM_back_to_back_flag(summary));
  KLMTrgSummary->setEKLM_back_to_back_flag(EKLM_back_to_back_flag(summary));
}


template <typename T1, typename T2>
auto push_linear_fit_to_KLMTrgFittedTrack(const T1& linear_fited,  T2& KLMTrgFittedTrack_)
{
  for (const auto& e : linear_fited) {
    if (slopeXY_t(e) > 1e8) {     continue;    }
    auto FittedTrack =  KLMTrgFittedTrack_.appendNew();
    FittedTrack->setSlopeXY(slopeXY_t(e)) ;
    FittedTrack->setInterceptXY(interceptXY_t(e));
    FittedTrack->setIpXY(ipXY_t(e)) ;
    FittedTrack->setPlane(plane(e));
    FittedTrack->setChisqXY(chisqXY_t(e));
    FittedTrack->setSubdetector(Subdetector(e));
    FittedTrack->setSection(section(e));
    FittedTrack->setSector(sector(e));
    FittedTrack->setNhits(Nhits_t(e));

  }

}

void KLMTriggerModule::event()
{

  try {
    StoreArray<KLMDigit> klmDigits;
    StoreArray<KLMTrgFittedTrack> KLMTrgFittedTrack_("KLMTrgFittedTrack");
    StoreObjPtr<KLMTrgSummary> KLMTrgSummary;

    auto hits = fill_vector(klmDigits.getEntries(),
                            [&](auto) -> event_nr      { return event_nr(m_event_nr); },
                            [&](auto Index) -> Subdetector   { return Subdetector(klmDigits[Index]->getSubdetector()); },
                            [&](auto Index) -> section       { return section(klmDigits[Index]->getSection());     },
                            [&](auto Index) -> isectors_t    { return to_i_sector(klmDigits[Index]->getSubdetector(), klmDigits[Index]->getSection()); },
                            [&](auto Index) -> sector        { return sector(klmDigits[Index]->getSector() - 1);  },
                            [&](auto Index) -> plane         { return plane(klmDigits[Index]->getPlane()  - (klmDigits[Index]->getSubdetector() == 2));  },
                            [&](auto Index) -> layer         { return layer(klmDigits[Index]->getLayer()  - 1);  },
                            [&](auto Index) -> strip         { return strip(klmDigits[Index]->getStrip());}
                           );


    sort(hits);
    drop_duplicates(hits);
    erase_remove_if(hits, [layerUsed = this->m_layerUsed ](const auto & ele)  {  return !contains(layerUsed, layer(ele));  });

    auto summary =  make_trg(hits,  m_event_nr, m_nLayerTrigger);
    fill_KLMTrgSummary(KLMTrgSummary, summary);


    auto linear_fit =  klm_trig_linear_fit(hits, m_geometry->m_data);
    push_linear_fit_to_KLMTrgFittedTrack(linear_fit, KLMTrgFittedTrack_);


    ++m_event_nr;
  } catch (const std::exception& er) {
    B2FATAL(er.what());
  }
}









