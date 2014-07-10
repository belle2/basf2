#include <analysis/VariableManager/Utility.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/logging/Logger.h>

std::string Belle2::Variable::makeROOTCompatible(std::string str)
{
  str.erase(std::remove(str.begin(), str.end(), '('), str.end());
  str.erase(std::remove(str.begin(), str.end(), ')'), str.end());
  return str;
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
  if (left != nullptr)
    delete left;
  if (right != nullptr)
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

  if ((pos =  str.find(" and ", begin)) != std::string::npos) {
    operation = AND;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 5));
    return true;
  }

  if ((pos =  str.find(" or ", begin)) != std::string::npos) {
    operation = OR;
    left = new Cut(str.substr(0, pos));
    right = new Cut(str.substr(pos + 4));
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

  return false;
}

bool Belle2::Variable::Cut::processTernaryNumericConditions(std::string str)
{

  unsigned long int pos1 = 0;
  unsigned long int pos2 = 0;
  if (((pos1 =  str.find("<")) != std::string::npos) and
      ((pos2 =  str.find("<", pos1 + 1)) != std::string::npos or (pos2 =  str.find(">", pos1 + 1)) != std::string::npos)) {
    operation = AND;
    left = new Cut(str.substr(0, pos2));
    if (str[pos1 + 1] == '=')
      pos1++;
    right = new Cut(str.substr(pos1 + 1));
    return true;
  }

  if (((pos1 =  str.find(">")) != std::string::npos) and
      ((pos2 =  str.find("<", pos1 + 1)) != std::string::npos or (pos2 =  str.find(">", pos1 + 1)) != std::string::npos)) {
    operation = AND;
    left = new Cut(str.substr(0, pos2));
    if (str[pos1 + 1] == '=')
      pos1++;
    right = new Cut(str.substr(pos1 + 1));
    return true;
  }

  return false;
}

unsigned long int Belle2::Variable::Cut::findMatchedParenthesis(std::string str)
{
  unsigned long int end = 1;
  if (str[0] == '[') {
    unsigned int count = 1;
    for (end = 1; end < str.size() and count > 0; ++end) {
      if (str[end] == '[') ++count;
      else if (str[end] == ']') --count;
    }

    if (count > 0)
      throw std::runtime_error("Cut string has an invalid format: " + str);
  }
  return end - 1;
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

  std::cout << operation << std::endl;
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
