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
  /// Mergeable class holding list of so far opened mille binaries and providing the binaries
  class MilleData : public Mergeable {
  public:
    /// Constructor
    MilleData() : Mergeable() {};
    /// Destructor
    virtual ~MilleData() { close(); }

    /// Implementation of merging
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
    /// Implementation of clearing
    virtual void clear() { m_files.clear(); }

    /// Open a new file and remember it. Filename should encode also process id!
    void open(string filename)
    {
      if (m_binary) {
        close();
      }
      m_binary = new gbl::MilleBinary(filename);
      m_files.push_back(filename);
    }
    /// Is some file already open?
    bool isOpen() { return !!m_binary; }
    void fill(gbl::GblTrajectory& trajectory)
    {
      if (m_binary) {
        trajectory.milleOut(*m_binary);
      }
    }
    /// Close current mille binary if opened
    void close()
    {
      if (m_binary) {
        delete m_binary;
        m_binary = nullptr;
      }
    }
    /// Get the list of all created files
    const vector<string>& getFiles() const { return m_files; }
    /// Copy by assignment
    MilleData& operator=(const MilleData& other)
    {
      close();
      m_files = other.m_files;
      return *this;
    }
    /// Construct from other object
    MilleData(const MilleData& other) : m_files(other.m_files), m_binary(nullptr) {}
  private:
    vector<string> m_files = {}; /**< List of already created file names */
    gbl::MilleBinary* m_binary = nullptr; //! Pointer to opened binary file (not streamed)

    ClassDef(MilleData, 1) /**< Mergeable list of opened mille binaries */
  };
}
