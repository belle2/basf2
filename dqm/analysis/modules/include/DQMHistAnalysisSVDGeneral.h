//+
// File : DQMHistAnalysisSVDGeneral.h
// Description : An example module for DQM histogram analysis
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI)
// Date : 20181128
//-

#pragma once

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TFile.h>
#include <TText.h>
#include <TPaveText.h>
#include <TCanvas.h>
#include <TH2F.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDGeneralModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisSVDGeneralModule();
    virtual ~DQMHistAnalysisSVDGeneralModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    //parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    float m_occError; /**<error level of the occupancy */
    float m_occWarning; /**< warning level of the occupancy */
    float m_occEmpty; /**<empty level of the occupancy */
    //! Parameters accesible from basf2 scripts
    //  protected:

    //! Data members
  private:

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

    TCanvas* m_cUnpacker = nullptr; /**<unpacker plot canvas */
    TH2F* m_hOccupancyU = nullptr; /**< occupnacy U histo */
    TCanvas* m_cOccupancyU = nullptr; /**< occupacy U histo canvas */
    TH2F* m_hOccupancyV = nullptr; /**< occupancy V histo */
    TCanvas* m_cOccupancyV = nullptr; /**< occupancy V histo canvas */

    TPaveText boxOcc(Int_t layer, Int_t ladder, Int_t sensor, Int_t color); /**< plot sensor-box in occupancy plot */
    Int_t findBinY(Int_t layer, Int_t sensor); /**< find Y bin corresponding to sensor, occupancy plot*/

    TPaveText* m_leg = nullptr; /**< occupancy plot legend */
    TPaveText* m_legProblem = nullptr; /**< occupancy plot legend, problem */
    TPaveText* m_legWarning = nullptr; /**< occupancy plot legend, warning */
    TPaveText* m_legNormal = nullptr; /**< occupancy plot legend, normal */
    TPaveText* m_legEmpty = nullptr; /**< occupancy plot legend, empty */
    TPaveText* m_legError = nullptr; /**< occupancy plot legend, error*/
    TText* m_yTitle = nullptr; /**< y axis title text*/


    //! IDs of all SXD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

  };
} // end namespace Belle2

