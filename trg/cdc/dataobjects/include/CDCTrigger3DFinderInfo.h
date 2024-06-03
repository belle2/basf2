#ifndef CDCTRIGGER3DFINDERINFO_H
#define CDCTRIGGER3DFINDERINFO_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  /** Extra info created by the 3D finder of the CDC trigger. */
  class CDCTrigger3DFinderInfo : public RelationsObject {
  public:
    /** default constructor */
    CDCTrigger3DFinderInfo():
      m_houghspace(std::vector<TVector3>()), m_ndreadout(std::vector<TVector3>()) { }

    /** constructor with arguments */
    CDCTrigger3DFinderInfo(std::vector<TVector3>& houghspace, std::vector<TVector3>& ndreadout):
      m_houghspace(houghspace), m_ndreadout(ndreadout) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTrigger3DFinderInfo() { }

    /** get list of cell member indices */
    std::vector<TVector3> getHoughSpace() const { return m_houghspace; }

    std::vector<TVector3> getNDReadout() const { return m_ndreadout; }

    // setters
    /** set int values */
    void setInfoInts(std::vector<TVector3> houghspace, std::vector<TVector3> ndreadout)
    {
      m_houghspace = houghspace;
      m_ndreadout = ndreadout;
    }

  protected:

    /** list of hough space weights */
    std::vector<TVector3> m_houghspace = {};

    /** list of cell indices of all cluster */
    std::vector<TVector3> m_ndreadout = {};

    //! Needed to make the ROOT object storable
    ClassDef(CDCTrigger3DFinderInfo, 1);
  };
}
#endif
