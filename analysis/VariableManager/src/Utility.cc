#include <analysis/VariableManager/Utility.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <stdexcept>

namespace Belle2 {

  namespace Variable {

    std::map<std::string, std::string> getSubstitutionMap()
    {
      return std::map<std::string, std::string> {
        {" ", "__sp"},
        {",", "__cm"},
        {":", "__cl"},
        {"=", "__eq"},
        {"<", "__st"},
        {">", "__gt"},
        {".", "__pt"},
        {"+", "__pl"},
        {"-", "__mi"},
        {"(", "__bo"},
        {")", "__bc"},
        {"{", "__co"},
        {"}", "__cc"},
        {"[", "__so"},
        {"]", "__sc"},
        {"`", "__to"},
        {"´", "__tc"},
        {"^", "__ha"},
        {"°", "__ci"},
        {"$", "__do"},
        {"§", "__pa"},
        {"%", "__pr"},
        {"!", "__em"},
        {"?", "__qm"},
        {";", "__sm"},
        {"#", "__hs"},
        {"*", "__mu"},
        {"/", "__sl"},
        {"\\", "__bl"},
        {"'", "__sq"},
        {"\"", "__dq"},
        {"~", "__ti"},
        {"-", "__da"},
        {"|", "__pi"},
        {"&", "__am"},
        {"@", "__at"},
      };
    }

    std::string makeROOTCompatible(std::string str)
    {
      if (str.find("__") != std::string::npos) {
        B2WARNING("String passed to makeROOTCompatible contains double-underscore __, which is used internally for escaping special characters. "
                  "It is recommended to avoid this. However escaping a string twice with makeROOTCompatible is safe, but will print this warning. "
                  "Passed string: " + str);
      }
      auto replace = getSubstitutionMap();
      for (auto& pair : replace) {
        boost::replace_all(str, pair.first, pair.second);
      }
      const static boost::regex blackList("[^a-zA-Z0-9_]");
      return boost::regex_replace(str, blackList, "");
    }

    std::string invertMakeROOTCompatible(std::string str)
    {
      auto replace = getSubstitutionMap();
      for (auto& pair : replace) {
        boost::replace_all(str, pair.second, pair.first);
      }
      return str;
    }

    bool almostEqualFloat(const float& a, const float& b)
    {
      assert(sizeof(float) == sizeof(int));
      return abs(*(int*)&a - * (int*)&b) <= 2 or (a == b);
    }

    unsigned long int findMatchedParenthesis(std::string str, char open, char close)
    {
      unsigned long int end = 1;
      if (str[0] == open) {
        unsigned int count = 1;
        for (end = 1; end < str.size() and count > 0; ++end) {
          if (str[end] == open) ++count;
          else if (str[end] == close) --count;
        }

        if (count > 0)
          throw std::runtime_error("Variable string has an invalid format: " + str);
      }
      return end - 1;
    }

    unsigned long int findIgnoringParenthesis(std::string str, std::string pattern, unsigned int begin = 0)
    {

      if (str.size() < pattern.size())
        return std::string::npos;

      for (unsigned int i = begin; i < str.size() - pattern.size(); ++i) {
        if (str[i] == '[') {
          i += findMatchedParenthesis(str.substr(i), '[', ']');
          continue;
        }
        if (str[i] == '(') {
          i += findMatchedParenthesis(str.substr(i), '(', ')');
          continue;
        }
        if (str[i] == '{') {
          i += findMatchedParenthesis(str.substr(i), '{', '}');
          continue;
        }

        for (unsigned int j = 0; j < pattern.size(); ++j) {
          if (str[i + j] != pattern[j]) {
            break;
          }
          if (j == pattern.size() - 1) {
            return i;
          }
        }
      }
      return std::string::npos;
    }

    std::vector<std::string> splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close)
    {

      std::vector<std::string> result;
      unsigned int lastdelimiter = 0;
      for (unsigned int i = 0; i < str.size(); ++i) {
        if (str[i] == open) {
          i += findMatchedParenthesis(str.substr(i), open, close);
          continue;
        }
        if (str[i] == delimiter) {
          result.push_back(str.substr(lastdelimiter, i - lastdelimiter));
          lastdelimiter = i + 1;
        }
      }
      std::string last = str.substr(lastdelimiter);
      if (last.size() != 0) {
        result.push_back(last);
      }
      return result;
    }

    std::unique_ptr<Cut> Cut::Compile(std::string str)
    {
      return std::unique_ptr<Cut>(new Cut(str));
    }

    Cut::Cut(std::string str) : operation(EMPTY), number(0), isNumeric(false), var(nullptr)
    {

      boost::algorithm::trim(str);
      if (str.empty()) {
        operation = EMPTY;
        return;
      }

      str = preprocess(str);
      if (not processLogicConditions(str)) {
        if (not processTernaryNumericConditions(str)) {
          if (not processBinaryNumericConditions(str)) {
            operation = NONE;
            try {
              number = Belle2::convertString<float>(str);
              isNumeric = true;
            } catch (std::invalid_argument&) {
              isNumeric = false;
              Variable::Manager& manager = Variable::Manager::Instance();
              var = manager.getVariable(str);
              if (var == nullptr) {
                throw std::runtime_error("Cut string has an invalid format: Variable not found: " + str);
              }
            }
          }
        }
      }

    }


    std::string Cut::preprocess(std::string str)
    {

      boost::algorithm::trim(str);
      if (str.empty())
        throw std::runtime_error("Cut string has an invalid format: Empty" + str);

      while (str.size() > 1 and findMatchedParenthesis(str) == str.size() - 1) {
        str = str.substr(1, str.size() - 2);
        boost::algorithm::trim(str);
        if (str.empty())
          throw std::runtime_error("Cut string has an invalid format: Empty" + str);

      }


      return str;

    }

    bool Cut::processLogicConditions(std::string str)
    {

      unsigned long int begin = findMatchedParenthesis(str);
      unsigned long int pos = 0;

      if ((pos =  findIgnoringParenthesis(str, " or ", begin)) != std::string::npos) {
        operation = OR;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 4)));
        return true;
      }

      if ((pos =  findIgnoringParenthesis(str, " and ", begin)) != std::string::npos) {
        operation = AND;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 5)));
        return true;
      }

      return false;

    }

    bool Cut::processBinaryNumericConditions(std::string str)
    {

      unsigned long int pos = 0;
      if ((pos =  findIgnoringParenthesis(str, "<=")) != std::string::npos) {
        operation = LE;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "<")) != std::string::npos) {
        operation = LT;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 1)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, ">=")) != std::string::npos) {
        operation = GE;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, ">")) != std::string::npos) {
        operation = GT;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 1)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "==")) != std::string::npos) {
        operation = EQ;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 2)));
        return true;
      }
      if ((pos =  findIgnoringParenthesis(str, "!=")) != std::string::npos) {
        operation = NE;
        left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos)));
        right = std::unique_ptr<Cut>(new Cut(str.substr(pos + 2)));
        return true;
      }

      return false;
    }

    bool Cut::processTernaryNumericConditions(std::string str)
    {

      for (auto& c : {"<", ">", "!", "="}) {

        unsigned long int pos1 = 0;
        unsigned long int pos2 = 0;

        if (((pos1 =  findIgnoringParenthesis(str, c)) != std::string::npos) and
            ((pos2 =  findIgnoringParenthesis(str, "<", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, ">", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, "!", pos1 + 2)) != std::string::npos
             or (pos2 =  findIgnoringParenthesis(str, "=", pos1 + 2)) != std::string::npos)) {
          operation = AND;
          left = std::unique_ptr<Cut>(new Cut(str.substr(0, pos2)));
          if (str[pos1 + 1] == '=')
            pos1++;
          right = std::unique_ptr<Cut>(new Cut(str.substr(pos1 + 1)));
          return true;
        }
      }

      return false;
    }

    bool Cut::check(const Particle* p) const
    {
      switch (operation) {
        case EMPTY:
          return true;
        case NONE:
          return this->get(p);
        case AND:
          return left->check(p) and right->check(p);
        case OR:
          return left->check(p) or right->check(p);
        case LT:
          return left->get(p) < right->get(p);
        case LE:
          return left->get(p) <= right->get(p);
        case GT:
          return left->get(p) > right->get(p);
        case GE:
          return left->get(p) >= right->get(p);
        case EQ:
          return almostEqualFloat(left->get(p), right->get(p));
        case NE:
          return not almostEqualFloat(left->get(p), right->get(p));
      }
      throw std::runtime_error("Cut string has an invalid format: Invalid operation");
      return false;
    }

    float Cut::get(const Particle* p) const
    {

      if (isNumeric) {
        return number;
      } else if (var != nullptr) {
        return var->function(p);
      } else {
        throw std::runtime_error("Cut string has an invalid format: Neither number nor variable name");
      }

    }

    void Cut::print() const
    {

      switch (operation) {
        case EMPTY: std::cout << "EMPTY" << std::endl; break;
        case NONE: std::cout << "NONE" << std::endl; break;
        case AND: std::cout << "AND" << std::endl; break;
        case OR: std::cout << "OR" << std::endl; break;
        case LT: std::cout << "LT" << std::endl; break;
        case LE: std::cout << "LE" << std::endl; break;
        case GT: std::cout << "GT" << std::endl; break;
        case GE: std::cout << "GE" << std::endl; break;
        case EQ: std::cout << "EQ" << std::endl; break;
        case NE: std::cout << "NE" << std::endl; break;
      }
      if (left != nullptr) {
        std::cout << "Left " << std::endl;
        left->print();
        std::cout << "End Left" << std::endl;
      }
      if (right != nullptr) {
        std::cout << "Right " << std::endl;
        right->print();
        std::cout << "End Right" << std::endl;
      }

    }
  }
}
