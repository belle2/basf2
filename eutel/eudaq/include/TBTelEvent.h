#ifndef TBTELEVENT_H
#define TBTELEVENT_H

// include standard c++
#include <vector>
#include <map>
#include <memory>

// include the datastore objects
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <testbeam/vxd/dataobjects/TelEventInfo.h>

class TBTelEvent {
public:
  /** Constructor. */
  TBTelEvent() : m_eventNo(0),
    m_runNo(0),
    m_triggerId(0),
    m_timeStamp(0),
    m_Planes(new std::vector<unsigned short>()),
    m_planeToId(),
    m_Digits() {

  }

  /** Get Event Number. */
  inline unsigned long long int getEventNumber() const { return m_eventNo; }
  /** Get Run Number. */
  inline unsigned long long int getRunNumber() const { return m_runNo; }
  /** Get Trigger ID. */
  inline unsigned long long int getTriggerId() const { return m_triggerId; }
  /** Get Time Stamp. */
  inline unsigned long long int getTimeStamp() const { return m_timeStamp; }
  /** Get a TelEventInfo object. Ownership is transferred to the user. */
  inline Belle2::TelEventInfo* getTelEventInfo() const { return new Belle2::TelEventInfo(m_eventNo, m_triggerId, m_timeStamp); }

  /** Set Event Number. */
  inline void setEventNumber(unsigned long long int evtNo) { m_eventNo = evtNo; }
  /** Set Run Number. */
  inline void setRunNumber(unsigned long long int runNo) { m_runNo = runNo; }
  /** Set Trigger Id. */
  inline void setTriggerId(unsigned long long int triggerId) { m_triggerId = triggerId; }
  /**  Time Stamp. */
  inline void setTimeStamp(unsigned long long int timeStamp) { m_timeStamp = timeStamp; }

  /** Get number of planes currently stored. */
  inline size_t getNumPlanes() const { return m_Planes->size(); }
  /** Get the plane number collection. */
  inline std::shared_ptr<const std::vector<unsigned short> > getPlanes() const { return m_Planes; }

  /** Add data for a full plane. */
  void addPlane(unsigned short planeNo,
                const std::vector<unsigned short>& cols,
                const std::vector<unsigned short>& rows,
                const std::vector<unsigned short>& signals);
  /** Add data for a single pixel. */
  void addPixel(unsigned short planeNo,
                unsigned short col,
                unsigned short row,
                unsigned short signal);

  /** Obtain the data of a single plane from its plane number. */
  std::shared_ptr<const std::vector<Belle2::TelDigit> > getPlaneDigits(unsigned short planeNo) const;
  /** Obtain the data of a single plane from its vector id. */
  std::shared_ptr<const std::vector<Belle2::TelDigit> > getDigits(size_t id) const { return m_Digits.at(id); }

  /** Obtain the plain number from its vector id. */
  unsigned short getPlaneNumber(size_t id) const { return m_Planes->at(id); }
private:
  /** Internal function for obtaining the index in the data vector for a
  given plane number. If there is no data present yet for this plane
  number, create the appropriate data structure and return resulting
  index. */
  size_t getIdFromPlane(unsigned short planeNo);

  /** The current event number. */
  unsigned long long int m_eventNo;
  /** The current run number. */
  unsigned long long int m_runNo;
  /** The trigger number of the current event. */
  unsigned long long int m_triggerId;
  /** The timestamp of the current event. */
  unsigned long long int m_timeStamp;
  /** Set storing the plane numbers kept for this event. */
  std::shared_ptr<std::vector<unsigned short> > m_Planes;
  /** Map storing a mapping of plane number to vector id. */
  std::map<unsigned short int, size_t> m_planeToId;

  /** Vector for storing the pixel values of each plane separately. The
  outer vector loop over the different plane numbers, the inner vector
  over the firing pixels. */
  std::vector<std::shared_ptr<std::vector<Belle2::TelDigit> > > m_Digits;
};

#endif
