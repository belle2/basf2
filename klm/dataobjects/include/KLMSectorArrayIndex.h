/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementArrayIndex.h>

namespace Belle2 {

  /**
   * KLM sector array index.
   */
  class KLMSectorArrayIndex : public KLMElementArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMSectorArrayIndex& Instance();

  private:

    /**
     * Constructor.
     */
    KLMSectorArrayIndex();

    /**
     * Destructor.
     */
    ~KLMSectorArrayIndex();

  };

}
