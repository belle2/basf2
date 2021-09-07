#include <ecl/dataobjects/ECLDsp.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

ECLDsp* ECLDsp::getByCellID(int cid)
{
  static StoreArray<ECLDsp> ecl_dsp_arr;
  for (auto& ecl_dsp : ecl_dsp_arr) {
    if (ecl_dsp.getCellId() == cid) return &ecl_dsp;
  }
  return nullptr;
}

