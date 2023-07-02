/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>
#include <map>

#include <svd/dataobjects/SVDHistograms.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <framework/dataobjects/EventT0.h>

#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>


namespace Belle2 {

  /**
   * Collection of histogram to be used in the calibration process
   */
  class SVDTimeCalibrationCollectorContainer : public TNamed {
  public:
    /** Default Constructor */
    SVDTimeCalibrationCollectorContainer()
      : // m_TH1F({})
        // , m_TH2F({})
        // ,
        m_name("name") {}

    /** Setter for Name */
    void SetName(const char* name) {m_name = name;}
    /** Getter for Name */
    const char* GetName() const {return m_name.Data();}
    /** Setter for directory  */
    void SetDirectory(TDirectory* dir)
    {
      if (fDirectory == dir) return;
      if (fDirectory) fDirectory->Remove(this);
      fDirectory = dir;
      if (fDirectory) {
        // fFunctions->UseRWLock();
        fDirectory->Append(this);
      }
    }
    void Reset()
    {
      m_TH1F.clear();
      m_TH2F.clear();
      m_name = "name";
      // for (auto hist : m_TH1F)
      //  hist.second->Reset();
      // for (auto hist : m_TH2F)
      //  hist.second->Reset();
    }

    void SetTH1FHistogram(TString name, TString title, int bins, double xmin, double xmax)
    {
      m_TH1F[name] = new TH1F(name, title, bins, xmin, xmax);
    }
    void SetTH1FHistogram(TH1F* hist)
    {
      hist->SetDirectory(0);
      m_TH1F[TString(hist->GetName())] = hist;
    }
    void SetTH2FHistogram(TH2F* hist)
    {
      hist->SetDirectory(0);
      m_TH2F[TString(hist->GetName())] = hist;
    }

    /** All Histograms */
    std::map<TString, TH1F*> m_TH1F; /**< 1D Histograms */
    std::map<TString, TH2F*> m_TH2F; /**< 2D Histograms */

  private:
    TString    m_name; /**< Name of the object */
    TDirectory* fDirectory; /**< Pointer to directory holding this histogram */
    // TList      *fFunctions; ///<->Pointer to list of functions (fits and user)
  };

  /**
   * Collector module used to create the histograms needed for the
   * SVD CoG-Time calibration
   */
  class SVDTimeCalibrationCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    SVDTimeCalibrationCollectorModule();

    /**
     * Initialize the module
     */
    void prepare() override final;

    /**
     * Called when entering a new run
     */
    void startRun() override final;

    /**
     * Event processor
     */
    void collect() override final;

  private:

    /**EventMetaData */
    StoreObjPtr<EventMetaData> m_emdata; /**< EventMetaData store object pointer*/

    /**SVDEventInfo */
    std::string m_svdEventInfo = "SVDEventInfo"; /**< Name of the SVDEventInfo store array used as parameter of the module*/
    StoreObjPtr<SVDEventInfo> m_svdEI; /**< SVDEventInfo store object pointer*/

    DBObjPtr<HardwareClockSettings> m_hwClock;  /**< systems clock*/

    /**SVDCluster */
    std::string m_svdClusters = "SVDClustersFromTracks"; /**< Name of the SVDClusters store array used as parameter of the module*/
    StoreArray<SVDCluster> m_svdCls; /**< SVDClusters store array*/

    /**EventT0 */
    std::string m_eventTime = "EventT0"; /**< Name of the EventT0 store object pointer used as parameter of the module*/
    StoreObjPtr<EventT0> m_eventT0; /**< EventT0 store object pointer*/

    /**SVDHistograms */
    SVDHistograms<TH2F>* m_hEventT0vsCoG = nullptr; /**< Scatter plot t0 vs t_raw (CoG)*/
    SVDHistograms<TH1F>* m_hEventT0 = nullptr; /**< EventT0 synchronized distribution*/
    SVDHistograms<TH1F>* m_hEventT0nosync = nullptr; /**< EventT0 NOT synchroinized distribution*/

    double m_rawCoGBinWidth = 2.; /**< Raw_CoG Bin Width [ns] for 2D-histogram */
    double m_minRawTimeForIoV = 0.; /**< Minimum value of the raw time distribution used to determine whether change IoV or not */
    double m_maxRawTimeForIoV = 150.; /**< Maxmum value of the raw time distribution used to determine whether change IoV or not */

    SVDTimeCalibrationCollectorContainer* m_containerObject; /**< Container to store all the histograms */
  };

} // end namespace Belle2
