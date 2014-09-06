package b2daq.dqm.graphics;

import b2daq.dqm.core.Axis;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GLine;

public class GAxisX extends GAxis {

    public GAxisX() {
        super();
        label_align = "center";
    }

    public GAxisX(Axis axis) {
        super(axis);
        label_align = "center";
    }

    public GAxisX(int nbin, double min, double max, String label,
            boolean fixMin, boolean fixMax, boolean logon) {
        super(nbin, min, max, label, fixMin, fixMax, logon);
        label_align = "center";
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        if (!isVisible()) {
            return;
        }
        canvas.setLine(getLine());
        canvas.setFont(getLabelsFont());
        double base = setTicks(canvas, label_align + " top");
        for (GValuedText label : labels) {
            if (label != null && label.getValue() >= get().getMin() && label.getValue() <= get().getMax()) {
                label.setPosition(x0 + eval(label.getValue()), y0 + label_offset);
                label.setFont(getLabelsFont());
                label.setAngle(getLabelAngle());
                label.setAlignment(label_align + " top");
                label.draw(canvas);
            }
        }
        for (GLine cline : tick_line_v) {
            if (cline != null && cline.getX1() >= get().getMin() && cline.getX1() <= get().getMax()) {
                double X = x0 + eval(cline.getX1());
                double Y = y0;
                canvas.drawLine(X, Y, X, Y - tick_length * cline.getY2());
            }
        }
        canvas.drawLine(x0, y0, x0 + length, y0);
        canvas.setFont(getFont());
        double x = x0 + length / 2;
        double y = y0 + padding_y * 0.5;
        String align = "center top";
        if (getTitlePosition() == GAxis.POSITION_RIGHT) {
            x = 0.9;
            align = "right top";
        } else if (getTitlePosition() == GAxis.POSITION_LEFT) {
            x = 0.1;
            align = "left top";
        }
        canvas.drawString(get().getTitle(), x, y, align);
        if (base < 2000 && base > -2000 && (base > 3 || base < -2)) {
            canvas.drawString("e"+(base<0?"-":"+")+(int)base, x0 + length, y0, "left middle");
        }
        if (use_border_line == true) {
            border_line0.draw(canvas);
            border_line1.draw(canvas);
        }
    }

    @Override
    public boolean hit(double x, double y) {
        return x >= x0 && x <= x0 + length && y >= y0 && y <= y0 + padding_y;
    }

    @Override
    public void mouseReleased() {
        if (focused == true) {
            focused = false;
            if (border_line1.getX1() == 0) {
                get().setFixedMin(false);
                get().setFixedMax(false);
            } else {
                double min = (get().getMax() - get().getMin()) * (border_line0.getX1() - x0) / length + get().getMin();
                double max = (get().getMax() - get().getMin()) * (border_line1.getX1() - x0) / length + get().getMin();
                get().setFixedMin(false);
                get().setFixedMax(false);
                if (min < max) {
                    get().setMin(min);
                    get().setMax(max);
                } else if (min > max) {
                    get().setMin(max);
                    get().setMax(min);
                }
                get().setFixedMin(true);
                get().setFixedMax(true);
            }
            border_line0.set(0, 0, 0, 0);
            border_line1.set(0, 0, 0, 0);
        }
    }
}
