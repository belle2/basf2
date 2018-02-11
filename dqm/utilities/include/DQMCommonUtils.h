/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "TH1F.h"
#include "TVectorT.h"


namespace Belle2 {

  /** Class collected functions usin for comparing of histograms on DQM:
    * usinf shape comparing, bin comparing, MPV, mean or RMS,
    * prepared for PXD, SVD, VXD and Track DQM histograms.
    *
    */
  class DQMCommonUtils {

  public:

    /** Function return index of sensor in plots.
      * Use for only PXD sensor counting, without SVD.
      * @param Layer Layer position of sensor
      * @param Ladder Ladder position of sensor
      * @param Sensor Sensor position of sensor
      * @return Index of sensor in plots.
      */
    static int getPXDSensorIndex(const int Layer, const int Ladder, const int Sensor);
    /** Function return index of sensor in plots.
      * Use for only PXD sensor counting, without SVD.
      * @param Index Index of sensor in plots.
      * @param Layer return Layer position of sensor
      * @param Ladder return Ladder position of sensor
      * @param Sensor return Sensor position of sensor
      */
    static void getIDsFromPXDIndex(const int Index, int& Layer, int& Ladder, int& Sensor);
    /** Function return index of chip in plots.
      * Use for only PXD sensor counting, without SVD.
      * @param Layer Layer position of sensor
      * @param Ladder Ladder position of sensor
      * @param Sensor Sensor position of sensor
      * @param Chip Chip position on sensor - DCDs or Switchers
      * @param IsU Info if Chip is on u direction (DCD)
      * @return Index of sensor in plots.
      */
    int getPXDChipIndex(const int Layer, const int Ladder, const int Sensor, const int Chip, const int IsU) const;
    /** Function return position indexes of chipID in plots.
      * Use for only PXD sensor counting, without SVD.
      * @param Index Index of sensor in plots.
      * @param Layer return Layer position of sensor
      * @param Ladder return Ladder position of sensor
      * @param Sensor return Sensor position of sensor
      * @param Chip return Chip position on sensor - DCDs or Switchers
      * @param IsU return info if Chip is on u direction (DCD)
      */
    void getIDsFromPXDChipIndex(const int Index, int& Layer, int& Ladder, int& Sensor, int& Chip, int& IsU) const;

    /** Function return index of SVD sensor in plots.
      * Use for only SVD sensor counting, without PXD.
      * @param Layer Layer position of sensor
      * @param Ladder Ladder position of sensor
      * @param Sensor Sensor position of sensor
      * @return Index of sensor in plots.
      */
    static int getSVDSensorIndex(const int Layer, const int Ladder, const int Sensor);
    /** Function return SVD index of sensor in plots.
      * Use for only SVD sensor counting, without PXD.
      * @param Index Index of sensor in plots.
      * @param Layer return Layer position of sensor
      * @param Ladder return Ladder position of sensor
      * @param Sensor return Sensor position of sensor
      */
    static void getIDsFromSVDIndex(const int Index, int& Layer, int& Ladder, int& Sensor);
    /** Function return index of chip in plots.
      * Use for only SVD sensor counting, without PXD.
      * @param Layer Layer position of sensor
      * @param Ladder Ladder position of sensor
      * @param Sensor Sensor position of sensor
      * @param Chip Chip position on sensor
      * @return Index of sensor in plots.
      */
    int getSVDChipIndex(const int Layer, const int Ladder, const int Sensor, const int Chip, const int IsU) const;
    /** Function return position indexes of chipID in plots.
      * Use for only SVD sensor counting, without PXD.
      * @param Index Index of sensor in plots.
      * @param Layer return Layer position of sensor
      * @param Ladder return Ladder position of sensor
      * @param Sensor return Sensor position of sensor
      * @param Chip return Chip position on sensor
      */
    void getIDsFromSVDChipIndex(const int Index, int& Layer, int& Ladder, int& Sensor, int& Chip, int& IsU) const;


    /** Function return index of layer in plots.
      * Use for only PXD+SVD sensor counting.
      * @param Layer Layer position.
      * @return Index of layer in plots.
      */
    static int getVXDLayerIndex(const int Layer);
    /** Function return index of layer in plots.
      * Use for only PXD+SVD sensor counting.
      * @param Index Index of layer in plots.
      * @param Layer return layer position.
      */
    static void getLayerIDsFromVXDLayerIndex(const int Index, int& Layer);
    /** Function return index of sensor in plots.
      * Use for only PXD+SVD sensor counting.
      * @param Layer Layer position of sensor.
      * @param Ladder Ladder position of sensor.
      * @param Sensor Sensor position of sensor.
      * @return Index of sensor in plots.
      */
    static int getVXDSensorIndex(const int Layer, const int Ladder, const int Sensor);
    /** Function return index of sensor in plots.
      * Use for only PXD+SVD sensor counting.
      * @param Index Index of sensor in plots.
      * @param Layer return Layer position of sensor.
      * @param Ladder return Ladder position of sensor.
      * @param Sensor return Sensor position of sensor.
      */
    static void getIDsFromVXDIndex(const int Index, int& Layer, int& Ladder, int& Sensor);

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
      * @param flag Histogram of flags.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int SetFlag(const int Type, const int bin, double* pars, const double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist) const;
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
      * @param flag Histogram of flags.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int SetFlag2(const int Type, const int bin, double* pars, const double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist) const;
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
        * @param flag Histogram of flags.
        * @return Indication of succes of realizing of condition, 1: OK.
        */
    // int SetFlag(const int Type, const int bin, double* pars, const double ratio, std::string name_hist, std::string name_refhist, TH1I* flaghist) const;
    //TH1* findHistLocal(TString& a);
    // TH1* GetHisto(TString histoname) const;

    /** Function for filling of TH1F histogram to database.
      * @param HistoBD Histogram for DB.
      */
    void CreateDBHisto(TH1F* HistoBD) const;
    /** Function for filling of TH1I histogram to database.
      * @param HistoBD Histogram for DB.
      */
    void CreateDBHisto(TH1I* HistoBD) const;

    /** Function for filling of group of TH1F histogram to database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to glue to one.
      */
    void CreateDBHistoGroup(TH1F** HistoBD, const int Number) const;
    /** Function for filling of group of TH1I histogram to database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to glue to one.
      */
    void CreateDBHistoGroup(TH1I** HistoBD, const int Number) const;

    /** Function for loading of TH1F histogram from database.
      * @param HistoBD Histogram for DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHisto(TH1F* HistoBD) const;
    /** Function for loading of TH1I histogram from database.
      * @param HistoBD Histogram for DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHisto(TH1I* HistoBD) const;

    /** Function for loading of group of TH1F histogram from database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to extract from DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHistoGroup(TH1F** HistoBD, const int Number) const;
    /** Function for loading of group of TH1I histogram from database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to extract from DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    int LoadDBHistoGroup(TH1I** HistoBD, const int Number) const;


  private:

    /** Number of PXD chips per sensor in u (DCD) (=4) on Belle II */
    const int c_nPXDChipsU = 4;
    /** Number of PXD chips per sensor in v (Switchers) (=6) on Belle II */
    const int c_nPXDChipsV = 6;
    /** Number of SVD chips per sensor in u,v in layer 3 (=6) on Belle II */
    const int c_nSVDChipsL3 = 6;
    /** Number of SVD chips per sensor in u in layers 4,5,6 (=6) on Belle II */
    const int c_nSVDChipsLu = 6;
    /** Number of SVD chips per sensor in v in layers 4,5,6 (=4) on Belle II */
    const int c_nSVDChipsLv = 4;
    /** Number of SVD strips per chip on Belle II */
    const int c_nSVDChannelsPerChip = 128;

  };

}
