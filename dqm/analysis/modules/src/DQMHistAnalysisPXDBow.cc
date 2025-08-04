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

#include <TString.h>
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
           "Layer_ladder_sensor numbers of the module which residual histogram will be plotted on the DQM",
           std::string("2.2.1"));
  addParam("statThreshold", m_statThreshold,
           "minimum number of entries needed to calculate the mean in the delta distribution of the sagitta",
           m_statThreshold);
  addParam("sagittaThreshold", m_sagittaThreshold,
           "threshold for the warning related to the sagitta",
           m_sagittaThreshold);
  addParam("roiThreshold", m_roiThreshold,
           "threshold for the warning related to the roi",
           m_roiThreshold);
  B2DEBUG(20, "DQMHistAnalysisPXDBow: Constructor done.");
}


void DQMHistAnalysisPXDBowModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisPXDBow: initialized.");

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  int valideModule = 0;
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD || aVxdID.getSensorNumber() != 1) continue;
    m_PXDModules.push_back(aVxdID); // reorder

    if (VxdID(m_moduleName) == aVxdID) valideModule++;

    // Epics for all the forward PXD module
    auto buff = (std::string)aVxdID;
    replace(buff.begin(), buff.end(), '.', '_');
    registerEpicsPV("PXD:meanResV:" + buff, "meanResV:" + (std::string)aVxdID);
    registerEpicsPV("PXD:sigmaResV:" + buff, "sigmaResV:" + (std::string)aVxdID);
    registerEpicsPV("PXD:sagitta:" + buff, "sagitta:" + (std::string)aVxdID);
  }

  if (valideModule == 0) {
    B2WARNING("Invalid moduleName, only PXD forward module are acceptable, nameModule parameter set to default (2.2.1)");
    m_moduleName = "2.2.1";
  }

  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  if (m_PXDModules.size() == 0) {
    B2ERROR("No PXDModules in Geometry found!");
  }

  m_cResV = new TCanvas("c_resV");

}

void DQMHistAnalysisPXDBowModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisPXDBow : beginRun called");

  m_cResV->Clear();

  /// get RoI size in the v coordinate
  DBObjPtr<ROICalculationParameters> roiParams;
  if (!roiParams.isValid()) B2WARNING("Cannot get roi parameters, ROI v half size defined as default (0.1 cm)");
  m_roiThreshold = roiParams->getSigmaSystV() * roiParams->getNumSigmaTotV() / 2;
}

void DQMHistAnalysisPXDBowModule::event()
{
  for (VxdID& aPXDModule : m_PXDModules) {
    bool enough = false, warnflag = false, errorflag = false;
    auto buff = (std::string)m_moduleName;
    std::replace(buff.begin(), buff.end(), '.', '_');

    TH1* hV = getDelta(m_histogramDirectoryName + "resV_" + buff, true);
    TH1* hS = getDelta(m_histogramDirectoryName + "sagitta_" + buff, true);
    if (hS != NULL && hV != NULL) {
      B2DEBUG(20, "Histos resS_" << buff << " and resV_" << buff << " found");
      if (hS->GetEntries() > m_statThreshold) {
        enough = true;

        // Epics PVs
        double meanResV = hV->GetMean();
        double stdResV = hV->GetStdDev();
        double resV = TMath::Abs(meanResV) + 3 * stdResV;
        double bowAmplitude = hS->GetMean();

        VxdID vxdid(m_moduleName);
        if (TMath::Abs(bowAmplitude) > 0.1) errorflag = true; /// error
        else if (resV > m_roiThreshold
                 || TMath::Abs(bowAmplitude) > m_sagittaThreshold) warnflag = true;/// warning

        setEpicsPV("meanResV:" + buff, meanResV);
        setEpicsPV("stdResV:" + buff, stdResV);
        setEpicsPV("sagitta:" + buff, bowAmplitude);
      } else
        enough = false;

      if (aPXDModule == VxdID(m_moduleName)) {
        EStatus status = makeStatus(enough, errorflag, warnflag);
        TH1* h = findHist(m_histogramDirectoryName + "resV_" + buff, true);
        if (h != NULL) {
          m_hResV.Clear();
          h->Copy(m_hResV);
          m_hResV.SetName("ResV");
          m_hResV.SetTitle("v residuals");
          m_cResV->Clear();
          m_cResV->cd();
          m_hResV.Draw();
          colorizeCanvas(m_cResV, status);
        }
      }
    } else B2DEBUG(20, "Histo resV_" << buff << " not found");
  }
}

void DQMHistAnalysisPXDBowModule::endRun()
{
  // if this function is not needed, please remove
  B2DEBUG(20, "DQMHistAnalysisPXDBow : endRun called");
}


void DQMHistAnalysisPXDBowModule::terminate()
{
  // if this function is not needed, please remove
  B2DEBUG(20, "terminate called");
}

