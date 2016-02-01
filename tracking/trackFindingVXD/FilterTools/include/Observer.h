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

  class Observer {
  public:

    /// exemplary draft for a notify-function of an observer
    template< class T, class someRangeType, class someHitType>
    static void notify(T , double,  someRangeType, const someHitType&, const someHitType&) {};

    Observer() {};
  };
}
