#pragma once

#include <string>
#include <vector>
#include <map>

#include <genfit/MilleBinary.h>
#include <genfit/GblTrajectory.h>
#include <framework/utilities/FileSystem.h>
#include <framework/pcore/Mergeable.h>

#include <TCollection.h>
#include <TFileMergeInfo.h>

using namespace std;
namespace Belle2 {
  class MilleData : public Mergeable {
  public:
    MilleData() : Mergeable(), m_files(), m_binary(0) {};
    virtual ~MilleData() { close(); }

    virtual void merge(const Mergeable* other)
    {
      auto* data = static_cast<const MilleData*>(other);
      const vector<string>& files = data->getFiles();
      for (auto& file : files) {
        bool exists = false;
        for (auto& myfile : m_files) {
          if (myfile == file) {
            exists = true;
            break;
          }
        }
        if (!exists)
          m_files.push_back(file);
      }
    }
    virtual void clear() {/* m_files.clear();*/ }
    /*
        Long64_t Merge(TCollection *hlist)
        {
          if (hlist) {
            MilleData *xh = 0;
            TIter nxh(hlist);
            while ((xh = (MilleData *) nxh())) {
              // Add this histogram to me
              merge(xh);
            }
          }
          return (Long64_t) getFiles().size();
        }
    */
    void open(string filename)
    {
      if (m_binary) {
        close();
      }
      m_binary = new gbl::MilleBinary(filename);
      m_files.push_back(filename);
    }
    bool isOpen() { return !!m_binary; }
    void fill(gbl::GblTrajectory& trajectory)
    {
      if (m_binary) {
        trajectory.milleOut(*m_binary);
      }
    }
    void close()
    {
      if (m_binary) {
        delete m_binary;
        m_binary = 0;
      }
      usleep(200);
    }
    const vector<string>& getFiles() const { return m_files; }
    void reset()
    {
      //m_files.clear();
    }
  private:
    vector<string> m_files;
    gbl::MilleBinary* m_binary; //! not streamed

    ClassDef(MilleData, 1) /**< class definition */
  };
}
