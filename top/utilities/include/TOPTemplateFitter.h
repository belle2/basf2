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
    @brief Parameters of the template function.
    We use a crystal ball function
    */
    struct TemplateParameters {
      double amplitude = 100.;
      double mean = 32.;
      double sigma = 1.81;
      double alpha = -0.45;
      double n = 18.06;
    };

    /**
    @brief structure holding values from template fit
    */
    struct FitResult {
      double amplitudeScaling, amplitudeScalingErr; /**< fitted amplitude and error*/
      double backgroundOffset, backgroundOffsetError; /**< fitted background offset and error*/
      double risingEdge, risingEdgeError;/**< fitted rising edge and error*/
    };

    /**
    @brief Variables used during template fit minimization
    */
    struct MinimizationSums {
      double S1;
      double Sx, Sxx;
      float Sy, Sxy, Syy;
    };

    /**
     *@brief Class to perform template fit on TOP waveform data
     *Minimzation method is described here http://wwwa1.kph.uni-mainz.de/Vorlesungen/SS11/Statistik/
     *@author Tobias Weber
     */
    class TOPTemplateFitter {

    public:

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
      static TemplateParameters& GetTemplateParameters() {return s_templateParameters;}

      /**
      @brief Returns the total number of template samples
      @return total number of template samples
      */
      static int GetTemplateSamples() {return s_totalTemplateSamples;}

      /**
      @brief Returns the template resolution
      @return template resolution
      */
      static int GetTemplateResolution() {return s_templateResolution;}

      /**
      @brief Sets the template parameters
      @param template Parameters
      */
      static void SetTemplateParameters(const TemplateParameters& params);

      /**
      @brief Set the total number of template samples
      @param total number of template samples
      */
      static void SetTemplateSamples(int nSamples);

      /**
      @brief Set the template resolution
      @param template resolution
      */
      static void SetTemplateResolution(int resolution);

      /**
      @brief Performs the template fit in sample space
      @param initial guess for rising edge position from CFD
      @param range of template fit*/
      void PerformTemplateFit(const double risingEdgeStart,
                              const double fitRange);

    private:

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

      /**
      @brief Intializes the template fit using default values
      */
      static void InitializeTemplateFit();

      const TOPRawWaveform m_wf; /** <raw sampled waveforms */
      const TOPSampleTimes m_sampleTimes; /** <provides timing correction */
      double m_averageRMS; /**average RMS of waveform samples, no database for this */

      static int s_totalTemplateSamples;/**< number of samples used for template*/
      static int s_templateResolution;/**< resolution of template with respect to normal sample spacing*/
      static TemplateParameters s_templateParameters;/**< parameters used for the template calculation*/
      static std::vector<double> s_templateSamples;/**<precomputed template samples*/

      //fit results
      FitResult m_result;/**< fit result from template fit*/
      double m_chisq;/**< chi square value from template fit */

    };

  }
}
