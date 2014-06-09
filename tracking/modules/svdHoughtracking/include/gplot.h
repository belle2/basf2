#define GPLOT_OPT "notitle with lines lt 1"
#define GPLOT_GEO_OPT "notitle with lines lt -1"
#define GPLOT_INIT_OPT "notitle with lines lt 2"

int plot_pos;

#define GPLOT_HEADER 1
#define GPLOT_PLOT 2
#define GPLOT_INPLOT 3
#define GPLOT_PLOTGEO 4
#define GPLOT_LINE 5
#define GPLOT_FOOTER 9

#define GPLOT_TMPL1 10

const char gplot_text[] = "set title \"2D tracks\" \n\
\n\
set xrange [-1e-0:1e-0] \n\
set yrange [-1e-0:1e-0]\n";

const char gplot_text2[] = "set notitle \n\
\n\
# Style options \n\
set style line 80 lt rgb \"#808080\"\n\
set style line 81 lt 0\n\
set style line 81 lt rgb \"#808080\"\n\
\n\
# Lines\n\
set style line 1 lt rgb \"#A00000\" lw 1 pt 1\n\
set style line 2 lt rgb \"#00A000\" lw 1 pt 6\n\
set style line 3 lt rgb \"#000000\" lw 1 pt 6\n\
\n\
# Points\n\
#set style points 3 lt rgb \"#0000A0\" pt 1\n\
\n\
# Description position\n\
set key top right\n\
\n\
# Grid and border style\n\
set grid back linestyle 81\n\
set border 3 back linestyle 80\n\
\n\
# No mirrors\n\
set xtics nomirror\n\
set ytics nomirror\n\
\n\
# Encoding\n\
set encoding utf8\n\
\n\
set xlabel \"x\"\n\
set ylabel \"y\"\n\
\n\
#set xrange [-0.2:0.2]\n\
#set yrange [-0.2:0.2]\n\
\n\
\n";

const char gplot_tmpl1[] = "set notitle \n\
\n\
# Style options \n\
set style line 80 lt rgb \"#808080\"\n\
set style line 81 lt 0\n\
set style line 81 lt rgb \"#808080\"\n\
\n\
# Lines\n\
set style line 1 lt rgb \"#A00000\" lw 1 pt 1\n\
set style line 2 lt rgb \"#00A000\" lw 1 pt 6\n\
set style line 3 lt rgb \"#000000\" lw 1 pt 6\n\
\n\
# Points\n\
#set style points 3 lt rgb \"#0000A0\" pt 1\n\
\n\
# Description position\n\
set key top right\n\
\n\
# Grid and border style\n\
set grid back linestyle 81\n\
set border 3 back linestyle 80\n\
\n\
# No mirrors\n\
set xtics nomirror\n\
set ytics nomirror\n\
\n\
# Encoding\n\
set encoding utf8\n\
\n\
set xlabel \"x\"\n\
set ylabel \"y\"\n\
\n\
\n";

const char gplot_geo[] = "plot [0:2*pi] 0.038 * cos(t), 0.038 * sin(t) notitle with lines linestyle 3\n\
replot 0.080 * cos(t), 0.080 * sin(t) notitle with lines linestyle 3\n\
replot 0.115 * cos(t), 0.115 * sin(t) notitle with lines linestyle 3\n\
replot 0.140 * cos(t), 0.140 * sin(t) notitle with lines linestyle 3\n\
\n";
