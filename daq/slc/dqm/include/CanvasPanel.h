#ifndef _Belle2_CanvasPanel_h
#define _Belle2_CanvasPanel_h

#include "daq/slc/dqm/GAxis.h"
#include "daq/slc/dqm/Shape.h"
#include "daq/slc/dqm/Panel.h"
#include "daq/slc/dqm/Pad.h"
#include "daq/slc/dqm/Legend.h"
#include "daq/slc/dqm/Graph1.h"

namespace Belle2 {

  class CanvasPanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;

  public:
    CanvasPanel() throw();
    CanvasPanel(const std::string& name,
                const std::string& title = "") throw();
    virtual ~CanvasPanel() throw();

  public:
    void add(Shape* obj) throw();
    void setFill(FillProperty* pro) throw();
    void setFill(const MonColor& color) throw();
    void setXAxis(GAxis* axis) throw();
    void setYAxis(GAxis* axis) throw();
    void setY2Axis(GAxis* axis) throw();
    void setColorAxis(GAxis* axis) throw();
    void setPad(Pad* pad) throw();
    void setLegend(Legend* legend) throw();
    void setTitle(const std::string& title) throw();
    void setTitlePosition(const std::string& title_position) throw();
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();
    std::vector<Shape*>& getObjects() throw() { return m_object_v; }
    Shape* getObject(size_t n) throw() { return m_object_v[n]; }
    Graph1* addGraph(int npoints) throw();
    GAxis* getXAxis() throw() { return m_x_axis; }
    GAxis* getYAxis() throw() { return m_y_axis; }
    GAxis* getY2Axis() throw() { return m_y2_axis; }
    GAxis* getColorAxis() throw() { return m_color_axis; }

  private:
    FillProperty* m_fill;
    std::vector<Shape*> m_object_v;
    GAxis* m_x_axis;
    GAxis* m_y2_axis;
    GAxis* m_y_axis;
    GAxis* m_color_axis;
    Pad* m_pad;
    Legend* m_legend;
    std::string m_title;
    std::string m_title_position;
    std::vector<Graph1*> m_graph_v;

  };

};

#endif
