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
#include <framework/datastore/RelationArray.h>

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBObjPtr.h>

using namespace std;
using namespace Belle2;


int DQMCommonUtils::SetFlag(int Type, int bin, const double* pars, double ratio, TH1F* hist, TH1F* refhist,
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


int DQMCommonUtils::SetFlag(int Type, int bin, const double* pars, double ratio, TH1I* hist, TH1I* refhist,
                            TH1I* flaghist) const
{
  auto histF = std::unique_ptr<TH1F>(new TH1F("histF", "histF", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(),
                                              hist->GetXaxis()->GetXmax()));
  auto refhistF = std::unique_ptr<TH1F>(new TH1F("refhistF", "refhistF", refhist->GetNbinsX(), refhist->GetXaxis()->GetXmin(),
                                                 refhist->GetXaxis()->GetXmax()));
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    histF->SetBinContent(j + 1, hist->GetBinContent(j + 1));
    refhistF->SetBinContent(j + 1, refhist->GetBinContent(j + 1));
  }
  int ret = SetFlag(Type, bin, pars, ratio, histF.get(), refhistF.get(), flaghist);
  return ret;
}


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


void DQMCommonUtils::CreateDBHistoGroup(TH1F** HistoDB, int number) const
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(number * HistoDB[0]->GetNbinsX() + 3);
  double* Content = new double[number * HistoDB[0]->GetNbinsX() + 3];
  Content[0] = HistoDB[0]->GetNbinsX();
  Content[1] = HistoDB[0]->GetXaxis()->GetXmin();
  Content[2] = HistoDB[0]->GetXaxis()->GetXmax();
  for (int j = 0; j < number; j++) {
    for (int i = 0; i < HistoDB[j]->GetNbinsX(); i++) {
      Content[j * HistoDB[j]->GetNbinsX() + i + 3] = HistoDB[j]->GetBinContent(i + 1);
    }
  }
  DBHisto->SetElements(Content);
  DBHisto.import(iov);
  delete [] Content;
}


void DQMCommonUtils::CreateDBHistoGroup(TH1I** HistoDB, int number) const
{
  IntervalOfValidity iov(0, 0, -1, -1);
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBImportObjPtr<TVectorD> DBHisto(Name.Data());
  DBHisto.construct(number * HistoDB[0]->GetNbinsX() + 3);
  double* Content = new double[number * HistoDB[0]->GetNbinsX() + 3];
  Content[0] = HistoDB[0]->GetNbinsX();
  Content[1] = HistoDB[0]->GetXaxis()->GetXmin();
  Content[2] = HistoDB[0]->GetXaxis()->GetXmax();
  for (int j = 0; j < number; j++) {
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


int DQMCommonUtils::LoadDBHistoGroup(TH1F** HistoDB, int number) const
{
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB[0]->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    for (int j = 0; j < number; j++) {
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


int DQMCommonUtils::LoadDBHistoGroup(TH1I** HistoDB, int number) const
{
  TString Name = Form("%s_Ref", HistoDB[0]->GetName());
  DBObjPtr<TVectorD> DBHisto(Name.Data());
  int ret = 0;
  if (DBHisto.isValid()) {
    ret = 1;
    if (HistoDB[0]->GetNbinsX() != (int)DBHisto->GetMatrixArray()[0]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmin() != DBHisto->GetMatrixArray()[1]) ret = 0;
    if (HistoDB[0]->GetXaxis()->GetXmax() != DBHisto->GetMatrixArray()[2]) ret = 0;
    for (int j = 0; j < number; j++) {
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

