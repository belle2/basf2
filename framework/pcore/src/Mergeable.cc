/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/Mergeable.h>

#include <TCollection.h>

using namespace Belle2;

Long64_t Mergeable::Merge(TCollection* hlist)
{
  Long64_t nMerged = 0;
  if (hlist) {
    const Mergeable* xh = nullptr;
    TIter nxh(hlist);
    while ((xh = dynamic_cast<Mergeable*>(nxh()))) {
      // Add xh to me
      merge(xh);
      ++nMerged;
    }
  }
  return nMerged;
}
