#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import subprocess


class LatexObject(object):
    """
    Common base class of all Latex Wrapper objects
    """

    def __init__(self):
        """
        Constructor, initialize output with empty string
        """
        #: Stores the outputted latex-code
        self.output = ''

    def __str__(self):
        """
        Transform object to string, in this case, just returns .the generated latex-code
        """
        return self.output

    def add(self):
        """
        Add latex-code to the output string.
        This method is usually overriden in the subclasses
        """
        return self

    def finish(self):
        """
        Finishes the generation of latex-code.
        E.g. adds end latex-commands
        This method is usually overriden in the subclasses
        """
        return self

    def save(self, filename, compile=True):
        """
        Saves the latex-code into a file, adds preamble and end of document,
        and compiles the code if requested.
            @param filename latex-code is stored in this file, should end on .tex
            @param compile compile the .tex file using pdflatex into a .pdf file
        """
        output = r"""
            \documentclass[10pt,a4paper]{article}
            \usepackage[latin1]{inputenc}
            \usepackage[T1]{fontenc}
            \usepackage{amsmath}
            \usepackage{amsfonts}
            \usepackage{amssymb}
            \usepackage{graphicx}
            \usepackage{caption}
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
            \usepackage{longtable}
            \usepackage{color}
            \usepackage{listings}

            \definecolor{gray}{rgb}{0.4,0.4,0.4}
            \definecolor{darkblue}{rgb}{0.0,0.0,0.6}
            \definecolor{cyan}{rgb}{0.0,0.6,0.6}

            \lstset{
              basicstyle=\ttfamily\scriptsize,
              columns=fullflexible,
              showstringspaces=false,
              commentstyle=\color{gray}\upshape
            }

            \lstdefinelanguage{XML}
            {
              morestring=[b]",
              morestring=[s]{>}{<},
              morecomment=[s]{<?}{?>},
              stringstyle=\color{black},
              identifierstyle=\color{darkblue},
              keywordstyle=\color{cyan},
              morekeywords={xmlns,version,type}% list your attributes here
            }

            \usepackage[load-configurations=abbreviations]{siunitx}
            \makeatletter
            % In newer versions of latex there is a problem with the calc package and tikz
            % http://tex.stackexchange.com/questions/289551/how-to-resolve-conflict-between-versions-of-texlive-and-pgf
            \def\pgfmathparse@#1{%
                % Stuff for calc compatiability.
                \let\real=\pgfmath@calc@real
                \let\minof=\pgfmath@calc@minof
                \let\maxof=\pgfmath@calc@maxof
                \let\ratio=\pgfmath@calc@ratio
                \let\widthof=\pgfmath@calc@widthof
                \let\heightof=\pgfmath@calc@heightof
                \let\depthof=\pgfmath@calc@depthof
                % No (math) units yet.
                \global\pgfmathunitsdeclaredfalse
                \global\pgfmathmathunitsdeclaredfalse
                % Expand expression so any reamining CSs are registers
                % or box dimensions (i.e. |\wd|, |\ht|, |\dp|).
                \edef\pgfmath@expression{#1}%
                    %
                    \expandafter\pgfmathparse@trynumber@loop\pgfmath@expression\pgfmath@parse@stop
                    %
                    % this here is the _real_ parser. it is invoked by
                    % \pgfmathparse@trynumber@loop if that says "this is no number"
                    %\pgfmathparse@@\pgfmath@parse@stop%
                }
            \makeatother
            \begin{document}
            """
        output += self.output
        output += r"\end{document}"

        with open(filename, 'w') as f:
            f.write(output)
        if compile:
            for i in range(0, 2):
                ret = subprocess.call(['pdflatex', '-halt-on-error', '-interaction=nonstopmode', filename])
                if ret != 0:
                    raise RuntimeError("pdflatex failed to create FEI summary PDF, please check.")
        return self


class LatexFile(LatexObject):
    """
    Convinience class implementing += operator, can be used instead of raw LatexObject to collect
    all the latex code in your project which should go into a common file.
    """

    def add(self, text=''):
        """
        Adds an object to the output
            @param text string or object with implicit string conversion (like LatexObject)
        """
        self.output += str(text)
        return self

    def __iadd__(self, text):
        """
        Adds an object to the output
            @param text string or object with implicit string conversion (like LatexObject)
        """
        self.add(text)
        return self


class String(LatexObject):
    """
    Used for wrapping conventionel text into latex-code.
    Has to possibility to handle python-style format-placeholders
    """

    def __init__(self, text=''):
        """
        Calls super-class initialize and adds initial text to output
            @param text intial text, usually you want to give a raw string r"some text"
        """
        super(String, self).__init__()
        #: output string
        self.output += str(text)

    def add(self, text=''):
        """
        Adds an object to the output, can contain python-placeholders
            @param text string or object with implicit string conversion (like LatexObject)
        """
        self.output += str(text)
        return self

    def finish(self, **kwargs):
        """
        Finish the generation of the string by replacing possible placehholders with the given dictionary
            @param kwargs dictionary used to replace placeholders
        """
        #: output string
        self.output = self.output.format(**kwargs) + '\n'
        return self


class Listing(LatexObject):
    """
    Used for wrapping code in a listing environment
    """

    def __init__(self, language='XML'):
        """
        Calls super-class initialize and adds initial text to output
            @param text intial text, usually you want to give a raw string r"some text"
        """
        super(Listing, self).__init__()
        #: output string
        self.output += r'\lstset{language=' + language + '}\n'
        self.output += r'\begin{lstlisting}[breaklines=true]' + '\n'

    def add(self, text=''):
        """
        Adds code to the output
            @param code which is wrapped in the listing environment
        """
        self.output += str(text)
        return self

    def finish(self, **kwargs):
        """
        Finish the generation of the lsiting environment
        """
        #: output string
        self.output += r'\end{lstlisting}'
        return self


class DefineColourList(LatexObject):
    """
    Defines the colourlist latex-command, which draws a bargraph with relative
    fractions indicated by colours using tikz.
    After including this object in you latex code the command \\bargraph is available.
    You should include only one of these objects in your latex code.
    """

    #: 6 default colours used for the bargraph
    colours = ["red", "green", "blue", "orange", "cyan", "purple"]

    def __init__(self):
        """
        Calls super-class init, adds definition of colourlist to latex code.
        """
        super(DefineColourList, self).__init__()
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
    """
    Adds a new section to your latex code with some additional commands
    to force a pagebreak and add a barrier for figure objects.
    """

    def __init__(self, name):
        """
        Calls super-class init and adds necessary latex commands to output.
        """
        super(Section, self).__init__()
        self.output += r"\raggedbottom" + '\n'
        self.output += r"\pagebreak[0]" + '\n'
        self.output += r"\FloatBarrier" + '\n'
        self.output += r"\section{" + str(name) + r"}" + '\n'


class SubSection(LatexObject):
    """
    Adds a new subsection to your latex code.
    """

    def __init__(self, name):
        """
        Calls super-class init and adds necessary latex commands to output.
        """
        super(SubSection, self).__init__()
        self.output += r"\subsection{" + str(name) + r"}" + '\n'


class SubSubSection(LatexObject):
    """
    Adds a new subsubsection to your latex code.
    """

    def __init__(self, name):
        """
        Calls super-class init and adds necessary latex commands to output.
        """
        super(SubSubSection, self).__init__()
        self.output += r"\subsubsection{" + str(name) + r"}" + '\n'


class Graphics(LatexObject):
    """
    Includes a series of image files into your latex code and centers them.
    """

    def __init__(self):
        """
        Calls super-class init and begins centered environment.
        """
        super(Graphics, self).__init__()
        self.output += r"\begin{center}" + '\n'

    def add(self, filename, width=0.7):
        """
        Include a image file.
            @param filename containing the image
            @param width texwidth argument of includegraphics
        """
        self.output += r"\includegraphics[width=" + str(width) + r"\textwidth]"
        self.output += r"{" + str(filename) + r"}" + '\n'
        return self

    def finish(self):
        """
        Ends centered environment
        """
        self.output += r"\end{center}" + '\n'
        return self


class Itemize(LatexObject):
    """
    Creates a itemized list in latex.
    """

    def __init__(self):
        """
        Calls super-class init and begins itemize
        """
        super(Itemize, self).__init__()
        #: number of items
        self.amount = 0
        self.output += r"\begin{itemize}"

    def add(self, item):
        """
        Adds another item.
            @param item string or object with implicit string conversion used as item
        """
        self.amount += 1
        self.output += r"\item " + str(item) + '\n'
        return self

    def finish(self):
        """
        Finishes the generation of latex-code.
        """
        if self.amount == 0:
            return ''
        self.output += r"\end{itemize}"
        return self


class LongTable(LatexObject):
    """
    Creates a longtable in latex. A longtable can span multiple pages
    and is automatically wrapped.
    """

    def __init__(self, columnspecs, caption, format_string, head):
        """
        Calls super-class init, begins centered environment and longtable environment.
        Defines caption and head of the table.
            @param columnspecs of the longtable, something like:
                    rclp{7cm} 4 columns, right-center-left aligned and one paragraph column with a width of 7cm
            @param caption string or object with implicit string conversion used as caption.
            @param format_string  python-style format-string used to generate a new row out of a given dictionary.
            @param head of the table
        """
        super(LongTable, self).__init__()
        self.output += r"\begin{center}" + '\n'
        self.output += r"\begin{longtable}{" + str(columnspecs) + r"}" + '\n'
        self.output += r"\caption{" + str(caption) + r"}\\" + '\n'
        self.output += r"\toprule" + '\n'
        self.output += head + r"\\" + '\n'
        self.output += r"\midrule" + '\n'
        #: python-style format-string used to generate a new row out of a given dictionary.
        self.format_string = format_string

    def add(self, *args, **kwargs):
        """
        Add a new row to the longtable by generating the row using the format_string given in init
        and the provided dictionary.
            @param args positional arguments used to generate the row using the python-style format string.
            @param kwargs dictionary used to generate the row using the python-style format-string.
        """
        self.output += self.format_string.format(*args, **kwargs) + r"\\" + '\n'
        return self

    def finish(self, tail=''):
        """
        Adds optional tail of the table, ends longtable and centered environment.
            @param tail optional tail, like head but at the bottom of the table.
        """
        self.output += r"\bottomrule" + '\n'
        if str(tail) != '':
            self.output += str(tail) + r"\\" + '\n'
            self.output += r"\bottomrule" + '\n'
        self.output += r"\end{longtable}" + '\n'
        self.output += r"\end{center}" + '\n'
        return self


class TitlePage(LatexObject):
    """
    Creates a latex title-page and optionally abstract and table-of-contents.
    You should include only one of these objects in your latex code.
    """

    def __init__(self, title, authors, abstract, add_table_of_contents=True, clearpage=True):
        """
        Sets author, date, title property, calls maketitle, optionalla adds abstract and table-of-contents.
            @param title of the latex file.
            @param authors of the latex file, so the person who write the corresponding python-code with this framework :-)
            @param abstract optional abstract placed on the title-page.
            @param add_table_of_contents bool indicating of table-of-contents should be included.
        """
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

        if clearpage:
            self.output += r"\clearpage" + '\n'
        if add_table_of_contents:
            self.output += r"\tableofcontents" + '\n'
            self.output += r"\FloatBarrier" + '\n'
            if clearpage:
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
        table.add(name='test' + str(i), bargraph=r'\plotbar{ %g/, %g/, %g/,}' % (r / n, g / n, b / n), r=r, g=g, b=b)
    n = (sr + sg + sb) / 100.0
    o += table.finish(tail=r"Total & \plotbar{ %g/, %g/, %g/,} & %g & %g & %g" % (sr / n, sg / n, sb / n, sr, sg, sb))

    o += Section("Graphic section")
    graphics = Graphics()
    images = subprocess.check_output("locate .png", shell=True).split('\n')[:-1]
    for i in range(4):
        image = random.choice(images)
        graphics.add(image, width=0.49)
    o += graphics.finish()

    from . import format
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
