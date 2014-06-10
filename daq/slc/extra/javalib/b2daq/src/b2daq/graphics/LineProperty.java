package b2daq.graphics;

import javafx.scene.paint.Paint;

public class LineProperty {

    private Paint color = null;
    private double width = 1;
    private String style = "";

    public LineProperty() {
    }

    public LineProperty(Paint c) {
        this(c, 1);
    }

    public LineProperty(Paint c, int w) {
        this(c, w, "solid");
    }

    public LineProperty(Paint c, int w, String style) {
        color = c;
        width = w;
        this.style = style;
    }

    public void setColor(Paint c) {
        color = c;
    }

    public void setWidth(double width) {
        this.width = width;
    }

    public void setStyle(String s) {
        style = s;
    }

    public Paint getColor() {
        return color;
    }

    public double getWidth() {
        return width;
    }

    public String getStyle() {
        return style;
    }

    public void copy(LineProperty pro) {
        if (pro == null) {
            return;
        }
        width = pro.width;
        color = pro.color;
        style = pro.style;
    }

}
