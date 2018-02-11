/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/utilities/DQMCommonUtils.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/RelationArray.h>

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBObjPtr.h>

using namespace std;
using namespace Belle2;


int DQMCommonUtils::getPXDSensorIndex(const int Layer, const int Ladder, const int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          return tempcounter;
        }
        tempcounter++;
      }
    }
  }
  return tempcounter;
}


void DQMCommonUtils::getIDsFromPXDIndex(const int Index, int& Layer, int& Ladder, int& Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          Layer = layer.getLayerNumber();
          Ladder = ladder.getLadderNumber();
          Sensor = sensor.getSensorNumber();
          return;
        }
        tempcounter++;
      }
    }
  }
}


int DQMCommonUtils::getPXDChipIndex(const int Layer, const int Ladder, const int Sensor, const int Chip,
                                    const int IsU) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          int iChip = Chip - 1;
          if (!IsU)
            iChip += (int)c_nPXDChipsU;
          return tempcounter + iChip;
        }
        tempcounter = tempcounter + (c_nPXDChipsU + c_nPXDChipsV);
      }
    }
  }
  return tempcounter;
}


void DQMCommonUtils::getIDsFromPXDChipIndex(const int Index, int& Layer, int& Ladder, int& Sensor, int& Chip,
                                            int& IsU) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        Layer = layer.getLayerNumber();
        Ladder = ladder.getLadderNumber();
        Sensor = sensor.getSensorNumber();
        int Chips = c_nPXDChipsU + c_nPXDChipsV;
        for (int iChip = 0; iChip < Chips; iChip++) {
          if (tempcounter + iChip == Index) {
            Layer = layer.getLayerNumber();
            Ladder = ladder.getLadderNumber();
            Sensor = sensor.getSensorNumber();
            Chip = iChip + 1;
            IsU = 1;
            if (iChip >= c_nPXDChipsU) {
              Chip = iChip + 1 - c_nPXDChipsU;
              IsU = 0;
            }
            return;
          }
        }
        tempcounter = tempcounter + (c_nPXDChipsU + c_nPXDChipsV);
      }
    }
  }
}


int DQMCommonUtils::getSVDSensorIndex(const int Layer, const int Ladder, const int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
  return tempcounter;
}


void DQMCommonUtils::getIDsFromSVDIndex(const int Index, int& Layer, int& Ladder, int& Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          Layer = layer.getLayerNumber();
          Ladder = ladder.getLadderNumber();
          Sensor = sensor.getSensorNumber();
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
}


int DQMCommonUtils::getSVDChipIndex(const int Layer, const int Ladder, const int Sensor, const int Chip,
                                    const int IsU) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
//  int Chip =
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          int iChip = Chip;
          if (Layer == 3)
            if (!IsU)
              iChip += c_nSVDChipsL3;
          if (Layer > 3)
            if (!IsU)
              iChip += c_nSVDChipsLu;
          return tempcounter + iChip;
        }
        if (Layer == 3)
          tempcounter = tempcounter + (2 * c_nSVDChipsL3);
        if (Layer > 3)
          tempcounter = tempcounter + (c_nSVDChipsLu + c_nSVDChipsLv);
      }
    }
  }
  return tempcounter;
}


void DQMCommonUtils::getIDsFromSVDChipIndex(const int Index, int& Layer, int& Ladder, int& Sensor, int& Chip,
                                            int& IsU) const
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() <= geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size()) continue;  // need SVD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        Layer = layer.getLayerNumber();
        Ladder = ladder.getLadderNumber();
        Sensor = sensor.getSensorNumber();
        int Chips = 0;
        if (layer.getLayerNumber() == 3)
          Chips = 2 * c_nSVDChipsL3;
        if (layer.getLayerNumber() > 3)
          Chips = c_nSVDChipsLu + c_nSVDChipsLv;
        for (int iChip = 0; iChip < Chips; iChip++) {
          if (tempcounter + iChip == Index) {
            Layer = layer.getLayerNumber();
            Ladder = ladder.getLadderNumber();
            Sensor = sensor.getSensorNumber();
            Chip = iChip;
            IsU = 1;
            if (Layer == 3) {
              if (iChip >= c_nSVDChipsL3) {
                Chip = iChip - c_nSVDChipsL3;
                IsU = 0;
              }
            }
            if (Layer > 3) {
              if (iChip >= c_nSVDChipsLu) {
                Chip = iChip - c_nSVDChipsLu;
                IsU = 0;
              }
            }
            return;
          }
        }
        if (Layer == 3)
          tempcounter = tempcounter + (2 * c_nSVDChipsL3);
        if (Layer > 3)
          tempcounter = tempcounter + (c_nSVDChipsLu + c_nSVDChipsLv);
      }
    }
  }
}


int DQMCommonUtils::getVXDLayerIndex(const int Layer)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (Layer == layer.getLayerNumber()) {
      return tempcounter;
    }
    tempcounter++;
  }
  return tempcounter;
}


void DQMCommonUtils::getLayerIDsFromVXDLayerIndex(const int Index, int& Layer)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    if (tempcounter == Index) {
      Layer = layer.getLayerNumber();
      return;
    }
    tempcounter++;
  }
}


int DQMCommonUtils::getVXDSensorIndex(const int Layer, const int Ladder, const int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          return tempcounter;
        }
        tempcounter++;
      }
    }
  }
  return tempcounter;
}


void DQMCommonUtils::getIDsFromVXDIndex(const int Index, int& Layer, int& Ladder, int& Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          Layer = layer.getLayerNumber();
          Ladder = ladder.getLadderNumber();
          Sensor = sensor.getSensorNumber();
          return;
        }
        tempcounter++;
      }
    }
  }
}


int DQMCommonUtils::SetFlag(const int Type, const int bin, double* pars, const double ratio, TH1F* hist, TH1F* refhist,
                            TH1I* flaghist) const
{
  int iret = 0;
  float WarningLevel = 6.0;
  float ErrorLevel = 10.0;
  auto temp = std::unique_ptr<TH1F>(new TH1F("temp", "temp", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(),
                                             hist->GetXaxis()->GetXmax()));
  double NEvents = 0;
  double flagInt = 0;
  double flagrInt = 0;
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    double val = hist->GetBinContent(j + 1);
    NEvents += val;
    val = val / ratio;
    temp->SetBinContent(j + 1, val);
    flagInt += temp->GetBinContent(j + 1);
    flagrInt += refhist->GetBinContent(j + 1);
  }
  if (NEvents < 100) {  // not enough information for comparition
    iret = -1;
    flaghist->SetBinContent(bin + 1, -1);
    return iret;
  }
  double flag  = temp->GetMean();
  double flagErr  = temp->GetMeanError();
  double flagRMS  = temp->GetRMS();
  double flagRMSErr  = temp->GetRMSError();
  double flagr  = refhist->GetMean();
  double flagrErr  = refhist->GetMeanError();
  double flagrRMS  = refhist->GetRMS();
  double flagrRMSErr  = refhist->GetRMSError();
  TString strDebugInfo = Form("Conditions for Flag--->\n   source %f %f+-%f %f+-%f\n  referen %f %f+-%f %f+-%f\n",
                              flagInt, flag, flagErr, flagRMS, flagRMSErr,
                              flagrInt, flagr, flagrErr, flagrRMS, flagrRMSErr
                             );
  B2DEBUG(130, strDebugInfo.Data());
  if (Type == 1) {  // counts+mean+RMS use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 2) { // counts use
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 3) { // mean use
    if (fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 4) { // RMS use
    if (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 5) { // counts+mean use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 9) { // bin content use
    flagInt = temp->GetBinContent(bin + 1);
    flagrInt = refhist->GetBinContent(bin + 1);
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 10) {
    float flag2  = refhist->Chi2Test(temp.get());
    flaghist->SetBinContent(bin + 1, 0);
    if (flag2 > pars[1])
      flaghist->SetBinContent(bin + 1, 2);
    if (flag2 > pars[0])
      flaghist->SetBinContent(bin + 1, 1);
    iret = 1;
  } else if (Type == 100) {
    flaghist->SetBinContent(bin + 1, 0);
    iret = 1;
  } else {
    flaghist->SetBinContent(bin + 1, -3);
    iret = -1;
  }
  strDebugInfo = Form("SetFlag---> %f, type %i\n", flaghist->GetBinContent(bin + 1), Type);
  B2DEBUG(130, strDebugInfo.Data());
  return iret;
}


int DQMCommonUtils::SetFlag2(const int Type, const int bin, double* pars, const double ratio, TH1I* hist, TH1I* refhist,
                             TH1I* flaghist) const
{
//  TH1F* histF = new TH1F("histF", "histF", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
//  TH1F* refhistF = new TH1F("refhistF", "refhistF", refhist->GetNbinsX(), refhist->GetXaxis()->GetXmin(),
//                            refhist->GetXaxis()->GetXmax());
  auto histF = std::unique_ptr<TH1F>(new TH1F("histF", "histF", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(),
                                              hist->GetXaxis()->GetXmax()));
  auto refhistF = std::unique_ptr<TH1F>(new TH1F("refhistF", "refhistF", refhist->GetNbinsX(), refhist->GetXaxis()->GetXmin(),
                                                 refhist->GetXaxis()->GetXmax()));
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    histF->SetBinContent(j + 1, hist->GetBinContent(j + 1));
    refhistF->SetBinContent(j + 1, refhist->GetBinContent(j + 1));
  }
  int ret = SetFlag(Type, bin, pars, ratio, histF.get(), refhistF.get(), flaghist);
//  int ret = SetFlag(Type, bin, pars, ratio, histF, refhistF, flaghist);
//  delete histF;
//  delete refhistF;
  return ret;
}

/*
int DQMCommonUtils::SetFlag(const int Type, const int bin, double* pars, const double ratio, std::string name_hist,
                                        std::string name_refhist, TH1I* flaghist) const
{
  int iret = 0;
  float WarningLevel = 6.0;
  float ErrorLevel = 10.0;

  TH1* hist, *refhist;

  hist = GetHisto(name_hist);
  if (!hist) return -1;
  refhist = GetHisto(name_refhist);
  if (!refhist) return -1;

  // What happens if they are TH1I, TH1D and not TH1F

  auto temp = std::unique_ptr<TH1F>(new TH1F("temp", "temp", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(),
                                             hist->GetXaxis()->GetXmax()));
  double NEvents = 0;
  double flagInt = 0;
  double flagrInt = 0;
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    double val = hist->GetBinContent(j + 1);
    NEvents += val;
    val = val / ratio;
    temp->SetBinContent(j + 1, val);
    flagInt += temp->GetBinContent(j + 1);
    flagrInt += refhist->GetBinContent(j + 1);
  }
  if (NEvents < 100) {  // not enough information for comparition
    iret = -1;
    flaghist->SetBinContent(bin + 1, -1);
    return iret;
  }
  double flag  = temp->GetMean();
  double flagErr  = temp->GetMeanError();
  double flagRMS  = temp->GetRMS();
  double flagRMSErr  = temp->GetRMSError();
  double flagr  = refhist->GetMean();
  double flagrErr  = refhist->GetMeanError();
  double flagrRMS  = refhist->GetRMS();
  double flagrRMSErr  = refhist->GetRMSError();
  TString strDebugInfo = Form("Conditions for Flag--->\n   source %f %f+-%f %f+-%f\n  referen %f %f+-%f %f+-%f\n",
                              flagInt, flag, flagErr, flagRMS, flagRMSErr,
                              flagrInt, flagr, flagrErr, flagrRMS, flagrRMSErr
                             );
  B2DEBUG(130, strDebugInfo.Data());
  if (Type == 1) {  // counts+mean+RMS use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 2) { // counts use
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 3) { // mean use
    if (fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 4) { // RMS use
    if (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 5) { // counts+mean use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 9) { // bin content use
    flagInt = temp->GetBinContent(bin + 1);
    flagrInt = refhist->GetBinContent(bin + 1);
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 10) {
    float flag2  = refhist->Chi2Test(temp.get());
    flaghist->SetBinContent(bin + 1, 0);
    if (flag2 > pars[1])
      flaghist->SetBinContent(bin + 1, 2);
    if (flag2 > pars[0])
      flaghist->SetBinContent(bin + 1, 1);
    iret = 1;
  } else if (Type == 100) {
    flaghist->SetBinContent(bin + 1, 0);
    iret = 1;
  } else {
    flaghist->SetBinContent(bin + 1, -3);
    iret = -1;
  }
  strDebugInfo = Form("SetFlag---> %f, type %i\n", flaghist->GetBinContent(bin + 1), Type);
  B2DEBUG(130, strDebugInfo.Data());
  return iret;
}
*/
/*
TH1* DQMCommonUtils::GetHisto(TString histoname) const
{
  TH1* hh1;
  hh1 = findHist(histoname.Data());
  if (hh1 == NULL) {
    B2INFO("Histo " << histoname << " not in memfile");
    // the following code sux ... is there no root function for that?


    // first search reference root file ... if ther is one
    if (m_refFile && m_refFile->IsOpen()) {
      TDirectory* d = m_refFile;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      B2INFO(myl);
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2INFO("Cd Dir " << tok);
            d = e;
          } else {
            B2INFO("cd failed " << tok);
          }
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2INFO("Histo " << histoname << " found in ref file");
          hh1 = (TH1*)obj;
        } else {
          B2INFO("Histo " << histoname << " found in ref file but wrong type");
        }
      } else {
        // seems find will only find objects, not keys, thus get the object on first access
        TIter next(d->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {
          TObject* obj2 = key->ReadObj() ;
          if (obj2->InheritsFrom("TH1")) {
            if (obj2->GetName() == tok) {
              hh1 = (TH1*)obj2;
              B2INFO("Histo " << histoname << " found as key -> readobj");
              break;
            }
          }
        }
        if (hh1 == NULL) B2INFO("Histo " << histoname << " NOT found in ref file " << tok);
      }
    }

    if (hh1 == NULL) {
      B2INFO("Histo " << histoname << " not in memfile or ref file");
      // the following code sux ... is there no root function for that?

      TDirectory* d = gROOT;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2INFO("Cd Dir " << tok);
            d = e;
          } else B2INFO("cd failed " << tok);
          d->cd();
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2INFO("Histo " << histoname << " found in mem");
          hh1 = (TH1*)obj;
        }
      } else {
        B2INFO("Histo " << histoname << " NOT found in mem");
      }
    }
  }

  if (hh1 == NULL) {
    B2INFO("Histo " << histoname << " not found");
  }

  return hh1;
}
*/

void DQMCommonUtils::CreateDBHisto(TH1F* HistoDB) const
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(HistoDB->GetNbinsX() + 3);
  double* Content = new double[HistoDB->GetNbinsX() + 3];
  Content[0] = HistoDB->GetNbinsX();
  Content[1] = HistoDB->GetXaxis()->GetXmin();
  Content[2] = HistoDB->GetXaxis()->GetXmax();
  for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
    Content[i + 3] = HistoDB->GetBinContent(i + 1);
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}


void DQMCommonUtils::CreateDBHisto(TH1I* HistoDB) const
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(HistoDB->GetNbinsX() + 3);
  double* Content = new double[HistoDB->GetNbinsX() + 3];
  Content[0] = HistoDB->GetNbinsX();
  Content[1] = HistoDB->GetXaxis()->GetXmin();
  Content[2] = HistoDB->GetXaxis()->GetXmax();
  for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
    Content[i + 3] = HistoDB->GetBinContent(i + 1);
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}


void DQMCommonUtils::CreateDBHistoGroup(TH1F** HistoDB, const int Number) const
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(Number * HistoDB[0]->GetNbinsX() + 3);
  double* Content = new double[Number * HistoDB[0]->GetNbinsX() + 3];
  Content[0] = HistoDB[0]->GetNbinsX();
  Content[1] = HistoDB[0]->GetXaxis()->GetXmin();
  Content[2] = HistoDB[0]->GetXaxis()->GetXmax();
  for (int j = 0; j < Number; j++) {
    for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
      Content[j * HistoDB[j]->GetNbinsX() + i + 3] = HistoDB[j]->GetBinContent(i + 1);
    }
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}


void DQMCommonUtils::CreateDBHistoGroup(TH1I** HistoDB, const int Number) const
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(Number * HistoDB[0]->GetNbinsX() + 3);
  double* Content = new double[Number * HistoDB[0]->GetNbinsX() + 3];
  Content[0] = HistoDB[0]->GetNbinsX();
  Content[1] = HistoDB[0]->GetXaxis()->GetXmin();
  Content[2] = HistoDB[0]->GetXaxis()->GetXmax();
  for (int j = 0; j < Number; j++) {
    for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
      Content[j * HistoDB[j]->GetNbinsX() + i + 3] = HistoDB[j]->GetBinContent(i + 1);
    }
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}


int DQMCommonUtils::LoadDBHisto(TH1F* HistoDB) const
{
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    if (ret) {
      for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
        HistoDB->SetBinContent(i + 1, (int)DBHisto->GetMatrixArray()[i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}


int DQMCommonUtils::LoadDBHisto(TH1I* HistoDB) const
{
  TString Name = Form("%s_Ref", HistoDB->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    if (ret) {
      for (int i = 0; i < HistoDB->GetNbinsX(); i++) {
        HistoDB->SetBinContent(i + 1, (int)DBHisto->GetMatrixArray()[i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}


int DQMCommonUtils::LoadDBHistoGroup(TH1F** HistoDB, const int Number) const
{
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB[0]->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    for (int j = 0; j < Number; j++) {
      for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
        HistoDB[j]->SetBinContent(i + 1, DBHisto->GetMatrixArray()[j * HistoDB[j]->GetNbinsX() + i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}


int DQMCommonUtils::LoadDBHistoGroup(TH1I** HistoDB, const int Number) const
{
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB[0]->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    for (int j = 0; j < Number; j++) {
      for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
        HistoDB[j]->SetBinContent(i + 1, DBHisto->GetMatrixArray()[j * HistoDB[j]->GetNbinsX() + i + 3]);
      }
    }
  }
  if (!ret) {
    B2INFO("ERROR to open reference histogram: " << Name.Data());
  }
  return ret;
}

