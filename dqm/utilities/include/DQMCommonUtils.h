/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys, Peter Kvasnicka                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "TH1F.h"
#include "TVectorT.h"
#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>
#include <set>
#include <memory>
#include <algorithm>
#include <numeric>

namespace Belle2 {

  /** The class collects utility functions for creating DQM histograms:
    * prepared for PXD, SVD, VXD and Track DQM histograms.
    */
  class DQMCommonUtils {

  public:

    /*********************************************************************
     * Methods constructing histograms
     *********************************************************************/

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
    static int SetFlag(int Type, int bin, const double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist);
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
    static int SetFlag(int Type, int bin, const double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist);

    /** Function for filling of TH1F histogram to database.
      * @param HistoBD Histogram for DB.
      */
    static void CreateDBHisto(TH1F* HistoBD);
    /** Function for filling of TH1I histogram to database.
      * @param HistoBD Histogram for DB.
      */
    static void CreateDBHisto(TH1I* HistoBD);

    /** Function for filling of group of TH1F histogram to database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to glue to one.
      */
    static void CreateDBHistoGroup(TH1F** HistoBD, int number);
    /** Function for filling of group of TH1I histogram to database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to glue to one.
      */
    static void CreateDBHistoGroup(TH1I** HistoBD, int number);

    /** Function for loading of TH1F histogram from database.
      * @param HistoBD Histogram for DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    static int LoadDBHisto(TH1F* HistoBD);
    /** Function for loading of TH1I histogram from database.
      * @param HistoBD Histogram for DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    static int LoadDBHisto(TH1I* HistoBD);

    /** Function for loading of group of TH1F histogram from database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to extract from DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    static int LoadDBHistoGroup(TH1F** HistoBD, int number);
    /** Function for loading of group of TH1I histogram from database.
      * @param HistoBD Histogram for DB.
      * @param Number Number of histograms to extract from DB.
      * @return Indication of succes of realizing of condition, 1: OK.
      */
    static int LoadDBHistoGroup(TH1I** HistoBD, int number);

  };
}
