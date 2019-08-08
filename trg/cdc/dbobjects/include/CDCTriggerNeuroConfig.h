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
    /** struct to hold a line of information of b2link format */
    struct B2FormatLine {
      int start;
      int end;
      int offset;
      std::string name;
      std::string description;
    };
    /** constructor */
    CDCTriggerNeuroConfig() {}

    /** destructor */
    virtual ~CDCTriggerNeuroConfig() {};

    /** function to directly set b2link format */
    void setB2Format(const std::vector<B2FormatLine>& format)
    {
      for (auto line : format) {
        addB2FormatLine(line);
      }
    }
    /** function to add line to b2link format, overloaded  */
    void addB2FormatLine(const B2FormatLine& line)
    {
      m_B2Format.push_back(line);
    }
    /** function to add line to b2link format, overloaded  */
    void addB2FormatLine(int start, int end, int offset, const std::string& name, const std::string& description)
    {
      B2FormatLine b;
      b.start = start;
      b.end = end;
      b.offset = offset;
      b.name = name;
      b.description = description;
      addB2FormatLine(b);
    }
    /** set name of neural network version  */
    void setNNName(const std::string& filename)
    {
      if (filename.size() <= 255) {
        m_NNName = filename;
      }
    }
    /** load MLP objects from file */
    void loadMLPs(const std::string& filename, const std::string& arrayname)
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
    /** add some notes to the verison of MLPS  */
    void setNNNotes(const std::string& notes)
    {
      if (notes.size() <= 255) {
        m_NNNotes = notes;
      }
    }
    /** set bool wether ETF is used or not */
    void setUseETF(bool b)
    {
      m_useETF = b;
    }
    /** add some notes about the preprocessing */
    void setPPNotes(const std::string& notes)
    {
      if (notes.size() <= 255) {
        m_PPNotes = notes;
      }
    }
    /** set the firmware version id */
    void setNNTFirmwareVersionID(const std::string& version)
    {
      if (version.size() <= 255) {
        m_NNTFirmwareVersionID = version;
      }
    }
    /** add a comment to the firmware version */
    void setNNTFirmwareComment(const std::string& notes)
    {
      if (notes.size() <= 255) {
        m_NNTFirmwareComment = notes;
      }
    }
    /** return b2link format  */
    std::vector<B2FormatLine> getB2Format()  const  {return m_B2Format;}
    /** return neural network name  */
    std::string getNNName()                  const  {return m_NNName;}
    /**return loaded CDCTriggerMPL objects  */
    std::vector<CDCTriggerMLP> getMLPs()     const  {return m_MLPs;}
    /**return neural network notes  */
    std::string getNNNotes()                 const  {return m_NNNotes;}
    /** return bool wether ETF is used or not  */
    bool getUseETF()                         const  {return m_useETF;}
    /**return notes on preprocessing  */
    std::string getPPNotes()                 const  {return m_PPNotes;}
    /** return firmware version  */
    std::string getNNTFirmwareVersionID()    const  {return m_NNTFirmwareVersionID;}
    /** returncomment on firmware version  */
    std::string getNNTFirmwareComment()      const  {return m_NNTFirmwareComment;}

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

