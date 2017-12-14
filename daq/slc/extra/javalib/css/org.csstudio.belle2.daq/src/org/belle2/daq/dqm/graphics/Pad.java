package org.belle2.daq.dqm.graphics;

import org.belle2.daq.graphics.GraphicsDrawer;
import org.belle2.daq.graphics.GRect;
import org.belle2.daq.graphics.HtmlColor;

public class Pad extends GRect {

    public Pad() {
        this(0.15, 0.17, 0.76, 0.72);
    }

    public Pad(double x, double y, double width, double height) {
        super(x, y, width, height);
        setFillColor(HtmlColor.WHITE);
        setLineColor(HtmlColor.BLACK);
        setLineWidth(1);
    }

    public void draw(GraphicsDrawer canvas) {
        canvas.setFill(getFill());
        canvas.setLine(getLine());
        canvas.drawRect(x, y, width, height);
    }

}
