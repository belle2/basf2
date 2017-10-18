/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLWAVEFORMS_H
#define ECLWAVEFORMS_H

#include <framework/dataobjects/DigitBase.h>
#include <algorithm>

namespace Belle2 {
  /*!
    Class to store ECL waveforms for entire calorimeter
  */
  //  class ECLWaveforms : public DigitBase {
  class ECLWaveforms : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLWaveforms() {}

    /** get data */
    std::vector<unsigned int>& getStore() { return _store;}

  private:
    std::vector<unsigned int> _store;
    ClassDef(ECLWaveforms, 1);/**< ClassDef */
  };
}
#endif
