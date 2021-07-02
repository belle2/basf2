/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
