/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store KLM reconstruction parameters in the database.
   */
  class KLMReconstructionParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMReconstructionParameters()
    {
    }

    /**
     * Destructor.
     */
    ~KLMReconstructionParameters()
    {
    }

    /**
     * Whether multi-strip digits are enabled.
     */
    bool multiStripDigitsEnabled() const
    {
      return m_MultiStripDigitsEnabled;
    }

    /**
     * Set whether multi-strip digits are enabled.
     */
    void setMultiStripDigitsEnabled(bool enabled)
    {
      m_MultiStripDigitsEnabled = enabled;
    }

  private:

    /** Whether multi-strip digits are enabled. */
    bool m_MultiStripDigitsEnabled = false;

    /** Class version. */
    ClassDef(Belle2::KLMReconstructionParameters, 2);

  };

}
