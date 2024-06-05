/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <TObject.h>
#include <TF1.h>
#include <TString.h>
#include <TROOT.h>
#include <TGraph.h>

#include <cmath>
#include <vector>

namespace Belle2 {

  /** class to contain the MC fudge factor formulae*/
  class SVDMCFudgeFactorFunction : public TObject {

  public:

    /** typedef of the return value of the fudge factor function*/
    typedef double (SVDMCFudgeFactorFunction::*fudgeFactorFunction)(double) const;

    /** returns the fudge factor, depending on the track's angle*/
    double getFudgeFactor(double trkAngle) const
    {
      fudgeFactorFunction f = m_implementations[m_current];
      return (this->*f)(trkAngle) ;
    }

    /** constructor */
    SVDMCFudgeFactorFunction()
    {
      // The m_implementations vector is static.
      // We have to initialize it just once.
      if (m_implementations.size() == 0) {
        m_implementations.push_back(&SVDMCFudgeFactorFunction::cheby_v0);
        m_implementations.push_back(&SVDMCFudgeFactorFunction::tgraph_v0);
      }

      m_current = m_implementations.size() - 1;

    };

    /** allows to choose the function version */
    void set_current(int current)
    {
      m_current = current;
    }

    //SETTERS FOR function ID = 0 (cheby_v0)
    /**
     * Set the vector of Chebyshev coefficients
     * Input:
     * @param c
     */
    void set_chebyCoeffs(std::vector<double> c)
    {
      m_chebyCoeffs = c;
    }

    //SETTERS FOR function ID = 1 (tgraph_v0)
    /**
     * Set vectors of TGraph points (x for angles, y for sigmas)
     * Input:
     * @param x
     * @param y
     */
    void set_graphPoints(std::vector<double> x, std::vector<double> y)
    {
      m_x = x;
      m_y = y;
    }

    /** copy constructor */
    SVDMCFudgeFactorFunction(const Belle2::SVDMCFudgeFactorFunction& a);

    /** operator = */
    SVDMCFudgeFactorFunction& operator=(const Belle2::SVDMCFudgeFactorFunction& a);

  private:
    /** function parameters & implementations*/

    /** ID = {0}, rel07: fudge factor parametrized with Chebyshev polynomial
     */

    /** Vector of Chebyshev coefficients */
    std::vector<double> m_chebyCoeffs;

    /** cheby_v0 implementation
     * @param trkAngle track's incident angle
     * @return fudge factor as a function of the track's angle
     */
    double cheby_v0(double trkAngle) const
    {
      TF1* f = (TF1*) gROOT->GetFunction(TString::Format("chebyshev%lu", m_chebyCoeffs.size() - 1));
      f->SetParameters(&m_chebyCoeffs[0]);

      return f->Eval(trkAngle);
    };

    /** ID = {1}, rel07: fudge factor parametrized with linear interpolation between graph points
     */

    /** Vectors of TGraph points for the angles */
    std::vector<double> m_x;
    /** Vectors of TGraph points for the sigmas */
    std::vector<double> m_y;

    /** tgraph_v0 implementation
     * @param trkAngle track's incident angle
     * @return fudge factor as a function of the track's angle
     */
    double tgraph_v0(double trkAngle) const
    {
      TGraph* g = new TGraph(m_x.size(), &m_x[0], &m_y[0]);
      double returnvalue = g->Eval(trkAngle);
      delete g;

      return returnvalue;
    };

    /** current function ID */
    int m_current;

    /** vector of functions for fudge factor computation, we use the m_current*/
    static std::vector < fudgeFactorFunction > m_implementations; //! Do not stream this, please throw it in the WC

    ClassDef(SVDMCFudgeFactorFunction, 2);
  };

}
