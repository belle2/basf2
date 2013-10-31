#ifndef _Belle2_GAxis_hh
#define _Belle2_GAxis_hh

#include "Axis.h"
#include "Shape.h"

#include <map>
#include <vector>

namespace Belle2 {

  class GAxis : public Axis, public Shape {

  public:
    GAxis(const std::string& name = "", const std::string& title = "",
          int nbin = 1, double min = 0, double max = 1);
    virtual ~GAxis() throw();

  public:
    std::string getTag() const throw() { return _tag; }
    std::string toXML() const throw();

  public:
    void setTickLine(LineProperty* pro) throw();
    void setTickFont(FontProperty* pro) throw();
    void setLabelFont(FontProperty* pro) throw();
    void setColorPattern(const std::vector<MonColor>& pattern) throw();
    void setColorPattern(int ncolors, const MonColor* pattern) throw();
    void setLabels(const std::vector<std::string>& label,
                   const std::vector<double>& value) throw();
    void setLabels(int nlabels, const std::string* label,
                   const double* value) throw();
    void setTag(const std::string& tag) throw() { _tag = tag; }
    void setLogScale(bool log_scale) throw() { _log_scale = log_scale; }
    void setTickLength(float length) throw() { _tick_length = length; }
    void setTitleOffset(float offset) throw() { _title_offset = offset; }
    void setLabelOffset(float offset) throw() { _label_offset = offset; }
    void setTimeFormat(const std::string& time_format) throw() {
      _time_format = time_format;
    }
    void setPosition(const std::string& position) throw() {
      _position = position;
    }
    void setX(float x) throw() { _x = x; }
    void setY(float y) throw() { _y = y; }
    void setPaddingX(float padding_x) throw() { _padding_x = padding_x; }
    void setPaddingY(float padding_y) throw() { _padding_y = padding_y; }

  private:
    LineProperty* _tick_line_pro;
    FontProperty* _tick_font_pro;
    FontProperty* _label_font_pro;
    std::string _tag;
    std::string _color_pattern;
    std::string _labels;
    bool _log_scale;
    float _title_offset;
    float _label_offset;
    std::string _time_format;
    std::string _position;
    float _tick_length;
    float _x, _y, _padding_x, _padding_y;

  };

};

#endif
