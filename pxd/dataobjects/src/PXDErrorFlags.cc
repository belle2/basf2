/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 * Created: 23.10.2017                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "pxd/dataobjects/PXDErrorFlags.h"

const std::string& Belle2::PXD::PXDError::getPXDBitErrorName(int bit)
{
  static const std::string undefined("undefined");
  /** Description strings for bit, index = bit number, thus keep order as above */
  static const std::string error_name[ONSEN_MAX_TYPE_ERR] = {
    // 0-3
    "XXXXTestBeam: Unknown DHE IDs", "Frame TrigNr vs ONSEN Trigger Frame mismatch", "Frame TrigNr vs EvtMeta mismatch", "ONSEN Trigger is not first frame",
    // 4-7
    "DHC_END missing/wrong placed", "NR of Frames too small", "ROI packet invalid size", "DATA outside of DHE",
    // 8-11
    "DHC_START is not second frame", "DHE IDs have wrong order in DHC", "Fixed size frame wrong size", "DHE CRC Error:",
    // 12-15
    "Unknown DHC type", "XXXMerger CRC Error", "Event Header Full Packet Size Error", "Event Header Magic Error",
    // 16-19
    "Event Header Frame Count Error", "Event header Frame Size Error", "HLTROI Magic Error", "Merger HLT/DATCON TrigNr Mismatch",
    // 20-23
    "DHP Size too small", "DHP-DHE DHEID mismatch", "DHP-DHE Port mismatch", "DHP Pix w/o row",
    // 24-27
    "DHE START/END ID mismatch", "DHE ID mismatch of START and this frame", "DHE_START w/o prev END", "Nr PXD data ==0",
    // 28-31
    "Missing Datcon", "NO DHC data for Trigger", "DHE active mismatch", "DHP active mismatch",
    // 32-35
    "SendUnfiltered but Filtered Frame Type", "!SendUnfiltered but Unfiltered Frame Type", "DHP has double header", "Error bit in frame header set",
    // 36-39
    "Error bit in GHOST frame header not set", "Suspicious Padding/Checksum in DHP ZSP", "DHC Words in Event mismatch", "DHE Words in Event mismatch",
    // 40-43
    "Row Overflow/out of bounds >=768", "DHP Frames not continuesly", "DHP Frame numbers of different frames differ>1", "DHP Frame differ from DHE Frame by >1",
    // 44-47
    "DHE ID is invalid", "DHC ID Start/End Mismatch", "DHE End w/o Start", "double DHC End",
    // 48-51
    "DHC vs EvtMeta mismatch for TriggerNr", "DHE vs EvtMeta mismatch for TriggerNr", "DHP COL overflow (unconnected drain lines)", "Unexpected (=unsupported) Frame Type",
    // 52-55
    "DHC vs EvtMeta mismatch for Exp/Run/Subrun", "DHC vs EvtMeta mismatch for TimeTag", "ONS_TRG: HLT vs EvtMeta mismatch", "ONS_TRG: DATCON vs EvtMeta mismatch",
    // 56-59
    "TriggerGate differ for DHEs in event", "Trigger FrameNr differ for DHEs in event", "DHP row w/o Pix", "DHE_START is not third frame",
    // 60-63
    "unused", "unused", "unused", "unused"
  };
  if (bit < 0 || bit >= ONSEN_MAX_TYPE_ERR) return undefined;
  return error_name[bit];
}

const std::string& Belle2::PXD::PXDError::getPXDBitMaskErrorName(EPXDErrMask mask)
{
  static const std::string undefined("no bit or more than one bit set");
  for (int i = 0; i < ONSEN_MAX_TYPE_ERR; i++) {
    if ((mask & (1 << i)) == mask) return getPXDBitErrorName(i);
  }
  return undefined;
}
