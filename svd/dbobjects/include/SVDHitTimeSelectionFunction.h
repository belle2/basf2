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
    bool isInTime(double svdTime, double svdTimeError = 0, double t0 = 0 , double t0Error = 0)
    {
      selFunction f = m_implementations[m_current];
      return (this->*f)(svdTime, svdTimeError, t0, t0Error) ;
    }


    /** constructor */
    SVDHitTimeSelectionFunction()
    {
      // The m_implementations vector is static.
      // We have to initialize it just once.
      if (m_implementations.size() == 0) {
        m_implementations.push_back(&SVDHitTimeSelectionFunction::firstVersion);
        m_implementations.push_back(&SVDHitTimeSelectionFunction::secondVersion);
        m_implementations.push_back(&SVDHitTimeSelectionFunction::thirdVersion);
      }

      m_current = 0; //firstVersion is the default //m_implementations.size() - 1;
      m_deltaT = 100; //ns
      m_nSigma = 100;
      m_tMin = -999; //ns
    };

    /** copy constructor */
    //    SVDHitTimeSelectionFunction(const Belle2::SVDHitTimeSelectionFunction& a);

    //implementation: function version
    /** set the function ID (version)*/
    void setFunctionID(int user_current) {m_current = user_current;}
    /** get the function ID (function version)*/
    int getFunctionID() {return m_current;}

    //implementation firstVersion, setters and getters
    /** set the minimum cluster time */
    void setMinTime(double tMin) { m_tMin = tMin; }
    /** returns the  minimum cluster time */
    float getMinTime() { return m_tMin; };

    //implementation secondVersion, setters and getters
    /** set the minimum time distance wrt t0 */
    void setDeltaTime(double deltaT) { m_deltaT = deltaT; }
    /** returns the minimum time distnace wrt t0 */
    float getDeltaTime() { return m_deltaT; };

    //implementation thirdVersion, setters and getters
    /** set the nSigma */
    void setNsigma(double nSigma) { m_nSigma = nSigma; }
    /** returns the  minimum cluster time */
    float getNsigma() { return m_nSigma; };



  private:

    /** function parameters & implementations*/

    /** FIRST VERSION, ID = 0: isOnTime if t > m_tMin */
    double m_tMin; /**< minimum cluster time*/
    bool firstVersion(double svdTime, double /* svdTimeError */, double /* t0 */, double /* t0Error */) const
    {
      return svdTime > m_tMin;
    };

    /** SECOND VERSION, ID = 1: isOnTime if |t - t0|< deltaT */
    double m_deltaT; /**< minimum time distance wrt t0 */
    bool secondVersion(double svdTime, double /* svdTimeError */, double t0, double /* t0Error */) const
    {
      return fabs(svdTime - t0) < m_deltaT;
    };


    /** THIRD VERSION, ID = 2: isOnTime if |t - t0|< nSigma*sigma */
    double m_nSigma; /**< number of Sigma */
    bool thirdVersion(double svdTime, double svdTimeError, double t0, double t0Error) const
    {
      float err2 = svdTimeError * svdTimeError + t0Error * t0Error;
      return (svdTime - t0) * (svdTime - t0) < m_nSigma * m_nSigma * err2;
    };

    /** current function ID */
    int m_current;
    /** vector of fuctions, we use the m_current*/
    static std::vector < selFunction > m_implementations; //! Do not stream this, please throw it in the WC


    ClassDef(SVDHitTimeSelectionFunction, 2)
  };

}
