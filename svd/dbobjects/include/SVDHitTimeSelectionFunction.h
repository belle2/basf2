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

    /** returns whether the hit came on time or not */
    bool isOnTime(double svdTime, double svdTimeError = 0, double t0 = 0 , double t0Error = 0)
    {
      selFunction f = m_implementations[m_current];
      return (this->*f)(svdTime, svdTimeError, t0, t0Error) ;
    }


    /** constructor */
    SVDHitTimeSelectionFunction(double tMin = -999.)
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

    /** copy constructor */
    SVDHitTimeSelectionFunction(const Belle2::SVDHitTimeSelectionFunction& a);

    //implementation firstVersion, setters and getters
    /** set the minimum cluster time */
    void setMinTime(double tMin)
    {
      m_tMin = tMin;
    }
    /** returns the  minimum cluster time */
    float getMinTime()
    {
      return m_tMin;
    };



  private:

    /** function parameters & implementations*/

    /** FIRST VERSION: isOnTime if t > m_tMin */
    double m_tMin; /**< minimum cluster time*/
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
