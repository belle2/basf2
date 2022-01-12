#include <ecl/dataobjects/ECLDigit.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

ECLDigit* ECLDigit::getByCellID(int cid)
{
  static StoreArray<ECLDigit> ecl_digits_arr;
  for (auto& ecl_dig : ecl_digits_arr) {
    if (ecl_dig.getCellId() == cid) return &ecl_dig;
  }
  return nullptr;
}

