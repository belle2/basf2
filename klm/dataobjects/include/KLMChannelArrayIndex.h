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

/* C++ headers. */
#include <map>

namespace Belle2 {

  /**
   * KLM channel array index.
   */
  class KLMChannelArrayIndex : public KLMElementArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMChannelArrayIndex& Instance();

  private:

    /**
     * Constructor.
     */
    KLMChannelArrayIndex();

    /**
     * Destructor.
     */
    ~KLMChannelArrayIndex();

  };

}
