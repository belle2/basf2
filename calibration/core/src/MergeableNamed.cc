#include <calibration/core/MergeableNamed.h>

#include <TCollection.h>

using namespace Belle2;

Long64_t MergeableNamed::Merge(TCollection* hlist)
{
  Long64_t nMerged = 0;
  if (hlist) {
    const MergeableNamed* xh = 0;
    TIter nxh(hlist);
    while ((xh = dynamic_cast<MergeableNamed*>(nxh()))) {
      // Add xh to me
      merge(xh);
      ++nMerged;
    }
  }
  return nMerged;
}
