#include "daq/slc/dqm/CanvasPanel.h"

#include "daq/slc/dqm/Graph1D.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

const std::string CanvasPanel::ELEMENT_TAG = "canvas";

CanvasPanel::CanvasPanel() throw()
  : Panel(), m_fill(NULL), m_object_v(), m_x_axis(NULL),
    m_y2_axis(NULL), m_y_axis(NULL), m_color_axis(NULL),
    m_pad(NULL), m_legend(NULL) {}

CanvasPanel::CanvasPanel(const std::string& name,
                         const std::string& title) throw()
  : Panel(name), m_fill(NULL), m_object_v(), m_x_axis(NULL),
    m_y2_axis(NULL), m_y_axis(NULL), m_color_axis(NULL),
    m_pad(NULL), m_legend(NULL), m_title(title) {}

CanvasPanel::~CanvasPanel() throw() {}

void CanvasPanel::add(Shape* obj) throw()
{
  if (obj != NULL) {
    m_object_v.push_back(obj);
  }
}

void CanvasPanel::setFill(FillProperty* pro) throw()
{
  if (pro != NULL) {
    m_fill = pro;
  }
}

void CanvasPanel::setFill(const MonColor& color) throw()
{
  if (m_fill == NULL) m_fill = new FillProperty(color);
  else m_fill->setColor(color);
}

void CanvasPanel::setXAxis(GAxis* axis) throw()
{
  if (axis != NULL) {
    m_x_axis = axis;
    m_x_axis->setTag("x-axis");
  }
}

void CanvasPanel::setYAxis(GAxis* axis) throw()
{
  if (axis != NULL) {
    m_y_axis = axis;
    m_y_axis->setTag("y-axis");
  }
}

void CanvasPanel::setY2Axis(GAxis* axis) throw()
{
  if (axis != NULL) {
    m_y2_axis = axis;
    m_y2_axis->setTag("y2-axis");
  }
}

void CanvasPanel::setColorAxis(GAxis* axis) throw()
{
  if (axis != NULL) {
    m_color_axis = axis;
    m_color_axis->setTag("color-axis");
  }
}

void CanvasPanel::setPad(Pad* pad) throw()
{
  if (pad != NULL) {
    m_pad = pad;
  }
}

void CanvasPanel::setLegend(Legend* legend) throw()
{
  if (legend != NULL) {
    m_legend = legend;
  }
}

void CanvasPanel::setTitle(const std::string& title) throw()
{
  m_title = title;
}

void CanvasPanel::setTitlePosition(const std::string& title_position) throw()
{
  m_title_position = title_position;
}

Graph1* CanvasPanel::addGraph(int npoints) throw()
{
  Graph1* g = new Graph1D("", "", npoints, 0, 1, 0, 1);
  m_graph_v.push_back(g);
  return g;
}

std::string CanvasPanel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << m_name << "' ";
  if (m_fill != NULL) ss << m_fill->toString();
  ss << "title-position='" << m_title_position << "'"">\n";
  if (m_title.size() > 0) {
    ss << "<title><![CDATA["
       << StringUtil::replace(m_title, "\n", "<br/>")
       << "]]></title>" << std::endl;
  }
  for (size_t n = 0; n < m_object_v.size(); n++) {
    if (m_object_v[n] != NULL) ss << m_object_v[n]->toXML();
  }
  if (m_x_axis != NULL) ss << m_x_axis->toXML();
  if (m_y_axis != NULL) ss << m_y_axis->toXML();
  if (m_y2_axis != NULL) ss << m_y2_axis->toXML();
  if (m_color_axis != NULL) ss << m_color_axis->toXML();
  if (m_pad != NULL) ss << m_pad->toXML();
  if (m_legend != NULL) ss << m_legend->toXML();
  for (size_t n = 0; n < m_graph_v.size(); n++) {
    ss << "<graph name='" << m_graph_v[n]->getName() << "' ";
    ss << "draw='";
    for (size_t m = 0; m < m_graph_v[n]->getData().size() / 2; m++) {
      ss << m_graph_v[n]->getPointX(m) << "," << m_graph_v[n]->getPointY(m);
      if (m < m_graph_v[n]->getData().size() / 2 - 1) {
        ss << ":";
      }
    }
    ss << "' ";
    if (m_graph_v[n]->getFill() != NULL)
      ss << m_graph_v[n]->getFill()->toString();
    if (m_graph_v[n]->getLine() != NULL)
      ss << m_graph_v[n]->getLine()->toString();
    ss << "/>" << std::endl;
  }
  ss << "</" << ELEMENT_TAG << ">" << std::endl;
  return ss.str();
}
