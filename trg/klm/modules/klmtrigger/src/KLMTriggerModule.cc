/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitri Liventsev                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/klm/modules/klmtrigger/KLMTriggerModule.h>
#include <trg/klm/modules/klmtrigger/group_helper.h>
#include <trg/klm/modules/klmtrigger/group_io.h>
#include <trg/klm/modules/klmtrigger/KLMAxis.h>

#include <trg/klm/modules/klmtrigger/KLM_Trig.h>
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

#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <iostream>




using namespace std;
using namespace Belle2;
using namespace Belle2::group_helper;











// part of unused old Trigger collection
const std::string m_klmtrackCollectionName = "TRGKLMTracks";
const std::string m_klmhitCollectionName = "TRGKLMHits";
// end




class KLMTriggerModule::geometry_data {
public:
  geometry_data(const std::string& FileName)
  {
    data = load_ttree<Belle2::geofit::KLM_geo_fit_t >(FileName, "my_ttree");
    std::sort(data.begin(), data.end());
  }
  std::vector<Belle2::geofit::KLM_geo_fit_t> data;
};


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KLMTrigger)
int geometryConverter_YZ(int section, int sector, int layer, int strip, bool isPhiReadout);
int geometryConverter_X(int layer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
struct compare {
  int key;
  explicit compare(int const& i): key(i) { }

  bool operator()(int const& i)
  {
    return (i == key);
  }
};


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

  addParam("LayerUsed", m_dummy_used_layers, "List of layers used for the simulation", string("0:16"));
  addParam("GEOMETRY_FILE", m_geometry_fileName, "path to the geometry file",
           std::string("/home/belle2/peschke/basf2/development/trg/klm/data/geometry_fit-2022-01-16.root"));

}





// cppcheck-suppress  noExplicitConstructor







void KLMTriggerModule::initialize()
{
  m_event_nr = 0;
  m_KLMTrgSummary.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
  std::cout <<  "KLMTrigger: m_dummy_used_layers " << m_dummy_used_layers << std::endl;
  m_layerUsed = layer_string_list_to_integer(m_dummy_used_layers);
  for (auto e : m_layerUsed) {
    std::cout <<  e << std::endl;
  }
  StoreArray<KLMDigit> klmDigits;
  klmDigits.isRequired();
  if (!klmDigits.isValid())
    return;

// unused
  StoreArray<KLMTriggerHit> klmTriggerHits(m_klmhitCollectionName);
  klmTriggerHits.registerInDataStore();
  klmTriggerHits.registerRelationTo(klmDigits);

  StoreArray<KLMTriggerTrack> klmTriggerTracks(m_klmtrackCollectionName);
  klmTriggerTracks.registerInDataStore();
  klmTriggerTracks.registerRelationTo(klmTriggerHits);
// end unused

  StoreArray<KLMTrgFittedTrack> KLMTrgFittedTrack_("KLMTrgFittedTrack");
  KLMTrgFittedTrack_.registerInDataStore();

  try {
    std::cout << m_geometry_fileName << std::endl;
    m_geo = std::make_shared<KLMTriggerModule::geometry_data>(m_geometry_fileName);
  } catch (const std::exception& er) {
    B2FATAL(er.what());
  }
}

void KLMTriggerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2DEBUG(100, "KLMTrigger: Experiment " << evtMetaData->getExperiment() << ", run " << evtMetaData->getRun());

}


void KLMTriggerModule::endRun()
{


}


int to_i_sector(int KLM_type_, int section_)
{
  if (KLM_type_ == KLMElementNumbers::c_BKLM && section_ == BKLMElementNumbers::c_BackwardSection) {
    return Belle2::KLM_TRG_definitions::i_backward_bklm;
  } else if (KLM_type_ == KLMElementNumbers::c_BKLM && section_ == BKLMElementNumbers::c_ForwardSection) {
    return Belle2::KLM_TRG_definitions::i_forward_bklm;
  } else if (KLM_type_ == KLMElementNumbers::c_EKLM && section_ == EKLMElementNumbers::c_BackwardSection) {
    return Belle2::KLM_TRG_definitions::i_backward_eklm;
  } else if (KLM_type_ == KLMElementNumbers::c_EKLM && section_ == EKLMElementNumbers::c_ForwardSection) {
    return Belle2::KLM_TRG_definitions::i_forward_eklm;
  }

  return 0;
}





void KLMTriggerModule::event()
{
  try {
    make_linear_fit();

    m_KLMTrgSummary.create();

    StoreArray<KLMDigit> klmDigits;

    auto hits = fill_vector(klmDigits,
                            [](auto klmdig) -> KLM_type   { return klmdig->getSubdetector(); },
                            [](auto klmdig) -> section_t  { return klmdig->getSection();     },
                            [](auto klmdig) -> isectors_t { return to_i_sector(klmdig->getSubdetector(), klmdig->getSection()); },
                            [](auto klmdig) -> sector_t   { return klmdig->getSector() - 1;  },
                            [](auto klmdig) -> plane_t    { return klmdig->getPlane()  - (klmdig->getSubdetector() == 2);  },
                            [](auto klmdig) -> layer_t    { return klmdig->getLayer()  - 1;  }
                           );


    auto summery =  make_trg(hits,  m_event_nr, m_nLayerTrigger, m_layerUsed);

    m_KLMTrgSummary->setBKLM_n_trg_sectors(BKLM_n_trg_sectors(summery));
    m_KLMTrgSummary->setEKLM_n_trg_sectors(EKLM_n_trg_sectors(summery));

    m_KLMTrgSummary->setSector_mask_Backward_Barrel(Sector_mask_Backward_Barrel(summery));
    m_KLMTrgSummary->setSector_mask_Forward_Barrel(Sector_mask_Forward_Barrel(summery));
    m_KLMTrgSummary->setSector_mask_Backward_Endcap(Sector_mask_Backward_Endcap(summery));
    m_KLMTrgSummary->setSector_mask_Forward_Endcap(Sector_mask_Forward_Endcap(summery));

    m_KLMTrgSummary->setSector_mask_OR_Backward_Barrel(Sector_mask_OR_Backward_Barrel(summery));
    m_KLMTrgSummary->setSector_mask_OR_Forward_Barrel(Sector_mask_OR_Forward_Barrel(summery));
    m_KLMTrgSummary->setSector_mask_OR_Backward_Endcap(Sector_mask_OR_Backward_Endcap(summery));
    m_KLMTrgSummary->setSector_mask_OR_Forward_Endcap(Sector_mask_OR_Forward_Endcap(summery));

    m_KLMTrgSummary->setBKLM_back_to_back_flag(BKLM_back_to_back_flag(summery));
    m_KLMTrgSummary->setEKLM_back_to_back_flag(EKLM_back_to_back_flag(summery));

    ++m_event_nr;
  } catch (const std::exception& er) {
    B2FATAL(er.what());
  }
}



template <typename T>
auto KLMDigits2VECTOR(int event_nr, const T& klmDigits)
{
  auto hits =  fill_vector(klmDigits,
                           [&](auto)       -> event_nr_t   { return event_nr; },
                           [](auto klmdig) -> KLM_type     { return klmdig->getSubdetector(); },
                           [](auto klmdig) -> section_t    { return klmdig->getSection(); },
                           [](auto klmdig) -> sector_t     { return klmdig->getSector();},
                           [](auto klmdig) -> plane_t      { return klmdig->getPlane();},
                           [](auto klmdig) -> layer_t      { return klmdig->getLayer();},
                           [](auto klmdig) -> strip_t      { return klmdig->getStrip();}
                          );
  sort(hits);
  drop_duplicates(hits);
  return hits;
}



template <typename T1, typename T2>
auto push_linear_fit_to_KLMTrgFittedTrack(const T1& linear_fited,  T2& KLMTrgFittedTrack_)
{
  for (const auto& e : linear_fited) {
    if (slopeXY_t(e) > 1e8) {
      continue;
    }
    auto FittedTrack =  KLMTrgFittedTrack_.appendNew();
    FittedTrack->setslopeXY(slopeXY_t(e)) ;
    FittedTrack->setInterceptXY(interceptXY_t(e));
    FittedTrack->setIpXY(ipXY_t(e)) ;
    FittedTrack->setChisqXY(chisqXY_t(e));
    FittedTrack->setSubdetector(KLM_type(e));
    FittedTrack->setSection(section_t(e));
    FittedTrack->setSector(sector_t(e));
    FittedTrack->setPlane(plane_t(e));
  }

}


void KLMTriggerModule::make_linear_fit()
{
  StoreArray<KLMDigit> klmDigits;
  StoreArray<KLMTrgFittedTrack> KLMTrgFittedTrack_("KLMTrgFittedTrack");

  auto hits = KLMDigits2VECTOR(m_event_nr, klmDigits);

  auto linear_fited =  klm_trig_linear_fit(hits, m_geo->data);


  push_linear_fit_to_KLMTrgFittedTrack(linear_fited, KLMTrgFittedTrack_);

}
