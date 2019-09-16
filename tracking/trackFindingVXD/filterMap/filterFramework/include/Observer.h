/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


namespace Belle2 {

  /** Observer base class which can be used to evaluate the VXDTF2's Filters */
  class Observer {
  public:
    /// exemplary draft for a notify-function of an observer
    template<class T, class someRangeType, class someHitType>
    static void notify(T , double,  someRangeType, const someHitType&, const someHitType&) {};

    /** An empty constructor for an empty class */
    explicit Observer() {};

    /** _static_ method used by the observed object to initialize the observer. */
    template <typename ... types>
    static void initialize(const types& ...) {};

    /** _static_ method used by the observed object to terminate the observer. */
    template <typename ... types>
    static void terminate(const types& ...) {};
  };
}
