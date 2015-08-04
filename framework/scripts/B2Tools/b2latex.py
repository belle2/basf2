#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess


class LatexObject(object):
    """
    Common base class of all Latex Wrapper objects
    """
    def __init__(self):
        self.output = ''

    def __str__(self):
        return self.output

    def add(self):
        return self

    def finish(self):
        return self

    def save(self, filename, compile=True):
        output = r"""
            \documentclass[10pt,a4paper]{article}
            \usepackage[latin1]{inputenc}
            \usepackage{amsmath}
            \usepackage{amsfonts}
            \usepackage{amssymb}
            \usepackage{graphicx}
            \usepackage{subcaption}
            \usepackage{lmodern}
            \usepackage{placeins}
            \usepackage{multicol}
            \usepackage{tikz}
            \usetikzlibrary{shapes.arrows,chains, positioning}
            \usepackage{booktabs} %professional tables
            \usepackage[left=2cm,right=2cm,top=2cm,bottom=2cm]{geometry}
            \usepackage{microtype} %optimises spacing, needs to go after fonts
            \usepackage{hyperref} %adds links (also in TOC), should be loaded at the very end
            \usepackage{caption}
            \usepackage{longtable}
            \usepackage[load-configurations=abbreviations]{siunitx}
            \begin{document}
            """
        output += self.output
        output += r"\end{document}"

        file(filename, 'w').write(output)
        if compile:
            for i in range(0, 2):
                ret = subprocess.call(['pdflatex', '-halt-on-error', '-interaction=nonstopmode', filename])
                if ret != 0:
                    raise RuntimeError("pdflatex failed to create FEI summary PDF, please check.")
        return self


class LatexFile(LatexObject):
    def add(self, text=''):
        self.output += str(text)
        return self

    def __iadd__(self, text):
        self.add(text)
        return self


class String(LatexObject):
    def __init__(self, text=''):
        super(String, self).__init__()
        self.output += str(text)

    def add(self, text=''):
        self.output += str(text)
        return self

    def finish(self, **kwargs):
        self.output = self.output.format(kwargs) + '\n'
        return self


class DefineColourList(LatexObject):
    def __init__(self):
        super(DefineColourList, self).__init__()
        self.colours = ["red", "green", "blue", "orange", "cyan", "purple"]
        self.output += r"\def\colourlist{{" + ', '.join('"%s"' % (c) for c in self.colours) + r"}}" + '\n'
        self.output += r"""
            \tikzset{nodeStyle/.style={text height=\heightof{A},text depth=\depthof{g}, inner sep = 0pt, node distance = -0.15mm}}
            \newcount\colourindex \colourindex=-1
            \newcommand{\plotbar}[1]{
            \begin{tikzpicture}[start chain=going right, nodes = {font=\sffamily}]
              \global\colourindex=-1
              \foreach \percent/\name in {
                #1
              } {
                \ifx\percent\empty\else               % If \percent is empty, do nothing
                  \global\advance\colourindex by 1
                  \ifnum"""
        self.output += str(len(self.colours) - 1)
        self.output += r"""5<\colourindex  %back to first colour if we run out
                    \global\colourindex=0
                  \fi
                  \pgfmathparse{\colourlist[\the\colourindex]} % Get color from cycle list
                  \edef\color{\pgfmathresult}         %   and store as \color
                  \node[nodeStyle, draw, on chain, fill={\color!40}, minimum width=\percent*1.0, minimum height=12] {\name};
                \fi
              };
            \end{tikzpicture}
            }
            """


class Section(LatexObject):
    def __init__(self, name):
        super(Section, self).__init__()
        self.output += r"\raggedbottom" + '\n'
        self.output += r"\pagebreak[0]" + '\n'
        self.output += r"\FloatBarrier" + '\n'
        self.output += r"\section{" + str(name) + r"}" + '\n'


class SubSection(LatexObject):
    def __init__(self, name):
        super(SubSection, self).__init__()
        self.output += r"\subsection{" + str(name) + r"}" + '\n'


class SubSubSection(LatexObject):
    def __init__(self, name):
        super(SubSubSection, self).__init__()
        self.output += r"\subsubsection{" + str(name) + r"}" + '\n'


class Graphics(LatexObject):
    def __init__(self):
        super(Graphics, self).__init__()
        self.output += r"\begin{center}" + '\n'

    def add(self, filename, width=0.7):
        self.output += r"\includegraphics[width=" + str(width) + r"\textwidth]"
        self.output += r"{" + str(filename) + r"}" + '\n'
        return self

    def finish(self):
        self.output += r"\end{center}" + '\n'
        return self


class Itemize(LatexObject):
    def __init__(self):
        super(Itemize, self).__init__()
        self.output += r"\begin{itemize}"

    def add(self, item):
        self.output += r"\item " + str(item) + '\n'
        return self

    def finish(self):
        self.output += r"\end{itemize}"
        return self


class LongTable(LatexObject):
    def __init__(self, columnspecs, caption, format_string, head):
        super(LongTable, self).__init__()
        self.output += r"\begin{center}" + '\n'
        self.output += r"\begin{longtable}{" + str(columnspecs) + r"}" + '\n'
        self.output += r"\caption{" + str(caption) + r"}\\" + '\n'
        self.output += r"\toprule" + '\n'
        self.output += head + r"\\" + '\n'
        self.output += r"\midrule" + '\n'
        self.format_string = format_string

    def add(self, **kwargs):
        self.output += self.format_string.format(**kwargs) + r"\\" + '\n'
        return self

    def finish(self, tail=''):
        self.output += r"\bottomrule" + '\n'
        if str(tail) != '':
            self.output += str(tail) + r"\\" + '\n'
            self.output += r"\bottomrule" + '\n'
        self.output += r"\end{longtable}" + '\n'
        self.output += r"\end{center}" + '\n'
        return self


class TitlePage(LatexObject):
    def __init__(self, title, authors, abstract, add_table_of_contents=True):
        super(TitlePage, self).__init__()
        self.output += r"\author{"
        for author in authors:
            self.output += author + r"\\"
        self.output += r"}" + '\n'
        self.output += r"\date{\today}" + '\n'
        self.output += r"\title{" + title + r"}" + '\n'
        self.output += r"\maketitle" + '\n'
        if abstract:
            self.output += r"\begin{abstract}" + '\n'
            self.output += abstract
            self.output += '\n' + r'\end{abstract}' + '\n'

        self.output += r"\clearpage" + '\n'
        if add_table_of_contents:
            self.output += r"\tableofcontents" + '\n'
            self.output += r"\FloatBarrier" + '\n'
            self.output += r"\clearpage" + '\n'


if __name__ == '__main__':
    import random
    import subprocess

    o = LatexFile()
    o += TitlePage(title='Automatic Latex Code Example',
                   authors=['Thomas Keck'],
                   abstract='This is an example for automatic latex code generation in basf2.',
                   add_table_of_contents=True).finish()

    o += Section("LongTable").finish()
    o += String(r"In the following subsection there's:")
    o += Itemize().add("a really long table").add("a coloured table").add("nothing else").finish()
    o += SubSection("A really long table")

    table = LongTable(r"lr", "A long table", "{name} & {value:.2f}", r"Name & Value in $\mathrm{cm}$")
    for i in range(60):
        table.add(name='test' + str(i), value=random.gauss(0.0, 1.0))
    o += table.finish()

    o += SubSection("A table with color and tail")
    colour_list = DefineColourList()
    o += colour_list.finish()

    table = LongTable(columnspecs=r"lcrrr",
                      caption="A coloured table for " +
                              ', '.join(('\\textcolor{%s}{%s}' % (c, m) for c, m in zip(colour_list.colours[:3], "RGB"))),
                      format_string="{name} & {bargraph} & {r:.2f} & {g:.2f} & {b:.2f}",
                      head=r"Name & Relative fractions & R - Value & G - Value & B - Value")
    sr = sg = sb = 0
    for i in range(10):
        r = random.uniform(0.1, 0.9)
        g = random.uniform(0.1, 0.9)
        b = random.uniform(0.1, 0.9)
        n = (r + g + b) / 100.0
        sr += r
        sg += g
        sb += b
        table.add(name='test' + str(i), bargraph=r'\plotbar{ %g/, %g/, %g/,}' % (r/n, g/n, b/n), r=r, g=g, b=b)
    n = (sr + sg + sb) / 100.0
    o += table.finish(tail=r"Total & \plotbar{ %g/, %g/, %g/,} & %g & %g & %g" % (sr/n, sg/n, sb/n, sr, sg, sb))

    o += Section("Graphic section")
    graphics = Graphics()
    images = subprocess.check_output("locate .png", shell=True).split('\n')[:-1]
    for i in range(4):
        image = random.choice(images)
        graphics.add(image, width=0.49)
    o += graphics.finish()

    import format
    o += Section("Format section e.g. " + format.decayDescriptor("B+ ==> mu+ nu gamma"))
    o += r"Some important channels in B physics"
    items = Itemize()
    items.add(format.decayDescriptor("B+ ==> mu+ nu gamma"))
    items.add(format.decayDescriptor("B0 ==> J/Psi K_S0"))
    items.add(format.decayDescriptor("B+ ==> tau+ nu"))
    o += items.finish()

    o += r"Use format.variable to add hyphenations to long variable names, so latex can do line breaks "
    o += format.variable("daughterProductOf(extraInfo(SignalProbability))")
    o += r" . Use format.string for stuff that contains special latex characters like " + format.string(r"_ ^ \ ")
    o += r" . Use format.duration for durations " + format.duration(20000) + " " + format.duration(0.00010)

    o.finish()
    o.save("test.tex", compile=True)
