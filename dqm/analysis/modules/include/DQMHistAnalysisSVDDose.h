/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ludovico Massaccesi                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TCanvas.h>
#include <TString.h>
#include <TPaveText.h>
#include <TH2.h>
#include <TH1.h>
#include <string>
#include <vector>

namespace Belle2 {
  class DQMHistAnalysisSVDDoseModule : public DQMHistAnalysisModule {
  public:
    DQMHistAnalysisSVDDoseModule();
    virtual ~DQMHistAnalysisSVDDoseModule();

  private:
    /** A struct to define the sensors group we average over.
     * See Belle2::SVD::SVDDQMDoseModule::SensorGroup.
     */
    typedef struct SensorGroup {
      TString nameSuffix;
      TString titleSuffix;
      int nStrips;
    } SensorGroup;

    void initialize() override final;
    void event() override final;
    void endRun() override final;

    void updateCanvases();

    /// Utility method
    template<typename T>
    inline T* findHistT(TString name) { return dynamic_cast<T*>(findHist(name.Data())); }

    /** Divide two histograms, ignoring errors on the second histogram.
     * The result is stored in the numerator histogram.
     * @param num The numerator histogram. Will be overwritten with the result.
     * @param den The denominator histogram. Must have the same bins as num.
     * @param scale Optional rescaling factor.
     *
     * In short, for each bin we have
     *  - `result_bin = scale * num_bin / den_bin`
     *  - `result_err = scale * num_err / den_bin`
     */
    void divide(TH2F* num, TH2F* den, float scale = 1.0f);

    /// Carries the content of the overflow bin into the last bin
    static void carryOverflowOver(TH1F* h);

    // Steerable data members (parameters)
    std::string m_pvPrefix; ///< Prefix for EPICS PVs
    bool m_useEpics; ///< Whether to update EPICS PVs

    // Data members for outputs
    MonitoringObject* m_monObj = nullptr; ///< Monitoring object for MiraBelle
    std::vector<TCanvas*> m_c_instOccu; ///< Canvases for the instantaneous occupancy
    std::vector<TCanvas*> m_c_occuLER; ///< Canvases for the occu. vs time after LER inj.
    std::vector<TCanvas*> m_c_occuHER; ///< Canvases for the occu. vs time after HER inj.
    TPaveText* m_legend = nullptr; ///< Legend of the inst. occu. plots

    /** List of sensors groups. Must match Belle2::SVD::SVDDQMDoseModule::c_sensorGroups.
     * Defined in DQMHistAnalysisSVDDose.cc.
     */
    static const std::vector<SensorGroup> c_sensorGroups;
  };
}