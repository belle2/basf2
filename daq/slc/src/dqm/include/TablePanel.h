#ifndef _Belle2_TablePanel_hh
#define _Belle2_TablePanel_hh

#include "dqm/Panel.h"

namespace Belle2 {

  class TablePanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;

  public:
    TablePanel() throw();
    TablePanel(const std::string& name, int cols = 1, int rows = 1) throw();
    virtual ~TablePanel() throw();

  public:
    void setCols(int cols) throw() { _cols = cols; }
    void setRows(int rows) throw() { _rows = rows; }
    int getCols() const throw() { return _cols; }
    int getRows() const throw() { return _rows; }
    void setWidthRatio(const std::string& width_ratio) throw() {
      _width_ratio = width_ratio;
    }
    void setHeightRatio(const std::string& height_ratio) throw() {
      _height_ratio = height_ratio;
    }
    const std::string& getWidthRatio() const throw() { return _width_ratio; }
    const std::string& setHeightRatio()const throw() { return _height_ratio; }
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  private:
    int _cols, _rows;
    std::string _width_ratio;
    std::string _height_ratio;

  };

};

#endif
