//+
// File : PXDRawDHHDumperModule.cc
// Description : Dump Raw Desy 2016 DHH event data
//
// Author : Bjoern Spruck
// Date : Updated on 20 - Jun - 2016
//-

#include <testbeam/vxd/modules/helper/PXDRawDHHDumper.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawDHHDumper)

void PXDRawDHHDumperModule::initialize(void)
{
  m_storeRaw.isRequired();
}

void PXDRawDHHDumperModule::event(void)
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
      string name = boost::str(boost::format("%08X_%08X_%d.dhh.dump") % tag % pxdTriggerNr % i);
      B2ERROR("PXDRawDHHDumper: Dump to " << name);
      fh = fopen(name.c_str(), "wt+");
      if (fh) {
        if (it.size() > 5) fwrite(&it.data()[5], 4, it.size() - 5, fh);
        fclose(fh);
      }
      i++;
    }
  }
}

void PXDRawDHHDumperModule::endian_swapper(void* a, unsigned int len)
{
  // Quick and Dirty swapper for BonnDAQ
  ubig16_t* p;
  ulittle16_t* q;
  p = (ubig16_t*)a;
  q = (ulittle16_t*)a;
  len /= 2;
  for (unsigned int i = 0; i < len; i++, q++, p++) { *q = *p;}
}

bool PXDRawDHHDumperModule::getTrigNr(RawDHH& px, unsigned int& innerDHH, unsigned int& tag)
{
  // Disable compiler warning for this variable
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
  int Frames_in_event;
  int fullsize;
  int datafullsize;

  if (px.size() <= 0 || px.size() > 16 * 1024 * 1024) {
    B2ERROR("PXD DHH Unpacker --> invalid packet size (32bit words) " << hex << px.size());
    return false;
  }
  std::vector<unsigned int> data(px.size());
  fullsize = px.size() * 4; /// in bytes ... rounded up to next 32bit boundary
  std::copy_n(px.data(), px.size(), data.begin());

  if (fullsize < 16) {
    B2ERROR("Data is to small to hold a valid Header! Will not unpack anything. Size:" << fullsize);
    return false;
  }

  unsigned int* dataptr;
  dataptr = &data[5];
  datafullsize = fullsize - 5 * 4; // minus header

  int ll = 0; // Offset in dataptr in bytes
  int j = 0;
  while (datafullsize - ll >= 16) {
    if (dataptr[0] != 0xCAFEBABE) {
      B2ERROR("Magic invalid in frame " << j << " (" << datafullsize << "," << ll << "): Will not unpack anything. Header corrupted! $" <<
              hex << dataptr[0] << " $" << hex << dataptr[1] << " $" << hex << dataptr[2] << " $" << hex << dataptr[3]);
      break;
    }

    int lo;// len of frame in bytes
    lo = dataptr[1];
    dataptr += 4; // Cafebabe header
    ll += 16;

    if (lo <= 0) {
      B2ERROR("size of frame invalid: " << j << " size $" << std::hex << lo << " at byte offset " << std::hex << ll);
      return false;
    }

    if (ll + lo > datafullsize) {
      B2ERROR("frames exceed packet size: " << j  << " size " << lo << " at byte offset " << ll << " of datafullsize " <<
              datafullsize << " of fullsize " << fullsize);
      return false;
    }
    if (lo & 0x3) {
      B2ERROR("SKIP Frame with Data with not MOD 4 length " << " ( $" << hex << lo << " ) ");
      lo = (lo + 3) & 0xFFFFFFFC; /// round up to next 32 bit boundary
    } else {
      B2INFO("unpack DHE(C) frame: " << j << " with size $" << std::hex << lo << " at byte offset " << std::hex << ll);
      endian_swapper((char*)dataptr, lo);
//      unpack_dhc_frame((char*)dataptr, lo, j, 256);
      if (unpack_dhc_frame((char*)dataptr, innerDHH, tag)) return true;

    }
    ll += lo;
    dataptr += lo / 4;
    j++;// framecounter
  }
  return false;
}

bool PXDRawDHHDumperModule::unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& tag)
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