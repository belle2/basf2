#ifndef _Belle2_TablePanel_h
#define _Belle2_TablePanel_h

#include "daq/slc/dqm/Panel.h"

namespace Belle2 {

  class TablePanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;

  public:
    TablePanel() throw();
    TablePanel(const std::string& name, int cols = 1, int rows = 1) throw();
    virtual ~TablePanel() throw();

  public:
    void setCols(int cols) throw() { m_cols = cols; }
    void setRows(int rows) throw() { m_rows = rows; }
    int getCols() const throw() { return m_cols; }
    int getRows() const throw() { return m_rows; }
    void setWidthRatio(const std::string& width_ratio) throw() {
      m_width_ratio = width_ratio;
    }
    void setHeightRatio(const std::string& height_ratio) throw() {
      m_height_ratio = height_ratio;
    }
    const std::string& getWidthRatio() const throw() { return m_width_ratio; }
    const std::string& setHeightRatio()const throw() { return m_height_ratio; }
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  private:
    int m_cols, m_rows;
    std::string m_width_ratio;
    std::string m_height_ratio;

  };

};

#endif
