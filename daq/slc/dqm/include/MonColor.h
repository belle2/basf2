#ifndef _Belle2_MonColor_h
#define _Belle2_MonColor_h

#include <string>

namespace Belle2 {

  class MonColor {

  public:
    static const MonColor NON;
    static const MonColor BLACK;
    static const MonColor WHITE;
    static const MonColor BLUE;
    static const MonColor CYAN;
    static const MonColor DARK_GRAY;
    static const MonColor GRAY;
    static const MonColor GREEN;
    static const MonColor LIGHT_GRAY;
    static const MonColor MAGENTA;
    static const MonColor ORANGE;
    static const MonColor PINK;
    static const MonColor RED;
    static const MonColor YELLOW;
    static const MonColor RUNNING_GREEN;

  public:
    MonColor() { setRGB(255, 255, 255); }
    MonColor(int r, int g, int b) { setRGB(r, g, b); }
    MonColor(const MonColor& color) { *this = color; }
    ~MonColor() throw() {}

  public:
    bool isNon() const throw();
    void setRGB(int red, int green, int blue) throw();
    int getRed() const throw() { return m_red; }
    int getGreen() const throw() { return m_green; }
    int getBlue() const throw() { return m_blue; }
    std::string toString() const throw();

  public:
    const MonColor& operator=(const MonColor& c) throw();
    bool operator==(const MonColor& c) const throw() {
      return m_red == c.m_red && m_green == c.m_green && m_blue == c.m_blue;
    };

  private:
    int m_red, m_blue, m_green;

  };

};

#endif
