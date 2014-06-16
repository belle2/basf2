package b2daq.dqm.graphics;

import b2daq.core.NumberString;
import b2daq.dqm.core.Axis;
import b2daq.graphics.FontProperty;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GLine;
import b2daq.graphics.GShape;
import java.util.ArrayList;
import java.util.Date;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;

public abstract class GAxis extends GShape {

    public final static int HAND_R = 1;
    public final static int HAND_L = -1;

    public final static int TIME_SCALE_NON = -1;
    public final static int TIME_SCALE_SEC = 0;
    public final static int TIME_SCALE_MIN = 1;
    public final static int TIME_SCALE_HOUR = 2;

    public final static int POSITION_CENTER = 0;
    public final static int POSITION_LEFT = 1;
    public final static int POSITION_RIGHT = 2;

    public final static int LABEL_POSITION_CENTER = 0;
    public final static int LABEL_POSITION_LEFT = 1;
    public final static int LABEL_POSITION_RIGHT = 2;

    protected Axis axis;

    protected double x0, y0, length, padding_x, padding_y;
    protected int hand = HAND_R;
    protected boolean auto_label = true;
    protected int time_scale = TIME_SCALE_NON;
    private boolean log_scale = false;
    private long update_time = (new Date()).getTime();

    private int title_position = POSITION_CENTER;

    protected ArrayList<GLine> tick_line_v = new ArrayList<>();
    private double title_offset = 0.01;
    protected double tick_length = 0.01;

    protected ObservableList<GValuedText> labels = FXCollections.observableArrayList();
    private FontProperty labelFont = new FontProperty();
    protected double label_offset = 0.01;
    protected double label_angle = 0;

    protected boolean use_border_line = false;
    protected GLine border_line0 = new GLine(0, 0, 0, 0, Color.BLACK);
    protected GLine border_line1 = new GLine(0, 0, 0, 0, Color.BLACK);
    protected String label_align;
    protected String label_position;
    private String time_format = "";
    private int ndivisions = 10;

    public GAxis() {
        super();
        axis = new Axis();
        setLineColor(Color.BLACK);
        getFont().setSize(0.8);
        labelFont.setSize(0.8);
    }

    public GAxis(Axis axis) {
        this.axis = axis;
        setLineColor(Color.BLACK);
        getFont().setSize(0.8);
        labelFont.setSize(0.8);
    }

    public GAxis(int nbin, double min, double max, String label,
            boolean fixMin, boolean fixMax, boolean log) {
        axis = new Axis(nbin, min, max, label);
        axis.setFixedMin(fixMin);
        axis.setFixedMax(fixMax);
        setLineColor(Color.BLACK);
        setLogScale(log);
        getFont().setSize(0.8);
        labelFont.setSize(0.8);
    }

    public Axis get() {
        return axis;
    }

    public void set(Axis axis) {
        this.axis = axis;
    }

    public void set(double x0, double y0, double length, double padding_x,
            double padding_y) {
        this.x0 = x0;
        this.y0 = y0;
        this.length = length;
        this.padding_x = padding_x;
        this.padding_y = padding_y;
    }

    public void setX(double x0) {
        this.x0 = x0;
    }

    public void setY(double y0) {
        this.y0 = y0;
    }

    public void setLength(double length) {
        this.length = length;
    }

    public void setPaddingX(double padding_x) {
        this.padding_x = padding_x;
    }

    public void setPaddingY(double padding_y) {
        this.padding_y = padding_y;
    }

    public double getX() {
        return x0;
    }

    public double getY() {
        return y0;
    }

    public double getLength() {
        return length;
    }

    public double getPaddingX() {
        return padding_x;
    }

    public double getPaddingY() {
        return padding_y;
    }

    public void addLabel(String label, double value) {
        auto_label = false;
        labels.add(new GValuedText(label, value, "top"));
    }

    public ObservableList<GValuedText> getLabels() {
        return labels;
    }

    public void setLabels(ObservableList<GValuedText> label_v) {
        auto_label = false;
        labels.setAll(label_v);
    }

    public void setLabels(String text) {
        String[] text_v = text.split(",");
        ArrayList<GValuedText> label = new ArrayList<>();
        for (String t : text_v) {
            String [] str = t.split(":");
            if (str.length > 0) {
                try {
            label.add(new GValuedText(str[0], Double.parseDouble(str[1]),
                    getLabelAlignment()));
                } catch(Exception e) {}
            }
        }
        auto_label = false;
        labels.setAll(label);
    }

    @Override
    public void setLineColor(Paint color) {
        super.setLineColor(color);
        border_line0.setLineColor(color);
        border_line1.setLineColor(color);
    }

    public double getScale() {
        if (isLogScale()) {
            return length / (Math.log(axis.getMax()) - Math.log(axis.getMin()));
        } else {
            return length / (axis.getMax() - axis.getMin());
        }
    }

    public void setTickLength(double tick) {
        tick_length = tick;
    }

    public double getTickLength() {
        return tick_length;
    }

    public void setHand(int hand) {
        this.hand = hand;
    }

    public void mousePressed(double x1, double y1, double x2, double y2) {
        focused = true;
        border_line0.set(x1, y1, x2, y2);
        use_border_line = true;
    }

    public void mouseReleased() {
        if (focused == true) {
            border_line0.set(0, 0, 0, 0);
            border_line1.set(0, 0, 0, 0);
        }
        focused = false;
        use_border_line = false;
    }

    public void mouseDragged(double x1, double y1, double x2, double y2) {
        if (focused == true) {
            border_line1.set(x1, y1, x2, y2);
        }
    }

    public void setLabelsFont(FontProperty ticks_font) {
        labelFont = ticks_font;
    }

    public FontProperty getLabelsFont() {
        return labelFont;
    }

    public void setTimeScale(int scale) {
        time_scale = scale;
        switch (time_scale) {
            case TIME_SCALE_SEC:
                ndivisions = 5;
                break;
            case TIME_SCALE_MIN:
                ndivisions = 10;
                break;
            case TIME_SCALE_HOUR:
                ndivisions = 10;
                break;
        }
    }

    public int getTimeScale() {
        return time_scale;
    }

    public void setTitleOffset(double title_offset) {
        this.title_offset = title_offset;
    }

    public double getTitleOffset() {
        return title_offset;
    }

    public void setTitle(String title) {
        axis.setTitle(title);
    }

    public String getTitle() {
        return axis.getTitle();
    }

    public void setTitlePosition(int title_position) {
        this.title_position = title_position;
    }

    public int getTitlePosition() {
        return title_position;
    }

    public void setTitlePosition(String title_position) {
        if (title_position.matches("center")) {
            this.title_position = POSITION_CENTER;
        } else if (title_position.matches("left")) {
            this.title_position = POSITION_LEFT;
        } else if (title_position.matches("right")) {
            this.title_position = POSITION_RIGHT;
        }
    }

    public String getTitlePositionString() {
        if (title_position == POSITION_CENTER) {
            return "center";
        } else if (title_position == POSITION_LEFT) {
            return "left";
        } else if (title_position == POSITION_RIGHT) {
            return "right";
        } else {
            return "";
        }
    }

    public boolean isLogScale() {
        return (axis.getMin() > 0) && log_scale;
    }

    public void setLogScale(boolean enable) {
        log_scale = enable;
    }

    protected void setTicks(GraphicsDrawer canvas, String align) {
        tick_line_v.clear();
        tick_line_v = new ArrayList<>();
        if (auto_label) {
            labels.clear();
        }
        if (time_scale == GAxis.TIME_SCALE_NON) {
            double dx = Math.pow(10d, Math.floor(Math.log10(axis.getMax() - axis.getMin())) - 1) * 2;// / ndivisions;
            if (axis.getMax() < dx * 25) {
                dx /= 2;
            }
            long n_min = (int) Math.floor(axis.getMin() / dx), n_max = (int) Math.floor(axis.getMax() / dx);
            int nR = 0;
            long n5_min = n_min - n_min % ndivisions, n5_max = n_max - n_max % ndivisions;
            while (nR < ndivisions) {
                if (NumberString.toString(n5_min * dx, nR).compareTo(
                        NumberString.toString(n5_max * dx, nR)) != 0) {
                    break;
                }
                nR++;
            }
            if (NumberString.toString((n5_max - ndivisions) * dx, nR).compareTo(
                    NumberString.toString(n5_max * dx, nR)) == 0) {
                nR++;
            }
            double x;
            if (isLogScale()) {
                int logMax = (int) Math.floor(Math.log10(axis.getMax()));
                int logMin = (int) Math.floor(Math.log10(axis.getMin()));
                for (int nlog = logMin; nlog <= logMax; nlog++) {
                    double ddx = Math.pow(10, nlog);
                    x = ddx;
                    if (auto_label) {
                        String s = NumberString.toString(x, nR);
                        labels.add(new GValuedText(s, x, align));
                    }
                    tick_line_v.add(new GLine(x, 0, x, 2));
                    for (int n = 1; n < 10; n++) {
                        x = n * ddx;
                        tick_line_v.add(new GLine(x, 0, x, 1));
                    }
                }
            } else {
                for (long n = n_min; n <= n_max; n++) {
                    x = n * dx;
                    if (x >= axis.getMin() && x <= axis.getMax()) {
                        if (n % ndivisions == 0) {
                            if (auto_label) {
                                String s = NumberString.toString(x, nR);
                                labels.add(new GValuedText(s, x, align));
                            }
                            tick_line_v.add(new GLine(x, 0, x, 2));
                        } else {
                            tick_line_v.add(new GLine(x, 0, x, 1));
                        }
                    }
                }
            }
        } else {
            long dx = 5;
            double time_max = axis.getMax();
            double range = axis.getMax() - axis.getMin();
            String format = "HH:mm:ss";
            double time_base = update_time - time_max * 1000;
            if (range >= 60 * 60 * 24 * 2) {
                format = "M/d";
                dx = 300 * 24;
            } else if (range > 60 * 60 * 2) {
                format = "M/d HH:mm";
                dx = 300 * 2;
            } else if (range > 60 * 2) {
                format = "HH:mm";
                dx = 60;
            } else if (range > 10) {
                format = "HH:mm";
                dx = 5;
            } else {
                format = "HH:mm";
                dx = 1;
            }
            if (time_format.length() > 0) {
                format = time_format;
            }
            double x = time_max;
            int n = 0;
            while (x >= 0) {
                if (n % ndivisions == 0) {
                    if (auto_label) {
                        String s = canvas.getTime((long) (time_base + x * 1000), format);
                        labels.add(new GValuedText(s, x, "left top"));
                    }
                    tick_line_v.add(new GLine(x, 0, x, 2));
                } else {
                    tick_line_v.add(new GLine(x, 0, x, 1));
                }
                n++;
                x -= dx;
            }
        }
    }

    public double eval(double v) {
        if (isLogScale()) {
            if (v <= 0) {
                return 0;
            }
            return (Math.log(v) - Math.log(axis.getMin())) * getScale();
        } else {
            return (v - axis.getMin()) * getScale();
        }
    }

    public void setUpdateTime(long update_time) {
        this.update_time = update_time;
    }

    public double getUpdateTime() {
        return update_time;
    }

    public void setLabelAngle(double label_angle) {
        this.label_angle = label_angle;
    }

    public double getLabelAngle() {
        return label_angle;
    }

    public void setLabelFontSize(double size) {
        labelFont.setSize(size);
    }

    public double getLabelFontSize() {
        return labelFont.getSize();
    }

    public void setLabelAlignment(String align) {
        label_align = align;
    }

    public String getLabelAlignment() {
        return label_align;
    }

    public void setLabelOffset(double label_offset) {
        this.label_offset = label_offset;
    }

    public double getLabelOffset() {
        return this.label_offset;
    }

    public void setTimeFormat(String time_format) {
        this.time_format = time_format;
    }

    public int getNdivisions() {
        return ndivisions;
    }

    public void setNdivisions(int ndiv) {
        if (ndiv > 0) {
            ndivisions = ndiv;
        }
    }

    public double getMin() {
        return axis.getMin();
    }

    public double getMax() {
        return axis.getMax();
    }

    public boolean getFixedMin() {
        return axis.isFixedMin();
    }

    public boolean getFixedMax() {
        return axis.isFixedMax();
    }

    public void setMax(double max) {
        axis.setMax(max);
    }

    public void setMin(double min) {
        axis.setMin(min);
    }

    public void setFixedMin(boolean fix) {
        axis.setFixedMin(fix);
    }

    public void setFixedMax(boolean fix) {
        axis.setFixedMax(fix);
    }
}
