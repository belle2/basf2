/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDDQMExpressRecoMODULE_H_
#define VXDDQMExpressRecoMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class VXDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    VXDDQMExpressRecoModule();
    /* Destructor */
    virtual ~VXDDQMExpressRecoModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:

    /** flag <0,1> for using digits only, no clusters will be required, default = 0 */
    int m_UseDigits = 0;
    /** flag <0,1> very special case for swap of u-v coordinates */
    int m_SwapPXD = 0;
    /** set granulation of histogram plots, default is 1 deg (1 mm), min = 0.02, max = 5.0 */
    float m_CorrelationGranulation = 1.0;
    /** flag <0,1> for using for testbeam (paralel particles in x direction), default = 0 */
    int m_IsTB = 0;

    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** SVDDigits StoreArray name */
    std::string m_storeSVDDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;
    /** SVDClusters StoreArray name */
    std::string m_storeSVDClustersName;
    /** PXDClustersToPXDDigits RelationArray name */
    std::string m_relPXDClusterDigitName;
    /** SVDClustersToSVDDigits RelationArray name */
    std::string m_relSVDClusterDigitName;

    /** Number of VXD layers on Belle II */
    int c_nVXDLayers;
    /** Number of PXD layers on Belle II */
    int c_nPXDLayers;
    /** Number of SVD layers on Belle II */
    int c_nSVDLayers;
    /** First VXD layer on Belle II */
    int c_firstVXDLayer;
    /** Last VXD layer on Belle II */
    int c_lastVXDLayer;
    /** First PXD layer on Belle II */
    int c_firstPXDLayer;
    /** Last PXD layer on Belle II */
    int c_lastPXDLayer;
    /** First SVD layer on Belle II */
    int c_firstSVDLayer;
    /** Last SVD layer on Belle II */
    int c_lastSVDLayer;
    /** Maximum No of PXD ladders on layer */
    unsigned int c_MaxLaddersInPXDLayer;
    /** Maximum No of SVD ladders on layer */
    unsigned int c_MaxLaddersInSVDLayer;
    /** Maximum No of PXD sensors on layer */
    unsigned int c_MaxSensorsInPXDLayer;
    /** Maximum No of SVD sensors on layer */
    unsigned int c_MaxSensorsInSVDLayer;

    /** Cut threshold of PXD signal for accepting to correlations, default = 0 ADU */
    float m_CutCorrelationSigPXD = 0;
    /** Cut threshold of SVD signal for accepting to correlations in u, default = 0 ADU */
    float m_CutCorrelationSigUSVD = 0;
    /** Cut threshold of SVD signal for accepting to correlations in v, default = 0 ADU */
    float m_CutCorrelationSigVSVD = 0;
    /** Cut threshold of SVD time window for accepting to correlations, default = 70 ns */
    float m_CutCorrelationTimeSVD = 70;

    /** Basic Directory in output file */
    TDirectory* m_oldDir;

    /** Name of file contain reference histograms, default=VXD-ReferenceHistos */
    std::string m_RefHistFileName = "vxd/data/VXD-DQMReferenceHistos.root";
    /** Number of events */
    int m_NoOfEvents;
    /** Number of events in reference histogram */
    int m_NoOfEventsRef;

    /** Using local files instead of DataBase for reference histogram, default=0 */
    int m_NotUseDB = 0;
    /** Create and fill reference histograms in DataBase, default=0 */
    int m_CreateDB = 0;

    /** Flags of correlations and hit maps from space points */
    TH1I* m_correlationsSPFlag;
    /** Flags of correlations and hit maps from space points - differencies in Phi*/
    TH1I* m_correlationsSP1DPhiFlag;
    /** Flags of correlations and hit maps from space points - differencies in Theta*/
    TH1I* m_correlationsSP1DThetaFlag;

    /** Correlations and hit maps from space points */
    TH2F** m_correlationsSP;
    /** Correlations and hit maps from space points - differencies in Phi*/
    TH1F** m_correlationsSP1DPhi;
    /** Correlations and hit maps from space points - differencies in Theta*/
    TH1F** m_correlationsSP1DTheta;


    /** Function return index of layer in plots.
       * @param Layer Layer position.
       * @return Index of layer in plots.
       */
    int getLayerIndex(int Layer);
    /** Function return index of layer in plots.
       * @param Index Index of layer in plots.
       * @param Layer return layer position.
       */
    void getLayerIDsFromLayerIndex(int Index, int* Layer);
    /** Function return index of sensor in plots.
       * @param Layer Layer position of sensor.
       * @param Ladder Ladder position of sensor.
       * @param Sensor Sensor position of sensor.
       * @return Index of sensor in plots.
       */
    int getSensorIndex(int Layer, int Ladder, int Sensor);
    /** Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor.
       * @param Ladder return Ladder position of sensor.
       * @param Sensor return Sensor position of sensor.
       */
    void getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor);
    /** Function return flag histogram filled based on condition from TH1F source.
       * Flag values:
       * -3: nonexisting Type
       * -2: histogram is missing or masked
       * -1: less than 100 samles, skip comparition
       *  0: good much with reference
       *  1: warning level = diff > 6 * sigma and < error level
       *  2: error level = diff > 10 * sigma
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flaghist Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist);
    /** Function return flag histogram filled based on condition from TH1I source.
       * Flag values:
       * -3: nonexisting Type
       * -2: histogram is missing or masked
       * -1: less than 100 samles, skip comparition
       *  0: good much with reference
       *  1: warning level = diff > 6 * sigma and < error level
       *  2: error level = diff > 10 * sigma
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flaghist Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist);
    /** Function return flag histogram filled based on condition from TH2F source.
       * Flag values:
       * -3: nonexisting Type
       * -2: histogram is missing or masked
       * -1: less than 100 samles, skip comparition
       *  0: good much with reference
       *  1: warning level = diff > 6 * sigma and < error level
       *  2: error level = diff > 10 * sigma
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flaghist Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH2F* hist, TH2F* refhist, TH1I* flaghist);

    /** Function for filling of group of TH1F histogram to database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to glue to one.
       */
    void CreateDBHistoGroup(TH1F** HistoBD, int Number);
    /** Function for filling of group of TH2F histogram to database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to glue to one.
       */
    void CreateDBHistoGroup(TH2F** HistoDB, int Number);

    /** Function for loading of group of TH1F histogram from database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to extract from DB.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int LoadDBHistoGroup(TH1F** HistoBD, int Number);
    /** Function for loading of group of TH2F histogram from database.
       * @param HistoBD Histogram for DB.
       * @param Number Number of histograms to extract from DB.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int LoadDBHistoGroup(TH2F** HistoBD, int Number);

  };

}
#endif

