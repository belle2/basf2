/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Weber                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dbobjects/TOPSampleTimes.h>

#include <vector>

namespace Belle2 {

  namespace TOP {

    using Point = std::pair<double, double>;

    /**
     *@brief Class to perform template fit on TOP waveform data
     *Minimzation method is described here http://wwwa1.kph.uni-mainz.de/Vorlesungen/SS11/Statistik/
     *@author Tobias Weber
     */
    class TOPTemplateFitter {

    public:

      /**
       *@brief Parameters of the template function.
       *We use a crystal ball function
       */
      struct TemplateParameters {
        double amplitude = 100.;/**< amplitude of template function */
        double risingEdge = 32.;/**< rising edge position of template in samples */
      };

      /**
       *@brief structure holding values from template fit
       */
      struct FitResult {
        double amplitude; /**< fitted amplitude */
        double amplitudeError; /**< fitted amplitude error*/
        double backgroundOffset;/**< fitted background offset */
        double backgroundOffsetError; /**< fitted background offset error*/
        double risingEdge;/**< fitted rising edge*/

        /**
         *@brief set the fit result variables to zero
         */
        void clear()
        {
          amplitude = 0;
          amplitudeError = 0;
          backgroundOffset = 0;
          backgroundOffsetError = 0;
          risingEdge = 0;
        }
      };

      /**
      *@brief Variables used during template fit minimization
      */
      struct MinimizationSums {
        double S1; /**< sum of sample weights */
        double Sx; /**< sum of template*weight */
        double Sxx;/**< sum of template*template*weight */
        double Sy; /**< sum of signal sample * weight */
        double Sxy;/**< sum of signal sample *template*weight */
        double Syy;/**< sum of signal sample * signal sample * weight */

        /**
         *@brief set sums used during template fit to initial values
         */
        void clear()
        {
          S1 = 0;
          Sx = 0;
          Sxx = 0;
          Sy = 0;
          Sxy = 0;
          Syy = 0;
        }
      };

      /**
      @brief full constructor
      @param TOP raw waveform samples
      @param database object holding time correction
      @param average sample RMS
      */
      TOPTemplateFitter(const TOPRawWaveform& wf,
                        const TOPSampleTimes& sampleTimes,
                        const double averageRMS);
      /**
      @brief default destructor
      */
      ~TOPTemplateFitter() {};

      /**
      @brief Returns fit chi square
      @return fit chi square
      */
      double getChisq() const {return m_chisq;}

      /**
      @brief Returns calculated chisq values
      @return chisq vector
      */
      const std::vector<double>& getChisqVector() const {return m_chisq_vec;}

      /**
      @brief Returns fitted values with errors
      @return fitted values with errors
      */
      const FitResult& getFitResult() const {return m_result;}

      /**
      @brief Returns the template parameters
      @return template parameters
      */
      static TemplateParameters& getTemplateParameters() {return s_templateParameters;}

      /**
      @brief Returns the total number of template samples
      @return total number of template samples
      */
      static int getTemplateSamples() {return s_totalTemplateSamples;}

      /**
      @brief Returns the template resolution
      @return template resolution
      */
      static int getTemplateResolution() {return s_templateResolution;}

      /**
       @brief Returns useParabola
       @return useParabola
       */
      static bool getUseParabola() {return s_useParabola;}

      /**
      @brief Sets the template parameters
      @param template Parameters
      */
      static void setTemplateParameters(const TemplateParameters& params);

      /**
      @brief Set the total number of template samples
      @param total number of template samples
      */
      static void setTemplateSamples(int nSamples);

      /**
      @brief Set the template resolution
      @param template resolution
      */
      static void setTemplateResolution(int resolution);

      /**
      @brief Enable Usage of parabola improvement
      @param enable/disable usage
      */
      static void setUseParabola(bool use) {s_useParabola = use;}

      /**
      @brief Intializes the template fit using default values
      */
      static void InitializeTemplateFit();

      /**
      @brief Prepares data and performs the template fit in sample space
      @param initial guess for rising edge position from CFD
      @param range of template fit*/
      void performTemplateFit(const double risingEdgeStart,
                              const double fitRange);

    private:

      /**
       *@brief performs the template fit
       *@param sample vector
       *@param pedestal vector
       *@param timing correction for samples
       *@param rising edge from constant fraction discrimination
       */
      void PerformTemplateFitMinimize(const std::vector<short>& samples, const std::vector<short>& pedestals,
                                      const std::vector<float>& timingCorrection, const double risingEdgeCFD, const double fitRange);

      /**
       *@brief  Compute the minimized parameters and chi square value
       *@param minimization sums for chisq calculation
       *@param minimized parameters
       *@return chi square
       */
      double ComputeMinimizedParametersAndChisq(const MinimizationSums& sums, FitResult& result);

      /**
       *@brief Calculate vertex coordinates of parabola given three data points
       *@param data point 1
       *@param data point 2
       *@param data point 3
       *@param vertex position
       */
      void CalculateParabolaVertex(const Point& p1, const Point& p2, const Point& p3,
                                   Point& vertex);

      const TOPRawWaveform m_wf; /**< raw sampled waveforms */
      const TOPSampleTimes m_sampleTimes; /**< provides timing correction */
      const double m_averageRMS; /**< average RMS of waveform samples, no database for this */

      static int s_totalTemplateSamples;/**< number of samples used for template*/
      static int s_templateResolution;/**< resolution of template with respect to normal sample spacing*/
      static TemplateParameters s_templateParameters;/**< parameters used for the template calculation*/
      static std::vector<double> s_templateSamples;/**< precomputed template samples*/
      static int s_fineOffsetRange;/**< range for offset between template and signal*/
      static bool s_templateReInitialize;/**< flag showing that the template samples have to be recomputed*/
      static bool s_useParabola; /**< try improving fit by making use of parabolic shape of chisq values*/

      //fit results
      FitResult m_result;/**< fit result from template fit*/
      double m_chisq;/**< chi square value from template fit */
      std::vector<double> m_chisq_vec;/**< all computed chi square values from template fit */

    };

  }
}
