/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Meggendorfer     fmegg@mpp.mpg.de                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>

using namespace Belle2;

void setconfig_nnt()
{
  // Creating payload object:
  DBImportObjPtr<CDCTriggerNeuroConfig> nc;
  nc.construct();
  //B2LinkFormat:
  //                  start,  end,    offset, name,                   description
  nc->add_B2FormatLine(0,      22,     0,      "ETF",                  "");
  nc->add_B2FormatLine(23,     43,     0,      "TSF8",                 "");
  nc->add_B2FormatLine(44,     64,     0,      "TSF6",                 "");
  nc->add_B2FormatLine(65,     85,     0,      "TSF4",                 "");
  nc->add_B2FormatLine(86,     106,    0,      "TSF2",                 "");
  nc->add_B2FormatLine(107,    127,    0,      "TSF0",                 "");
  nc->add_B2FormatLine(128,    134,    0,      "Phi",                  "");
  nc->add_B2FormatLine(135,    141,    0,      "Omega",                "");
  nc->add_B2FormatLine(142,    351,    0,      "TSF1",                 "");
  nc->add_B2FormatLine(352,    561,    0,      "TSF3",                 "");
  nc->add_B2FormatLine(562,    771,    0,      "TSF5",                 "");
  nc->add_B2FormatLine(772,    981,    0,      "TSF7",                 "");
  nc->add_B2FormatLine(982,    1170,   0,      "TSF Selected",         "");
  nc->add_B2FormatLine(1172,   1521,   0,      "MLP Input",            "");
  nc->add_B2FormatLine(1522,   1524,   0,      "NetSel",               "");
  nc->add_B2FormatLine(1525,   1537,   0,      "MLP Output 0",         "");
  nc->add_B2FormatLine(1538,   1550,   0,      "MLP Output 1",         "");
  nc->add_B2FormatLine(1551,   1551,   0,      "Enable NNT",           "");
  nc->add_B2FormatLine(1552,   1552,   0,      "active etf",           "");
  nc->add_B2FormatLine(1553,   1558,   0,      "active 2d",            "");
  nc->add_B2FormatLine(1559,   1578,   0,      "active tsf7",          "");
  nc->add_B2FormatLine(1579,   1598,   0,      "active tsf5",          "");
  nc->add_B2FormatLine(1599,   1618,   0,      "active tsf3",          "");
  nc->add_B2FormatLine(1619,   1638,   0,      "active tsf1",          "");
  nc->add_B2FormatLine(1639,   1639,   0,      "active nnt",           "");
  nc->add_B2FormatLine(1640,   1975,   0,      "persistor stereo",     "");
  nc->add_B2FormatLine(1976,   1999,   0,      "empty bits",           "");
  nc->add_B2FormatLine(2000,   2015,   0,      "NNT Clock Counter",    "");
  nc->add_B2FormatLine(2016,   2031,   0,      "'00000' & B2I Clock",  "");
  nc->add_B2FormatLine(2032,   2047,   0,      "hex'dddd'",            "");

  //set neural network filename:
  nc->set_NNName("v3.0.0_Neuro20170109_20170109");

  // loading MLPs:
  nc->load_MLPs("trg/cdc/data/Neuro20170109.root", "MLPs");

  //add notes for expert networks
  nc->set_NNNotes("Default weights from the basf2 data directory");

  //define to use the ETF:
  nc->set_useETF(true);

  //add preprocessing notes:
  nc->set_PPNotes("default");

  //set firmware version id:
  nc->set_NNTFirmwareVersionID("v0.0.0_default_20181120");

  // add comment about firmware:
  nc->set_NNTFirmwareComment("not known yet");

  IntervalOfValidity iov(1003, 0, 1003, -1);
  nc.import(iov);
}

int main()
{
  setconfig_nnt();
}
