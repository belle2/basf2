/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      c_ProductionDebug = 0x0401, /**< Production Debugging Data Format*/
      c_IRS3B = 0xFE01, /**< waveforms based on Kurtis' packets */
      c_GigE  = 0xFE02, /**< gigE waveforms */
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
                                                     RawDataType::c_ProductionDebug,
                                                     RawDataType::c_IRS3B,
                                                     RawDataType::c_GigE,
                                                     RawDataType::c_Draft
                                                    };

  }
}
