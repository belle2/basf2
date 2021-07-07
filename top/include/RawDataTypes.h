/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace TOP {

    /**
     * Scoped enum for TOP raw data types.
     * Note: some enumerators will gone in future
     */
    enum class RawDataType {
      c_Type0Ver1 = 0x0001, /**< Feature-extracted data, interim */
      c_Type1Ver1 = 0x0101, /**< Passthru, interim */
      c_Type2Ver1 = 0x0201, /**< Feature-extracted + raw waveforms, interim */
      c_Type3Ver1 = 0x0301, /**< Feature-extracted + ped-subtracted waveforms, interim */
      c_Type0Ver16 = 0x0010, /**< Feature-extracted data */
      c_Type1Ver16 = 0x0110, /**< Passthru */
      c_Type2Ver16 = 0x0210, /**< Feature-extracted + raw waveforms */
      c_Type3Ver16 = 0x0310, /**< Feature-extracted + ped-subtracted waveforms */
      c_ProductionDebug01 = 0x0401, /**< Production Debugging Data Format 01*/
      c_ProductionDebug02 = 0x0402, /**< Production Debugging Data Format 02 with extended heap information*/
      c_Draft = 0xFF00  /**< tentative production format */
    };

    /**
     * array containing all defined data types from above. needed to check whether a given datatype word exists or not.
     * there is a way of writing this so we don't have to manually keep the array up-to-date, but it involves large amounts of template magic
     */
    static const RawDataType membersRawDataType[] = {RawDataType::c_Type0Ver1,
                                                     RawDataType::c_Type0Ver1,
                                                     RawDataType::c_Type1Ver1,
                                                     RawDataType::c_Type2Ver1,
                                                     RawDataType::c_Type3Ver1,
                                                     RawDataType::c_Type0Ver16,
                                                     RawDataType::c_Type1Ver16,
                                                     RawDataType::c_Type2Ver16,
                                                     RawDataType::c_Type3Ver16,
                                                     RawDataType::c_ProductionDebug01,
                                                     RawDataType::c_ProductionDebug02,
                                                     RawDataType::c_Draft
                                                    };

  }
}
