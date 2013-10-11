#ifndef _B2DAQ_XMLClass_hh
#define _B2DAQ_XMLClass_hh

#include <string>
#include <vector>
#include <map>

#include <util/Serializable.hh>

namespace B2DAQ {
  
  class XMLClass : public Serializable {

  public:
    enum ValueType { OBJECT, ENUM, TEXT8, TEXT16,
		     TEXT32, TEXT64, TEXT256,
		     BOOLEAN, INT8, INT16,
		     INT32, INT64, FLOAT32, FLOAT64};
  public:
    XMLClass(const std::string& name)
      : _name(name) {}
    XMLClass() : _name() {}
    virtual ~XMLClass() throw() {}

  public:
    void add(const std::string& type, 
	     const std::string& label,
	     const std::string& value);
    void add(const std::string& type, 
	     const std::string& label, double value);
    void add(const std::string& type, 
	     const std::string& label, int value);
    void set(const std::string& label, const std::string& value);
    void set(const std::string& label, double value);
    void set(const std::string& label, int value);
    const std::string getText(const std::string& label);
    int getInt(const std::string& label);
    double getFloat(const std::string& label);
    unsigned int size();

    const std::string toSQLConfig();
    const std::string toSQLNames();
    const std::string toSQLValues();
    const std::string toNSMStructDef(int version = 1);

    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  private:
    std::string _name;
    std::vector<std::string> _label_v;
    std::map<std::string, ValueType> _type_v;
    std::map<std::string, XMLClass> _class_v;
    std::map<std::string, std::string> _text_v;
    std::map<std::string, double> _float_v;
    std::map<std::string, int> _int_v;
    std::map<std::string, bool> _boolean_v;

  };

}

#endif

