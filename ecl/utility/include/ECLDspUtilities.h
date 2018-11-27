/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

namespace Belle2 {

  class ECLDspData;

  namespace ECL {
    /**
     * @brief Convert ECLDspData from *.dat file to Root object.
     *
     * @param[in] raw_file Path to dsp??.dat file.
     * @return ECLDspData object
     */
    ECLDspData* readEclDsp(const char* raw_file, int boardNumber);

    /**
     * @brief Convert ECLDspData from Root object to *.dat file.
     *
     * @param[in] raw_file Path to dsp??.dat file to be created.
     * @param[in] obj      Object to be written
     */
    void writeEclDsp(const char* raw_file, ECLDspData* obj);
  }
}

