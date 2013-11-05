#include "HistoPackage.h"

#include "HistoFactory.h"

#include "system/Time.h"

#include "base/Date.h"

#include <sstream>

using namespace Belle2;

const char HistoPackage::ObjectFlag = 0xDC;

HistoPackage::HistoPackage()
{
  _name = "";
  _update_id = 0;
}

HistoPackage::HistoPackage(const std::string& name)
{
  _name = name;
  _update_id = 0;
}

HistoPackage::~HistoPackage() throw()
{
  for (size_t n = 0; n < _histo_v.size(); n++) {
    delete _histo_v.at(n);
  }
}

MonObject* HistoPackage::setMonObject(int index, MonObject* h)
{
  _histo_v.at(index) = h;
  return h;
}

MonObject* HistoPackage::addMonObject(MonObject* h)
{
  _histo_v.push_back(h);
  return h;
}

Histo* HistoPackage::setHisto(int index, Histo* h)
{
  _histo_v.at(index) = h;
  return h;
}

Histo* HistoPackage::addHisto(Histo* h)
{
  _histo_v.push_back(h);
  return h;
}

Histo* HistoPackage::getHisto(const std::string& name) throw()
{
  for (size_t n = 0; n < _histo_v.size(); n++) {
    if (_histo_v[n]->getName() == name &&
        (_histo_v[n]->getDataType().find("H1") != std::string::npos ||
         _histo_v[n]->getDataType().find("H2") != std::string::npos)) {
      return (Histo*)_histo_v[n];
    }
  }
  return NULL;
}

void HistoPackage::reset() throw()
{
  _update_id = 0;
  for (int n = 0; n < getNHistos(); n++) {
    _histo_v.at(n)->reset();
  }
}

void HistoPackage::clear() throw()
{
  reset();
  _name = "";
  for (size_t n = 0; n < _histo_v.size(); n++) {
    delete _histo_v.at(n);
  }
  _histo_v = std::vector<MonObject*>();
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

void HistoPackage::writeObject(Belle2::Writer& writer) const
throw(Belle2::IOException)
{
  writeConfig(writer);
  writeContents(writer, true);
}

void HistoPackage::writeConfig(Belle2::Writer& writer) const
throw(Belle2::IOException)
{
  writer.writeString(getName());
  writer.writeInt(getNHistos());
  for (int n = 0; n < getNHistos(); n++) {
    writer.writeInt(n);
    writer.writeString(getMonObject(n)->getDataType());
    getMonObject(n)->writeConfig(writer);
  }
}

void HistoPackage::writeContents(Belle2::Writer& writer,
                                 bool updateAll) const
throw(Belle2::IOException)
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

void HistoPackage::readObject(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  readConfig(reader);
  readContents(reader);
}

void HistoPackage::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  clear();
  setName(reader.readString());
  int nhistos = reader.readInt();
  int hid;
  for (int n = 0; n < nhistos; n++) {
    hid = reader.readInt();
    if (hid < 0 || hid > getNHistos()) {
      std::stringstream ss;
      ss << "Illigal histo number (n = " << n << ", hid = " << hid << ")";
      throw (Belle2::IOException(__FILE__, __LINE__, ss.str()));
    }
    addMonObject(HistoFactory::create(reader));
  }
}

void HistoPackage::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
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
      throw (Belle2::IOException(__FILE__, __LINE__, ""));
    }
    if (hid < 0 || hid > getNHistos()) {
      std::stringstream ss;
      ss << "Illigal histo number (n = " << n << ", hid = " << hid << ")";
      throw (Belle2::IOException(__FILE__, __LINE__, ss.str()));
    }
    getMonObject(hid)->readContents(reader);
    getMonObject(hid)->setUpdated(true);
  }
}

