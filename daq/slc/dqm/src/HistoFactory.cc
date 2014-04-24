#include "daq/slc/dqm/HistoFactory.h"

#include "daq/slc/dqm/Histo1C.h"
#include "daq/slc/dqm/Histo1S.h"
#include "daq/slc/dqm/Histo1I.h"
#include "daq/slc/dqm/Histo1F.h"
#include "daq/slc/dqm/Histo1D.h"
#include "daq/slc/dqm/Histo2C.h"
#include "daq/slc/dqm/Histo2S.h"
#include "daq/slc/dqm/Histo2I.h"
#include "daq/slc/dqm/Histo2F.h"
#include "daq/slc/dqm/Histo2D.h"
#include "daq/slc/dqm/Graph1C.h"
#include "daq/slc/dqm/Graph1S.h"
#include "daq/slc/dqm/Graph1I.h"
#include "daq/slc/dqm/Graph1F.h"
#include "daq/slc/dqm/Graph1D.h"
#include "daq/slc/dqm/TimedGraph1C.h"
#include "daq/slc/dqm/TimedGraph1S.h"
#include "daq/slc/dqm/TimedGraph1I.h"
#include "daq/slc/dqm/TimedGraph1F.h"
#include "daq/slc/dqm/TimedGraph1D.h"
#include "daq/slc/dqm/MonCircle.h"
#include "daq/slc/dqm/MonLabel.h"
#include "daq/slc/dqm/MonLine.h"
#include "daq/slc/dqm/MonRect.h"
#include "daq/slc/dqm/MonPieTable.h"
#include "daq/slc/dqm/MonArrow.h"
#include "daq/slc/dqm/HorizontalRange.h"
#include "daq/slc/dqm/VerticalRange.h"
#include "daq/slc/dqm/BevelRange.h"

using namespace Belle2;

MonObject* HistoFactory::create(const std::string& type)
{
  if (type.find("H1C") != std::string::npos) return new Histo1C();
  if (type.find("H1S") != std::string::npos) return new Histo1S();
  if (type.find("H1I") != std::string::npos) return new Histo1I();
  if (type.find("H1F") != std::string::npos) return new Histo1F();
  if (type.find("H1D") != std::string::npos) return new Histo1D();
  if (type.find("H2C") != std::string::npos) return new Histo2C();
  if (type.find("H2S") != std::string::npos) return new Histo2S();
  if (type.find("H2I") != std::string::npos) return new Histo2I();
  if (type.find("H2F") != std::string::npos) return new Histo2F();
  if (type.find("H2D") != std::string::npos) return new Histo2D();
  if (type.find("G1C") != std::string::npos) return new Graph1C();
  if (type.find("G1S") != std::string::npos) return new Graph1S();
  if (type.find("G1I") != std::string::npos) return new Graph1I();
  if (type.find("G1F") != std::string::npos) return new Graph1F();
  if (type.find("G1D") != std::string::npos) return new Graph1D();
  if (type.find("TGC") != std::string::npos) return new TimedGraph1C();
  if (type.find("TGS") != std::string::npos) return new TimedGraph1S();
  if (type.find("TGI") != std::string::npos) return new TimedGraph1I();
  if (type.find("TGF") != std::string::npos) return new TimedGraph1F();
  if (type.find("TGD") != std::string::npos) return new TimedGraph1D();
  if (type.find("MCC") != std::string::npos) return new MonCircle();
  if (type.find("MLN") != std::string::npos) return new MonLine();
  if (type.find("MLB") != std::string::npos) return new MonLabel();
  if (type.find("MRT") != std::string::npos) return new MonRect();
  if (type.find("MPT") != std::string::npos) return new MonPieTable();
  if (type.find("MAW") != std::string::npos) return new MonArrow();
  if (type.find("R1H") != std::string::npos) return new HorizontalRange();
  if (type.find("R1V") != std::string::npos) return new VerticalRange();
  if (type.find("R1B") != std::string::npos) return new BevelRange();
  return NULL;
}

MonObject* HistoFactory::create(Reader& reader) throw(IOException)
{
  const std::string type = reader.readString();
  MonObject* h = create(type);
  if (h != NULL) {
    h->readConfig(reader);
    return h;
  } else {
    throw (IOException("Unknown object type : \"%s\"", type.c_str()));
  }
}

MonObject* HistoFactory::createFull(Reader& reader) throw(IOException)
{
  std::string type = reader.readString();
  MonObject* h = create(type);
  if (h != NULL) {
    h->readObject(reader);
    return h;
  } else {
    throw (IOException("Unknown object type : \"%s\"", type.c_str()));
  }
}
