# graph.py
# class to show a moving graph
# written by Roger Woollett

from sys import version_info
if version_info[0] < 3:
    import Tkinter as tk
else:
    import tkinter as tk
import time

# import numpy as np
mycolors = ['red', 'green', 'blue', 'yellow', 'black', 'orange', 'gray', 'gold']


class TGraph:
    # class to represent a single trace in a ScrollGraph

    def __init__(self, master, npoints, name, title, color, width):

        self.master = master
        self.canvas = master.canvas
        self.canvas.create_line((0, 0, 0, 0), tag=name, fill=color, width=width)
        self.npoints = npoints
        self.name = name
        self.points = []
        self.xmin = 0
        self.xmax = 0
        self.ymin = 0
        self.ymax = 0
# self.xpoints = np.empty([1000, 1], dtype=float)
# self.ypoints = np.empty([1000, 1], dtype=float)

    def addPoint(self, x, y):
        """
        Update the cached data lists with new sensor values.
        """
        if x > self.xmax:
            self.xmax = x
        if x < self.xmin:
            self.xmin = x
        if x > self.ymax:
            self.ymax = y
        if x < self.ymin:
            self.ymin = y

        self.points.append([float(x), float(y)])
# np.append(self.xpoints,float(x))
# np.append(self.ypoints,float(y))
# self.points = self.points[-1 * self.npoints:]

        return

    def redraw(self):
        """
        Update the canvas graph lines from the cached data lists.
        The lines are scaled to match the canvas size as the window may
        be resized by the user.
        """
        if not self.canvas.winfo_exists():
            return
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

# y0 = h - (self.ypoints-self.master.ymin)/(self.master.ymax-self.master.ymin)*h
# x0 = (self.xpoints-self.master.xmin)/(self.master.xmax-self.master.xmin)*w
# coordsX = lambda l: [item for sublist in l for item in sublist]

        coordsX = []

        if (self.master.xscrollable and self.xmax > self.master.xmax):
            dx = self.master.xmax - self.master.xmin
            self.master.xmax += 0.75 * dx
            self.master.xmin += 0.75 * dx
            self.master.xaxis.redraw()

        yredraw = 0
        if (self.master.yminauto and self.ymin < self.master.ymin):
            self.master.ymin = self.ymin
            yredraw = 1
        if (self.master.ymaxauto and self.ymax > self.master.ymax):
            self.master.ymax = self.ymax
            yredraw = 1
        if yredraw:
            self.master.yaxis.redraw(1)
        if self.master.selected[self.name].get():
            for point in self.points:
                x = (point[0] - self.master.xmin) / (self.master.xmax - self.master.xmin) * w
                y = h - (point[1] - self.master.ymin) / (self.master.ymax - self.master.ymin) * h
                if x > self.master.xoffset and x < w:
                    coordsX.append(float(x))
                    coordsX.append(float(y))
        else:
            coordsX.append(0)
            coordsX.append(0)
            coordsX.append(0)
            coordsX.append(0)
        if len(coordsX) > 2 and self.canvas.winfo_exists():
            self.canvas.coords(self.name, *coordsX)


class TAxis:
    # class to show  axis with ticks

    def __init__(self, parent, name, ndiv, width=2, *args, **kwargs):
        [big, small] = str(ndiv).split(".", 2)
        self.parent = parent
        self.width = width
        self.dx = [[int(big), 10], [int(big) * int(small), 5]]
        self.name = name
        self.labels = []
        self.ticks = []
        self.format = "{:3.2G}"
        self.date = 0
        self.dateformat = "%Y/%m/%d\n%H:%M"

    def setLabelsFormat(self, fmt):
        self.format = fmt

    def setDateLabels(self, mode, fmt="%Y/%m/%d\n%H:%M"):
        self.date = mode
        self.dateformat = fmt

    def redraw(self, redrawlines=0):
        # print("Redrawing Axis ", self.name, self.parent.xmax ,self.parent.xmin)
        # zbrisi osi
        if redrawlines:
            for el in self.ticks:
                self.parent.canvas.delete(el)
            self.ticks = []
        if len(self.ticks) == 0:
            redrawlines = 1
        for el in self.labels:
            self.parent.canvas.delete(el)
        self.labels = []
        o = [self.parent.xoffset, self.parent.yoffset]
        wsize = [int(self.parent.canvas['width']) - self.parent.xoffset, int(self.parent.canvas['height']) - self.parent.yoffset]
        if redrawlines:
            if self.name == "Y":
                self.ticks.append(self.parent.canvas.create_line(o[0], 0, o[0], wsize[1], width=self.width))
                self.ticks.append(self.parent.canvas.create_line(wsize[0] + o[0], 0, wsize[0] + o[0], wsize[1], width=self.width))
            else:
                self.ticks.append(self.parent.canvas.create_line(o[0], 0, wsize[0] + o[0], 0, width=self.width))
                self.ticks.append(self.parent.canvas.create_line(o[0], wsize[1], wsize[0] + o[0], wsize[1], width=self.width))

        for ix in range(2):
            dx = self.dx[ix]
            if dx[0] > 0:
                tick = dx[1]
                j = 0
                while j < dx[0]:
                    if self.name == "Y":
                        yc = j * wsize[1] / dx[0]
                        yctxt = (self.parent.ymax - self.parent.ymin) * yc / wsize[1] + self.parent.ymin
                        x = o[0]
                        y = wsize[1] - yc

                        if ix == 0:
                            if self.date:
                                txt = time.strftime(self.dateformat, time.localtime(int(yctxt)))
                            else:
                                txt = self.format.format(yctxt)
                            self.labels.append(self.parent.canvas.create_text(10, y, text=txt, anchor="w"))
                        if redrawlines:
                            self.ticks.append(self.parent.canvas.create_line(x, y, tick + x, y, width=self.width))

                    else:
                        xc = j * wsize[0] / dx[0]
                        xctxt = (self.parent.xmax - self.parent.xmin) * xc / wsize[0] + self.parent.xmin
                        x = o[0] + xc
                        y = wsize[1]

                        if ix == 0:
                            if self.date:
                                txt = time.strftime(self.dateformat, time.localtime(int(xctxt)))
                            else:
                                txt = self.format.format(xctxt)
                            self.labels.append(self.parent.canvas.create_text(x, y + o[1] - 20, text=txt, anchor="w"))
                        if redrawlines:
                            self.ticks.append(self.parent.canvas.create_line(x, y, x, y - tick, width=self.width))
                    j += 1


class TCanvas:
    # show data for one core

    def __init__(self, parent, title, xmin, xmax, ymin, ymax):
        global mycolors
        self.parent = parent
        col0 = 30

        self.npoints = 100

        self.height = 500
        self.width = 800
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax
        self.xoffset = 35
        self.yoffset = 35
        self.xscrollable = 0
        self.yminauto = 0
        self.ymaxauto = 0
        [titlem, titlex, titley] = title.split(";", 2)
        tk.Label(parent, text=titlem, width=20, fg='black', pady=0, bd=0).pack()
        fmain = tk.Frame(parent)
        f0 = tk.Frame(fmain)

        self.winlegend = tk.Frame(f0)

        tk.Label(self.winlegend, text="Channels:", width=20, fg='black', pady=0, bd=0).pack()
        self.winlegend.pack(side='left')

        tk.Label(f0, text=titley, fg='black', pady=0, bd=0).pack(side='left')

        self.canvas = tk.Canvas(
            f0,
            background="white",
            width=self.width +
            self.xoffset,
            height=self.height +
            self.yoffset,
            bd=0,
            highlightthickness=0)

        self.yaxis = TAxis(self, "Y", 10.4)
        self.xaxis = TAxis(self, "X", 10.4)
        self.xaxis.setDateLabels(1, "%Y/%m/%d\n%H:%M")
        self.yaxis.redraw()
        self.xaxis.redraw()
        self.canvas.pack(side='left')
        f0.pack(side='top')
        tk.Label(fmain, text=titlex, fg='black', pady=0, bd=0).pack(side='top')
        fmain.pack()

        self.graphs = {}
        self.legend = {}
        self.selected = {}
# self.test = []

    def setXscrollable(self, mode):
        self.xscrollable = mode

    def setYminAuto(self, mode):
        self.yminauto = mode

    def setYmaxAuto(self, mode):
        self.ymaxauto = mode

    def setXoffset(self, offset):
        self.xoffset = offset

    def setYoffset(self, offset):
        self.yoffset = offset

    def addPoint(self, id, x, y):
        # print("add Point " , id, x ,y)
        self.graphs["id" + str(id)].addPoint(x, y)

    def addGraph(self, id, title, color, width):
        global mycolors
        name = "id" + str(id)
        self.graphs[name] = TGraph(self, self.canvas, name, title, mycolors[color % 8], width)

        self.selected[name] = tk.IntVar()
        self.selected[name].set(1)
        self.legend[name] = tk.Checkbutton(self.winlegend, text=title, fg=mycolors[color % 8], variable=self.selected[name])
        self.legend[name].pack(side='top')

# self.test.append(tk.IntVar())
# tk.Checkbutton(self.winlegend, text=name, fg = mycolors[color%8], variable=self.test[-1]).pack()
    def deleteGraph(self, name):
        print("Deleting ..." + name)

    def redraw(self):
        if not self.parent.winfo_exists():
            return
        w = self.canvas.winfo_width() - self.xoffset
        h = self.canvas.winfo_height() - self.yoffset
        if (w != self.width or h != self.height):
            self.height = int(self.canvas['height']) - self.xoffset
            self.width = int(self.canvas['width']) - self.yoffset
            self.xaxis.redraw()
        for name in self.graphs:
            self.graphs[name].redraw()
