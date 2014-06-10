package b2daq.dqm.graphics;

import b2daq.dqm.core.Axis;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GLine;

public class GAxisY extends GAxis {

    public GAxisY() {
        super();
        label_align = "middle";
    }

    public GAxisY(Axis axis) {
        super(axis);
        label_align = "middle";
    }

    public GAxisY(int nbin, double min, double max, String label,
            boolean fixMin, boolean fixMax, boolean logon) {
        super(nbin, min, max, label, fixMin, fixMax, logon);
        label_align = "middle";
    }

    public GAxisY(int nbin, double min, double max, String label) {
        super(nbin, min, max, label, false, false, false);
        label_align = "middle";
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        if (!isVisible()) {
            return;
        }
        canvas.setLine(getLine());
        canvas.setFont(getLabelsFont());
        String align = (hand == HAND_R) ? "right " : "left ";
        setTicks(canvas, align);
        for (GValuedText label : labels) {
            if (label != null && label.getValue() >= get().getMin() && label.getValue() <= get().getMax()) {
                label.setPosition(x0 - hand * label_offset, y0 - eval(label.getValue()));
                label.setFont(getLabelsFont());
                label.setAngle(getLabelAngle());
                label.setAlignment(align + label_align);
                label.draw(canvas);
            }
        }
        for (GLine cline : tick_line_v) {
            if (cline != null && cline.getX1() >= get().getMin() && cline.getX1() <= get().getMax()) {
                double X = x0;
                double Y = y0 - eval(cline.getX1());
                canvas.drawLine(X, Y, X + hand * tick_length * cline.getY2(), Y);
            }
        }
        canvas.drawLine(x0, y0, x0, y0 - length);
        double title_offset = (hand == HAND_L) ? hand * padding_x * 0.45 : hand * padding_x * 0.80;
        align = "top";
        canvas.setFont(getFont());
        double x = x0 - title_offset;
        double y = y0 - length * 0.5;
        if (getTitlePosition() == GAxis.POSITION_CENTER) {
            align += " center";
        } else if (getTitlePosition() == GAxis.POSITION_LEFT) {
            y = 0.9;
            align += " left";
        } else if (getTitlePosition() == GAxis.POSITION_RIGHT) {
            y = 0.1;
            align += " right";
        }
        canvas.drawString(get().getTitle(), x, y, align, -90);
        if (use_border_line == true) {
            border_line0.draw(canvas);
            border_line1.draw(canvas);
        }
    }

    @Override
    public boolean hit(double x, double y) {
        if (hand == HAND_R && x <= x0 && x >= x0 - padding_x && y <= y0 && y >= y0 - length) {
            return true;
        }
        return hand == HAND_L && x >= x0 && x <= x0 + padding_x && y <= y0 && y >= y0 - length;
    }

    @Override
    public void mouseReleased() {
        if (focused == true) {
            focused = false;
            if (border_line1.getY1() == 0) {
                get().setFixedMin(false);
                get().setFixedMax(false);
            } else {
                double min = (get().getMax() - get().getMin()) * (y0 - border_line0.getY1()) / length + get().getMin();
                double max = (get().getMax() - get().getMin()) * (y0 - border_line1.getY1()) / length + get().getMin();
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
