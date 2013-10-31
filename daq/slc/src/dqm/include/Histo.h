#ifndef _Belle2_Histo_hh
#define _Belle2_Histo_hh

#include "MonObject.h"

#include "GAxis.h"
#include "NumberArray.h"

#include <string>

namespace Belle2 {

  class Histo : public MonObject {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Histo() throw();
    Histo(const Histo& h) throw();
    Histo(const std::string& name, const std::string& title) throw();
    virtual ~Histo() throw();

  protected:
    std::string _title;
    Axis _axis_x;
    Axis _axis_y;
    Axis _axis_z;
    NumberArray* _data;
    std::string _draw_option;
    GAxis* _linked_axis;

  protected:
    void init(const std::string& title) throw();

  public:
    const std::string& getTitle() const throw() {  return _title;  }
    const Axis& getAxisX() const throw() {  return _axis_x;  }
    const Axis& getAxisY() const throw() {  return _axis_y;  }
    const Axis& getAxisZ() const throw() {  return _axis_z;  }
    Axis& getAxisX() throw() {  return _axis_x;  }
    Axis& getAxisY() throw() {  return _axis_y;  }
    Axis& getAxisZ() throw() {  return _axis_z;  }

    void setTitle(const std::string& title) throw() { _title = title;  }
    void setAxisX(const Axis& axis) throw() { _axis_x = axis;  }
    void setAxisY(const Axis& axis) throw() { _axis_y = axis;  }
    void setAxisZ(const Axis& axis) throw() { _axis_z = axis;  }

    void fill(double v) throw();
    void fill(double vx, double vy) throw();
    double getEntries() const throw();
    double getMean() const throw();
    double getRMS(int axis = 1) const throw();
    virtual void reset() throw() {}
    virtual void resetAxis(int nbinx, double xmin, double xmax) {}
    virtual void resetAxis(int nbinx, double xmin, double xmax,
                           int nbiny, double ymin, double ymax) {}
    virtual double getLatestPoint() const throw() { return 0; }
    virtual double getPoint(int) const throw() { return 0; }
    virtual double getPointX(int) const throw() { return 0; }
    virtual double getPointY(int) const throw() { return 0; }
    virtual double getBinContent(int) const throw() { return 0; }
    virtual double getBinContent(int, int) const throw() { return 0; }
    virtual double getOverFlow() const throw() { return 0; }
    virtual double getOverFlowX(int) const throw() { return 0; }
    virtual double getOverFlowY(int) const throw() { return 0; }
    virtual double getUnderFlow() const throw() { return 0; }
    virtual double getUnderFlowX(int) const throw() { return 0; }
    virtual double getUnderFlowY(int) const throw() { return 0; }
    virtual void addPoint(double) throw() {}
    virtual void setPoint(int, double) throw() {}
    virtual void setPointX(int, double) throw() {}
    virtual void setPointY(int, double) throw() {}
    virtual void setBinContent(int, double) throw() {}
    virtual void setBinContent(int, int, double) throw() {}
    virtual void setOverFlow(double) throw() {}
    virtual void setOverFlowX(int, double) throw() {}
    virtual void setOverFlowY(int, double) throw() {}
    virtual void setUnderFlow(double) throw() {}
    virtual void setUnderFlowX(int, double) throw() {}
    virtual void setUnderFlowY(int, double) throw() {}

    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);

    virtual double getMaximum() const throw() = 0;
    virtual double getMinimum() const throw() = 0;
    virtual void setMaximum(double data) throw() = 0;
    virtual void setMinimum(double data) throw() = 0;
    virtual void fixMaximum(double data) throw() = 0;
    virtual void fixMinimum(double data) throw() = 0;
    virtual int getDim() const throw() = 0;
    NumberArray& getData() throw() { return *_data; }
    const NumberArray& getData() const throw() { return *_data; }

  public:
    virtual std::string getTag() const throw();
    virtual std::string toXML() const throw();
    virtual std::string toString() const throw();
    void setDrawOption(const std::string& option) {
      _draw_option = option;
    }
    void setLinkedAxis(GAxis* axis) {
      _linked_axis = axis;
    }

  };

};

#endif
