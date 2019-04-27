#ifndef SVDHOTSTRIPFINDERMODULE_H
#define SVDHOTSTRIPFINDERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <vxd/dataobjects/VxdID.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>
#include <TCollection.h>
#include <TList.h>
#include <TH3F.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {
  /** A module template.
   *
   *  A detailed description of your module.
   */
  class SVDHotStripFinderModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    SVDHotStripFinderModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~SVDHotStripFinderModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, StoreObjPtrs, relations) here,
     *  see the respective class documentation for details.
     */
    virtual void initialize() override;

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun() override;

    /** Clean up anything you created in initialize(). */
    virtual void terminate() override;


    /* user-defined parameters */
    StoreArray<SVDShaperDigit> m_storeDigits; /**< shaper digits store array*/
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data store array*/
    std::string m_rootFileName;   /**< root file name */
    std::string m_ShaperDigitName; /**< shaper digits name*/
    float m_thr; /**< Threshold cut for Hot strip finder*/
    int m_base;



    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */


  private:
    //define your own data members here



    static const int m_nLayers = 4;// to avoid empty layers we start from 0
    static const int m_nLadders = 16;
    static const int m_nSensors = 5;
    static const int m_nSides = 2;

    const int nBins = 1500;

    unsigned int sensorsOnLayer[4];


    TList* m_histoList_occu;/**< occupancy for low charge clusters */

    SVDHistograms<TH1F>* hm_occupancy = nullptr; /**< strip occupancy per sensor*/
    SVDHistograms<TH1F>* hm_hot_strips = nullptr; /**< hot strips per sensor*/
    SVDHistograms<TH1F>* hm_occupancy_after = nullptr; /**< strip occupancy after removal of hot strips, per sensor*/
    SVDHistograms<TH1F>* hm_dist = nullptr;
    SVDHistograms<TH1F>* hm_dist1 = nullptr;
    SVDHistograms<TH2F>* hm_dist12 = nullptr;

    SVDSummaryPlots* m_hHotStripsSummary = nullptr; /**< hot strip summary  histo */

    TH1F*  h_tot_dqm;
    TH1F*  h_tot_dqm1;
    TH1F*  h_tot_dist;
    TH1F*  h_tot_dist1;
    TH2F*  h_tot_dist12;
    TH1F*  h_nevents;


    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = nullptr);  /**< thf */

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = nullptr);  /**< thf */


  protected:
    //definition of input parameters

  };
}
#endif
