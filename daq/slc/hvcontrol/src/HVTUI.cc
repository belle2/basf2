#include "daq/slc/hvcontrol/HVTUI.h"
#include "daq/slc/hvcontrol/HVMessage.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#include <unistd.h>
#include <ncurses.h>

using namespace Belle2;

void HVTUI::init(int mode, int interval)
{
  LogFile::setStdErr(false);
  m_mode = mode;
  m_interval = interval;
  m_allon = true;
  m_nstore = 0;
  m_nrecall = 0;

  m_ir = 0;
  m_ic = 3;
  m_it = 0;
  m_edit = false;
  m_pad = 0;
  m_pad_y = 0;

  initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLUE);
  init_pair(2, COLOR_GREEN, COLOR_BLUE);
  init_pair(3, COLOR_YELLOW, COLOR_BLUE);
  init_pair(10, COLOR_WHITE, COLOR_BLUE);
  bkgd(COLOR_PAIR(10));

  noecho();
  cbreak();
  scrollok(stdscr, FALSE);
  keypad(stdscr, TRUE);
  int w, h;
  getmaxyx(stdscr, h, w);
  int pad_width = w;
  int pad_height = getNChannels() + 2;
  m_pad = newpad(pad_height, pad_width);
  wbkgd((WINDOW*)m_pad, COLOR_PAIR(10));

  int xs = 0;
  xs = add(xs, "  Crate | ");
  xs = add(xs, "  Slot | ");
  xs = add(xs, "   Ch | ");
  xs = add(xs, "  Switch | ");
  xs = add(xs, "  RampUp[V] | ");
  xs = add(xs, "RampDown[V] | ");
  xs = add(xs, " Vdemand[V] | ");
  xs = add(xs, " Vlimit[V] | ");
  xs = add(xs, " Climit[uA] | ");
  xs = add(xs, "  State | ");
  xs = add(xs, "  Vmon[V] | ");
  xs = add(xs, "  Cmon[uA] | ");

  erase();
  move(1, 0);
  addstr("|  ALL         [ON]:[OFF] ");
  addstr("|  Store  [0]                                        ");
  addstr("|  Recall  [0]                                                |");
  move(3, 0);
  addstr("|");
  for (size_t i = 0; i < m_rhead.c.size(); i++) {
    addstr(m_rhead.c[i].label.c_str());
  }

  werase((WINDOW*)m_pad);
  for (int i = 0; i < getNChannels(); i++) {
    wmove((WINDOW*)m_pad, i + 1, 0);
    int crate, slot, channel;
    getChannel(i, crate, slot, channel);
    wprintw((WINDOW*)m_pad, "|     %.2d | ", crate);
    wprintw((WINDOW*)m_pad, "    %.2d | ", slot);
    wprintw((WINDOW*)m_pad, "   %.2d | ", channel);
    for (size_t ic = 3; ic < m_rhead.c.size(); ic++) {
      m_table[i].c = m_rhead.c;
      for (size_t is = 0; is < m_rhead.c[ic].label.size() - 2; is++) {
        waddstr((WINDOW*)m_pad, " ");
      }
      waddstr((WINDOW*)m_pad, "| ");
    }
  }
  for (int i = 0; i < xs; i++) {
    mvprintw(0, i, "-");
    mvprintw(2, i, "-");
    mvwprintw((WINDOW*)m_pad, 0, i, "-");
    mvwprintw((WINDOW*)m_pad, getNChannels() + 1, i, "-");
  }
  wmove((WINDOW*)m_pad, gety(m_ir), getx(m_ic));
  refresh();
  prefresh((WINDOW*)m_pad, 0, 0, 4, 0, LINES - 1, COLS - 1);
}

HVTUI::~HVTUI() throw()
{
  endwin();
}

int HVTUI::getx(int ic)
{
  return m_rhead.c[ic].xs + m_rhead.c[ic].width - 2;
}

int HVTUI::gety(int ir)
{
  return ir + 1;
}

int HVTUI::add(int xs, const std::string& label)
{
  Cell c = {label, xs, (int)label.size(), ""};
  xs += label.size();
  m_rhead.c.push_back(c);
  return xs;
}

void HVTUI::setValue(int ic, int ir, const std::string& value)
{
  for (int i = 0; i < m_table[ir].c[ic].width - 3; i++) {
    mvwaddch((WINDOW*)m_pad, gety(ir), m_table[ir].c[ic].xs + 1 + i, ' ');
  }
  m_table[ir].c[ic].value = value;
  mvwaddstr((WINDOW*)m_pad, gety(ir), getx(ic) - value.size(), value.c_str());
}

void HVTUI::setNumber(int ic, int ir, float v,
                      const char* format)
{
  if (m_ic == ic && m_ir == ir && m_edit) return;
  if (m_mode == 0) {
    setValue(ic, ir, Belle2::StringUtil::form(format, v));
  } else {
    setValue(ic, ir, Belle2::StringUtil::form("%X", (int)v));
  }
}

void HVTUI::setSwitchOn(int i, int state)
{
  if (state > 0) wattrset((WINDOW*)m_pad, COLOR_PAIR(2) | A_REVERSE);
  setValue(3, i, state > 0 ? " ON" : "OFF");
  wattrset((WINDOW*)m_pad, COLOR_PAIR(10));
}

void HVTUI::run()
{
  if (m_interval > 0) {
    PThread(new Listener(this));
    //PThread(new Monitor(m_interval, this));
  }
  int w, h;
  getmaxyx(stdscr, h, w);
  char buf[10];
  while (1) {
    int key = getch();
    if (key == '\t') {
      m_it++;
      if (m_it == 4) m_it = 0;
    }
    if (m_it == 2) {
      int len = strlen("|  ALL         [ON]:[OFF] |  Store  [0]") - 1;
      if (key == '\n' && m_nstore > 0) {
        store(m_nstore);
      } else if (isdigit(key)) {
        buf[0] = key;
        int n = atoi(buf);
        if (n >= 0 && n < 7) {
          m_nstore = n;
          mvaddstr(1, len - 1, StringUtil::form("%d", m_nstore).c_str());
        }
      } else {
        switch (key) {
          case KEY_UP:
            m_nstore++;
            if (m_nstore > 6) m_nstore = 6;
            break;
          case KEY_DOWN:
            m_nstore--;
            if (m_nstore < 0) m_nstore = 0;
            break;
        }
        mvaddstr(1, len - 1, StringUtil::form("%d", m_nstore).c_str());
      }
      move(1, len);
      prefresh((WINDOW*)m_pad, m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
      refresh();
    } else if (m_it == 3) {
      int len = strlen("|  ALL         [ON]:[OFF] |  Store  [0]                                        |  Recall  [0]") - 1;
      if (key == '\n' && m_nrecall > 0) {
        recall(m_nrecall);
      } else if (isdigit(key)) {
        buf[0] = key;
        int n = atoi(buf);
        if (n >= 0 && n < 7) {
          m_nrecall = n;
          mvaddstr(1, len - 1, StringUtil::form("%d", m_nrecall).c_str());
        }
      } else {
        switch (key) {
          case KEY_UP: m_nrecall++; break;
          case KEY_DOWN: m_nrecall--; break;
        }
        if (m_nrecall < 0) m_nrecall = 0;
        if (m_nrecall > 6) m_nrecall = 6;
        mvaddstr(1, len - 1, StringUtil::form("%d", m_nrecall).c_str());
      }
      move(1, len);
      prefresh((WINDOW*)m_pad, m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
      refresh();
    } else if (m_it == 1) {
      switch (key) {
        case KEY_LEFT: m_allon = true; break;
        case KEY_RIGHT: m_allon = false; break;
      }
      int len = strlen("|  ALL         [ON]:[OFF]");
      if (key == '\n') {
        setSwitch(0, 0, 0, m_allon);
      }
      move(1, len - (m_allon ? 7 : 1));
      prefresh((WINDOW*)m_pad, m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
      refresh();
    } else if (m_it == 0) {
      bool isw = false;
      bool isbs = false;
      switch (key) {
        case KEY_UP: m_ir--; m_edit = false; break;
        case KEY_DOWN: m_ir++; m_edit = false; break;
        case KEY_LEFT: m_ic--; m_edit = false; break;
        case KEY_RIGHT: m_ic++; m_edit = false; break;
        case KEY_DC:
        case KEY_BACKSPACE: isbs = true; break;
        case '\t': m_edit = false; break;
        default: isw = true; break;
      }
      if (m_ic > (int)m_rhead.c.size() - 4) m_ic = m_rhead.c.size() - 4;
      if (m_ic <= 3) m_ic = 3;
      if (m_ir < 0) {
        if (gety(m_ir) < 1) {
          m_pad_y--;
          if (m_pad_y < 0) m_pad_y = 0;
          prefresh((WINDOW*)m_pad, m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
          refresh();
        }
        m_ir = 0;
      }
      if (m_ir >= getNChannels()) {
        if (gety(m_ir) > h - 5 + m_pad_y) {
          m_pad_y++;
          prefresh((WINDOW*)m_pad, m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
          refresh();
        }
        m_ir = getNChannels() - 1;
      }
      if (isbs && m_ic < 9) {
        m_edit = true;
        std::string value = m_table[m_ir].c[m_ic].value;
        setValue(m_ic, m_ir, value.substr(0, value.size() - 1));
      }
      if (isw) {
        if (m_ic == 3) {
          if (key == '\n' || key == ' ') {
            m_edit = false;
            std::string value = m_table[m_ir].c[m_ic].value;
            int crate, slot, channel;
            getChannel(m_ir, crate, slot, channel);
            bool switchon = !StringUtil::find(value, "ON");
            setSwitch(crate, slot, channel, switchon);
            //usleep(500000);
            //requestSwitch(crate, slot, channel);
          }
        } else {
          if (key == '\n') {
            m_edit = false;
            std::string value = m_table[m_ir].c[m_ic].value;
            int crate, slot, channel;
            getChannel(m_ir, crate, slot, channel);
            double v = 0;
            if (m_mode == 0) v = atof(value.c_str());
            else v = strtoul(("0x" + value).c_str(), NULL, 16);
            if (m_ic == 4) {
              setRampUpSpeed(crate, slot, channel, v);
            } else if (m_ic == 5) {
              setRampDownSpeed(crate, slot, channel, v);
            } else if (m_ic == 6) {
              setVoltageDemand(crate, slot, channel, v);
            } else if (m_ic == 7) {
              setVoltageLimit(crate, slot, channel, v);
            } else if (m_ic == 8) {
              setCurrentLimit(crate, slot, channel, v);
            }
            continue;
          }
          if (m_ic < 9 && ((m_mode != 0 && isalpha(key)) || isdigit(key) || key == '.')) {
            setValue(m_ic, m_ir, m_table[m_ir].c[m_ic].value + (char)key);
            m_edit = true;
          }
        }
      }
      wmove((WINDOW*)m_pad, gety(m_ir), getx(m_ic));
      prefresh((WINDOW*)m_pad, m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
    }
  }
}

void HVTUI::getValueAll()
{
  for (int i = 0; i < getNChannels(); i++) {
    int crate = m_table[i].crate;
    int slot = m_table[i].slot;
    int channel = m_table[i].channel;
    requestSwitch(crate, slot, channel);
    requestRampUpSpeed(crate, slot, channel);
    requestRampDownSpeed(crate, slot, channel);
    requestVoltageDemand(crate, slot, channel);
    requestVoltageLimit(crate, slot, channel);
    requestCurrentLimit(crate, slot, channel);
    requestState(crate, slot, channel);
    requestVoltageMonitor(crate, slot, channel);
    requestCurrentMonitor(crate, slot, channel);
  }
}

int HVTUI::getIndex(int crate, int slot, int channel)
{
  for (int i = 0; i < getNChannels(); i++) {
    if (m_table[i].crate == crate &&
        m_table[i].slot == slot &&
        m_table[i].channel == channel) {
      return i;
    }
  }
  return -1;
}

void HVTUI::getChannel(int index, int& crate, int& slot, int& channel)
{
  if (index >= 0 && index < getNChannels()) {
    crate = m_table[index].crate;
    slot = m_table[index].slot;
    channel = m_table[index].channel;
  }
}

void HVTUI::Listener::run()
{
  while (true) {
    HVMessage msg(m_tui->wait());
    int index = m_tui->getIndex(msg.getCrate(), msg.getSlot(), msg.getChannel());
    if (index < 0) continue;
    float* pars = msg.getParams();
    switch (msg.getType()) {
      case HVMessage::ALL: {
        m_tui->setSwitchOn(index, *(int*)(&pars[4]));
        m_tui->setNumber(4, index, pars[5]);
        m_tui->setNumber(5, index, pars[6]);
        m_tui->setNumber(6, index, pars[7]);
        m_tui->setNumber(7, index, pars[8]);
        m_tui->setNumber(8, index, pars[9]);
      } break;
      case HVMessage::SWITCH: {
        m_tui->setSwitchOn(index, *(int*)(&pars[4]));
      } break;
      case HVMessage::STATE: {
        int flag = *(int*)(&pars[4]);
        std::string sflag = "UNKNOWN";
        switch (flag) {
          case HVMessage::OFF: sflag = "OFF"; break;
          case HVMessage::ON:  sflag = " ON"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(2) | A_REVERSE); break;
          case HVMessage::OVP: sflag = "OVP"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(1) | A_REVERSE); break;
          case HVMessage::OCP: sflag = "OCP"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(1) | A_REVERSE); break;
          case HVMessage::RAMPUP: sflag = "RUP"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(3) | A_REVERSE); break;
          case HVMessage::RAMPDOWN: sflag = "RDW"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(3) | A_REVERSE); break;
          case HVMessage::TRIP: sflag = "TRIP"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(1) | A_REVERSE); break;
          case HVMessage::ETRIP: sflag = "ETRIP"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(1) | A_REVERSE); break;
          case HVMessage::INTERLOCK: sflag = "ILCK"; wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(1) | A_REVERSE); break;
        }
        m_tui->setValue(9, index, sflag);
        wattrset((WINDOW*)m_tui->m_pad, COLOR_PAIR(10));
      } break;
      case HVMessage::RAMPUP_SPEED: {
        m_tui->setNumber(4, index, pars[4]);
      } break;
      case HVMessage::RAMPDOWN_SPEED: {
        m_tui->setNumber(5, index, pars[4]);
      } break;
      case HVMessage::VOLTAGE_DEMAND: {
        m_tui->setNumber(6, index, pars[4]);
      } break;
      case HVMessage::VOLTAGE_LIMIT: {
        m_tui->setNumber(7, index, pars[4]);
      } break;
      case HVMessage::CURRENT_LIMIT: {
        m_tui->setNumber(8, index, pars[4]);
      } break;
      case HVMessage::VOLTAGE_MON: {
        m_tui->setNumber(10, index, pars[4], "%.3f");
      } break;
      case HVMessage::CURRENT_MON: {
        m_tui->setNumber(11, index, pars[4], "%.3f");
      } break;
      default: break;
    }
    if (m_tui->m_it == 2) {
      move(1, strlen("|  ALL         [ON]:[OFF] |  Store  [0]") - 1);
      prefresh((WINDOW*)m_tui->m_pad, m_tui->m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
      refresh();
    } else if (m_tui->m_it == 3) {
      move(1, strlen("|  ALL         [ON]:[OFF] |  Store  [0]                                        |  Recall  [0]") - 1);
      prefresh((WINDOW*)m_tui->m_pad, m_tui->m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
      refresh();
    } else if (m_tui->m_it == 1) {
      move(1, strlen("|  ALL         [ON]:[OFF]") - (m_tui->m_allon ? 7 : 1));
      prefresh((WINDOW*)m_tui->m_pad, m_tui->m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
      refresh();
    } else if (m_tui->m_it == 0) {
      wmove((WINDOW*)m_tui->m_pad, m_tui->gety(m_tui->m_ir), m_tui->getx(m_tui->m_ic));
      refresh();
      prefresh((WINDOW*)m_tui->m_pad, m_tui->m_pad_y, 0, 4, 0, LINES - 1, COLS - 1);
    }
  }
}

void HVTUI::Monitor::run()
{
  int si = 0;
  while (true) {
    for (int i = 0; i < m_tui->getNChannels(); i++) {
      int crate = m_tui->m_table[i].crate;
      int slot = m_tui->m_table[i].slot;
      int channel = m_tui->m_table[i].channel;
      m_tui->requestSwitch(crate, slot, channel);
      m_tui->requestState(crate, slot, channel);
      m_tui->requestVoltageMonitor(crate, slot, channel);
      m_tui->requestCurrentMonitor(crate, slot, channel);
    }
    if (si % 2 == 0) {
      m_tui->getValueAll();
    }
    sleep(m_interval);
    si++;
  }
}
