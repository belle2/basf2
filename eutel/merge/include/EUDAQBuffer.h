/*
 * EUDAQBuffer.h
 *
 *  Created on: Apr 1, 2016
 *      Author: stolzenberg
 */

#ifndef EUDAQBUFFER_H_
#define EUDAQBUFFER_H_

// include standard c++
#include <sys/time.h>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <algorithm>
#include <limits>

// Include BASF2 Base stuff
#include <framework/core/Module.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

// load some parts of the PXD
#include <pxd/dataobjects/PXDDigit.h>
#include <vxd/dataobjects/VxdID.h>
//#include <rawdata/dataobjects/RawFTSW.h>

// load some parts of the eudaq
#include <eutel/eudaq/FileReader.h>
#include <eutel/eudaq/DetectorEvent.h>
#include <eutel/eudaq/RawDataEvent.h>
#include <eutel/eudaq/PluginManager.h>
#include <eutel/eudaq/Utils.h>
#include <eutel/eudaq/TBTelEvent.h>

// load the datastore objects
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <testbeam/vxd/dataobjects/TelEventInfo.h>

#include <testbeam/vxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

/** EUDAQBuffer: BUffer filled with EUDAQ Events.
 */

// PXD digit type: First entry: SensorID, second/third/fourth entry: column/row/ADC value of a PXD digit
typedef std::tuple<unsigned short, short, short, float> short_PXDdigit_type;
// TEL digit type: First entry: SensorID, second/third entry: column/row value of a telescope digit
typedef std::tuple<unsigned short, short, short> short_TELdigit_type;
// EUDAQEvent: First entry: Trigger ID, second/third entry: Teldigits/PXDdigits vector
typedef std::tuple<short int, std::vector<short_TELdigit_type>, std::vector<short_PXDdigit_type>> EUDAQEvent;

using namespace Belle2;
/*
// EUDAQ event structure consisting of a trigger number, an PXD subevent and a telescope subevents for every plane.
// All parameters can be read out and set
struct EUDAQEvent{
      int TriggerNumber;
      std::vector<short_PXDdigit_type> PXDSubEvent;
      std::vector<short_TELdigit_type> TelSubEvents;
};
*/
class EUDAQBuffer {

private:

  // Maximal number of events in the buffer
  int m_buffersize;

  // Current Position of the first event in the buffer (Pos. 0 first event in the EUDAQ raw file)
  int m_position;

  // EUDAQ Filereader for readout of eudaq raw telescope and PXD events
  eudaq::FileReader* m_reader;

  // Vector with size m_buffersize filled with EUDAQ events (first event defined by m_position)
  std::vector<EUDAQEvent> EUDAQEventCollection;

public:

  // Set the buffersize of the buffer, also set the m_position to zero
  void SetBufferSize(int buffersize) {m_buffersize = buffersize; m_position = 0; }


  // Set the file reader for this buffer
  void SetFileReader(eudaq::FileReader* reader) {m_reader = reader; }


  // Return the buffersize of the buffer
  int GetBufferSize() {return m_buffersize; }


  // Initialize buffer, afterwards it should contain m_buffersize events. The first event should be event 0 in the raw file
  void initialize()
  {

    for (int evt_iterator = 0; evt_iterator < m_buffersize; evt_iterator++) {

      //Before reading out go to next event (BORE event leads to a crash)
      if (!m_reader->NextEvent()) {
        B2INFO("No more usable telescope events to acquire. Something is wrong, the buffersize should probably be reduced.");
        break;
      }

      const eudaq::Event& ev = m_reader->GetEvent();
      bool result = AddEventToBuffer(ev);
      if (result == true) {B2DEBUG(10, "Successfully added event to buffer!");}
      else {B2WARNING("There was an Error! Wasn't able to add event to buffer!");}
    }
  }


  // Add the PXD and Telescope Subevents of a EUDAQ Event ev to the Buffer
  bool AddEventToBuffer(const eudaq::Event& ev)
  {

    B2DEBUG(10, "Add event to Buffer!");


    if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

      B2DEBUG(75, "Detector Event created");

      TBTelEvent tbEvt = eudaq::PluginManager::ConvertToTBTelEvent(* detEv);

      B2DEBUG(10, "TBEvent: Event: " << tbEvt.getEventNumber()
              << ", NumPlanes: " << tbEvt.getNumTelPlanes()
              << ", TrigID: " << tbEvt.getTriggerId());


      EUDAQEvent eudaqevt = CreateEUDAQEventTuple(tbEvt);
      EUDAQEventCollection.push_back(eudaqevt);

      B2DEBUG(75, "Created EUDAQ event tuple and added to buffer! ");

    }

    else {
      return false;
    }

    return true;
  }


  // Returns the EUDAQ event at buffer position pos
  EUDAQEvent GetEUDAQEvent(int pos) { return EUDAQEventCollection[pos];}


  // Creates a EUDAQ Event tuple from a tbEvent
  EUDAQEvent CreateEUDAQEventTuple(TBTelEvent tbEvt)
  {

    short int currentTLUTagFromEUDAQ = short(tbEvt.getEventNumber() % 32768);
    // it is really important to note here that we must manually set the trigger ID
    // after the conversion step has finished. This is due to the fact that a certain
    // TLU ID Mask was used, which in turn is not considered in the eudaq part.
    tbEvt.setTriggerId(currentTLUTagFromEUDAQ);

    // Get the telescope digits for the buffer
    std::vector<short_TELdigit_type> TelSubEvents;
    for (size_t plane = 0; plane < tbEvt.getNumTelPlanes(); ++plane) {
      const std::shared_ptr<const std::vector<TelDigit> > TELdigits = tbEvt.getTelDigits(plane);
      int numdigits = 0;
      for (const TelDigit& TELdigit : *TELdigits) {
        short_TELdigit_type TELdtuple =
          std::make_tuple(TELdigit.getSensorID().getID(), TELdigit.getUCellID(), TELdigit.getVCellID());
        TelSubEvents.push_back(TELdtuple);
        numdigits++;
      }

      B2DEBUG(75, "Telescope subevent of plane " << plane << " read out:\n" << "Trigger number: " << currentTLUTagFromEUDAQ << "\n" <<
              "Number of digits: " << numdigits);

    }



    // Get the digits from the pxd events from the eudaq data stream
    std::vector<short_PXDdigit_type> PXDSubEvent;
    short_PXDdigit_type PXDdtuple;

    if (tbEvt.getNumPXDPlanes() > 0) {
      // Readout the PXD digits for the first PXD sensor
      const std::shared_ptr<const std::vector<PXDDigit> > PXDdigits = tbEvt.getPXDDigits(0);
      for (const PXDDigit& PXDdigit : *PXDdigits) {
        PXDdtuple = std::make_tuple(PXDdigit.getSensorID().getID(), PXDdigit.getUCellID(), PXDdigit.getVCellID(), PXDdigit.getCharge());
        PXDSubEvent.push_back(PXDdtuple);
      }
    }

    else {
      // If there is no PXD sensor just store a single digit without any information
      PXDdtuple = std::make_tuple(0, 0, 0, 0);
      PXDSubEvent.push_back(PXDdtuple);
    }

    B2DEBUG(75, "PXD subevent read out:\n" << "Trigger number: " << currentTLUTagFromEUDAQ << "\n" << "Number of digits: " <<
            PXDSubEvent.size());

    EUDAQEvent eudaqevt = std::make_tuple(currentTLUTagFromEUDAQ, TelSubEvents, PXDSubEvent);

    return eudaqevt;
  }



  // Update buffer after successful digit matching
  // The matched event is removed and the next event is added to the buffer
  bool Update(int matchposition)
  {

    // First decrease the position of all events above bufferposition by one
    // This also deletes the event entry at bufferposition
    for (int counter = matchposition; counter < m_buffersize - 1; counter++) {
      EUDAQEventCollection.at(counter) = EUDAQEventCollection[counter + 1];
    }

    // Then get the next event from the file reader (if it does exist)
    if (!m_reader->NextEvent()) {
      B2INFO("No more usable telescope events to acquire. Stop filling buffer!");
      return false;
    }
    // Replace the last buffer entry with the new event
    else {
      const eudaq::Event& ev = m_reader->GetEvent();
      if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

        B2DEBUG(75, "Detector Event created");

        TBTelEvent tbEvt = eudaq::PluginManager::ConvertToTBTelEvent(* detEv);
        EUDAQEventCollection.at(m_buffersize - 1) = CreateEUDAQEventTuple(tbEvt);
      }
      return true;
    }
  }

};


#endif /* EUDAQBUFFER_H_ */
