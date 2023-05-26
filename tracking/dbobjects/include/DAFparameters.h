/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

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
    * @param probabilitycut probability cut used in setProbCut and addProbCut method from DAF
    */
    void setProbabilityCut(const float probabilitycut)
    {
      m_ProbabilityCut = probabilitycut;
    }
    /** Set the start and end temperatures, and the number of iterations for the annealing scheme
    * @param temperaturestart Starting temperature for annealing scheme
    * @param temperaturefinal Final temperature for annealing scheme
    * @param numberofiterations number of iterations for annealing scheme
    */
    void setAnnealingScheme(const float temperaturestart, const float temperaturefinal, const int numberofiterations)
    {
      m_TemperatureStart = temperaturestart;
      m_TemperatureFinal = temperaturefinal;
      m_NumberOfIterations = numberofiterations;
    }
    /** Set the minimum number of iterations for pValue check
    * @param minimumiterations number of iterations for annealing scheme
    */
    void setMinimumIterations(const int minimumiterations)
    {
      m_MinimumIterations = minimumiterations;
    }
    /** Set the maximum number of iterations
    * @param maximumiterations max number of iterations of annealing scheme
    * We suggest to avoid setting MaxIter < NIter
    */
    void setMaximumIterations(const int maximumiterations)
    {
      m_MaximumIterations = maximumiterations;
    }
    /** Set the minimum number of iterations for pValue check
    * @param minimumiterationsforpval min number of iterations of annealing scheme before pvalue check
    */
    void setMinimumIterationsForPVal(const int minimumiterationsforpval)
    {
      m_MinimumIterationsForPVal = minimumiterationsforpval;
    }
    /** Set the maximum number of failed hits after which the fit should be cancelled.
    * (exception during construction of plane, extrapolation etc.)
    * -1 means don't cancel
    * @param maximumfailedhits max number of failed hits
    */
    void setMaximumFailedHits(const int maximumfailedhits)
    {
      m_MaximumFailedHits = maximumfailedhits;
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
    float getProbabilityCut() const
    {
      return m_ProbabilityCut;
    }
    /** Get the start and end temperatures and number of iterations for the annealing scheme
    * returns a tuple with the elements in the following order: (TStart, TEnd, NIter)
    */
    std::tuple<float, float, int> getAnnealingScheme() const
    {
      return std::tuple<float, float, int>(m_TemperatureStart, m_TemperatureFinal, m_NumberOfIterations);
    }
    /** Get the minimum number of iterations of annealing scheme
    */
    int getMinimumIterations() const
    {
      return m_MinimumIterations;
    }
    /** Get the maximum number of iterations of annealing scheme
    */
    int getMaximumIterations() const
    {
      return m_MaximumIterations;
    }
    /** Get the minimum number of iterations for pValue check
    */
    int getMinimumIterationsForPVal() const
    {
      return m_MinimumIterationsForPVal;
    }
    /** Get the maximum number of failed hits after which the fit should be cancelled.
    * (exception during construction of plane, extrapolation etc.)
    * -1 means don't cancel
    */
    int getMaximumFailedHits() const
    {
      return m_MaximumFailedHits;
    }

  private:
    /** The DeltaPValue for p-value convergence criterion */
    float m_DeltaPValue = 1;
    /** The DeltaWeight for weights convergence criterion */
    float m_DeltaWeight = 0.001;
    /** The probability cut for weight calculation */
    float m_ProbabilityCut = 0.001;
    /** The annealing scheme :
    * Start temperature */
    float m_TemperatureStart = 100;
    /** End temperature */
    float m_TemperatureFinal = 0.1;
    /** Number of iterations */
    int m_NumberOfIterations = 5;
    /** The minimum number of iterations of annealing scheme */
    int m_MinimumIterations = 5;
    /** The maximum number of iterations of annealing scheme */
    int m_MaximumIterations = 9;
    /** The minimum number of iterations for pValue check */
    int m_MinimumIterationsForPVal = 5;
    /** The maximum number of failed hits after which the fit should be cancelled.*/
    int m_MaximumFailedHits = 5;


    ClassDef(DAFparameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
