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

/* Belle2 headers. */
#include <klm/dataobjects/KLMElementArrayIndex.h>

namespace Belle2 {

  /**
   * KLM plane array index.
   */
  class KLMPlaneArrayIndex : public KLMElementArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMPlaneArrayIndex& Instance();

  private:

    /**
     * Constructor.
     */
    KLMPlaneArrayIndex();

    /**
     * Destructor.
     */
    ~KLMPlaneArrayIndex();

  };

}
