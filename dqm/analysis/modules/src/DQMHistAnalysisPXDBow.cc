/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisPXDBow.h>

#include <tracking/dbobjects/ROICalculationParameters.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDBow);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDBowModule::DQMHistAnalysisPXDBowModule()
  : DQMHistAnalysisModule()
{
  setDescription("DQMHistAnalysisModule to monitor the PXD bowing");

  /// Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where the histogram is placed",
           std::string("PXDBow/"));
  addParam("moduleName", m_moduleName,
           "Layer_ladder_sensor numbers of the module which residual histogram will be plotted on the DQM, if empty all histogram are plotted",
           std::string("2.2.1"));
  addParam("statThreshold", m_statThreshold,
           "minimum number of entries needed to calculate the mean in the delta distribution of the sagitta",
           m_statThreshold);
  addParam("sagittaThreshold", m_sagittaThreshold,
           "threshold for the warning related to the sagitta",
           m_sagittaThreshold);
  addParam("sagittaErrorThreshold", m_sagittaErrorThreshold,
           "threshold for the error related to the sagitta",
           m_sagittaErrorThreshold);
  addParam("roiThreshold", m_roiThreshold,
           "threshold for the warning related to the roi",
           m_roiThreshold);
  B2DEBUG(20, "DQMHistAnalysisPXDBow: Constructor done.");
}


void DQMHistAnalysisPXDBowModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisPXDBow: initialized.");

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  /// collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  int validModule = 0;
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD || aVxdID.getSensorNumber() != 1) continue;/// only forward PXD modules
    m_PXDModules.push_back(aVxdID);

    auto buff = (std::string)aVxdID;
    /// register delta
    if (!hasDeltaPar(m_histogramDirectoryName, "resV_" + buff))
      addDeltaPar(m_histogramDirectoryName, "resV_" + buff, HistDelta::c_Entries, m_statThreshold, 1);
    if (!hasDeltaPar(m_histogramDirectoryName, "sagitta_" + buff))
      addDeltaPar(m_histogramDirectoryName, "sagitta_" + buff, HistDelta::c_Entries, m_statThreshold, 1);
    /// Epics for all the forward PXD module
    replace(buff.begin(), buff.end(), '.', '_');
    registerEpicsPV("PXD:meanResV:" + buff, "meanResV:" + (std::string)aVxdID);
    registerEpicsPV("PXD:sigmaResV:" + buff, "sigmaResV:" + (std::string)aVxdID);
    registerEpicsPV("PXD:sagitta:" + buff, "sagitta:" + (std::string)aVxdID);
    /// list of canvases
    if (m_moduleName == "") { /// one canvas for each forward PXD module
      m_cResV[buff] = new TCanvas((std::string("c_resV_") + buff).c_str());
    } else if (VxdID(m_moduleName) == aVxdID) { ///only one canvas
      m_cResV[buff] = new TCanvas((std::string("c_resV_") + buff).c_str());
      validModule++;
    }

  }

  if (validModule == 0 and m_moduleName != "") {
    B2WARNING("Invalid moduleName, only empty value or PXD forward modules are acceptable, nameModule parameter set to default (2.2.1)");
    m_moduleName = "2.2.1";
  } else if (m_moduleName == "") B2INFO("Plotting the histogram for all forward sensors");
  else B2INFO("Plotting histogram for module " << m_moduleName);

  if (m_PXDModules.size() == 0) {
    B2WARNING("No PXDModules in Geometry found!");
  }

}

void DQMHistAnalysisPXDBowModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisPXDBow : beginRun called");

  for (auto& pair : m_cResV) {
    pair.second->Clear();
  }

  /// get RoI size in the v coordinate
  DBObjPtr<ROICalculationParameters> roiParams;
  if (!roiParams.isValid()) B2WARNING("Cannot get roi parameters, ROI v half size defined as default (0.1 cm)");
  else m_roiThreshold = roiParams->getSigmaSystV() * roiParams->getNumSigmaTotV() / 2;
}

void DQMHistAnalysisPXDBowModule::event()
{
  for (VxdID& aPXDModule : m_PXDModules) {
    auto buff = (std::string)aPXDModule;
    std::replace(buff.begin(), buff.end(), '.', '_');

    TH1* hV = getDelta(m_histogramDirectoryName + "resV_" + buff, 0, true);
    TH1* hS = getDelta(m_histogramDirectoryName + "sagitta_" + buff, true);
    if (hS != NULL && hV != NULL) {
      bool enough = false, warnflag = false, errorflag = false;
      B2DEBUG(20, "Histos resS_" << buff << " and resV_" << buff << " found");
      if (hS->GetEntries() > m_statThreshold) {
        enough = true;

        /// Epics PVs
        double meanResV = hV->GetMean();
        double stdResV = hV->GetStdDev();
        double resV = std::abs(meanResV) + 3 * stdResV;
        double bowAmplitude = hS->GetMean();

        if (std::abs(bowAmplitude) > m_sagittaErrorThreshold) errorflag = true; /// error
        else if (resV > m_roiThreshold
                 || std::abs(bowAmplitude) > m_sagittaThreshold) warnflag = true;/// warning

        setEpicsPV("meanResV:" + buff, meanResV);
        setEpicsPV("stdResV:" + buff, stdResV);
        setEpicsPV("sagitta:" + buff, bowAmplitude);
      } else
        enough = false;

      if (m_moduleName == "") {
        plotCanvas(enough, errorflag, warnflag, buff);
      } else if (aPXDModule == VxdID(m_moduleName)) {
        plotCanvas(enough, errorflag, warnflag, buff);
      } else B2WARNING("Invalid input for the parameter m_moduleName");
    } else B2DEBUG(20, "Histo resV_" << buff << " not found");
  }
}

void DQMHistAnalysisPXDBowModule::plotCanvas(bool enough, bool errorflag, bool warnflag, std::string buff)
{
  EStatus status = makeStatus(enough, errorflag, warnflag);
  TH1* h = findHist(m_histogramDirectoryName + "resV_" + buff, true);
  if (h != NULL) {
    m_hResV.Clear();
    h->Copy(m_hResV);
    m_hResV.SetName((std::string("ResV_") + buff).c_str());
    m_hResV.SetTitle((std::string("v residuals ") + buff).c_str());
    m_cResV[buff]->Clear();
    m_cResV[buff]->cd();
    m_hResV.Draw();
    colorizeCanvas(m_cResV[buff], status);
  }
}
