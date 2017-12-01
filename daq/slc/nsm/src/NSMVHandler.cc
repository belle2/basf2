#include "daq/slc/nsm/NSMVHandler.h"

using namespace Belle2;

NSMVHandler* NSMVHandler::create(const NSMVar& var)
{
  const std::string node = var.getNode();
  const std::string name = var.getName();
  if (var.getType() == NSMVar::INT) {
    if (var.getLength() == 0) {
      return new NSMVHandlerInt(node, name, false, true);
    }
    return new NSMVHandlerIntArray(node, name, false, true);
  } else if (var.getType() == NSMVar::FLOAT) {
    if (var.getLength() == 0) {
      return new NSMVHandlerFloat(node, name, false, true);
    }
    return new NSMVHandlerFloatArray(node, name, false, true);
  } else if (var.getType() == NSMVar::TEXT) {
    return new NSMVHandlerText(node, name, false, true);
  }
  return NULL;
}

bool NSMVHandlerInt::handleGet(NSMVar& var)
{
  int val;
  if (handleGetInt(val)) {
    var = val;
    return true;
  }
  return false;
}

bool NSMVHandlerInt::handleSet(const NSMVar& var)
{
  if (var.getType() == NSMVar::INT&& var.getLength() == 0) {
    if (handleSetInt(var.getInt())) {
      m_var = var.getInt();
      return true;
    }
  }
  return false;
}

bool NSMVHandlerFloat::handleGet(NSMVar& var)
{
  float val;
  if (handleGetFloat(val)) {
    var = val;
    return true;
  }
  return false;
}

bool NSMVHandlerFloat::handleSet(const NSMVar& var)
{
  if (var.getType() == NSMVar::FLOAT && var.getLength() == 0) {
    if (handleSetFloat(var.getFloat())) {
      m_var = var.getFloat();
      return true;
    }
  }
  return false;
}

bool NSMVHandlerText::handleGet(NSMVar& var)
{
  std::string val;
  if (handleGetText(val)) {
    var = val;
    return true;
  }
  return false;
}

bool NSMVHandlerText::handleSet(const NSMVar& var)
{
  if (var.getType() == NSMVar::TEXT && var.getLength() > 0) {
    if (handleSetText(var.getText())) {
      m_var = var.getText();
      return true;
    }
  }
  return false;
}

bool NSMVHandlerIntArray::handleGetIntArray(std::vector<int>& val)
{
  const int* pv = (const int*)m_var.get();
  val = std::vector<int>();
  for (int i = 0; i < m_var.getLength(); i++) {
    val.push_back(pv[i]);
  }
  return true;
}

bool NSMVHandlerIntArray::handleGet(NSMVar& var)
{
  std::vector<int> val;
  if (handleGetIntArray(val)) {
    var = val;
    return true;
  }
  return false;
}

bool NSMVHandlerIntArray::handleSet(const NSMVar& var)
{
  if (var.getType() == NSMVar::INT&& var.getLength() > 0) {
    const int* pv = (const int*)var.get();
    std::vector<int> val;
    for (int i = 0; i < var.getLength(); i++) {
      val.push_back(pv[i]);
    }
    if (handleSetIntArray(val)) {
      m_var = val;
      return true;
    }
  }
  return false;
}

bool NSMVHandlerFloatArray::handleGetFloatArray(std::vector<float>& val)
{
  const float* pv = (const float*)m_var.get();
  val = std::vector<float>();
  for (int i = 0; i < m_var.getLength(); i++) {
    val.push_back(pv[i]);
  }
  return true;
}

bool NSMVHandlerFloatArray::handleGet(NSMVar& var)
{
  std::vector<float> val;
  if (handleGetFloatArray(val)) {
    var = val;
    return true;
  }
  return false;
}

bool NSMVHandlerFloatArray::handleSet(const NSMVar& var)
{
  if (var.getType() == NSMVar::FLOAT && var.getLength() > 0) {
    const float* pv = (const float*)var.get();
    std::vector<float> val;
    for (int i = 0; i < var.getLength(); i++) {
      val.push_back(pv[i]);
    }
    if (handleSetFloatArray(val)) {
      m_var = val;
      return true;
    }
  }
  return false;
}
