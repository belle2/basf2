#!/usr/bin/python3.5

import sys
import tkinter as tk
import tkinter.font as tkFont
import math
import time
from tkinter import filedialog
from tkinter import ttk

from graph import TCanvas
import os

# print('sys.argv[0] =', sys.argv[0])
pathname = os.path.dirname(sys.argv[0])
print('path =', pathname)
# print('full path =', os.path.abspath(pathname))


import tkinter.messagebox as msg
import threading
import time

from b2daq import *
import threading


class TestNSMCallback(NSMCallback):

    def __init__(self):
        super(TestNSMCallback, self).__init__()
        self.available = False

    def init(self):
        self.addInt("ival", int(0))
        self.addFloat("fval", float(0))
        self.addText("tval", "konno")
        self.available = True
        print("Init done")

    def timeout(self):
        self.setInt("ival", 100)
        print("Timeout... %f" % float(val))


def th_func(callback):
    callback.run("store01")


def sprintf(fmt, *args):
    return fmt.format(args)


def print(*args, **kwargs):
    fmt = args[0]
    margs = args[1:]
    # print(fmt.format(*margs))
    mainapp.gprint(fmt.format(*margs))


gChannel = []
gChannel.append(["Crate", "I", "%d", "crate", ["set"], 1])
gChannel.append(["Slot", "I", "%d", "slot", ["set"], 1])
gChannel.append(["Ch.", "I", "%d", "channel", ["set"], 1])
gChannel.append(["RampUp", "F", "%f", "rampup", ["get", "set"], 5])
gChannel.append(["RampDown", "F", "%f", "rampdown", ["get", "set"], 5])
gChannel.append(["Vmax", "F", "%f", "vlimit", ["get", "set"], 5])
gChannel.append(["Imax", "F", "%f", "climit", ["get", "set"], 5])
gChannel.append(["Vset", "F", "%f", "vdemand", ["get", "set"], 5])
gChannel.append(["Vmon", "F", "%f", "vmon", ["get"], 5])
gChannel.append(["Imon", "F", "%f", "imon", ["get"], 5])
gChannel.append(["Status", "I", "%x", "status", ["get"], 5])
gChannel.append(["TurnOn", "S", "%d", "turnon", ["set"], 5])

gNsm = {'name': "ARICHLV"}

gLimits = []
gLimits.append({'name': "U1.5V", 'rampup': 1, 'rampdown': 0, 'vlimit': 1.7, 'climit': 1, 'vdemand': 1.5, 'turnon': "true"})
gLimits.append({'name': "U3.8V", 'rampup': 1, 'rampdown': 0, 'vlimit': 4, 'climit': 1, 'vdemand': 3.8, 'turnon': "true"})
gLimits.append({'name': "U2V", 'rampup': 1, 'rampdown': 0, 'vlimit': 2.2, 'climit': 1, 'vdemand': 2.0, 'turnon': "true"})
gLimits.append({'name': "U-2V", 'rampup': 1, 'rampdown': 0, 'vlimit': -2.2, 'climit': 1, 'vdemand': -2.0, 'turnon': "true"})

id = 0
crate = 0
slot = 0
channel = 0
sector = 0
gMap = []
for sector in range(6):
    for i in range(4):
        group = 0
        for limits in gLimits:
            g = {}
            g['vmon'] = 0
            g['imon'] = 0
            g['status'] = 0
            g['crate'] = crate
            g['slot'] = slot
            g['channel'] = channel
            g['sector'] = sector
            g['group'] = group
            g['id'] = id
            g.update(limits)
            gMap.append(g)
            id += 1
            group += 1
            channel += 1
            if channel == 8:
                channel = 0
                slot += 1
            if slot == 6:
                slot = 0
                crate += 1


class Channel:

    def __init__(self, master, name, id, type):
        self.name = name
        self.id = id
        self.master = master
        self.frame = tk.Frame(self.master)
        self.type = type
        if type == "sector":
            self.monitor()
        else:
            self.global_status()
        self.frame.pack()

    def global_status(self):
        global gChannel
        opt = "left"
        anchor = 'e'
        nd = 4

        tk.Label(self.frame, text=self.name, width=5, anchor=anchor, font=font).pack(side=opt)
        self.clist = []
        self.var = []
        nl = 0
        # img = tk.PhotoImage(file="power.gif")
        for x in range(6):
            self.var.append(tk.IntVar())
            self.var[nl].set(0)
            width = 8
            relief = 'flat'
            if self.id >= 0:
                if self.type == -2:
                    self.clist.append(
                        tk.Checkbutton(
                            self.frame,
                            variable=self.var[nl],
                            bd=nd,
                            width=width - 3,
                            relief=relief,
                            anchor=anchor,
                            font=font))
                else:
                    self.clist.append(
                        tk.Label(
                            self.frame,
                            textvariable=self.var[nl],
                            bd=nd,
                            width=width,
                            relief=relief,
                            anchor=anchor,
                            font=font))
            else:
                self.clist.append(tk.Label(self.frame, text='Sector' + str(x), bd=nd, width=width, anchor=anchor, font=font))
            self.clist[nl].pack(side=opt)
            nl += 1
        tk.Button(self.frame, text='', width=1, state='disabled', relief='flat', font=font).pack(side=opt)

    def monitor(self):
        global font
        global gChannel
        global gMap
        opt = "left"
        anchor = 'e'
        nd = 4

        tk.Label(self.frame, text=self.name, width=5, anchor=anchor, font=font).pack(side=opt)
        self.clist = []
        self.var = []
        nl = 0
        for x in gChannel:
            if x[1] == "I":
                self.var.append(tk.IntVar())
            elif x[1] == 'F':
                self.var.append(tk.DoubleVar())
            else:
                self.var.append(tk.StringVar())
            self.var[nl].set(gMap[self.id][x[3]])
            width = max(x[5], len(x[0]))
            if ('get' in x[4]):
                relief = 'groove'
            else:
                relief = 'flat'
            if self.id >= 0:
                self.clist.append(
                    tk.Label(
                        self.frame,
                        textvariable=self.var[nl],
                        bg="white",
                        bd=nd,
                        width=width,
                        relief=relief,
                        anchor=anchor,
                        font=font))
            else:
                self.clist.append(tk.Label(self.frame, text=x[0], bg="yellow", bd=nd, width=width, anchor=anchor, font=font))

            self.clist[nl].pack(side=opt)

            nl += 1
        if (self.id >= 0):
            tk.Button(self.frame, text='...', width=1, command=self.edit, font=font).pack(side=opt)
        else:
            tk.Button(self.frame, text='', width=1, state='disabled', font=font).pack(side=opt)

    def edit(self):
        global font
        global gChannel
        width = 20
        nd = 4
        opt = "top"
        relief = "groove"
        anchor = 'e'
        self.eWindow = tk.Toplevel(self.master)
        self.eWindow.wm_title("Edit Channel parameters")
        self.eFrame = tk.Frame(self.eWindow)
        tk.Label(self.eFrame, text=self.name, width=10, anchor=anchor, font=font).pack(side=opt)

        self.elist = []
        self.evar = []
        nl = 0
        for x in gChannel:
            f0 = tk.Frame(self.eFrame)
            l0 = tk.Label(f0, text=x[0], width=20)
            if x[1] == "I":
                self.evar.append(tk.IntVar())
            elif x[1] == 'F':
                self.evar.append(tk.DoubleVar())
            else:
                self.evar.append(tk.StringVar())
            self.evar[nl].set(self.var[nl].get())
            if ('set' in x[4]):
                self.elist.append(tk.Entry(f0, textvariable=self.evar[nl], bd=nd, width=width, relief=relief, font=font))
            else:
                self.elist.append(
                    tk.Label(
                        f0,
                        textvariable=self.evar[nl],
                        bd=nd,
                        width=width,
                        relief=relief,
                        anchor=anchor,
                        font=font))
            l0.pack(side="left")
            self.elist[nl].pack(side="left")
            f0.pack(side=opt)
            nl += 1
        f0 = tk.Frame(self.eFrame)
        tk.Button(f0, text='Discard', width=5, command=self.close_window, font=font).pack(side="left")
        tk.Button(f0, text='Set', width=5, command=self.nsmset, font=font).pack(side="left")
        f0.pack(side=opt)
        self.eFrame.pack()

    def close_window(self):
        self.eWindow.destroy()

    def nsmset(self):
        nl = 0
        for x in gChannel:
            print(x[1] + " " + str(self.evar[nl].get()))
            nl += 1

    def nsmget(self):
        nl = 0
        for x in gChannel:
            # call nsmget
            val = 0
            self.var[nl].set(val)
            nl += 1

        self.close_window()


class Sector:

    def __init__(self, master, name, id):
        self.master = master
        self.id = id
        self.frame = tk.Frame(self.master)
        self.channels = []
        self.counter = 0
        self.name = name
        if id < 0:
            opt = id
            mcnt = 0
            nch = 16
        else:
            opt = "sector"
            mcnt = id
            nch = 16

        self.opt = opt

        Channel(self.frame, "Name", -1, opt)

        for i in range(nch):
            id = i + mcnt * nch
            name = gMap[id]['name']
            self.channels.append(Channel(self.frame, name, id, opt))
        self.frame.pack()

    def new_canvas(self, parent, name, width, height):
        global font

        self.frame = tk.Frame(parent, width=width, height=height)
        if name == "Vmon":
            self.vmon = TCanvas(self.frame, "Vmon;time;U(V)", time.time() - 400, time.time() + 32, -5, 10)
            self.vmon.setXscrollable(1)
        else:
            self.imon = TCanvas(self.frame, "Imon;time;U(V)", time.time() - 400, time.time() + 32, -5, 10)
            self.imon.setXscrollable(1)

        for channel in self.channels:
            if name == "Vmon":
                self.vmon.addGraph(channel.id, channel.name, color=channel.id, width=2)
            else:
                self.imon.addGraph(channel.id, channel.name, color=channel.id, width=2)

        self.frame.pack()
        return self.frame

    def redraw(self):
        if hasattr(self, 'vmon'):
            self.vmon.redraw()
        if hasattr(self, 'imon'):
            self.imon.redraw()

    def addPoint(self):
        nl = 0
        for channel in self.channels:
            val = channel.var[6].get()
            # x = float(self.counter)
            x = time.time()
            y = channel.id % 16 + math.sin(x * 0.1)
            if hasattr(self, 'vmon'):
                self.vmon.addPoint(channel.id, x, y)
            if hasattr(self, 'imon'):
                self.imon.addPoint(channel.id, x, y * y / 20.)
        nl += 1
        self.counter += 1


class TimeViewCanvas:

    def __init__(self, parent, name, width, height):
        self.parent = parent
        self.width = width
        self.height = height
        self.name = name

    def show(self):
        if (hasattr(self, 'window') and self.window.winfo_exists()):
            self.window.update()
            self.window.deiconify()
        else:
            self.init()

    def init(self):
        self.window = tk.Toplevel(self.parent)
        self.window.wm_title(self.name)
        self.notebook = ttk.Notebook(self.window)

        for sector in self.parent.sectors:
            tab = ttk.Frame(self.notebook)
            self.notebook.add(tab, text=sector.name)
            sector.new_canvas(tab, self.name, self.width, self.height)
        self.notebook.pack()
        f = tk.Frame(self.window)
        tk.Button(f, text='Close', width=5, command=lambda: self.close(), font=font).pack(side="left")
        tk.Button(f, text='Export to eps', width=10, command=lambda: self.epsexport(), font=font).pack(side="left")
        f.pack()

    def epsexport(self):
        selected = self.notebook.tab(self.notebook.select(), "text")
        for sector in self.parent.sectors:
            if sector.name == selected:
                file = "arichlv-{}-{}.eps".format(sector.name, time.strftime("%Y_%m_%d_%H_%M", time.localtime()))
                sector.canvas.canvas.postscript(file=file, colormode='color')

    def redraw(self):
        if not hasattr(self, 'notebook'):
            return
        if not self.notebook.winfo_exists():
            return
        # nb.index(nb.select())
        selected = self.notebook.tab(self.notebook.select(), "text")
        for sector in self.parent.sectors:
            if sector.name == selected:
                sector.redraw()

    def close(self):
        self.window.withdraw()
#    self.window.destroy()


class MainApplication(tk.Frame):

    def __init__(self, parent, *args, **kwargs):
        global font
        tk.Frame.__init__(self, parent, *args, **kwargs)
        # create a custom font
        font = tkFont.Font(family="Arial", size=10)
        s = ttk.Style()
        s.configure('.', font=font)
        self.counter = 0
        # create a menu bar
        mainMenu = tk.Menu(parent)
        parent.config(menu=mainMenu)

        # create a file menu with an exit entry
        # you may need to add more entries
        fileMenu = tk.Menu(mainMenu)
        fileMenu.add_command(label='Exit', command=self.quit)
        fileMenu.add_command(label='Export Configuration', command=lambda: self.exportSettings())
        mainMenu.add_cascade(label='File', menu=fileMenu)

        # create a view menu
        viewMenu = tk.Menu(mainMenu)
        viewMenu.add_command(label='Time Evolution of Vmon', command=lambda: self.histoVmon.show())
        viewMenu.add_command(label='Time Evolution of Imon', command=lambda: self.histoImon.show())
        mainMenu.add_cascade(label='View', menu=viewMenu)

        # any main menu should have a help entry
        helpMenu = tk.Menu(mainMenu)
        helpMenu.add_command(label='Expert panel', command=self.expert_panel)
        helpMenu.add_command(label='About', command=self.show_about)
        mainMenu.add_cascade(label='Help', menu=helpMenu)

        self.ctime = tk.StringVar()
        self.ctime.set("")
        tk.Label(parent, textvariable=self.ctime, font=font).pack()
        self.p = ttk.Panedwindow(parent, orient="horizontal")

        self.state = ttk.Labelframe(self.p, text='State', width=200, height=100)
        self.mainstate = tk.StringVar()
        self.mainstate.set("Unknown")
        self.lmainstate = tk.Label(self.state, textvariable=self.mainstate, width=15, font=font, bg='yellow')
        self.lmainstate.pack()
        self.photo = tk.PhotoImage(file=pathname + "/logo.gif")
        self.w = tk.Label(self.state, image=self.photo)
#   self.w.photo = self.photo
        self.w.pack()

# second pane
        self.main = ttk.Labelframe(self.p, text='Main Commands', width=300, height=100)
        f1 = ttk.Frame(self.main)
        self.turnon = tk.Button(f1, text='TURNON', width=25, command=self.turnoncb, font=font)
        self.standby = tk.Button(f1, text='STANDBY', width=25, command=self.standbycb, font=font)
        self.turnon.pack(side="left")
        self.standby.pack(side="left")
        f1.pack()
        f2 = ttk.Frame(self.main)
        self.turnoff = tk.Button(f2, text='TURNOFF', width=25, command=self.turnoffcb, font=font)
        self.recover = tk.Button(f2, text='RECOVER', width=25, command=self.recovercb, font=font)
        self.turnoff.pack(side="left")
        self.recover.pack(side="left")
        f2.pack()
#    self.export = tk.Button(self.main, text = 'Export settings', width = 25, command = self.exportSettings,font=font)
#    self.export.pack(side = "left")
#    self.quit = tk.Button(self.main, text = 'Exit', width = 25, command = self.quitcb,font=font)
#    self.quit.pack(side = "left")
        self.p.add(self.state)
        self.p.add(self.main)
        self.p.pack()

        self.status = ttk.Labelframe(self, text='ARICH LV Status', width=500, height=100)

        self.tab = ttk.Notebook(self.status)
        self.sectors = []
        self.window = []
        for i in range(-1, 6):
            tab = ttk.Frame(self.tab)
            if i >= 0:
                name = 'Sector ' + str(i + 1)
                self.tab.add(tab, text=name)
                self.sectors.append(Sector(tab, name, i))
            else:
                name = 'General'
                self.tab.add(tab, text='General')
                Sector(tab, name, i)
        self.tab.pack()

        self.status.pack()
        self.histoVmon = TimeViewCanvas(self, "Vmon", 800, 600)
        self.histoImon = TimeViewCanvas(self, "Imon", 800, 600)
        f3 = ttk.Frame(self)
        self.scrollbar = tk.Scrollbar(f3)
        self.scrollbar.pack(side='right', fill='y')
        self.message = tk.Text(f3, width=80, height=9, font=font)
        self.message.pack(side='left', fill='y')
        self.scrollbar.config(command=self.message.yview)
        self.message.config(yscrollcommand=self.scrollbar.set)
        f3.pack()
        self.timerinterval = 1000
        t0 = threading.Thread(target=self.start_timer)
        t0.daemon = True
        t0.start()

    def show_about(self):
        # show the about box
        msg.showinfo(
            'About',
            'Belle II ARICH LV Control\nprogram for control of the \nBelle II ARICH LV Power Supplies\nRok Pestotnik@2017')

    def turnoncb(self):
        print("Turn On")

    def turnoffcb(self):
        print("Turn Off")

    def standbycb(self):
        print("Standby")

    def recovercb(self):
        print("Recover")

    def quitcb(self):
        sys.exit()

    def gprint(self, txt):
        self.message.insert('end', txt)
        self.message.see("end")

    def start_timer(self):
        self.addPoint()
        self.redraw()
#      self.gprint(time.ctime()+"\n")
        self.ctime.set(time.ctime())

        self.after(self.timerinterval, self.start_timer)

    def expert_panel(self):
        global font
        self.expertwindow = tk.Toplevel(self.master)
        self.expertwindow.wm_title("ARICH expert panel")
        self.expertframe = tk.Frame(self.expertwindow)

        self.expert = Sector(self.expertframe, "Expert panel", -2)

        f0 = tk.Frame(self.expertframe)
        tk.Label(f0, text="With selected: ", width=15, font=font).pack(side="left")
        tk.Button(f0, text='Switch on', width=15, command=lambda: self.expertCmd(1), font=font).pack(side="left")
        tk.Button(f0, text='Switch off', width=15, command=lambda: self.expertCmd(2), font=font).pack(side="left")
        tk.Button(f0, text='Set Vset to', width=15, command=lambda: self.expertCmd(3), font=font).pack(side="left")
        self.setvalue = tk.DoubleVar()
        tk.Entry(f0, textvariable=self.setvalue, width=10, font=font).pack(side="left")
        f0.pack()
        tk.Button(self.expertframe, text='Close', width=5, command=self.closeWindow, font=font).pack()

        self.expertframe.pack()

    def expertCmd(self, type):
        print("Cmd {}\n", type)
        nc = 0
        for channel in self.expert.channels:
            ns = 0
            for sector in channel.var:
                if sector.get():
                    print("Sector {}, Channel {}\n", ns, nc)
                ns += 1
            nc += 1

    def closeWindow(self):
        self.expertwindow.destroy()

    def exportSettings(self):
        global gChannel

        # define options for opening or saving a file
        self.file_opt = options = {}
        options['defaultextension'] = ''
        options['filetypes'] = [('all files', '.*'), ('config files', '.conf')]
        options['initialdir'] = '.'
        options['initialfile'] = 'arichlv-input.conf'
        options['parent'] = self
        options['title'] = 'Save configuration file as'
        # filename = filedialog.asksaveasfilename(**self.file_opt)
        filename = filedialog.asksaveasfilename(initialdir=".", filetypes=(
            ('all files', '.*'), ('config files', '.conf')), title="Save configuration file as")
        if len(filename) == 0:
            return
        f = open(filename, 'w')
        f.write("config   : test:001\n")
        f.write("nodename : ARICHLV\n")
        nsectors = 0
        for sector in self.sectors:
            f.write("\n\n")
            if nsectors % 3 == 0:

                ncrate = nsectors // 3
                f.write("crate[{}].name   : arich-mpod{}.kek.jp\n".format(ncrate, 1 - ncrate))
                f.write("crate[{}].port   : 25\n".format(ncrate))

            for channel in sector.channels:
                nl = 0
                ncrate = channel.var[0].get()
                nslot = channel.var[1].get()
                nch = channel.var[2].get()

                for x in gChannel:
                    if ('set' in x[4]):
                        if channel.id >= 0:
                            f.write("crate[{}].channel[{}].{}\t:{}\n".format(ncrate, channel.id % 48, x[3], channel.var[nl].get()))
                    nl += 1
                f.write("\n\n")
            nsectors += 1
        f.close()
        print("Settings are exported to {}\n", filename)

    def exists(self, name, obj=None):
        if obj is None:
            return name in locals()
        else:
            return hasattr(obj, name)

    def addPoint(self):
        for sector in self.sectors:
            sector.addPoint()

    def redraw(self):
        self.histoVmon.redraw()
        self.histoImon.redraw()


def toggle_font(event):
    if event.keysym == '0':
        font['size'] = -12
    elif event.keysym == 'plus':
        if font['size'] > -31:
            font['size'] = font['size'] - 1
    elif event.keysym == 'minus':
        if font['size'] < -6:
            font['size'] = font['size'] + 1


if __name__ == "__main__":
    root = tk.Tk()
    VERSION = 1.0
    root.title("ARICH LV Control v%.1f" % VERSION)
    mainapp = MainApplication(root)
    mainapp.pack(side="top", fill="both", expand=True)
    print("Tk Version {} {} {} {}\n", tk.TkVersion, 1, 2, 3)
    root.bind('<Control-plus>', toggle_font)
    root.bind('<Control-minus>', toggle_font)
    root.bind('<Control-0>', toggle_font)

    callback = TestNSMCallback()
    args = [callback]
    threading.Thread(target=th_func, args=args).start()
    if callback.available:
        val = callback.getText("ARICHLV", "crate[1].slot[1].channel[7].switch")
        print("get crate[1].slot[1].channel[7].switch = %s" % val)
        if val == "ON":
            print("set crate[1].slot[1].channel[7].switch = OFF")
            callback.setText("ARICHLV", "crate[1].slot[1].channel[7].switch", "OFF")
        if val == "OFF":
            print("set crate[1].slot[1].channel[7].switch = ON")
            callback.setText("ARICHLV", "crate[1].slot[1].channel[7].switch", "ON")

    root.mainloop()
