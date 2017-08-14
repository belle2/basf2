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

namespace Belle2 {

  namespace TOP {

    /**
    @brief structure holding values from template fit
    */
    struct FitResult {
      double m_amplitude, m_amplitudeErr; /**< fitted amplitude and error*/
      double m_backgroundOffset, m_backgroundOffsetError; /**< fitted background offset and error*/
      double m_risingEdge, m_risingEdgeError;/**< fitted rising edge and error*/
    };

    /**
     *@brief Class to perform template fit on TOP waveform data
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
      @return fit chi square*/
      double getChisq() const {return m_chisq;}

      /**
      @brief Returns fitted values with errors
      @return fitted values with errors*/
      const FitResult& getFitResult() const {return m_result;}

      /**
      @brief Performs the template fit
      @param initial guess for rising edge position from CFD
      @param range of template fit*/
      void PerformTemplateFit(const double risingEdgeStart,
                              const double fitRange);

    private:
      const TOPRawWaveform m_wf; /** <raw sampled waveforms */
      const TOPSampleTimes m_sampleTimes; /** <provides timing correction */
      double m_averageRMS; /**average RMS of waveform samples, no database for this */

      //fit results
      FitResult m_result;/**< fit result from template fit*/
      double m_chisq;/**< chi square value from template fit */
      int m_ndf;/**< number of degrees of freedom of template fit*/
    };

  }
}
