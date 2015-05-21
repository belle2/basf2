#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/nsm_read_argv.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>

#include <ncurses.h>
#include <cstring>
#include <string>
#include <vector>

int getx(int ic);
int gety(int ir);
int add(int xs, const std::string& label);
void setValue(int ic, int ir, const std::string& value);
void setNode(int ir, const std::string& node);
void setState(int ir, std::string state);
void setConfig(int ir, const std::string& config);
void init_cui(int nnodes);

int help(const char** argv)
{
  printf("usage : %s <nodename> "
         "[-n myname] [-c conf] [-g]\n", argv[0]);
  printf("options: -c : set conf file \"conf\" (default:slowcontrol)\n");
  printf("options: -n : set nsm user name (default:env of USER)\n");
  printf("options: -g : use nsm.global (default:nsm)\n");
  return 0;
}

namespace Belle2 {

  class RCViewStateHandler : public NSMVHandlerText {

  public:
    RCViewStateHandler(const std::string& name, int id)
      : NSMVHandlerText(name, false, true), m_id(id) {}
    virtual ~RCViewStateHandler() throw() {}
    virtual bool handleSetText(const std::string& val)
    {
      ::setState(m_id, val);
      ::refresh();
      return true;
    }

  private:
    int m_id;
  };

  class RCViewConfigHandler : public NSMVHandlerText {

  public:
    RCViewConfigHandler(const std::string& name, int id)
      : NSMVHandlerText(name, false, true), m_id(id) {}
    virtual ~RCViewConfigHandler() throw() {}
    virtual bool handleSetText(const std::string& val)
    {
      ::setConfig(m_id, val);
      ::refresh();
      return true;
    }

  private:
    int m_id;
  };

  class RCViewCallback : public NSMCallback {

  public:
    RCViewCallback(const NSMNode& node, int argc, char** argv)
      : NSMCallback(5), m_argc(argc), m_argv(argv)
    {
      setNode(node);
    }
    virtual ~RCViewCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw()
    {
      try {
        NSMNode node(m_argv[1]);
        NSMCommunicator::connected(node.getName());
        int nnodes = 0;
        get(node, "nnodes", nnodes);
        init_cui(nnodes);
        ::setNode(0, node.getName());
        get(node, new RCViewStateHandler("rcstate", 0));
        get(node, new RCViewConfigHandler("rcconfig", 0));
        for (int i = 0; i < nnodes; i++) {
          std::string nodename;
          get(node, StringUtil::form("node[%d].name", i), nodename);
          ::setNode(i + 1, nodename);
          nodename = StringUtil::tolower(nodename);
          get(node, new RCViewStateHandler(nodename + ".rcstate", i + 1));
          try {
            get(node, new RCViewConfigHandler(nodename + ".rcconfig", i + 1), 1);
          } catch (const TimeoutException& e) {
          }
          ::refresh();
        }
      } catch (const NSMNotConnectedException& e) {
        endwin();
        printf("node %s is not online\n", m_argv[1]);
        exit(1);
      } catch (const NSMHandlerException& e) {
        endwin();
        printf("NSM error %s\n", e.what());
        exit(1);
      } catch (const IOException& e) {
        endwin();
        printf("timeout: %s\n", e.what());
        exit(1);
      }
    }

  private:
    int m_argc;
    char** m_argv;
  };

}

using namespace Belle2;

int main(int argc, const char** argv)
{
  ConfigFile config("slowcontrol");
  std::string name, username;
  char** argv_in = new char* [argc];
  int argc_in = nsm_read_argv(argc, argv, help, argv_in, config, name, username, 1);
  LogFile::open(StringUtil::form("%s/%s", argv[0], argv[1]), LogFile::ERROR);
  const std::string hostname = config.get(name + ".host");
  const int port = config.getInt(name + ".port");
  RCViewCallback* callback = new RCViewCallback(NSMNode(username), argc_in, argv_in);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();
  return 0;
}

struct cell {
  std::string label;
  int xs;
  int width;
  std::string value;
};

struct raw {
  std::vector<cell> c;
  int ys;
};

std::vector<struct raw> table;

struct raw rhead;

int getx(int ic)
{
  return rhead.c[ic].xs;// + rhead.c[ic].width - 3;
}

int gety(int ir)
{
  return ir + 3;
}

int add(int xs, const std::string& label)
{
  cell c = {label, xs, (int)label.size(), std::string("")};
  xs += label.size();
  rhead.c.push_back(c);
  return xs;
}

void setValue(int ic, int ir, const std::string& value)
{
  table[ir].c[ic].value = value;
  mvaddstr(gety(ir), getx(ic), value.c_str());
}

void setNode(int ir, const std::string& node)
{
  setValue(1, ir, node);
}

void setState(int ir, std::string state)
{
  int color = 0;
  if (state == "UNKNOWN") {
    color = 4;
  } else if (state == "OFF") {
    color = 6;
  } else if (state == "RUNNING") {
    color = 2;
  } else if (state == "READY") {
    color = 3;
  } else if (state == "NOTREADY") {
    color = 1;
  } else {
    color = 5;
  }
  if (state == "CONFIGURING") {
    state = "CONFIGURE";
  } else if (state == "RECOVERING") {
    state = "RECOVER";
  }
  if (color > 0) {
    attrset(COLOR_PAIR(color) | A_REVERSE | A_BOLD);
  }
  setValue(2, ir, Belle2::StringUtil::form(" %9s ", state.c_str()));
  attrset(COLOR_PAIR(10));
}

void setConfig(int ir, const std::string& config)
{
  setValue(3, ir, config);
}

void init_cui(int nnodes)
{
  initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLACK, COLOR_WHITE);
  init_pair(5, COLOR_MAGENTA, COLOR_WHITE);
  init_pair(6, COLOR_WHITE, COLOR_BLACK);
  init_pair(10, COLOR_WHITE, COLOR_BLUE);
  bkgd(COLOR_PAIR(10));

  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  int w, h;
  getmaxyx(stdscr, h, w);

  int xs = 0;
  xs = add(xs, "| Id  | ");
  xs = add(xs, "Node        | ");
  xs = add(xs, "State       | ");
  xs = add(xs, "Config                     | ");
  erase();
  move(1, 0);
  for (size_t i = 0; i < rhead.c.size(); i++) {
    addstr(rhead.c[i].label.c_str());
  }
  for (int i = 0; i < nnodes + 1; i++) {
    move(i + 3, 0);
    table.push_back(rhead);
    printw("| %.2d  | ", i);
    for (size_t ic = 1; ic < rhead.c.size(); ic++) {
      for (size_t is = 0; is < rhead.c[ic].label.size() - 2; is++) {
        addstr(" ");
      }
      addstr("| ");
    }
  }
  for (int i = 0; i < xs - 1; i++) {
    mvprintw(0, i, "-");
    mvprintw(2, i, "-");
    mvprintw(nnodes + 4, i, "-");
  }
  refresh();
}

