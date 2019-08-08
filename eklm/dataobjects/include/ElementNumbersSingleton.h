/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMElementNumbers.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * EKLM element numbers (singleton version).
     * This class is not a dataobject but is used by other EKLM dataobjects.
     */
    class ElementNumbersSingleton : public EKLMElementNumbers {

    public:

      /**
       * Instantiation.
       */
      static const ElementNumbersSingleton& Instance();

    private:

      /**
       * Constructor.
       */
      ElementNumbersSingleton();

      /**
       * Destructor.
       */
      ~ElementNumbersSingleton();

    };

  }

}
