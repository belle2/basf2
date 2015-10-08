#include <framework/pcore/Mergeable.h>

#include <TCollection.h>

using namespace Belle2;

Long64_t Mergeable::Merge(TCollection* hlist)
{
  Long64_t nMerged = 0;
  if (hlist) {
    const Mergeable* xh = 0;
    TIter nxh(hlist);
    while ((xh = dynamic_cast<Mergeable*>(nxh()))) {
      // Add xh to me
      merge(xh);
      ++nMerged;
    }
  }
  return nMerged;
}
