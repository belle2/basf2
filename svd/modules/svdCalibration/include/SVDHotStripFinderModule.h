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

#include <svd/calibration/SVDOccupancyCalibrations.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>
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
    float m_thr;
    int m_base;
    bool m_useHSFinderV1 = true;
    float m_absThr; /**< Absolute Occupancy Threshold cut for Hot strip finder*/
    float m_relOccPrec; /**< Relative precision on occupancy which is defined to be negligible for the hit background rate estimate. */
    bool m_verbose; /**< False by default, it allows to switch on the printing of all found HS.*/

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

    //dbobject related parameters
    float m_zs; /**< zero suppression cut for the input shaper digits */
    int m_firstExp; /**< first valid experiment */
    int m_firstRun; /**< first valid run */
    int m_lastExp; /**< last valid experiment */
    int m_lastRun; /**<last valid run */

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
    SVDHistograms<TH1F>* hm_occAll = nullptr; /**< occupancy distribution - all strips*/
    SVDHistograms<TH1F>* hm_occHot = nullptr; /**< occupancy distribution - hot strips*/
    SVDHistograms<TH1F>* hm_occAfter = nullptr; /**< occupancy distribution - not hot strips*/

    SVDHistograms<TH1F>* hm_dist = nullptr;
    SVDHistograms<TH1F>* hm_dist1 = nullptr;
    SVDHistograms<TH2F>* hm_dist12 = nullptr;

    SVDSummaryPlots* m_hHotStripsSummary = nullptr; /**< hot strip summary  histo */

    TH1F*  h_tot_dqm;   /* number of hot strips per sensor */
    TH1F*  h_tot_dqm1;  /* number of hot strips per sensor  for layer 3*/
    TH1F*  h_tot_dist;  /* relative occupancy histogram */
    TH1F*  h_tot_dist1; /* absolute occupany histogram */
    TH2F*  h_tot_dist12; /* 2d distributiuons of occupancies */
    TH1F*  h_nevents;  /* number of events counting */


    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = nullptr);  /**< thf */
    //HSFinder algorithm (L&G)

    bool theHSFinder(double* stripOccAfterAbsCut, int* hsflag, int nstrips);

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = nullptr);  /**< thf */

  protected:
    //definition of input parameters

  };
}
#endif
