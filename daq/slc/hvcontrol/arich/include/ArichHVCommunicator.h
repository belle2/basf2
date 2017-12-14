#ifndef _Belle2_ArichHVCommunicator_h
#define _Belle2_ArichHVCommunicator_h

#include "daq/slc/hvcontrol/arich/ArichHVMessage.h"
#include "daq/slc/hvcontrol/arich/ArichHVUnit.h"

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>

#include <list>
#include <fstream>

namespace Belle2 {

  typedef std::vector<ArichHVUnit> ArichHVUnitList;
  typedef ArichHVUnitList::iterator ArichHVUnitListIter;

  class ArichHVCommunicator {

  public:
    ArichHVCommunicator() {}
    ArichHVCommunicator(int crateid, const std::string& host,
                        int port, int usech, int debug = 0);
    ~ArichHVCommunicator() throw() {}

  public:
    void connect() throw(IOException);

    void setSwitchOn(int unit, int ch, bool switchon) throw(IOException);
    void setRampUpSpeed(int unit, int ch, float rampup) throw(IOException);
    void setRampDownSpeed(int unit, int ch, float rampdown) throw(IOException);
    void setVoltageDemand(int unit, int ch, float voltage) throw(IOException);
    void setVoltageLimit(int unit, int ch, float voltage) throw(IOException);
    void setCurrentLimit(int unit, int ch, float current) throw(IOException);

    void requestValueAll(int unit, int ch) throw(IOException);
    void requestSwitch(int unit, int ch) throw(IOException);
    void requestRampUpSpeed(int unit, int ch) throw(IOException);
    void requestRampDownSpeed(int unit, int ch) throw(IOException);
    void requestVoltageDemand(int unit, int ch) throw(IOException);
    void requestVoltageLimit(int unit, int ch) throw(IOException);
    void requestCurrentLimit(int unit, int ch) throw(IOException);
    void requestVoltageMonitor(int unit, int ch) throw(IOException);
    void requestCurrentMonitor(int unit, int ch) throw(IOException);

    int getSwitch(int unit, int ch) throw(IOException);
    float getRampUpSpeed(int unit, int ch) throw(IOException);
    float getRampDownSpeed(int unit, int ch) throw(IOException);
    float getVoltageDemand(int unit, int ch) throw(IOException);
    float getVoltageLimit(int unit, int ch) throw(IOException);
    float getCurrentLimit(int unit, int ch) throw(IOException);
    int getState(int unit, int ch) throw(IOException);
    float getVoltageMonitor(int unit, int ch) throw(IOException);
    float getCurrentMonitor(int unit, int ch) throw(IOException);
    const HVValue& getAll(int unit, int ch) throw(IOException);

    void store(int index) throw(IOException);
    void recall(int index) throw(IOException);

    void switchOn(int unit, int ch) throw(IOException);
    void switchOff(int unit, int ch) throw(IOException);

  public:
    int getId() const { return m_crateid; }
    int getNUnits() const throw() { return m_unit.size(); }
    ArichHVUnitList getUnits() throw() { return m_unit; }
    const ArichHVUnitList getUnits() const throw() { return m_unit; }
    void addUnit(const ArichHVUnit& unit) { m_unit.push_back(unit); }
    ArichHVUnit& getUnit(int index) { return m_unit[index]; }
    const ArichHVUnit& getUnit(int index) const { return m_unit[index]; }
    bool useChannel() const { return m_usech; }

  public:
    std::list<ArichHVMessage> read() throw(IOException);
    void send(ArichHVMessage& msg) throw(IOException);
    ArichHVUnitListIter find(int unit, int channel);

  private:
    int m_crateid;
    TCPSocket m_socket;
    bool m_available;
    bool m_usech;
    int m_debug;
    Mutex m_mutex;
    TCPSocketWriter m_writer;
    TCPSocketReader m_reader;
    ArichHVUnitList m_unit;

  };

}

#endif
