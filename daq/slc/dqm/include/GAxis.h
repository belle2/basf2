#ifndef _Belle2_GAxis_h
#define _Belle2_GAxis_h

#include "daq/slc/dqm/Axis.h"
#include "daq/slc/dqm/Shape.h"

#include <map>
#include <vector>

namespace Belle2 {

  class GAxis : public Axis, public Shape {

  public:
    GAxis(const std::string& name = "", const std::string& title = "",
          int nbin = 1, double min = 0, double max = 1);
    virtual ~GAxis() throw();

  public:
    std::string getTag() const throw() { return m_tag; }
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
    void setTag(const std::string& tag) throw() { m_tag = tag; }
    void setLogScale(bool log_scale) throw() { m_log_scale = log_scale; }
    void setTickLength(float length) throw() { m_tick_length = length; }
    void setTitleOffset(float offset) throw() { m_title_offset = offset; }
    void setLabelOffset(float offset) throw() { m_label_offset = offset; }
    void setTimeFormat(const std::string& time_format) throw() {
      m_time_format = time_format;
    }
    void setPosition(const std::string& position) throw() {
      m_position = position;
    }
    void setX(float x) throw() { m_x = x; }
    void setY(float y) throw() { m_y = y; }
    void setPaddingX(float padding_x) throw() { m_padding_x = padding_x; }
    void setPaddingY(float padding_y) throw() { m_padding_y = padding_y; }

  private:
    LineProperty* m_tick_line_pro;
    FontProperty* m_tick_font_pro;
    FontProperty* m_label_font_pro;
    std::string m_tag;
    std::string m_color_pattern;
    std::string m_labels;
    bool m_log_scale;
    float m_title_offset;
    float m_label_offset;
    std::string m_time_format;
    std::string m_position;
    float m_tick_length;
    float m_x, m_y, m_padding_x, m_padding_y;

  };

};

#endif
