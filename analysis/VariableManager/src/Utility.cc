#include <analysis/VariableManager/Utility.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <stdexcept>

std::string Belle2::Variable::makeROOTCompatible(std::string str)
{
  const static boost::regex blackList("[^a-zA-Z0-9_]");
  return boost::regex_replace(str, blackList, "");
}

bool almostEqualFloat(const float& a, const float& b)
{
  assert(sizeof(float) == sizeof(int));
  return abs(*(int*)&a - * (int*)&b) <= 2;
}

unsigned long int Belle2::Variable::findMatchedParenthesis(std::string str, char open, char close)
{
  unsigned long int end = 1;
  if (str[0] == open) {
    unsigned int count = 1;
    for (end = 1; end < str.size() and count > 0; ++end) {
      if (str[end] == open) ++count;
      else if (str[end] == close) --count;
    }

    if (count > 0)
      throw std::runtime_error("String has an invalid format: " + str);
  }
  return end - 1;
}

std::vector<std::string> Belle2::Variable::splitOnDelimiterAndConserveParenthesis(std::string str, char delimiter, char open, char close)
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

Belle2::Variable::Cut::Cut(Parameter str) : left(nullptr), right(nullptr), operation(EMPTY), number(0), isNumeric(false), var(nullptr) { init(str); }

void Belle2::Variable::Cut::init(Parameter str)
{

  clean();

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
          number = boost::lexical_cast<float>(str);
          isNumeric = true;
        } catch (boost::bad_lexical_cast&) {
          isNumeric = false;
          Variable::Manager& manager = Variable::Manager::Instance();
          var = manager.getVariable(str);
          if (var == nullptr) {
            throw std::runtime_error("Cut string has an invalid format: Variable not found " + str);
          }
        }
      }
    }
  }
}

void Belle2::Variable::Cut::clean()
{
  delete left;
  delete right;
  left = nullptr;
  right = nullptr;
}

Belle2::Variable::Cut::~Cut()
{
  clean();
}

std::string Belle2::Variable::Cut::preprocess(std::string str)
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

bool Belle2::Variable::Cut::processLogicConditions(std::string str)
{

  unsigned long int begin = findMatchedParenthesis(str);
  unsigned long int pos = 0;

  if ((pos =  str.find(" or ", begin)) != std::string::npos && str.substr(begin, pos - begin).find("[") == std::string::npos) {
    operation = OR;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 4));
    return true;
  }

  if ((pos =  str.find(" and ", begin)) != std::string::npos && str.substr(begin, pos - begin).find("[") == std::string::npos) {
    operation = AND;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 5));
    return true;
  }

  return false;

}

bool Belle2::Variable::Cut::processBinaryNumericConditions(std::string str)
{

  unsigned long int pos = 0;
  if ((pos =  str.find("<=")) != std::string::npos) {
    operation = LE;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 2));
    return true;
  }
  if ((pos =  str.find("<")) != std::string::npos) {
    operation = LT;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 1));
    return true;
  }
  if ((pos =  str.find(">=")) != std::string::npos) {
    operation = GE;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 2));
    return true;
  }
  if ((pos =  str.find(">")) != std::string::npos) {
    operation = GT;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 1));
    return true;
  }
  if ((pos =  str.find("==")) != std::string::npos) {
    operation = EQ;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 2));
    return true;
  }
  if ((pos =  str.find("!=")) != std::string::npos) {
    operation = NE;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 2));
    return true;
  }

  return false;
}

bool Belle2::Variable::Cut::processTernaryNumericConditions(std::string str)
{

  unsigned long int pos1 = 0;
  unsigned long int pos2 = 0;
  for (auto & c : {"<", ">", "!", "="}) {
    if (((pos1 =  str.find(c)) != std::string::npos) and
        ((pos2 =  str.find("<", pos1 + 2)) != std::string::npos or (pos2 =  str.find(">", pos1 + 2)) != std::string::npos
         or (pos2 =  str.find("!", pos1 + 2)) != std::string::npos or (pos2 =  str.find("=", pos1 + 2)) != std::string::npos)) {
      operation = AND;
      left = new Cut(str.substr(0, pos2));
      if (str[pos1 + 1] == '=')
        pos1++;
      right = new Cut(str.substr(pos1 + 1));
      return true;
    }
  }

  return false;
}

bool Belle2::Variable::Cut::check(const Particle* p)
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
  throw std::runtime_error("Cut string has an invalid format: Invalid Operation");
  return false;
}

float Belle2::Variable::Cut::get(const Particle* p)
{

  if (isNumeric) {
    return number;
  } else if (var != nullptr) {
    return var->function(p);
  } else {
    throw std::runtime_error("Cut string has an invalid format: Neither number nor variable name");
  }

}

void Belle2::Variable::Cut::print()
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
