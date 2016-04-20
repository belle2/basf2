//+
// File : PXDTriggerFixerModule.cc
// Description : Desy 2016 evenet mismatch fixer
//
// Author : Bjoern Spruck, with help by Martin Ritter
// Date : 20 - Apr - 2016
//-

#include <pxd/modules/pxdUnpacking/PXDTriggerFixer.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDTriggerFixer)

void PXDTriggerFixerModule::initialize(void)
{
  m_storeRaw.required();
}

void PXDTriggerFixerModule::event(void)
{
  // erst mal muss man die beiden trigger nummern finden ...
  TClonesArray* rawdata = m_storeRaw.getPtr();
  unsigned int pxdTriggerNr = 0x10000, triggerNr = 0x10000;
  for (auto& it : m_storeRaw) {
    if (getTrigNr(it, pxdTriggerNr, triggerNr)) break; // only first (valid) one
  }
  pxdTriggerNr &= 0xFFFF;
  triggerNr &= 0xFFFF;

  m_previous_events.insert(pxdTriggerNr, *rawdata);

  setReturnValue(true);
  if (triggerNr != pxdTriggerNr) {
    if (!m_previous_events.retrieve(triggerNr, *rawdata)) {
      B2WARNING("Could not trigger offset for HLT $" << hex << triggerNr << " and DHH $" << hex << pxdTriggerNr);
      setReturnValue(false);
    } else {
      B2INFO("Fixed trigger offset for #$" << hex << triggerNr << " and " << hex << pxdTriggerNr);
    }
  }
  // jetzt sollte es passen.
}


bool PXDTriggerFixerModule::getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT)
{
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD Unpacker --> invalid packet size (32bit words) " << hex << px.size());
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
  if (Frames_in_event < 0 || Frames_in_event > 256) {
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
      B2INFO("unpack DHE(C) frame: " << j << " with size " << lo << " at byte offset in dataptr " << ll);
      if (unpack_dhc_frame(ll + (char*)dataptr, innerDHH, outerHLT)) return true;
      ll += lo; /// no rounding needed
    }
  }
  return false;
}

bool PXDTriggerFixerModule::unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT)
{
  // DHE like before, but now 4 bits
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