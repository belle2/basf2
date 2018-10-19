/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <functional>
#include <vector>
#include <framework/logging/Logger.h>
namespace Belle2 {

  class SVDHitTimeSelectionFunction : public TObject {

  public:


    typedef bool (SVDHitTimeSelectionFunction::*selFunction)(double, double, double, double) const;

    /** returns the  value of raw_time, depending on the trigger bin*/
    bool isOnTime(double svdTime, double svdTimeError = 0, double t0 = 0 , double t0Error = 0)
    {
      selFunction f = m_implementations[m_current];
      return (this->*f)(svdTime, svdTimeError, t0, t0Error) ;
    }

    /** constructor */
    SVDHitTimeSelectionFunction(double tMin)
    {
      m_tMin = tMin;

      // The m_implementations vector is static.
      // We have to initialize it just once.
      if (m_implementations.size() == 0) {
        m_implementations.push_back(&SVDHitTimeSelectionFunction::firstVersion);
        //m_implementations.push_back(
        //  &SVDHitTimeSelectionFunction::betterVersion);
      }

      m_current = m_implementations.size() - 1;

    };


    void set_tMin(double tMin)
    {
      m_tMin = tMin;
    }
    /** copy constructor */
    SVDHitTimeSelectionFunction(const Belle2::SVDHitTimeSelectionFunction& a);


  private:

    /** function parameters & implementations*/

    /** FIRST VERSION: correctedValue = t * scale[tb] + bias[tb] */
    double m_tMin; /**< trigger-bin dependent bias*/
    /** first version implementation*/
    bool firstVersion(double svdTime, double /* svdTimeError */, double /* t0 */, double /* t0Error */) const
    {
      return svdTime > m_tMin;
    };

    /** current function ID */
    int m_current;
    /** vector of fuctions, we use the m_current*/
    static std::vector < selFunction > m_implementations; //! Do not stream this, please throw it in the WC


    ClassDef(SVDHitTimeSelectionFunction, 1)
  };

}
