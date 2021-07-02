/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>
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
    void setFEEData(KLMChannelNumber strip, KLMScintillatorFEEData* data);

    /**
     * Get scintillator FEE data.
     * @param[in] strip Strip (channel) number.
     */
    const KLMScintillatorFEEData* getFEEData(KLMChannelNumber strip) const;

  private:

    /** Scintillator FEE parameters. */
    std::map<KLMChannelNumber, KLMScintillatorFEEData> m_FEEParameters;

    /** Class version. */
    ClassDef(Belle2::KLMScintillatorFEEParameters, 1);

  };

}
