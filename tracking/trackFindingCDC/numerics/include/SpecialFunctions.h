/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Standard sinus cardinalis function sinc x = sin x / x
    double sinc(double x);

    /**
     *  An implementation of the function asin x / x which safely evaluates near x=0
     *
     *  Naming asinc (arc sine cardinalis) in analogy to the sine cardinalis function.
     */
    double asinc(double x);
  }
}
