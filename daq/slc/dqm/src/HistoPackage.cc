#include "daq/slc/dqm/HistoPackage.h"

#include "daq/slc/dqm/HistoFactory.h"

#include "daq/slc/base/Date.h"
#include "daq/slc/system/Time.h"

#include <sstream>

using namespace Belle2;

const char HistoPackage::ObjectFlag = 0xDC;

HistoPackage::HistoPackage()
{
  m_name = "";
  m_update_id = 0;
}

HistoPackage::HistoPackage(const std::string& name)
{
  m_name = name;
  m_update_id = 0;
}

HistoPackage::~HistoPackage() throw()
{
  for (size_t n = 0; n < m_histo_v.size(); n++) {
    delete m_histo_v.at(n);
  }
}

MonObject* HistoPackage::setMonObject(int index, MonObject* h)
{
  m_histo_v.at(index) = h;
  return h;
}

MonObject* HistoPackage::addMonObject(MonObject* h)
{
  m_histo_v.push_back(h);
  return h;
}

Histo* HistoPackage::setHisto(int index, Histo* h)
{
  m_histo_v.at(index) = h;
  return h;
}

Histo* HistoPackage::addHisto(Histo* h)
{
  m_histo_v.push_back(h);
  return h;
}

Histo* HistoPackage::getHisto(const std::string& name) throw()
{
  for (size_t n = 0; n < m_histo_v.size(); n++) {
    if (m_histo_v[n]->getName() == name &&
        (m_histo_v[n]->getDataType().find("H1") != std::string::npos ||
         m_histo_v[n]->getDataType().find("H2") != std::string::npos)) {
      return (Histo*)m_histo_v[n];
    }
  }
  return NULL;
}

void HistoPackage::reset() throw()
{
  m_update_id = 0;
  for (int n = 0; n < getNHistos(); n++) {
    m_histo_v.at(n)->reset();
  }
}

void HistoPackage::clear() throw()
{
  reset();
  m_name = "";
  for (size_t n = 0; n < m_histo_v.size(); n++) {
    delete m_histo_v.at(n);
  }
  m_histo_v = std::vector<MonObject*>();
}

bool HistoPackage::hasHisto(const std::string& name) throw()
{
  for (size_t n = 0; n < m_histo_v.size(); n++) {
    if (m_histo_v.at(n)->getName() == name) return true;
  }
  return false;
}

std::string HistoPackage::toString() const throw()
{
  std::stringstream script;
  script << "<?xml version='1.0'?>" << std::endl;
  script << "<package name='" + getName() + "' " << std::endl
         << "update-id='" << getUpdateId() << "' >" << std::endl;
  for (int n = 0; n < getNHistos(); n++) {
    script << getMonObject(n)->toString();
  }
  script << "</package>" << std::endl;

  return script.str();
}

void HistoPackage::writeObject(Writer& writer) const
throw(IOException)
{
  writeConfig(writer);
  writeContents(writer, true);
}

void HistoPackage::writeConfig(Writer& writer) const
throw(IOException)
{
  writer.writeString(getName());
  writer.writeInt(getNHistos());
  for (int n = 0; n < getNHistos(); n++) {
    writer.writeInt(n);
    writer.writeString(getMonObject(n)->getDataType());
    getMonObject(n)->writeConfig(writer);
  }
}

void HistoPackage::writeContents(Writer& writer,
                                 bool updateAll) const
throw(IOException)
{
  writer.writeInt(getUpdateId());
  writer.writeLong(getUpdateTime());
  size_t nupdated = 0;
  for (int n = 0; n < getNHistos(); n++) {
    if (updateAll || getMonObject(n)->isUpdated()) {
      nupdated++;
    }
  }
  writer.writeInt(nupdated);
  for (int n = 0; n < getNHistos(); n++) {
    if (updateAll || getMonObject(n)->isUpdated()) {
      writer.writeChar(ObjectFlag);
      writer.writeInt(n);
      getMonObject(n)->writeContents(writer);
    }
  }
}

void HistoPackage::readObject(Reader& reader)
throw(IOException)
{
  readConfig(reader);
  readContents(reader);
}

void HistoPackage::readConfig(Reader& reader)
throw(IOException)
{
  clear();
  setName(reader.readString());
  int nhistos = reader.readInt();
  int hid;
  for (int n = 0; n < nhistos; n++) {
    hid = reader.readInt();
    if (hid < 0 || hid > getNHistos()) {
      throw (IOException("Illigal histo number (n = %d, hid = %d)", n, hid));
    }
    addMonObject(HistoFactory::create(reader));
  }
}

void HistoPackage::readContents(Reader& reader)
throw(IOException)
{
  setUpdateId(reader.readInt());
  setUpdateTime(reader.readLong());
  int nhistos = reader.readInt();
  char c;
  int hid;
  for (int n = 0; n < nhistos; n++) {
    c = reader.readChar();
    hid = reader.readInt();
    if (c != ObjectFlag) {
      throw (IOException("Illigal object flag:%d", (int)c));
    }
    if (hid < 0 || hid > getNHistos()) {
      throw (IOException("Illigal histo number (n = %d, hid = %d)", n, hid));
    }
    getMonObject(hid)->readContents(reader);
    getMonObject(hid)->setUpdated(true);
  }
}

