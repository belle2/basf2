/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
// #include <vxd/dataobjects/VxdID.h>

#include <string>
#include <iostream>
#include <iomanip>      // std::setprecision

namespace Belle2 {

  /** this observer does simply print the name of the SelectionVariable and the result of its value-function as a Warning(if failed) or as an Info (if succeeded) */
  class Observer3HitPrintResults {
  public:
    /** notifier producing a info message if SelectionVariable was accepted and a Warning if otherwise */
    template<class Var, class RangeType>
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       const RangeType& range,
                       const typename Var::argumentType& outerHit,
                       const typename Var::argumentType& centerHit,
                       const typename Var::argumentType& innerHit)
    {

      std::stringstream outputStream;
      outputStream << filterType.name()
                   << " with outer-/center-/innerhit: "
                   << outerHit.getPosition().PrintStringXYZ()
                   << "/"
                   << centerHit.getPosition().PrintStringXYZ()
                   << "/"
                   << innerHit.getPosition().PrintStringXYZ()
                   << " having indices "
                   << outerHit.getArrayIndex()
                   << "/"
                   << centerHit.getArrayIndex()
                   << "/"
                   << innerHit.getArrayIndex()
                   << " and VxdIDs "
                   << outerHit.getVxdID()
                   << "/"
                   << centerHit.getVxdID()
                   << "/"
                   << innerHit.getVxdID()
                   << " results in "
                   << (range.contains(fResult) ? std::setprecision(6) : std::setprecision(18))
                   << fResult
                   << " & accepted: "
                   << (range.contains(fResult) ? std::string(">true<") : std::string(">false<"))
                   << " in range "
                   << range.getInf()
                   << "/"
                   << range.getSup();

      B2DEBUG(5, outputStream.str());
      /// for debugging:
      //       if (range.contains(fResult)) {
      //         B2INFO(outputStream.str())
      //       } else {
      //         B2WARNING(outputStream.str())
      //       }
    }


    /// empty constructor:
    Observer3HitPrintResults() {};


    /** _static_ method used by the observed object to initialize the observer.
     */
    template <  typename ... types >
    static void initialize(const types& ...) {};


    /** _static_ method used by the observed object to terminate the observer.
     */
    template <  typename ... types >
    static void terminate(const types& ...) {};
  };
}
