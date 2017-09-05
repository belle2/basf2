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
        double amplitude = 100.;
        double mean = 32.;
        double sigma = 1.81;
        double alpha = -0.45;
        double n = 18.06;
      };

      /**
       *@brief structure holding values from template fit
       */
      struct FitResult {
        double amplitude, amplitudeError; /**< fitted amplitude and error*/
        double backgroundOffset, backgroundOffsetError; /**< fitted background offset and error*/
        double risingEdge;/**< fitted rising edge*/

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
        double S1;
        double Sx, Sxx;
        double Sy, Sxy, Syy;

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
       *@brief compute the rising edge of the template (which is easily done for the Gaussian center
       *of the crystal ball.
       *@return rising edge
       */
      double GetTemplateRisingEdge();

      /**
      @brief crystal ball function
      @param sample position
      @return value of crystall ball function
      */
      static double Crystalball(const double x);

      /**
      @brief Calculate the value of the Template Function at given sample.
      @param sample position. Can be fractional number
      @return value of template function
      */
      static double CalculateTemplate(const double x);

      const TOPRawWaveform m_wf; /** <raw sampled waveforms */
      const TOPSampleTimes m_sampleTimes; /** <provides timing correction */
      double m_averageRMS; /**average RMS of waveform samples, no database for this */

      static int s_totalTemplateSamples;/**< number of samples used for template*/
      static int s_templateResolution;/**< resolution of template with respect to normal sample spacing*/
      static TemplateParameters s_templateParameters;/**< parameters used for the template calculation*/
      static std::vector<double> s_templateSamples;/**<precomputed template samples*/
      static int s_fineOffsetRange;/**<range for offset between template and signal*/
      static bool s_templateReInitialize;/**<flag showing that the template samples have to be recomputed*/

      //fit results
      FitResult m_result;/**< fit result from template fit*/
      double m_chisq;/**< chi square value from template fit */

    };

  }
}
