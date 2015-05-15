/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef BEAST_FANGS_GEOMETRY_FANGSCREATOR_H
#define BEAST_FANGS_GEOMETRY_FANGSCREATOR_H

#include <beast/claws/geometry/CLAWSCreator.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the FANGS detector */
  namespace fangs {

    /** The creator for the FANGS geometry. */
    class FANGSCreator : public claws::CLAWSCreator {
    public:
      FANGSCreator();
    };

  }
}

#endif /* BEAST_FANGS_GEOMETRY_FANGSCREATOR_H */
