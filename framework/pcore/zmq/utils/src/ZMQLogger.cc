/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/utils/ZMQLogger.h>
#include <boost/property_tree/json_parser.hpp>
#include <ostream>
#include <stdexcept>

using namespace Belle2;

std::string ZMQLogger::getMonitoringJSON() const
{
  std::stringstream buffer;
  buffer << "{";
  bool first = true;
  for (const auto& keyValue : m_monitoring) {
    if (not first) {
      buffer << ", ";
    }
    first = false;
    buffer << "\"" << keyValue.first << "\": ";
    buffer << std::visit(toJSON{}, keyValue.second);
  }
  buffer << "}" << std::endl;
  return buffer.str();
}

void ZMQLogger::increment(const std::string& key)
{
  std::visit(Incrementor{}, m_monitoring[key]);
}

void ZMQLogger::decrement(const std::string& key)
{
  std::visit(Decrementor{}, m_monitoring[key]);
}

void ZMQLogger::logTime(const std::string& key)
{
  auto current = std::chrono::system_clock::now();
  auto displayTime = std::chrono::system_clock::to_time_t(current);
  log(key, std::ctime(&displayTime));
}

std::string ZMQLogger::toJSON::operator()(long value)
{
  return std::to_string(value);
}

std::string ZMQLogger::toJSON::operator()(double value)
{
  return std::to_string(value);
}

std::string ZMQLogger::toJSON::operator()(const std::string& value)
{
  return "\"" + boost::property_tree::json_parser::create_escapes(value) + "\"";
}

void ZMQLogger::Incrementor::operator()(long& value)
{
  value += 1;
}

void ZMQLogger::Incrementor::operator()(double& value)
{
  value += 1;
}

void ZMQLogger::Incrementor::operator()(std::string&)
{
  throw std::domain_error("Can not increment a string type");
}

void ZMQLogger::Decrementor::operator()(long& value)
{
  value -= 1;
}

void ZMQLogger::Decrementor::operator()(double& value)
{
  value -= 1;
}

void ZMQLogger::Decrementor::operator()(std::string&)
{
  throw std::domain_error("Can not decrement a string type");
}
