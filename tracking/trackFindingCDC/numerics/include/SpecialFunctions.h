/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
