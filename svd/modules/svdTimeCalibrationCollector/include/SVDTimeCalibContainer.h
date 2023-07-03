/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


#include <map>
#include <iostream>

#include "TNamed.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCollection.h"
#include "TString.h"

namespace Belle2 {

  /**
   * Collection of histogram to be used in the calibration process
   */
  class SVDTimeCalibContainer : public TNamed {
  public:
    /** Default Constructor */
    SVDTimeCalibContainer(TString name = "name", TString title = "title")
      : TNamed(name, title)
    {
      fDirectory = nullptr;

      if (AddDirectoryStatus()) {
        fDirectory = gDirectory;
        if (fDirectory) {
          fDirectory->Append(this, true);
        }
      }
    }
    ~SVDTimeCalibContainer()
    {
      std::cout << " SVDTimeCalibContainer destructor called " << std::endl;
      m_TH1F.clear();
      m_TH2F.clear();
      if (fDirectory) {
        fDirectory->Remove(this);
        fDirectory = nullptr;
      }
      std::cout << " SVDTimeCalibContainer destructor exited " << std::endl;
    };

    /** Setter for directory  */
    void SetDirectory(TDirectory* dir)
    {
      if (fDirectory == dir) return;
      if (fDirectory) fDirectory->Remove(this);
      fDirectory = dir;
      if (fDirectory) {
        fDirectory->Append(this);
      }
      for (auto hist : m_TH1F)
        hist.second->SetDirectory(fDirectory);
      for (auto hist : m_TH2F)
        hist.second->SetDirectory(fDirectory);
    }

    bool AddDirectoryStatus() { return fgAddDirectory; }
    void AddDirectory(bool add) { fgAddDirectory = add; }

    void Reset()
    {
      for (auto hist : m_TH1F)
        hist.second->Reset();
      for (auto hist : m_TH2F)
        hist.second->Reset();
    }

    void Copy(TObject& obj) const override
    {
      if (((SVDTimeCalibContainer&)obj).fDirectory) {
        ((SVDTimeCalibContainer&)obj).fDirectory->Remove(&obj);
        ((SVDTimeCalibContainer&)obj).fDirectory = nullptr;
      }
      TNamed::Copy(obj);
      ((SVDTimeCalibContainer&)obj).m_TH1F = m_TH1F;
      ((SVDTimeCalibContainer&)obj).m_TH2F = m_TH2F;
      if (fgAddDirectory && gDirectory) {
        gDirectory->Append(&obj);
        ((SVDTimeCalibContainer&)obj).fDirectory = gDirectory;
      } else
        ((SVDTimeCalibContainer&)obj).fDirectory = nullptr;
    }

    TObject* Clone(const char* newname = "") const override
    {
      SVDTimeCalibContainer* obj = new SVDTimeCalibContainer(newname);
      Copy(*obj);
      if (newname && strlen(newname)) {
        obj->SetName(newname);
      }
      return obj;
    }

    void Merge(TCollection* li)
    {
      Merge<TH1F>(li, m_TH1F);
      Merge<TH2F>(li, m_TH2F);
    }

    template<class T>
    void Merge(TCollection* li, std::map<TString, std::shared_ptr<T>> histMap)
    {
      for (auto hist : histMap) {
        auto name = hist.second->GetName();
        TList list;
        list.SetOwner(false);
        TIter next(li);
        while (TObject* obj = next())
          list.Add(obj->FindObject(name));
        hist.second->Merge(&list);
      }
    }

    bool fgAddDirectory = true;   ///<! Flag to add histograms to the directory

    /** All Histograms */
    std::map<TString, std::shared_ptr<TH1F>> m_TH1F; /**< 1D Histograms */
    std::map<TString, std::shared_ptr<TH2F>> m_TH2F; /**< 2D Histograms */

  private:
    TDirectory* fDirectory; /**< Pointer to directory holding this */
    TDirectory* m_Dir;
    TString m_histoPath;

    SVDTimeCalibContainer(const SVDTimeCalibContainer&) = delete;
    SVDTimeCalibContainer& operator=(const SVDTimeCalibContainer&) = delete;
  };

} // end namespace Belle2
