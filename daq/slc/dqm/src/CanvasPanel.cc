#include "daq/slc/dqm/CanvasPanel.h"

#include "daq/slc/dqm/Graph1D.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

const std::string CanvasPanel::ELEMENT_TAG = "canvas";

CanvasPanel::CanvasPanel() throw()
  : Panel(), _fill(NULL), _object_v(), _x_axis(NULL),
    _y2_axis(NULL), _y_axis(NULL), _color_axis(NULL),
    _pad(NULL), _legend(NULL) {}

CanvasPanel::CanvasPanel(const std::string& name,
                         const std::string& title) throw()
  : Panel(name), _fill(NULL), _object_v(), _x_axis(NULL),
    _y2_axis(NULL), _y_axis(NULL), _color_axis(NULL),
    _pad(NULL), _legend(NULL), _title(title) {}

CanvasPanel::~CanvasPanel() throw() {}

void CanvasPanel::add(Shape* obj) throw()
{
  if (obj != NULL) {
    _object_v.push_back(obj);
  }
}

void CanvasPanel::setFill(FillProperty* pro) throw()
{
  if (pro != NULL) {
    _fill = pro;
  }
}

void CanvasPanel::setFill(const MonColor& color) throw()
{
  if (_fill == NULL) _fill = new FillProperty(color);
  else _fill->setColor(color);
}

void CanvasPanel::setXAxis(GAxis* axis) throw()
{
  if (axis != NULL) {
    _x_axis = axis;
    _x_axis->setTag("x-axis");
  }
}

void CanvasPanel::setYAxis(GAxis* axis) throw()
{
  if (axis != NULL) {
    _y_axis = axis;
    _y_axis->setTag("y-axis");
  }
}

void CanvasPanel::setY2Axis(GAxis* axis) throw()
{
  if (axis != NULL) {
    _y2_axis = axis;
    _y2_axis->setTag("y2-axis");
  }
}

void CanvasPanel::setColorAxis(GAxis* axis) throw()
{
  if (axis != NULL) {
    _color_axis = axis;
    _color_axis->setTag("color-axis");
  }
}

void CanvasPanel::setPad(Pad* pad) throw()
{
  if (pad != NULL) {
    _pad = pad;
  }
}

void CanvasPanel::setLegend(Legend* legend) throw()
{
  if (legend != NULL) {
    _legend = legend;
  }
}

void CanvasPanel::setTitle(const std::string& title) throw()
{
  _title = title;
}

void CanvasPanel::setTitlePosition(const std::string& title_position) throw()
{
  _title_position = title_position;
}

Graph1* CanvasPanel::addGraph(int npoints) throw()
{
  Graph1* g = new Graph1D("", "", npoints, 0, 1, 0, 1);
  _graph_v.push_back(g);
  return g;
}

std::string CanvasPanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << _name << "' ";
  if (_fill != NULL) ss << _fill->toString();
  ss << "title-position='" << _title_position << "'"">\n";
  if (_title.size() > 0) {
    ss << "<title><![CDATA["
       << StringUtil::replace(_title, "\n", "<br/>")
       << "]]></title>" << std::endl;
  }
  for (size_t n = 0; n < _object_v.size(); n++) {
    if (_object_v[n] != NULL) ss << _object_v[n]->toXML();
  }
  if (_x_axis != NULL) ss << _x_axis->toXML();
  if (_y_axis != NULL) ss << _y_axis->toXML();
  if (_y2_axis != NULL) ss << _y2_axis->toXML();
  if (_color_axis != NULL) ss << _color_axis->toXML();
  if (_pad != NULL) ss << _pad->toXML();
  if (_legend != NULL) ss << _legend->toXML();
  for (size_t n = 0; n < _graph_v.size(); n++) {
    ss << "<graph name='" << _graph_v[n]->getName() << "' ";
    ss << "draw='";
    for (size_t m = 0; m < _graph_v[n]->getData().size() / 2; m++) {
      ss << _graph_v[n]->getPointX(m) << "," << _graph_v[n]->getPointY(m);
      if (m < _graph_v[n]->getData().size() / 2 - 1) {
        ss << ":";
      }
    }
    ss << "' ";
    if (_graph_v[n]->getFill() != NULL)
      ss << _graph_v[n]->getFill()->toString();
    if (_graph_v[n]->getLine() != NULL)
      ss << _graph_v[n]->getLine()->toString();
    ss << "/>" << std::endl;
  }
  ss << "</" << ELEMENT_TAG << ">" << std::endl;
  return ss.str();
}
