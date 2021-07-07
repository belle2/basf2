/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/eklm/geometry/TransformData.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Transformation data (global, aligned): singleton version.
     */
    class TransformDataGlobalAligned : public TransformData {

    public:

      /**
       * Instantiation.
       */
      static const TransformDataGlobalAligned& Instance();

    private:

      /**
       * Constructor.
       */
      TransformDataGlobalAligned();

      /**
       * Destructor.
       */
      ~TransformDataGlobalAligned();

    };

  }

}
