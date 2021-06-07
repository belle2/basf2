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

#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>


using namespace std;
using namespace Belle2;
using namespace group_helper;

//! Total number of sectors (eight, a constant)
const int c_TotalSectors = 8;

//! Total number of sections
const int c_TotalSections = 6;

//! Total number of sections
const int c_TotalSections_per_EKLM_BKLM = 2;

//! Total number of layers (fifteen, a constant)
const int c_TotalLayers = 15;




const int i_forward_eklm = 2;
const int i_backward_eklm = 3;
const int i_forward_bklm = 0;
const int i_backward_bklm = 1;


// part of unused old Trigger collection
const std::string m_klmtrackCollectionName = "TRGKLMTracks";
const std::string m_klmhitCollectionName = "TRGKLMHits";
// end


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(KLMTrigger)

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

std::vector<int> layer_string_list_to_integer(const std::string& instr)
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
KLMTriggerModule::KLMTriggerModule() : Module()
{
  setDescription("KLM trigger simulation");
  setPropertyFlags(c_ParallelProcessingCertified);


  addParam("nLayerTrigger", m_nLayerTrigger, "", 5);

  addParam("LayerUsed", m_dummy_used_layers, "List of layers used for the simulation", string("2:16"));



  addParam("TRGKLMSummery", m_klmTriggerSummery,  "Name of the StoreArray holding the Trigger Summery", string("TRGKLMSummery"));

}

void KLMTriggerModule::initialize()
{

  B2INFO("KLMTrigger: m_dummy_used_layers " << m_dummy_used_layers);
  m_layerUsed = layer_string_list_to_integer(m_dummy_used_layers);

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


  StoreArray<KLMTrgSummary> KLMTrgSummaries(m_klmTriggerSummery);
  KLMTrgSummaries.registerInDataStore();
  KLMTrgSummaries.registerRelationTo(klmDigits);


}

void KLMTriggerModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2DEBUG(100, "KLMTrigger: Experiment " << evtMetaData->getExperiment() << ", run " << evtMetaData->getRun());

}


void KLMTriggerModule::endRun()
{

}






// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(KLM_type, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(section_t, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(sector_t, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(layer_t, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(layer_count, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(layer_mask, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(n_triggered, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(sector_mask, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(sector_mask_or, int);// cppcheck-suppress  noExplicitConstructor

AXIS_NAME(n_sections_trig, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(back2back_t, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(isectors_t, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(TriggerCut, int);// cppcheck-suppress  noExplicitConstructor
AXIS_NAME(vetoCut, int);

int to_i_sector(int KLM_type_, int section_)
{
  if (KLM_type_ == KLMElementNumbers::c_BKLM  && section_ == BKLMElementNumbers::c_BackwardSection) {
    return i_backward_bklm;
  } else if (KLM_type_ == KLMElementNumbers::c_BKLM  && section_ == BKLMElementNumbers::c_ForwardSection) {
    return i_forward_bklm;
  } else if (KLM_type_ == KLMElementNumbers::c_EKLM  && section_ == EKLMElementNumbers::c_BackwardSection) {
    return i_backward_eklm;
  } else if (KLM_type_ == KLMElementNumbers::c_EKLM  && section_ == EKLMElementNumbers::c_ForwardSection) {
    return i_forward_eklm;
  }
  B2FATAL("unecpected KLM type");
  return 0;
}


template <typename T>
int to_i_sector(const T& e)
{
  return to_i_sector(KLM_type(e) , section_t(e));
}


template <typename AXIS_NAME_T, typename CONTAINER_T>
auto to_bit_mask(const CONTAINER_T& container)
{
  return std::accumulate(container.begin(), container.end(), 0,  [](const auto & lhs, const auto & rhs) {  return lhs | (1 << AXIS_NAME_T(rhs));});
}

unsigned int countBits(unsigned int n)
{
  return std::bitset<16>(n).count();
}


template <typename CONTAINER_T>
auto to_sector_bit_mask(const CONTAINER_T& container, TriggerCut TriggerCut_ , vetoCut vetoCut_ = 0)
{
  int ret = 0;
  auto back = container.back();
  for (const auto& e : container) {
    int bitcount      = countBits(layer_mask(e));
    int bitcount_or   = countBits(layer_mask(back) | layer_mask(e));
    int bitcount_back = countBits(layer_mask(back));
    if (bitcount >= TriggerCut_) {
      ret |= (1 << sector_t(e));
    } else if (
      bitcount_or >= TriggerCut_
      && bitcount_back < vetoCut_
      && bitcount < vetoCut_
    ) {
      ret |= (1 << sector_t(e));
      ret |= (1024);
    }
    back = e;
  }
  return ret;
}


void KLMTriggerModule::event()
{



  StoreArray<KLMTrgSummary> klmTriggerSummery(m_klmTriggerSummery);
  if (!klmTriggerSummery.isValid())
    return;

  auto summery = klmTriggerSummery.appendNew();
  StoreArray<KLMDigit> klmDigits;


  auto hits = fill_vector(klmDigits,
                          [](auto klmdig) -> KLM_type  { return klmdig->getSubdetector(); },
                          [](auto klmdig) -> section_t { return klmdig->getSection(); },
                          [](auto klmdig) -> sector_t  { return klmdig->getSector() - 1;},
                          [](auto klmdig) -> layer_t   { return klmdig->getLayer() - 1;}
                         );


  sort(hits);
  drop_duplicates(hits);


  auto grouped = group<KLM_type, section_t, sector_t>::apply(hits,
                                                             [](const auto & e1) -> layer_count { return e1.size(); },
                                                             [](const auto & e1) -> layer_mask  { return to_bit_mask<layer_t>(e1);}
                                                            );


  sort(grouped);


  auto summery2 = group<KLM_type>::apply(grouped,
  [&](const auto & e1) -> n_sections_trig    {
    return count_if(e1, group_helper::greater_equal<int>{m_nLayerTrigger  }, layer_count());
  });


  auto n_triggered_sectors2 = group<KLM_type, section_t>::apply(grouped,
                              [&](const auto & e1) -> sector_mask    { return to_sector_bit_mask(e1, TriggerCut(m_nLayerTrigger));},
                              [&](const auto & e1) -> sector_mask_or { return to_sector_bit_mask(e1, TriggerCut(m_nLayerTrigger), vetoCut(m_nLayerTrigger));},
                              [ ](const auto & e1) -> isectors_t     { return to_i_sector(e1[0]); }
                                                               );


  auto summery1 = group<KLM_type>::apply(n_triggered_sectors2,
  [](const auto & e1) -> back2back_t {
    return  count_if(e1, group_helper::greater<int>{0}, sector_mask()) >= c_TotalSections_per_EKLM_BKLM;
  }
                                        );

  summery->setBKLM_n_trg_sectors(first_or_default(summery2, KLM_type(KLMElementNumbers::c_BKLM), 0 , n_sections_trig{}));
  summery->setEKLM_n_trg_sectors(first_or_default(summery2, KLM_type(KLMElementNumbers::c_EKLM), 0 , n_sections_trig{}));

  summery->setSector_mask_Backward_Barrel(first_or_default(n_triggered_sectors2, isectors_t(i_backward_bklm), 0 , sector_mask{}));
  summery->setSector_mask_Forward_Barrel(first_or_default(n_triggered_sectors2, isectors_t(i_forward_bklm) , 0 , sector_mask{}));
  summery->setSector_mask_Backward_Endcap(first_or_default(n_triggered_sectors2, isectors_t(i_backward_eklm), 0 , sector_mask{}));
  summery->setSector_mask_Forward_Endcap(first_or_default(n_triggered_sectors2, isectors_t(i_forward_eklm) , 0 , sector_mask{}));

  summery->Sector_mask_OR_Backward_Barrel = first_or_default(n_triggered_sectors2, isectors_t(i_backward_bklm), 0 , sector_mask_or{});
  summery->Sector_mask_OR_Forward_Barrel = first_or_default(n_triggered_sectors2, isectors_t(i_forward_bklm) , 0 , sector_mask_or{});
  summery->Sector_mask_OR_Backward_Endcap = first_or_default(n_triggered_sectors2, isectors_t(i_backward_eklm), 0 , sector_mask_or{});
  summery->Sector_mask_OR_Forward_Endcap = first_or_default(n_triggered_sectors2, isectors_t(i_forward_eklm) , 0 , sector_mask_or{});

  summery->setBKLM_back_to_back_flag(first_or_default(summery1, KLM_type(KLMElementNumbers::c_BKLM), 0 , back2back_t{}));
  summery->setEKLM_back_to_back_flag(first_or_default(summery1, KLM_type(KLMElementNumbers::c_EKLM), 0 , back2back_t{}));

}