//+
// File : storageoutput.h
// Description : Sequential ROOT I/O output module for DAQ
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//        9  - Dec - 2013 ; update for DAQ
//-

#ifndef _Belle2_HistAnalysisModule_h
#define _Belle2_HistAnalysisModule_h

#include <framework/core/Module.h>

#include <daq/slc/apps/dqmviewd/HistMemory.h>

#include <string>
#include <map>

namespace Belle2 {

  class DQMHistAnalysisModule : public Module {

  public:
    enum Type {
      INT,
      FLOAT,
      TEXT
    };
    typedef std::map<std::string, Type> ParamTypeList;
    typedef std::map<std::string, int> IntValueList;
    typedef std::map<std::string, float> FloatValueList;
    typedef std::map<std::string, std::string> TextList;
    typedef std::map<std::string, TH1*> HistList;

  private:
    static ParamTypeList g_parname;
    static IntValueList g_vint;
    static FloatValueList g_vfloat;
    static TextList g_text;
    static HistList g_hist;

  public:
    static TH1* findHist(const std::string& histname);
    static TH1* findHist(const std::string& dirname,
                         const std::string& histname);
    static void setIntValue(const std::string& parname, int vint);
    static void setFloatValue(const std::string& parname, float vfloat);
    static void setText(const std::string& parname, const std::string& text);

  public:
    static void addHist(const std::string& dirname,
                        const std::string& histname, TH1* h);
    static void resetHist() { g_hist = std::map<std::string, TH1*>(); }
    static HistList& getHists() { return g_hist; }
    static ParamTypeList& getParNames() { return g_parname; }
    static IntValueList& getIntValues() { return g_vint; }
    static FloatValueList& getFloatValues() { return g_vfloat; }
    static TextList& getTexts() { return g_text; }

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisModule();
    virtual ~DQMHistAnalysisModule();

    // Data members
  private:

  };
} // end namespace Belle2

#endif
