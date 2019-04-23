#ifndef SVDFHSMODULE_H
#define SVDFHSMODULE_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

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
  class SVDfhsModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    SVDfhsModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~SVDfhsModule();

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
    std::string m_rootFileName;   /**< root file name */
    std::string m_ShaperDigitName; /**< */
    std::string m_RecoDigitName; /**< */
    std::string m_ClusterName; /**< */
    std::string m_TrackFitResultName; /**< */
    std::string m_TrackName; /**< */
    bool m_is2017TBanalysis; /**< true if we analyze 2017 TB data*/
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


    //Hot strips search
    TH1F*  h_occupancy[m_nLayers][m_nLadders][m_nSensors][m_nSides]; /** hot strips search**/
    TH1F*  h_occupancy_after[m_nLayers][m_nLadders][m_nSensors][m_nSides]; /** hot strips search**/
    TH1F*  h_dist[m_nLayers][m_nLadders][m_nSensors][m_nSides]; /** hot strips search**/
    TH1F*  h_tot_dqm;
    TH1F*  h_tot_dqm1;
    TH1F*  h_tot_dist;
    TH1F*  h_dist1[m_nLayers][m_nLadders][m_nSensors][m_nSides]; /** hot strips search**/
    TH1F*  h_tot_dist1;
    TH2F*  h_dist12[m_nLayers][m_nLadders][m_nSensors][m_nSides]; /** hot strips search**/
    TH2F*  h_tot_dist12;
    TH1F*  h_nevents;





    int getSensor(int layer, int sensor, bool isTB)
    {
      int result = 0;
      if (isTB) {
        if (layer == 0)
          result = sensor - 1;
        else if (layer == 1 || layer == 2)
          result = sensor - 2;
        else if (layer == 3)
          result = sensor - 3;
      } else result = sensor - 1;
      // test
      result = sensor - 1;
      //

      return result;
    }


    //list of functions to create histograms:
    TH1F* createHistogram1D(const char* name, const char* title,
                            Int_t nbins, Double_t min, Double_t max,
                            const char* xtitle, TList* histoList = NULL);  /**< thf */

    TH2F* createHistogram2D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            TList* histoList = NULL);  /**< thf */

    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t minX, Double_t maxX, const char* titleX,
                            Int_t nbinsY, Double_t minY, Double_t maxY, const char* titleY,
                            Int_t nbinsZ, Double_t minZ, Double_t maxZ, const char* titleZ,
                            TList* histoList = NULL);  /**< thf */


    TH3F* createHistogram3D(const char* name, const char* title,
                            Int_t nbinsX, Double_t* binsX, const char* titleX,
                            Int_t nbinsY, Double_t* binsY, const char* titleY,
                            Int_t nbinsZ, Double_t* binsZ, const char* titleZ,
                            TList* histoList = NULL);  /**< thf */

    TH1* duplicateHistogram(const char* newname, const char* newtitle,
                            TH1* h, TList* histoList = NULL);  /**< thf */


    TH1F* createHistogramsRatio(const char* name, const char* title,
                                TH1* hNum, TH1* hDen, bool isEffPlot,
                                int axisRef);  /**< thf */



    void addEfficiencyPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL, TH3F* h3_MCParticle = NULL);  /**< \
                                  efficiency */
    void addInefficiencyPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL,
                              TH3F* h3_MCParticle = NULL);  /**< inefficiency */
    void addPurityPlots(TList* graphList = NULL, TH3F* h3_xPerMCParticle = NULL, TH3F* h3_MCParticle = NULL);  /**< puri\
                              ty */






  protected:
    //definition of input parameters

  };
}
#endif
