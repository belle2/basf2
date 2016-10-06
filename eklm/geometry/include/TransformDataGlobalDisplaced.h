/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_TRANSFORMDATAGLOBALDISPLACED_H
#define EKLM_TRANSFORMDATAGLOBALDISPLACED_H

/* Belle2 headers. */
#include <eklm/geometry/TransformData.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Transformation data (global, displaced): singleton version.
     */
    class TransformDataGlobalDisplaced : public TransformData {

    public:

      /**
       * Instantiation.
       */
      static const TransformDataGlobalDisplaced& Instance();

    private:

      /**
       * Constructor.
       */
      TransformDataGlobalDisplaced();

      /**
       * Destructor.
       */
      ~TransformDataGlobalDisplaced();

    };

  }

}

#endif

