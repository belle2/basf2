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
    bool m_printCanvas;
    float m_occError;
    float m_occWarning;
    float m_occEmpty;
    //! Parameters accesible from basf2 scripts
    //  protected:

    //! Data members
  private:

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

    TCanvas* m_cUnpacker = nullptr;
    TH2F* m_hOccupancyU = nullptr;
    TCanvas* m_cOccupancyU = nullptr;
    TH2F* m_hOccupancyV = nullptr;
    TCanvas* m_cOccupancyV = nullptr;

    TPaveText boxOcc(Int_t layer, Int_t ladder, Int_t sensor, Int_t color);
    Int_t findBinY(Int_t layer, Int_t sensor);

    TPaveText* m_leg = nullptr;
    TPaveText* m_legProblem = nullptr;
    TPaveText* m_legWarning = nullptr;
    TPaveText* m_legNormal = nullptr;
    TPaveText* m_legEmpty = nullptr;
    TPaveText* m_legError = nullptr;
    TText* m_yTitle = nullptr;


    //! IDs of all SXD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

  };
} // end namespace Belle2

