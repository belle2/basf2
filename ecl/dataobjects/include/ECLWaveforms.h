/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
