/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/KLMScintillatorFEEData.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <cstdint>
#include <map>

namespace Belle2 {

  /**
   * Class to store EKLM alignment data in the database.
   */
  class KLMScintillatorFEEParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMScintillatorFEEParameters();

    /**
     * Destructor.
     */
    ~KLMScintillatorFEEParameters();

    /**
     * Set scintillator FEE data.
     * @param[in] strip Strip (channel) number.
     * @param[in] data  Scintillator FEE data.
     */
    void setFEEData(uint16_t strip, KLMScintillatorFEEData* data);

    /**
     * Get scintillator FEE data.
     * @param[in] strip Strip (channel) number.
     */
    const KLMScintillatorFEEData* getFEEData(uint16_t strip) const;

  private:

    /** Scintillator FEE parameters. */
    std::map<uint16_t, KLMScintillatorFEEData> m_FEEParameters;

    /** Class version. */
    ClassDef(Belle2::KLMScintillatorFEEParameters, 1);

  };

}
