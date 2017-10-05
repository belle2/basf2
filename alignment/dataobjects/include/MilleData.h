#pragma once

#include <string>
#include <vector>

#include <calibration/core/MergeableNamed.h>
#include <genfit/MilleBinary.h>
#include <genfit/GblTrajectory.h>

namespace Belle2 {
  /// Mergeable class holding list of so far opened mille binaries and providing the binaries
  class MilleData : public MergeableNamed {
  public:
    /// Constructor. Set doublePrecision to true to write binary files with doubles instead of floats
    explicit MilleData(bool doublePrecision = false) : MergeableNamed(), m_doublePrecision(doublePrecision) {};
    /// Destructor
    virtual ~MilleData() { close(); }

    /// Implementation of merging
    virtual void merge(const MergeableNamed* other);
    /// Implementation of clearing
    virtual void clear() { m_files.clear(); m_numRecords = 0; }
    /// Open a new file and remember it. Filename should encode also process id!
    void open(std::string filename);
    /// Is some file already open?
    bool isOpen() { return !!m_binary; }
    /// Write a GBL trajectory to the binary file
    void fill(gbl::GblTrajectory& trajectory);
    /// Close current mille binary if opened
    void close();
    /// Get the list of all created files
    const std::vector<std::string>& getFiles() const { return m_files; }
    /// Copy by assignment (if some file on LHS is opened, it is closed during this operation; file pointers not transfered - new file to be opened)
    MilleData& operator=(const MilleData& other);
    /// Construct from other object (pointer to binary file is not transfered - new file has to be opened by new object)
    MilleData(const MilleData& other) : MergeableNamed(other), m_doublePrecision(other.m_doublePrecision), m_files(other.m_files),
      m_binary(nullptr),
      m_numRecords(other.m_numRecords) {}

    /// Get number of records (trajectories) written to binary files
    int getNumRecords() {return m_numRecords;}
    /// Are files written with double precision?
    bool hasDoublePrecision() {return m_doublePrecision;}
    /** Root-like Reset function for "template compatibility" with ROOT objects. Alias for clear(). */
    virtual void Reset() {clear();}
    /// Root-like SetDirectory function for "template compatibility" with ROOT objects. Does nothing.
    virtual void SetDirectory(TDirectory*) {}
  private:
    /// Use double-precision for binary files
    bool m_doublePrecision{false};
    std::vector<std::string> m_files{}; /**< List of already created file names */
    /// Pointer to current binary file
    gbl::MilleBinary* m_binary{nullptr}; //! Pointer to opened binary file (not streamed)
    /// Number of written trajectories
    int m_numRecords{0};

    ClassDef(MilleData, 2) /**< Mergeable list of opened mille binaries */
  };
}
