/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_ELEMENTNUMBERSSINGLETON_H
#define EKLM_ELEMENTNUMBERSSINGLETON_H

/* Belle2 headers. */
#include <eklm/dataobjects/ElementNumbers.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * EKLM element numbers (singleton version).
     */
    class ElementNumbersSingleton : public ElementNumbers {

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

#endif

