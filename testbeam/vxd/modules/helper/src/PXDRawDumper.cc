//+
// File : PXDRawDumperModule.cc
// Description : Dump Raw Desy 2016 ONSEN event data
//
// Author : Bjoern Spruck
// Date : Updated on 20 - Jun - 2016
//-

#include <testbeam/vxd/modules/helper/PXDRawDumper.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawDumper)

void PXDRawDumperModule::initialize(void)
{
  m_storeRaw.isRequired();
}

void PXDRawDumperModule::event(void)
{
  // erst mal muss man die beiden trigger nummern finden ...

  unsigned int pxdTriggerNr = 0x0, tag = 0x0;
  for (auto& it : m_storeRaw) {
    if (getTrigNr(it, pxdTriggerNr, tag)) break; // only first (valid) one
  }

  if (pxdTriggerNr != 0x0 && tag != 0x0) {
  } else { // else if put same upcounter in
    static int nr1 = 1000, nr2 = 1000;
    pxdTriggerNr = nr1++;
    tag = nr2++;
  }
  {
    int i = 0;
    for (auto& it : m_storeRaw) {
      FILE* fh;
      string name = boost::str(boost::format("%08X_%08X_%d.pxd.dump") % tag % pxdTriggerNr % i);
      B2ERROR("PXDRawDumper: Dump to " << name);
      fh = fopen(name.c_str(), "wt+");
      if (fh) {
        fwrite(it.data(), 4, it.size(), fh);
        fclose(fh);
      }
      i++;
    }
  }
}


bool PXDRawDumperModule::getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& tag)
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
      if (unpack_dhc_frame(ll + (char*)dataptr, innerDHH, tag)) return true;
      ll += lo; /// no rounding needed
    }
  }
  return false;
}

bool PXDRawDumperModule::unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& tag)
{
// DHC envelope, new
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_START  0xB
#define DHC_FRAME_HEADER_DATA_TYPE_DHC_END    0xC

  switch (((*(ubig16_t*)data) & 0x7800) >> 11) {
    case DHC_FRAME_HEADER_DATA_TYPE_DHC_START: {
      unsigned int hi = ((ubig16_t*)data)[2];
      innerDHH = (hi << 16) | ((ubig16_t*)data)[1];
      tag = ((ubig32_t*)data)[3];
      return true;
    }
    default:
      break;

  }
  return false;
}