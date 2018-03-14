//+
// File : PXDTriggerShifterModule.cc
// Description : Desy 2016 evenet mismatch fixer
//
// Author : Bjoern Spruck, with help by Martin Ritter; Tadeas Bilka
// Date : 20 - Apr - 2016
//-

#include <testbeam/vxd/modules/unpacking/PXDTriggerShifter.h>
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
REG_MODULE(PXDTriggerShifter)

PXDTriggerShifterModule::PXDTriggerShifterModule(void)
{
  // Module is NOT allowed to be run in parallel!!!
  // Reason: we buffer previous events in a lock up table ... the order is important!

  // Module Description
  setDescription("Find PXD data for event which has been shifted by offset");

  //Parameter definition
  addParam("offset", m_offset, "Trigger offset of PXD w.r.t. SVD trigger number", int(-1));

}

void PXDTriggerShifterModule::initialize(void)
{
  m_storeRaw.isRequired();
}

void PXDTriggerShifterModule::terminate(void)
{
  B2INFO("PXDTriggerShifter fixed: " << m_fixed << " not found: " << m_notfixed << " not needed: " << m_notneeded);
}

void PXDTriggerShifterModule::event(void)
{
  StoreObjPtr<EventMetaData> evtPtr;/// what will happen if it does not exist???

  unsigned int triggerNrEvtMeta = evtPtr->getEvent();

  // first, we have to find the trigger numbers from the pxd data ...
  TClonesArray* rawdata = m_storeRaw.getPtr();
  unsigned int triggerNrPxdDHC = 0x10000;// triggerNrPxdDHC is the DHC one
  unsigned int triggerNrPxdHLT = 0x10000;// triggerNrPxdHLT the HLT/Onsen one

  for (auto& it : m_storeRaw) {
    if (getTrigNr(it, triggerNrPxdDHC, triggerNrPxdHLT)) break; // only first (valid) one
  }

  unsigned int triggerNr = triggerNrEvtMeta & 0xFFFF; // We match with TrigNr from EvtMeta

  // Keep current raw data in MRU cache with its original pxd trigger number (the one from DHC)
  if (triggerNrPxdDHC != 0x10000) m_previous_events.insert(triggerNrPxdDHC & 0xFFFF, *rawdata);

  if (triggerNr == 0x10000) { // invalid
    m_storeRaw.clear();// To be sure remove not fitting data
    setReturnValue(false);
    m_notfixed++;
    return;
  }
  triggerNr = (triggerNr + m_offset) & 0xFFFF; /// including offset

  setReturnValue(true);
  if (triggerNr != (triggerNrPxdDHC & 0xFFFF) || triggerNrPxdDHC == 0x10000) {
    // Now try to replace current raw data with raw data with current trigger number + offset
    // Main problem with this approach, if we do not find the correct data, the previous data
    // is still there ... thus checking the return value is mandatory!
    m_storeRaw.clear();// To be sure, remove not fitting data
    if (!m_previous_events.retrieve(triggerNr, *rawdata)) {
      B2WARNING("Could not find data for offset for HLT $" << hex << triggerNr << " and DHH $" << hex << triggerNrPxdDHC);
      setReturnValue(false);
      m_notfixed++;
    } else {
      B2INFO("Fixed trigger offset for #$" << hex << triggerNr << " and " << hex << triggerNrPxdDHC);
      m_fixed++;
    }
  } else {
    // no need to fix
    m_notneeded++;
  }
}

bool PXDTriggerShifterModule::getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Trigger Shifter --> invalid packet size (32bit words) " << hex << px.size());
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
      if (unpack_dhc_frame(ll + (char*)dataptr, innerDHH, outerHLT)) return true;
      ll += lo; /// no rounding needed
    }
  }
  return false;
}

bool PXDTriggerShifterModule::unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT)
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
      innerDHH = ((ubig16_t*)data)[1];
      return true;
    }
    case DHC_FRAME_HEADER_DATA_TYPE_DHE_START: {
      // workaround for BonnDAQ ...
      innerDHH = ((ubig16_t*)data)[1];
      return true;
    }
    case DHC_FRAME_HEADER_DATA_TYPE_ONSEN_TRG: {
      outerHLT = ((ubig16_t*)data)[1];
      //outerHLT = ((ubig32_t*)data)[2];
      break;
    }
    default:
      break;

  }
  return false;
}
