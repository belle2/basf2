//+
// File : DQMHistAnalysisSVDGeneral.h
// Description : An example module for DQM histogram analysis
//
// Author : Giulia Casarosa
// Date : 20181128
//-

#pragma once

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/geometry/SensorInfoBase.h>

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
    TCanvas* m_cUnpacker = nullptr;
    TH2F* m_hOccupancyUtext = nullptr;
    TH2F* m_hOccupancyUcolz = nullptr;
    TCanvas* m_cOccupancyU = nullptr;
    TH2F* m_hOccupancyVtext = nullptr;
    TH2F* m_hOccupancyVcolz = nullptr;
    TCanvas* m_cOccupancyV = nullptr;

    //! IDs of all SXD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

  };
} // end namespace Belle2

