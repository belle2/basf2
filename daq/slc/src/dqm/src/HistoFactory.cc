#include "HistoFactory.hh"

#include "Histo1C.hh"
#include "Histo1S.hh"
#include "Histo1I.hh"
#include "Histo1F.hh"
#include "Histo1D.hh"
#include "Histo2C.hh"
#include "Histo2S.hh"
#include "Histo2I.hh"
#include "Histo2F.hh"
#include "Histo2D.hh"
#include "Graph1C.hh"
#include "Graph1S.hh"
#include "Graph1I.hh"
#include "Graph1F.hh"
#include "Graph1D.hh"
#include "TimedGraph1C.hh"
#include "TimedGraph1S.hh"
#include "TimedGraph1I.hh"
#include "TimedGraph1F.hh"
#include "TimedGraph1D.hh"
#include "MonCircle.hh"
#include "MonLabel.hh"
#include "MonLine.hh"
#include "MonRect.hh"
#include "MonPieTable.hh"
#include "MonArrow.hh"
#include "HorizontalRange.hh"
#include "VerticalRange.hh"
#include "BevelRange.hh"

using namespace B2DQM;

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

MonObject* HistoFactory::create(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  const std::string type = reader.readString();
  MonObject* h = create(type);
  if (h != NULL) {
    h->readConfig(reader);
    return h;
  } else {
    throw (B2DAQ::IOException(__FILE__, __LINE__,
                              std::string("Illigal data type : \"") + type + "\""));
  }
}

MonObject* HistoFactory::createFull(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  std::string type = reader.readString();
  MonObject* h = create(type);
  if (h != NULL) {
    h->readObject(reader);
    return h;
  } else {
    throw (B2DAQ::IOException(__FILE__, __LINE__,
                              std::string("Illigal data type : \"") + type + "\""));
  }
}
