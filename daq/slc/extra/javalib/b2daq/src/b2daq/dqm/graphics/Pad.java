package b2daq.dqm.graphics;

import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GRect;
import javafx.scene.paint.Color;

public class Pad extends GRect {

    public Pad() {
        this(0.13, 0.15, 0.74, 0.70);
    }

    public Pad(double x, double y, double width, double height) {
        super(x, y, width, height);
        setFillColor(Color.WHITE);
        setLineColor(Color.BLACK);
        setLineWidth(1);
    }

    public void draw(GraphicsDrawer canvas) {
        canvas.setFill(getFill());
        canvas.setLine(getLine());
        canvas.drawRect(x, y, width, height);
    }

}
