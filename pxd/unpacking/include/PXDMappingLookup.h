/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck, Peter Kodys                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {

  namespace PXD {
    /** Class to make the mapping between u/v cell ID of pixels back to
     * DCD drain lines, pixel row/col, DCD and Switcher IDs
     * Details:
     * Belle Note: BELLE2-NOTE-TE-2015-01  "The vertex detector numbering scheme"
     * https://docs.belle2.org/record/243/files/Belle%20II%20note%200010.pdf
     * PXD WhiteBook 3.1.3 Sensor Design and Appendix #3
     * https://confluence.desy.de/display/BI/PXD+WebHome?preview=/34029260/56330158/PXDwb.pdf
     *
     */
    class PXDMappingLookup {

    public:

      /** Maps row/col of inner forward (IF) and outer backward (OB) modules of the PXD to U/V cell */
      static void map_rc_to_uv_IF_OB(unsigned int& row_u, unsigned int& col_v, const unsigned int dhp_id, const  unsigned int dhe_ID);

      /** Maps row/cols of inner backward (IB) and outer forward (OF) modules of the PXD to U/V cell */
      static void map_rc_to_uv_IB_OF(unsigned int& row_u, unsigned int& col_v, const  unsigned int dhp_id, const  unsigned int dhe_ID);

      /** Maps  U/V cell of inner forward (IF) and outer backward (OB) modules of the PXD to row/col/dhpid */
      static void map_uv_to_rc_IF_OB(unsigned int& row_u, unsigned int& col_v,  unsigned int& dhp_id, const  unsigned int dhe_ID);

      /** Maps U/V cell of inner backward (IB) and outer forward (OF) modules of the PXD to row/col/dhpid */
      static void map_uv_to_rc_IB_OF(unsigned int& row_u, unsigned int& col_v,  unsigned int& dhp_id, const  unsigned int dhe_ID);

      /** get ID of DCD for giving pixel, range: 1..4.
       * @param u pixels coordinate in u
       * @param v pixels coordinate in u
       * @param sensorID sensor details for PXD sensors
       * @returns DCD ID
       */
      static int getDCDID(const int u, const int v, const VxdID sensorID);

      /** get ID of SWB for giving pixel, range: 1..6.
       * @param v pixels coordinate in u
       * @returns SWB ID
       */
      static int getSWBID(const int v);

      /** write out LUT to file for cross-check */
      static void write_mapping_to_file(void);

      /** write out inverse LUT to file for cross-check */
      static void write_inversemapping_to_file(void);

      /** consistency check */
      static void check(void);
    };
  }
}

