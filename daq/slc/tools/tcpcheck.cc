#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>

#include <arpa/inet.h>

struct tcpinfo {
  std::string host_port;
  bool islocal;
};

int main(int argc, char** argv)
{
  bool isall = argc == 1;
  std::vector<tcpinfo> info_v;
  while (argc > 0) {
    const std::string arg = *argv;
    tcpinfo info;
    if (arg == "-r") {
      argv++;
      argc--;
      info.islocal = false;
      info.host_port = *argv;
    } else if (arg == "-l") {
      argv++;
      argc--;
      info.islocal = true;
      info.host_port = *argv;
    } else {
      info.islocal = true;
      info.host_port = arg;
    }
    info_v.push_back(info);
    argv++;
    argc--;
  }

  std::ifstream fin("/proc/net/tcp");
  std::string line;
  std::stringstream ss;
  std::string sl, local_address, rem_address, st, queue;
  getline(fin, line);
  while (fin && getline(fin, line)) {
    ss.str("");
    ss << line;
    ss >> sl >> local_address >> rem_address >> st >> queue;
    int host = strtoul(local_address.substr(6, 2).c_str(), NULL, 16);

    std::stringstream ss;
    ss << host;
    host = strtoul(local_address.substr(4, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(local_address.substr(2, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(local_address.substr(0, 2).c_str(), NULL, 16);
    ss << "." << host;
    int port = strtoul(local_address.substr(9).c_str(), NULL, 16);
    ss << ":" << port;
    std::string host_local = ss.str();
    ss.str("");
    host = strtoul(rem_address.substr(6, 2).c_str(), NULL, 16);
    ss << host;
    host = strtoul(rem_address.substr(4, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(rem_address.substr(2, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(rem_address.substr(0, 2).c_str(), NULL, 16);
    ss << "." << host;
    port = strtoul(rem_address.substr(9).c_str(), NULL, 16);
    ss << ":" << port;
    std::string host_remote = ss.str();
    ss.str("");
    for (size_t i = 0; i < info_v.size(); i++) {
      if (isall || (info_v[i].islocal && info_v[i].host_port == host_local) ||
          (!info_v[i].islocal && info_v[i].host_port == host_remote)) {
        std::cout << "local = " << host_local << ", "
                  << "remote = " << host_remote << ", ";
        int sti = strtoul(st.substr(0).c_str(), NULL, 16);
        std::cout << "st = " << sti;
        int tx_queue = strtoul(queue.substr(0, 8).c_str(), NULL, 16);
        std::cout << ", tx_queue = " << tx_queue;
        int rx_queue = strtoul(queue.substr(9).c_str(), NULL, 16);
        std::cout << ", rx_queue = " << rx_queue;
        std::cout << std::endl;
        if (isall) break;
      }
    }
  }
  fin.close();
  fin.open("/proc/net/tcp6");
  getline(fin, line);
  while (fin && getline(fin, line)) {
    ss.str("");
    ss << line;
    ss >> sl >> local_address >> rem_address >> st >> queue;
    int host = strtoul(local_address.substr(6, 2).c_str(), NULL, 16);

    std::stringstream ss;
    ss << host;
    host = strtoul(local_address.substr(4, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(local_address.substr(2, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(local_address.substr(0, 2).c_str(), NULL, 16);
    ss << "." << host;
    int port = strtoul(local_address.substr(9).c_str(), NULL, 16);
    ss << ":" << port;
    std::string host_local = ss.str();
    ss.str("");
    host = strtoul(rem_address.substr(6, 2).c_str(), NULL, 16);
    ss << host;
    host = strtoul(rem_address.substr(4, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(rem_address.substr(2, 2).c_str(), NULL, 16);
    ss << "." << host;
    host = strtoul(rem_address.substr(0, 2).c_str(), NULL, 16);
    ss << "." << host;
    port = strtoul(rem_address.substr(9).c_str(), NULL, 16);
    ss << ":" << port;
    std::string host_remote = ss.str();
    ss.str("");
    for (size_t i = 0; i < info_v.size(); i++) {
      if (isall || (info_v[i].islocal && info_v[i].host_port == host_local) ||
          (!info_v[i].islocal && info_v[i].host_port == host_remote)) {
        std::cout << "local = " << host_local << ", "
                  << "remote = " << host_remote << ", ";
        int sti = strtoul(st.substr(0).c_str(), NULL, 16);
        std::cout << "st = " << sti;
        int tx_queue = strtoul(queue.substr(0, 8).c_str(), NULL, 16);
        std::cout << ", tx_queue = " << tx_queue;
        int rx_queue = strtoul(queue.substr(9).c_str(), NULL, 16);
        std::cout << ", rx_queue = " << rx_queue;
        std::cout << std::endl;
        if (isall) break;
      }
    }
  }
  return 0;
}
