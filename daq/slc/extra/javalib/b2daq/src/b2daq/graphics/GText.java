package b2daq.graphics;

import javafx.scene.paint.Color;

public class GText extends GShape {

    private String text;
    private String alignment;
    private double angle = 0;
    private double x, y;

    public GText() {
        this("", 0, 0, "left");
    }

    public GText(String str, double x, double y, String align) {
        this(str, x, y, align, Color.BLACK);
    }

    public GText(String str, double x, double y, String align, Color font_color) {
        set(str, x, y);
        alignment = align;
        setFontColor(font_color);
    }

    public GText(String str, double x, double y, String align, double angle, Color font_color) {
        set(str, x, y);
        alignment = align;
        this.angle = angle;
        setFontColor(font_color);
    }

    public void setText(String str) {
        text = str;
    }

    public String getText() {
        return text;
    }

    public void set(String str, double x, double y) {
        text = str;
        this.x = x;
        this.y = y;
    }

    public void setAngle(double angle) {
        this.angle = angle;
    }

    public void setAlignment(String align) {
        alignment = align;
    }

    public void setPosition(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public void setX(double x) {
        this.x = x;
    }

    public void setY(double y) {
        this.y = y;
    }

    public double  getX() {
        return this.x;
    }

    public double getY() {
        return this.y;
    }

    public double getAngle() {
        return this.angle;
    }

    public String getAlignment() {
        return alignment;
    }

    @Override
    public void draw(GraphicsDrawer c) {
        c.setFont(getFont());
        if (angle == 0) {
            c.drawString(text, x, y, alignment);
        } else {
            c.drawString(text, x, y, alignment, angle);
        }
    }

}
