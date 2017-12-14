#ifndef TELDATAREADERTEST_H
#define TELDATAREADERTEST_H

// include standard c++
#include <string>
#include <vector>
#include <map>

// include BASF2 module class
#include <framework/core/Module.h>

// include ROOT for histogram output
#include <TFile.h>
#include <TH2I.h>

namespace Belle2 {
  class TelDataReaderTestModule : public Module {
  public:
    TelDataReaderTestModule();
    virtual ~TelDataReaderTestModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();
  protected:

  private:
    /** Output file name for root file */
    std::string m_outFileName;

    // file handle
    TFile* m_file;
    std::vector<TH2I*> m_maps;
    std::map<unsigned short int, unsigned short int> m_planeToId;

    // mimosa telescope parameters
    unsigned short int m_nPxCol;
    unsigned short int m_nPxRow;
    unsigned short int m_nPlanes;
    std::vector<unsigned short int> m_planeNos;

    // binning parameters
    unsigned short int m_nBinsCol;
    unsigned short int m_nBinsRow;
  };
}
#endif
