/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      void setOutputName(const std::string& outputName) {m_outputName = outputName;}

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

      /** Setter for m_tRatioMinNom */
      void setTRatioMin(double tRatioMin) {m_tRatioMinNom = tRatioMin;}

      /** Getter for m_tRatioMinNom */
      double getTRatioMin() {return m_tRatioMinNom;}

      /** Setter for m_tRatioMaxNom */
      void setTRatioMax(double tRatioMax) {m_tRatioMaxNom = tRatioMax;}

      /** Getter for m_tRatioMaxNom */
      double getTRatioMax() {return m_tRatioMaxNom;}

      /** Setter for m_tRatioMinHiStat */
      void setTRatioMinHiStat(double tRatioMin) {m_tRatioMinHiStat = tRatioMin;}

      /** Getter for m_tRatioMinHiStat */
      double getTRatioMinHiStat() {return m_tRatioMinHiStat;}

      /** Setter for m_tRatioMaxHiStat */
      void setTRatioMaxHiStat(double tRatioMax) {m_tRatioMaxHiStat = tRatioMax;}

      /** Getter for m_tRatioMaxHiStat */
      double getTRatioMaxHiStat() {return m_tRatioMaxHiStat;}

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
      virtual EResult calibrate() override;

    private:

      /**..Parameters to control Novosibirsk fit to energy deposited in each crystal by mu+mu- events */
      std::string m_outputName = "eclGammaGammaEAlgorithm.root"; /**< file name for histogram output */
      int m_cellIDLo = 1;  /**<  First cellID to be fit */
      int m_cellIDHi = 8736;  /**<  Last cellID to be fit */
      int m_minEntries = 150;  /**<  Minimum entries to fit a crystal */
      int m_highStatEntries = 25000; /**< Adjust fit range above this many entries */
      int m_maxIterations = 10; /**< no more than maxIteration iterations */
      double m_tRatioMinNom =
        0.45;  /**< Fit range is adjusted so that fit at lower endpoint is between tRatioMin and tRatioMax of peak */
      double m_tRatioMaxNom = 0.70; /**< Fit range is adjusted so that fit at lower endpoint is between tRatioMin and tRatioMax of peak */
      double m_tRatioMinHiStat =
        0.70;  /**< Fit range is adjusted so that fit at lower endpoint is between tRatioMin and tRatioMax of peak */
      double m_tRatioMaxHiStat =
        0.95; /**< Fit range is adjusted so that fit at lower endpoint is between tRatioMin and tRatioMax of peak */
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


