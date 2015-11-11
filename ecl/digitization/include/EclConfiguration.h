#ifndef ECLCONFIGURATION_H_
#define ECLCONFIGURATION_H_
namespace Belle2 {

  class  EclConfiguration {
  public:
    static EclConfiguration& get()
    {
      static EclConfiguration instance;
      return instance;
    }
    bool background() const { return m_background; }
    void setBackground(bool val) { m_background = val; }
  private:
    EclConfiguration() {};
    bool m_background;
  };

}

#endif
