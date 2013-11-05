#ifndef _Belle2_CanvasPanel_hh
#define _Belle2_CanvasPanel_hh

#include "dqm/GAxis.h"
#include "dqm/Shape.h"
#include "dqm/Panel.h"
#include "dqm/Pad.h"
#include "dqm/Legend.h"
#include "dqm/Graph1.h"

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
    std::vector<Shape*>& getObjects() throw() { return _object_v; }
    Shape* getObject(size_t n) throw() { return _object_v[n]; }
    Graph1* addGraph(int npoints) throw();
    GAxis* getXAxis() throw() { return _x_axis; }
    GAxis* getYAxis() throw() { return _y_axis; }
    GAxis* getY2Axis() throw() { return _y2_axis; }
    GAxis* getColorAxis() throw() { return _color_axis; }

  private:
    FillProperty* _fill;
    std::vector<Shape*> _object_v;
    GAxis* _x_axis;
    GAxis* _y2_axis;
    GAxis* _y_axis;
    GAxis* _color_axis;
    Pad* _pad;
    Legend* _legend;
    std::string _title;
    std::string _title_position;
    std::vector<Graph1*> _graph_v;

  };

};

#endif
