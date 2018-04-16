/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Analyze histograms of amplitudes for each ECL crystal from gamma pair  *
 * events. Code can either find most-likely energy deposit for each       *
 * crystal (MC) or calibration constant for each crystal (data)           *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <ecl/calibration/eclGammaGammaEAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace ECL {

    /** Calibrate ecl crystals using gamma pair events */
    class eclGammaGammaEAlgorithm : public CalibrationAlgorithm {
    public:

      /**..Constructor */
      eclGammaGammaEAlgorithm();

      /**..Destructor */
      virtual ~eclGammaGammaEAlgorithm() {}

      /** Setter for m_outputName */
      void setOutputName(std::string outputName) {m_outputName = outputName;}

      /** Getter for m_outputName */
      std::string getOutputName() {return m_outputName;}

      /** Setter for m_cellIDLo */
      void setCellIDLo(int cellIDLo) {m_cellIDLo = cellIDLo;}

      /** Getter for m_cellIDLo */
      int getCellIDLo() {return m_cellIDLo;}

      /** Setter for m_cellIDHi */
      void setCellIDHi(int cellIDHi) {m_cellIDHi = cellIDHi;}

      /** Getter for m_cellIDHi */
      int getCellIDHi() {return m_cellIDHi;}

      /** Setter for m_minEntries */
      void setMinEntries(int minEntries) {m_minEntries = minEntries;}

      /** Getter for m_minEntries */
      int getMinEntries() {return m_minEntries;}

      /** Setter for m_maxIterations */
      void setMaxIterations(int maxIterations) {m_maxIterations = maxIterations;}

      /** Getter for m_maxIterations */
      int getMaxIterations() {return m_maxIterations;}

      /** Setter for m_tRatioMin */
      void setTRatioMin(double tRatioMin) {m_tRatioMin = tRatioMin;}

      /** Getter for m_tRatioMin */
      double getTRatioMin() {return m_tRatioMin;}

      /** Setter for m_tRatioMax */
      void setTRatioMax(double tRatioMax) {m_tRatioMax = tRatioMax;}

      /** Getter for m_tRatioMax */
      double getTRatioMax() {return m_tRatioMax;}

      /** Setter for m_upperEdgeThresh */
      void setUpperEdgeThresh(double upperEdgeThresh) {m_upperEdgeThresh = upperEdgeThresh;}

      /** Getter for m_upperEdgeThresh */
      double getUpperEdgeThresh() {return m_upperEdgeThresh;}

      /** Setter for m_performFits */
      void setPerformFits(bool performFits) {m_performFits = performFits;}

      /** Getter for m_performFits */
      bool getPerformFits() {return m_performFits;}

      /** Setter for m_findExpValues */
      void setFindExpValues(bool findExpValues) {m_findExpValues = findExpValues;}

      /** Getter for m_findExpValues */
      bool getFindExpValues() {return m_findExpValues;}

      /** Setter for m_storeConst */
      void setStoreConst(int storeConst) {m_storeConst = storeConst;}

      /** Getter for m_storeConst */
      int getStoreConst() {return m_storeConst;}


    protected:

      /**..Run algorithm on events */
      virtual EResult calibrate();

    private:

      /**..Parameters to control Novosibirsk fit to energy deposited in each crystal by mu+mu- events */
      std::string m_outputName = "eclGammaGammaEAlgorithm.root"; /**< file name for histogram output */
      int m_cellIDLo = 1;  /**<  First cellID to be fit */
      int m_cellIDHi = 8736;  /**<  Last cellID to be fit */
      int m_minEntries = 150;  /**<  Minimum entries to fit a crystal */
      int m_maxIterations = 10; /**< no more than maxIteration iterations */
      double m_tRatioMin = 0.45;  /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak */
      double m_tRatioMax = 0.60; /**< Fit range is adjusted so that fit at upper endpoint is between tRatioMin and tRatioMax of peak */
      double m_upperEdgeThresh = 0.02; /**< Upper edge is where the fit = upperEdgeThresh * peak value */
      bool m_performFits = true;  /**<  if false, input histograms are copied to output, but no fits are done */
      bool m_findExpValues =
        false;  /**< if true, fits are used to find expected energy deposit for each crystal instead of the calibration constant */
      int m_storeConst = 0; /**< controls which values are written to the database.
                         0 : store value found by successful fits, or -|input value| otherwise;
                         -1 : do not store values
                         1 : store values if every fit for [cellIDLo,cellIDHi] was successful */

      /** Characterize fit status */
      int fitOK = 16; /**< fit is OK */
      int iterations = 8; /**< fit reached max number of iterations, but is useable */
      int atLimit = 4; /**< a parameter is at the limit; upper edge is found from histogram, not fit */
      int poorFit = 3; /**< low chi square; upper edge is found from histogram, not fit */
      int noPeak = 2; /**< Novosibirsk component of fit is negligible; upper edge is found from histogram, not fit */
      int notFit = -1; /**< no fit performed; no constants found for this crystal */

    };
  }
} // namespace Belle2


