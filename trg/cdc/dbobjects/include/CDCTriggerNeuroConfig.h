/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Meggendorfer                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <TFile.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
#include <string>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /** The payload class for all CDC Neurotrigger information
   *
   */

  class CDCTriggerNeuroConfig: public TObject {
  public:
    struct B2FormatLine {
      int start;
      int end;
      int offset;
      std::string name;
      std::string description;
    };
    CDCTriggerNeuroConfig() {}

    virtual ~CDCTriggerNeuroConfig() {};

    void set_B2Format(const std::vector<B2FormatLine>& format)
    {
      for (auto line : format) {
        add_B2FormatLine(line);
      }
    }
    void add_B2FormatLine(const B2FormatLine& line)
    {
      m_B2Format.push_back(line);
    }
    void add_B2FormatLine(int start, int end, int offset, std::string name, std::string description)
    {
      B2FormatLine b;
      b.start = start;
      b.end = end;
      b.offset = offset;
      b.name = name;
      b.description = description;
      add_B2FormatLine(b);
    }
    void set_NNName(const std::string& filename)
    {
      if (filename.size() <= 255) {
        m_NNName = filename;
      }
    }
    void load_MLPs(const std::string& filename, const std::string& arrayname)
    {
      TFile datafile(filename.c_str(), "READ");
      TObjArray* MLPs = (TObjArray*)datafile.Get(arrayname.c_str());
      if (!MLPs) {
        datafile.Close();
      }
      m_MLPs.clear();
      for (int isector = 0; isector < MLPs->GetEntriesFast(); ++isector) {
        CDCTriggerMLP* expert = dynamic_cast<CDCTriggerMLP*>(MLPs->At(isector));
        if (expert) m_MLPs.push_back(*expert);
      }
      MLPs->Clear();
      delete MLPs;
      datafile.Close();
    }
    void set_NNNotes(const std::string& notes)
    {
      if (notes.size() <= 255) {
        m_NNNotes = notes;
      }
    }
    void set_useETF(bool b)
    {
      m_useETF = b;
    }
    void set_PPNotes(const std::string& notes)
    {
      if (notes.size() <= 255) {
        m_PPNotes = notes;
      }
    }
    void set_NNTFirmwareVersionID(const std::string& version)
    {
      if (version.size() <= 255) {
        m_NNTFirmwareVersionID = version;
      }
    }
    void set_NNTFirmwareComment(const std::string& notes)
    {
      if (notes.size() <= 255) {
        m_NNTFirmwareComment = notes;
      }
    }
    std::vector<B2FormatLine> get_B2Format()  const  {return m_B2Format;}
    std::string get_NNName()                  const  {return m_NNName;}
    std::vector<CDCTriggerMLP> get_MLPs()     const  {return m_MLPs;}
    std::string get_NNNotes()                 const  {return m_NNNotes;}
    bool get_useETF()                         const  {return m_useETF;}
    std::string get_PPNotes()                 const  {return m_PPNotes;}
    std::string get_NNTFirmwareVersionID()    const  {return m_NNTFirmwareVersionID;}
    std::string get_NNTFirmwareComment()      const  {return m_NNTFirmwareComment;}

  private:
    // B2Format
    std::vector<B2FormatLine> m_B2Format = {};

    // Used neurotrigger filename
    std::string m_NNName;

    // weights of expert networks
    std::vector<CDCTriggerMLP> m_MLPs;

    // short field for notes
    std::string m_NNNotes;

    /** switch wether the ETF is used or the first priority time of the
     * TSF is  used during preprocessing**/
    bool m_useETF = true;

    // short field for notes
    std::string m_PPNotes;

    /** Firmware Version ID **/
    std::string m_NNTFirmwareVersionID;

    /** Short comment on Firmware **/
    std::string m_NNTFirmwareComment;

    ClassDef(CDCTriggerNeuroConfig, 1); /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

