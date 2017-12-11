//+
// File : PXDTriggerFixerModule.cc
// Description : Desy 2016 event mismatch fixer, deprecated unless we mess it up
//
// Author : Bjoern Spruck, with help by Martin Ritter
// Date : 20 - Apr - 2016, Updated 03 - Jun - 2016
//-

#include <testbeam/vxd/modules/unpacking/PXDTriggerFixer.h>
#include <boost/spirit/home/support/detail/endian.hpp>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDTriggerFixer)

PXDTriggerFixerModule::PXDTriggerFixerModule(void)
{
  // the module CAN NOT run in parallel mode due to the buffering of previous events.
  // Module Description
  setDescription("Find PXD data for event which has been shifted by offset");

  //Parameter definition
  addParam("offset", m_offset, "Trigger offset of PXD content w.r.t. Event trigger number", int(0));
  addParam("refs_HLT", m_refs_HLT, "Use HLT trigger number as reference with PXD packet", false);
  addParam("refs_DHC", m_refs_DHC, "Use DHC trigger number as reference with PXD packet", false);
  addParam("refs_DHE", m_refs_DHE, "Use (first) DHE trigger number as reference with PXD packet", false);
  addParam("wants_Meta", m_wants_Meta, "Look for EvtMeta trigger number", false);
  addParam("wants_HLT", m_wants_HLT, "Look for HLT trigger number", false);
  addParam("wants_DHC", m_wants_DHC, "Look for DHC trigger number", false);
}

void PXDTriggerFixerModule::initialize(void)
{
  m_storeRaw.isRequired();

  if (!m_wants_Meta && !m_wants_HLT && !m_wants_DHC) B2FATAL("No source for wanted look up trigger number given (EvtMeta, HLT, DHC)");
  if (!m_refs_HLT && !m_refs_DHC && !m_refs_DHE) B2FATAL("No source for reference trigger number given (HLT, DHC, DHE)");
}

void PXDTriggerFixerModule::terminate(void)
{
  B2INFO("PXDTriggerFixerModule fixed: " << m_fixed << " not found: " << m_notfixed << " not needed: " << m_notneeded);
}

void PXDTriggerFixerModule::event(void)
{
  StoreObjPtr<EventMetaData> evtPtr;/// what will happen if it does not exist???

  unsigned int triggerNrEvt = evtPtr->getEvent();

  // first, we have to find the trigger numbers from the pxd data ...
  TClonesArray* rawdata = m_storeRaw.getPtr();
  unsigned int triggerNrDHE = 0x10000, triggerNrDHC = 0x10000, triggerNrHLT = 0x10000;
  for (auto& it : m_storeRaw) {
    if (getTrigNr(it, triggerNrDHE, triggerNrDHC, triggerNrHLT)) break; // only first (valid) one
  }

  B2INFO("Fixed trigger offset: Event Meta #$" << hex << triggerNrEvt << " HLT " << triggerNrHLT << " DHC "  << triggerNrDHC <<
         " DHE "  << triggerNrDHE << " Offset " << dec << m_offset);

  // Attention: triggerNrHLT does not necessary is identical to Event Meta, if
  // the event builder has been shifting events!

  unsigned int triggerReference = 0x10000;/// trigger number from the package
  if (m_refs_HLT) triggerReference = triggerNrHLT;
  if (m_refs_DHC) triggerReference = triggerNrDHC;
  if (m_refs_DHE) triggerReference = triggerNrDHE;

  // only store if a valid trigger number in PXD packet
  if (triggerReference != 0x10000) m_previous_events.insert(triggerReference & 0xFFFF, *rawdata);


  unsigned int triggerWanted = 0x10000;/// trigger number to look for
  if (m_wants_Meta) triggerWanted = triggerNrEvt;
  if (m_wants_HLT) triggerWanted = triggerNrHLT;
  if (m_wants_DHC) triggerWanted = triggerNrDHC;

  if (triggerWanted == 0x10000) { // invalid
    setReturnValue(false);
    m_notfixed++;
    return;
  }

  triggerWanted = (triggerWanted + m_offset) & 0xFFFF; /// including offset

  setReturnValue(true);
  if (triggerWanted != (triggerReference & 0xFFFF) || triggerReference == 0x10000) {
    // Now try to replace current raw data with raw data with current trigger number + offset
    // Main problem with this approach, if we do not find the correct data, the previous data
    // is still there ... thus checking the return value is mandatory!
    if (!m_previous_events.retrieve(triggerWanted, *rawdata)) {
      B2WARNING("Could not find data for offset: wanted $" << hex << triggerWanted << " and refs $"  << triggerReference);
      setReturnValue(false);
      m_notfixed++;
    } else {
      B2INFO("Fixed trigger offset for wanted #$" << hex << triggerWanted << " and refs "  << triggerReference);
      m_fixed++;
    }
  } else {
    // no need to fix
    m_notneeded++;
  }
}

bool PXDTriggerFixerModule::getTrigNr(RawPXD& px, unsigned int& innerDHE, unsigned int& innerDHC, unsigned int& outerHLT)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Trigger Fixer --> invalid packet size (32bit words) " << hex << px.size());
    return false;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());


  if (fullsize < 8) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    return false;
  }

  if (data[0] != 0xCAFEBABE && data[0] != 0xBEBAFECA) {
    B2ERROR("Magic invalid: Will not unpack anything. Header corrupted! " << hex << data[0]);
    return false;
  }

  Frames_in_event = ((ubig32_t*)data.data())[1];
  if (Frames_in_event < 1 || Frames_in_event > 250) {
    B2ERROR("Number of Frames invalid: Will not unpack anything. Header corrupted! Frames in event: " << Frames_in_event);
    return false;
  }

  unsigned int* tableptr;
  tableptr = &data[2]; // skip header!!!

  unsigned int* dataptr;
  dataptr = &tableptr[Frames_in_event];
  datafullsize = fullsize - 2 * 4 - Frames_in_event * 4; // minus header, minus table

  int ll = 0; // Offset in dataptr in bytes
  for (int j = 0; j < Frames_in_event; j++) {
    int lo;/// len of frame in bytes
    lo = ((ubig32_t*)tableptr)[j];
    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << "size " << lo << " at byte offset in dataptr " << ll);
      return false;
    }
    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset in dataptr " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      return false;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      ll += (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      if (unpack_dhc_frame(ll + (char*)dataptr, innerDHE, innerDHC, outerHLT)) return true;
      ll += lo; /// no rounding needed
    }
  }
  return false;
}

bool PXDTriggerFixerModule::unpack_dhc_frame(void* data, unsigned int& innerDHE, unsigned int& innerDHC, unsigned int& outerHLT)
{
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_RAW     0x0
#define DHC_FRAME_HEADER_DATA_TYPE_DHP_ZSD     0x5
#define DHC_FRAME_HEADER_DATA_TYPE_FCE_RAW     0x1 //CLUSTER FRAME
#define DHC_FRAME_HEADER_DATA_TYPE_COMMODE     0x6
#define DHC_FRAME_HEADER_DATA_TYPE_GHOST       0x2
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_START   0x3
#define DHC_FRAME_HEADER_DATA_TYPE_DHE_END     0x4
// DHC envelope, new
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC
// Onsen processed data, new
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_DHP     0xD
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_FCE     0x9
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_ROI     0xF
#define DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG     0xE

  switch (((*(ubig16_t*)data) & 0x7800) >> 11) {
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_START: {
      innerDHC = ((ubig16_t*)data)[1];
      break;
    }
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_START: {
      innerDHE = ((ubig16_t*)data)[1];
      return true;
    }
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG: {
      outerHLT = ((ubig16_t*)data)[1];
      // well how about HLT trigger values as reference?
      break;
    }
    default:
      break;

  }
  return false;
}
