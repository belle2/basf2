#include "daq/slc/dqm/Histo.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

const std::string Histo::ELEMENT_TAG = "histo";

Histo::Histo() throw() : MonObject(""), _linked_axis(NULL)
{
  init("");
}

Histo::Histo(const Histo& histo) throw()
  : MonObject(histo._name), _linked_axis(NULL)
{
  setTitle(histo.getTitle());
  _axis_x.setTitle(histo._axis_x.getTitle());
  _axis_y.setTitle(histo._axis_y.getTitle());
  _axis_z.setTitle(histo._axis_z.getTitle());
}

Histo::Histo(const std::string& name, const std::string& title) throw()
  : MonObject(name), _linked_axis(NULL)
{
  init(title);
}

Histo::~Histo() throw()
{
  if (_data != NULL) delete _data;
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
  _title = ss[0].str();
  _axis_x.setTitle(ss[1].str());
  _axis_y.setTitle(ss[2].str());
  _axis_z.setTitle(ss[3].str());
}

void Histo::fill(double vx) throw()
{
  int Nx = _axis_x.getNbins() + 1;
  double dbinx = (_axis_x.getMax() - _axis_x.getMin()) / _axis_x.getNbins();

  if (vx < _axis_x.getMin()) {
    Nx = 0;
  } else if (vx <= _axis_x.getMax()) {
    for (int nx = 0; nx < _axis_x.getNbins() ; nx++) {
      if (vx >= dbinx * nx + _axis_x.getMin() && vx <= dbinx * (nx + 1) + _axis_x.getMin()) {
        Nx = nx + 1;
        break;
      }
    }
  }
  _data->set(Nx, _data->get(Nx) + 1);
}

void Histo::fill(double vx, double vy) throw()
{
  int Nx = _axis_x.getNbins() + 1;
  int Ny = _axis_y.getNbins() + 1;
  double dbinx = (_axis_x.getMax() - _axis_x.getMin()) / _axis_x.getNbins();
  double dbiny = (_axis_y.getMax() - _axis_y.getMin()) / _axis_y.getNbins();

  if (vx < _axis_x.getMin()) {
    Nx = 0;
  } else if (vx <= _axis_x.getMax()) {
    for (int nx = 0; nx < _axis_x.getNbins() ; nx++) {
      if (vx >= dbinx * nx + _axis_x.getMin() && vx <= dbinx * (nx + 1) + _axis_x.getMin()) {
        Nx = nx + 1; break;
      }
    }
  }
  if (vy < _axis_y.getMin()) {
    Ny = 0;
  } else if (vy <= _axis_y.getMax()) {
    for (int ny = 0; ny < _axis_y.getNbins() ; ny++) {
      if (vy >= dbiny * ny + _axis_y.getMin() && vy <= dbiny * (ny + 1) + _axis_y.getMin()) {
        Ny = ny + 1; break;
      }
    }
  }
  int N = Nx + (_axis_x.getNbins() + 2) * Ny;
  _data->set(N, _data->get(N) + 1);
}

double Histo::getEntries() const throw()
{
  double entries = 0;
  for (size_t n = 0; n < _data->size() ; n++) {
    entries += _data->get(n);
  }
  return entries;
}

double Histo::getMean() const throw()
{
  double entries = getEntries();
  if (entries == 0) return -1;

  double mean = 0;
  for (int n = 0; n < _axis_x.getNbins() ; n++) {
    mean += getBinContent(n) * (n + 0.5) * (_axis_x.getMax() - _axis_x.getMin()) / _axis_x.getNbins();
  }
  return mean / entries;
}

double Histo::getRMS(int axis) const throw()
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
         << "data='" << _data->toString() << "' >" << std::endl
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
  if (_linked_axis != NULL) ss << "linked-axis='" << _linked_axis->getName() << "' ";
  if (_axis_y.isFixedMin()) ss << "minimum='" << _axis_y.getMin() << "' ";
  if (_axis_y.isFixedMax()) ss << "maximum='" << _axis_y.getMax() << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  if (_draw_option.size() > 0) ss << "draw-option='" << _draw_option << "' ";
  ss << ">" << std::endl
     << "<title><![CDATA["
     << Belle2::replace(getTitle(), "\n", "<br/>")
     << "]]></title>" << std::endl
     << "</histo>" << std::endl;
  return ss.str();
}

void Histo::readObject(Belle2::Reader& reader) throw(Belle2::IOException)
{
  readConfig(reader);
  readContents(reader);
}

void Histo::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
  _title = reader.readString();
}

void Histo::writeObject(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  writeConfig(writer);
  writeContents(writer);
}

void Histo::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
  writer.writeString(_title);
}

void Histo::readContents(Belle2::Reader& reader) throw(Belle2::IOException)
{
  _data->readObject(reader);
}

void Histo::writeContents(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  _data->writeObject(writer);
}

