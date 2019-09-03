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

/* C++ headers. */
#include <map>

/* Belle2 headers. */
#include <klm/dataobjects/KLMElementArrayIndex.h>

namespace Belle2 {

  /**
   * KLM module array index.
   */
  class KLMModuleArrayIndex : public KLMElementArrayIndex {

  public:

    /**
     * Instantiation.
     */
    static const KLMModuleArrayIndex& Instance();

  private:

    /**
     * Constructor.
     */
    KLMModuleArrayIndex();

    /**
     * Destructor.
     */
    ~KLMModuleArrayIndex();

  };

}
