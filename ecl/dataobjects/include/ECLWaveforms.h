/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {
  /*!
    Class to store ECL waveforms for entire calorimeter
  */
  class ECLWaveforms : public TObject {
  public:
    /** default constructor for ROOT */
    ECLWaveforms() {}

    /** get data */
    std::vector<unsigned int>& getStore() { return m_store;}

  private:
    std::vector<unsigned int> m_store; /**< the data */
    ClassDef(ECLWaveforms, 1); /**< ClassDef */
  };
}
