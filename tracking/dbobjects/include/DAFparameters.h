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
#include <tuple>

namespace Belle2 {

  /** The payload containing the DAF parameters.
   */
  class DAFparameters: public TObject {

  public:

    /** Default constructor */
    DAFparameters() {}
    /** Destructor */
    ~DAFparameters() {}

    /** Set the DeltaPValue for p-value convergence criterion
     * @param pvalue Delta pvalue for convergence; [0,1]
     */
    void setDeltaPValue(const float pvalue)
    {
      m_DeltaPValue = pvalue;
    }
    /** Set the DeltaWeight for weights convergence criterion
     * @param weight Delta weight for convergence; [0,1]
     */
    void setDeltaWeight(const float weight)
    {
      m_DeltaWeight = weight;
    }
    /** Set the probability cut for the weight calculation for the hits
    * @param probcut probability cut used in setProbCut and addProbCut method from DAF
    */
    void setProbCut(const float probcut)
    {
      m_ProbCut = probcut;
    }
    /** Set the start and end temperatures, and the number of iterations for the annealing scheme
    * @param Tstart Starting temperature for annealing scheme
    * @param Tfinal Final temperature for annealing scheme
    * @param niter number of iterations for annealing scheme
    */
    void setAnnealingScheme(const float Tstart, const float Tfinal, const int niter)
    {
      //m_AnnealingScheme = std::tuple<float, float, int>(Tstart, Tfinal, niter);
      m_TStart = Tstart;
      m_TFinal = Tfinal;
      m_NIter = niter;
    }
    /** Set the minimum number of iterations for pValue check
    * @param miniter number of iterations for annealing scheme
    */
    void setMinIter(const int miniter)
    {
      m_MinIter = miniter;
    }
    /** Set the maximum number of iterations
    * @param maxiter max number of iterations of annealing scheme
    * We suggest to avoid setting MaxIter < NIter
    */
    void setMaxIter(const int maxiter)
    {
      m_MaxIter = maxiter;
    }
    /** Set the minimum number of iterations for pValue check
    * @param miniterforpval min number of iterations of annealing scheme before pvalue check
    */
    void setMinIterForPVal(const int miniterforpval)
    {
      m_MinIterForPVal = miniterforpval;
    }
    /** Set the maximum number of failed hits after which yhe fit should be cancelled.
    * (exception during construction of plane, extrapolation etc.)
    * -1 means don't cancel
    * @param maxfailedhits max number of failed hits
    */
    void setMaxFailedHits(const int maxfailedhits)
    {
      m_MaxFailedHits = maxfailedhits;
    }



    /** Get the DeltaPValue for p-value convergence criterion
     */
    float getDeltaPValue() const
    {
      return m_DeltaPValue;
    }
    /** Get the DeltaWeight for weight convergence criterion
     */
    float getDeltaWeight() const
    {
      return m_DeltaWeight;
    }
    /** Get the probability cut for the weight calculation for the hits
    */
    float getProbCut() const
    {
      return m_ProbCut;
    }
    /** Get the start and end temperatures and number of iterations for the annealing scheme
    */
    std::tuple<float, float, int> getAnnealingScheme() const
    {
      return std::tuple<float, float, int>(m_TStart, m_TFinal, m_NIter);
    }
    /** Get the minimum number of iterations of annealing scheme
    */
    int getMinIter() const
    {
      return m_MinIter;
    }
    /** Get the maximum number of iterations of annealing scheme
    */
    int getMaxIter() const
    {
      return m_MaxIter;
    }
    /** Get the minimum number of iterations for pValue check
    */
    int getMinIterForPVal() const
    {
      return m_MinIterForPVal;
    }
    /** Get the maximum number of failed hits after which the fit should be cancelled.
    * (exception during construction of plane, extrapolation etc.)
    * -1 means don't cancel
    */
    int getMaxFailedHits() const
    {
      return m_MaxFailedHits;
    }

  private:
    /** The DeltaPValue for p-value convergence criterion */
    float m_DeltaPValue;
    /** The DeltaWeight for weights convergence criterion */
    float m_DeltaWeight;
    /** The probability cut for weight calculation */
    float m_ProbCut;
    /** The annealing scheme :
    /** Start temperature */
    float m_TStart;
    /** End temperature */
    float m_TFinal;
    /** Number of iterations */
    int m_NIter;
    /** The minimum number of iterations of annealing scheme */
    int m_MinIter;
    /** The maximum number of iterations of annealing scheme */
    int m_MaxIter;
    /** The minimum number of iterations for pValue check */
    int m_MinIterForPVal;
    /** The maximum number of failed hits after which the fit should be cancelled.*/
    int m_MaxFailedHits;


    ClassDef(DAFparameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
