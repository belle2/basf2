/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <fstream>

#include <TH1D.h>
#include <TH2F.h>
#include <TH1I.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include<TGraphErrors.h>

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <reconstruction/utility/CDCDedxMeanPred.h>
#include <reconstruction/utility/CDCDedxSigmaPred.h>
#include <reconstruction/utility/CDCDedxHadSat.h>
#include <framework/gearbox/Const.h>

#include <reconstruction/dbobjects/CDCDedxHadronCor.h>

// enum gstatus {OK, Failed};

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx injection time (HER/LER)
   */
  class CDCDedxHadSatAlgorithm : public CalibrationAlgorithm {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the algorithm.
    */
    CDCDedxHadSatAlgorithm();

    /**
    * Destructor
    */
    virtual ~CDCDedxHadSatAlgorithm() {}

    /**
    * function to decide merged vs relative calibration
    */
    void setMergePayload(bool value = true) {m_isMerge = value;}

    /**
    * function to enable monitoring plots
    */
    void setMonitoringPlots(bool value = false) {m_ismakePlots = value;}


    /**
    * function to set dedx hist parameters
    */
    void setDedxPars(int value, double min, double max)
    {
      m_dedxBins = value;
      m_dedxMin = min;
      m_dedxMax = max;
    }

    /**
    * function to set chi hist parameters
    */
    void setChiPars(int value, double min, double max)
    {
      m_chiBins = value;
      m_chiMin = min;
      m_chiMax = max;
    }

    void getExpRunInfo();


  protected:

    /**
    * CDC dE/dx Injection time algorithm
    */
    virtual EResult calibrate() override;

  private:

    std::map<int, std::vector<TH1F*>> hdedxnosat_bgcosth, hdedx_bgcosth, hchi_bgcosth;
    std::vector<TH2F*> hdedxvscosth_bg, hdedxnosatvscosth_bg;
    std::vector<TH1F*> hdedx_costh, hdedxnosat_costh;

    std::map<int, std::vector<double>> sumcos, sumbg, sumres_square;
    std::map<int, std::vector<int>> sumsize;
    std::map<int, std::vector<double>> means, errors;

    int m_dedxBins; /**< bins for dedx histogram */
    double m_dedxMin; /**< min range of dedx */
    double m_dedxMax; /**< max range of dedx */

    int m_bgBins; /**< bins for dedx histogram */
    double m_bgMin; /**< min range of dedx */
    double m_bgMax; /**< max range of dedx */

    int m_cosBins; /**< bins for dedx histogram */
    double m_cosMin; /**< min range of dedx */
    double m_cosMax; /**< max range of dedx */

    int m_chiBins; /**< bins for chi histogram */
    double m_chiMin; /**< min range of chi */
    double m_chiMax; /**< max range of chi */

    std::vector< double > m_dedx;      // a vector to hold dE/dx measurements
    std::vector< double > m_dedxerror; // a vector to hold dE/dx errors
    std::vector< double > m_betagamma; // a vector to hold beta-gamma values
    std::vector< double > m_costheta;  // a vector to hold cos(theta) values

    bool m_ismakePlots; /**< produce plots for monitoring */
    bool m_isMerge; /**< merge payload when rel constant*/

    std::string m_suffix; /**< string suffix for object names */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< db object for dE/dx resolution parameters */

  };
} // namespace Belle2