/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  namespace PXD {
    class PXDMappingLookup {

    public:

      /** Maps row/col of inner forward (IF) and outer backward (OB) modules of the PXD to U/V cell */
      static void map_rc_to_uv_IF_OB(unsigned int& row_u, unsigned int& col_v, unsigned int dhp_id, unsigned int dhe_ID);

      /** Maps row/cols of inner backward (IB) and outer forward (OF) modules of the PXD to U/V cell */
      static void map_rc_to_uv_IB_OF(unsigned int& row_u, unsigned int& col_v, unsigned int dhp_id, unsigned int dhe_ID);

      /** Maps  U/V cell of inner forward (IF) and outer backward (OB) modules of the PXD to row/col/dhpid */
      static void map_uv_to_rc_IF_OB(unsigned int& row_u, unsigned int& col_v, unsigned int& dhp_id, unsigned int dhe_ID);

      /** Maps U/V cell of inner backward (IB) and outer forward (OF) modules of the PXD to row/col/dhpid */
      static void map_uv_to_rc_IB_OF(unsigned int& row_u, unsigned int& col_v, unsigned int& dhp_id, unsigned int dhe_ID);

      /** write out LUT to file for cross-check */
      static void write_mapping_to_file(void);

      /** write out inverse LUT to file for cross-check */
      static void write_inversemapping_to_file(void);

      /** consistency check */
      static void check(void);
    };
  }
}

