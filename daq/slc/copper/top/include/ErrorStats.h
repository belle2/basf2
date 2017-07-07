#ifndef __ERRORSTATS__
#define __ERRORSTATS__

#include <map>
#include <string>

class ErrorStats {
public:
  ErrorStats();
  ~ErrorStats();

  void PrintErrorSummary();

  void ShowErrorStat(const std::string& key);
  void ShowErrorStats();
  void ShowTransactionStats();

  void IncrementErrorCount(const std::string& key, const bool quite = true);
  void IncrementTransactionCount(const std::string& key);

  int GetHslbTransactionCount(const std::string& key);

private:

  void InitErrorStats();
  void InitTransactionCount();

  std::map<std::string, int> m_errorCount;
  std::map<std::string, int> m_transactionCount;
};

static ErrorStats errorStats;

#endif
