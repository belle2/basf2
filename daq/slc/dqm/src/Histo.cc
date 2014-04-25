#include "daq/slc/dqm/Histo.h"

#include <daq/slc/base/StringUtil.h>

#include <sstream>

using namespace Belle2;

const std::string Histo::ELEMENT_TAG = "histo";

Histo::Histo() throw() : MonObject(""), m_linked_axis(NULL)
{
  init("");
}

Histo::Histo(const Histo& histo) throw()
  : MonObject(histo.m_name), m_linked_axis(NULL)
{
  setTitle(histo.getTitle());
  m_axis_x.setTitle(histo.m_axis_x.getTitle());
  m_axis_y.setTitle(histo.m_axis_y.getTitle());
  m_axis_z.setTitle(histo.m_axis_z.getTitle());
}

Histo::Histo(const std::string& name, const std::string& title) throw()
  : MonObject(name), m_linked_axis(NULL)
{
  init(title);
}

Histo::~Histo() throw()
{
  if (m_data != NULL) delete m_data;
}

void Histo::init(const std::string& title) throw()
{
  std::stringstream ss[4];
  int ns = 0;
  const int size = title.size();
  const char* c = title.c_str();
  for (int m = 0; m < 4; m++) ss[m].str("");
  for (int n = 0; n < size; n++) {
    if (c[n] == ';') {
      ns++;
      if (ns == 4) break;
      continue;
    }
    ss[ns] << c[n];
  }
  m_title = ss[0].str();
  m_axis_x.setTitle(ss[1].str());
  m_axis_y.setTitle(ss[2].str());
  m_axis_z.setTitle(ss[3].str());
}

void Histo::fill(double vx) throw()
{
  int Nx = m_axis_x.getNbins() + 1;
  double dbinx = (m_axis_x.getMax() - m_axis_x.getMin()) / m_axis_x.getNbins();

  if (vx < m_axis_x.getMin()) {
    Nx = 0;
  } else if (vx <= m_axis_x.getMax()) {
    for (int nx = 0; nx < m_axis_x.getNbins() ; nx++) {
      if (vx >= dbinx * nx + m_axis_x.getMin() && vx <= dbinx * (nx + 1) + m_axis_x.getMin()) {
        Nx = nx + 1;
        break;
      }
    }
  }
  m_data->set(Nx, m_data->get(Nx) + 1);
}

void Histo::fill(double vx, double vy) throw()
{
  int Nx = m_axis_x.getNbins() + 1;
  int Ny = m_axis_y.getNbins() + 1;
  double dbinx = (m_axis_x.getMax() - m_axis_x.getMin()) / m_axis_x.getNbins();
  double dbiny = (m_axis_y.getMax() - m_axis_y.getMin()) / m_axis_y.getNbins();

  if (vx < m_axis_x.getMin()) {
    Nx = 0;
  } else if (vx <= m_axis_x.getMax()) {
    for (int nx = 0; nx < m_axis_x.getNbins() ; nx++) {
      if (vx >= dbinx * nx + m_axis_x.getMin() && vx <= dbinx * (nx + 1) + m_axis_x.getMin()) {
        Nx = nx + 1; break;
      }
    }
  }
  if (vy < m_axis_y.getMin()) {
    Ny = 0;
  } else if (vy <= m_axis_y.getMax()) {
    for (int ny = 0; ny < m_axis_y.getNbins() ; ny++) {
      if (vy >= dbiny * ny + m_axis_y.getMin() && vy <= dbiny * (ny + 1) + m_axis_y.getMin()) {
        Ny = ny + 1; break;
      }
    }
  }
  int N = Nx + (m_axis_x.getNbins() + 2) * Ny;
  m_data->set(N, m_data->get(N) + 1);
}

double Histo::getEntries() const throw()
{
  double entries = 0;
  for (size_t n = 0; n < m_data->size() ; n++) {
    entries += m_data->get(n);
  }
  return entries;
}

double Histo::getMean() const throw()
{
  double entries = getEntries();
  if (entries == 0) return -1;

  double mean = 0;
  for (int n = 0; n < m_axis_x.getNbins() ; n++) {
    mean += getBinContent(n) * (n + 0.5) * (m_axis_x.getMax() - m_axis_x.getMin()) / m_axis_x.getNbins();
  }
  return mean / entries;
}

double Histo::getRMS(int) const throw()
{
  return -1;
}

std::string Histo::toString() const throw()
{
  std::stringstream script("");
  script << "<histo data-type='" << getDataType() << "' "
         << "name='" << getName() << "' "
         << "title='" << getTitle() << "' "
         << "tab='" << (int) getTabId() << "' "
         << "position='" << (int) getPositionId() << "' " << std::endl
         << "data='" << m_data->toString() << "' >" << std::endl
         << "<axis-x nbins='" << getAxisX().getNbins() << "' "
         << "min='" << getAxisX().getMin() << "' "
         << "max='" << getAxisX().getMax() << "' "
         << "title='" << getAxisX().getTitle() << "' />" << std::endl
         << "<axis-y nbins='" << getAxisY().getNbins() << "' "
         << "min='" << getAxisY().getMin() << "' "
         << "max='" << getAxisY().getMax() << "' "
         << "title='" << getAxisY().getTitle() << "' />" << std::endl
         << "</histo>" << std::endl;
  return script.str();
}

std::string Histo::getTag() const throw()
{
  return ELEMENT_TAG;
}
std::string Histo::toXML() const throw()
{
  std::stringstream ss("");
  ss << "<histo "
     << "name='" << getName() << "' ";
  if (m_linked_axis != NULL) ss << "linked-axis='" << m_linked_axis->getName() << "' ";
  if (m_axis_y.isFixedMin()) ss << "minimum='" << m_axis_y.getMin() << "' ";
  if (m_axis_y.isFixedMax()) ss << "maximum='" << m_axis_y.getMax() << "' ";
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  if (m_draw_option.size() > 0) ss << "draw-option='" << m_draw_option << "' ";
  ss << ">" << std::endl
     << "<title><![CDATA["
     << StringUtil::replace(getTitle(), "\n", "<br/>")
     << "]]></title>" << std::endl
     << "</histo>" << std::endl;
  return ss.str();
}

void Histo::readObject(Reader& reader) throw(IOException)
{
  readConfig(reader);
  readContents(reader);
}

void Histo::readConfig(Reader& reader) throw(IOException)
{
  MonObject::readConfig(reader);
  m_title = reader.readString();
}

void Histo::writeObject(Writer& writer) const throw(IOException)
{
  writeConfig(writer);
  writeContents(writer);
}

void Histo::writeConfig(Writer& writer) const throw(IOException)
{
  MonObject::writeConfig(writer);
  writer.writeString(m_title);
}

void Histo::readContents(Reader& reader) throw(IOException)
{
  m_data->readObject(reader);
}

void Histo::writeContents(Writer& writer) const throw(IOException)
{
  m_data->writeObject(writer);
}

