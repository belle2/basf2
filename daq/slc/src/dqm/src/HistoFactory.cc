#include "dqm/HistoFactory.h"

#include "dqm/Histo1C.h"
#include "dqm/Histo1S.h"
#include "dqm/Histo1I.h"
#include "dqm/Histo1F.h"
#include "dqm/Histo1D.h"
#include "dqm/Histo2C.h"
#include "dqm/Histo2S.h"
#include "dqm/Histo2I.h"
#include "dqm/Histo2F.h"
#include "dqm/Histo2D.h"
#include "dqm/Graph1C.h"
#include "dqm/Graph1S.h"
#include "dqm/Graph1I.h"
#include "dqm/Graph1F.h"
#include "dqm/Graph1D.h"
#include "dqm/TimedGraph1C.h"
#include "dqm/TimedGraph1S.h"
#include "dqm/TimedGraph1I.h"
#include "dqm/TimedGraph1F.h"
#include "dqm/TimedGraph1D.h"
#include "dqm/MonCircle.h"
#include "dqm/MonLabel.h"
#include "dqm/MonLine.h"
#include "dqm/MonRect.h"
#include "dqm/MonPieTable.h"
#include "dqm/MonArrow.h"
#include "dqm/HorizontalRange.h"
#include "dqm/VerticalRange.h"
#include "dqm/BevelRange.h"

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

MonObject* HistoFactory::create(Belle2::Reader& reader) throw(Belle2::IOException)
{
  const std::string type = reader.readString();
  MonObject* h = create(type);
  if (h != NULL) {
    h->readConfig(reader);
    return h;
  } else {
    throw (Belle2::IOException(__FILE__, __LINE__,
                               std::string("Illigal data type : \"") + type + "\""));
  }
}

MonObject* HistoFactory::createFull(Belle2::Reader& reader) throw(Belle2::IOException)
{
  std::string type = reader.readString();
  MonObject* h = create(type);
  if (h != NULL) {
    h->readObject(reader);
    return h;
  } else {
    throw (Belle2::IOException(__FILE__, __LINE__,
                               std::string("Illigal data type : \"") + type + "\""));
  }
}
