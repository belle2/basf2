/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
