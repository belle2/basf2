/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementArrayIndex.h>

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
