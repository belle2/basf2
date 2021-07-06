/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPASICPedestals.h>
#include <framework/logging/Logger.h>

#include <set>
#include <iostream>

using namespace std;

namespace Belle2 {

  int TOPASICPedestals::setPedestals(const TProfile* profile, double average)
  {
    int bad = c_WindowSize;

    if (!profile) return bad;

    if (profile->GetNbinsX() != c_WindowSize) {
      B2ERROR("TOPASICPedestals::setPedestals:  TProfile with wrong number of bins");
      return bad;
    }

    if (profile->GetEntries() == 0) {
      B2WARNING("TOPASICPedestals::setPedestals:  TProfile with no entries");
      return bad;
    }

    std::vector<unsigned> values;
    std::vector<unsigned> errors;
    for (int i = 0; i < c_WindowSize; i++) {
      values.push_back(int(profile->GetBinContent(i + 1) + average + 0.5));
      errors.push_back(int(profile->GetBinError(i + 1) + 0.5));
    }

    unsigned maxDif = 1 << c_Bits; maxDif--;
    unsigned maxErr = 1 << (sizeof(unsigned short) * 8 - c_Bits); maxErr--;
    m_offset = getOptimizedOffset(values, errors, maxDif, maxErr);

    for (int i = 0; i < c_WindowSize; i++) {
      unsigned dif = values[i] - m_offset;
      if (dif > maxDif) continue;
      unsigned err = errors[i];
      if (err > maxErr) continue;
      if (err == 0) err = 1;
      m_pedestals[i] = dif + (err << c_Bits);
      bad--;
    }

    return bad;

  }


  unsigned TOPASICPedestals::getOptimizedOffset(const std::vector<unsigned>& values,
                                                const std::vector<unsigned>& errors,
                                                unsigned maxDif,
                                                unsigned maxErr)
  {

    std::set<unsigned> sortedValues;
    for (unsigned i = 0; i < values.size(); i++) {
      if (errors[i] > maxErr) continue;
      sortedValues.insert(values[i]);
    }

    unsigned nprev = sortedValues.size();
    unsigned nl = 0;
    for (auto it = sortedValues.begin(); it != sortedValues.end(); ++it) {
      unsigned nr = 0;
      for (auto back = sortedValues.crbegin(); *back - *it > maxDif; ++back) {
        nr++;
      }
      unsigned nbad = nl + nr;
      if (nbad == 0) return *it;
      if (nbad >= nprev) return *(--it);
      nprev = nbad;
      nl++;
    }

    return *sortedValues.begin();

  }

} // end Belle2 namespace
