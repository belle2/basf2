/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_TRANSFORMDATAGLOBALALIGNED_H
#define EKLM_TRANSFORMDATAGLOBALALIGNED_H

/* Belle2 headers. */
#include <eklm/geometry/TransformData.h>

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

#endif

