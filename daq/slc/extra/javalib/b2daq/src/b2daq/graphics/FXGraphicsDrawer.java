package b2daq.graphics;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import javafx.geometry.VPos;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import javafx.scene.shape.ArcType;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.TextAlignment;
import javafx.scene.transform.Affine;

public class FXGraphicsDrawer extends GraphicsDrawer {

    private GraphicsContext _graphics;
    private Font _j_font;
    private double _font_real_size = 30;

    public FXGraphicsDrawer(int width, int height) {
        this(width, height, 12);
    }

    public FXGraphicsDrawer(int width, int height, int font_size) {
        super(width, height);
        _font = new FontProperty(Color.BLACK, "Arial", font_size, FontWeight.BOLD);
        _j_font = Font.font(_font.getFamily(), _font.getWeight(), _font_real_size * _font.getSize());
    }

    @Override
    public void setFont(FontProperty font) {
        _font = font;
        _j_font = Font.font(_font.getFamily(), _font.getWeight(), _font_real_size * _font.getSize());
    }

    @Override
    public void setFontSize(double size) {
        _font.setSize(size);
        _j_font = Font.font(_font.getFamily(), _font.getWeight(), _font_real_size * _font.getSize());
    }

    public void setFontRealSize(double size) {
        _font_real_size = size;
    }

    public FXGraphicsDrawer() {
        this(100, 100, 12);
    }

    public void setGraphics(GraphicsContext g) {
        _graphics = g;
        _graphics.setFont(_j_font);
        _graphics.setLineWidth(2.0f);
    }

    public GraphicsContext getGraphics() {
        return _graphics;
    }

    @Override
    public void drawLine(double x1, double y1, double x2, double y2) {
        if (getLine().getColor() != null) {
            _graphics.setStroke(getLine().getColor());
            _graphics.setLineWidth(getLine().getWidth());
            _graphics.strokeLine(x1 * getWidth(), y1 * getHeight(),
                    x2 * getWidth(), y2 * getHeight());
        }
    }

    @Override
    public void drawPolygon(double[] x, double[] y) {
        drawPolygon(0, -1, x, y);
    }

    @Override
    public void drawPolygon(int begin, int end, double[] x, double[] y) {
        drawPolyline(begin, end, x, y);
    }

    @Override
    public void drawPolyline(double[] x, double[] y) {
        drawPolyline(0, -1, x, y);
    }

    @Override
    public void drawPolyline(int begin, int end, double[] x, double[] y) {
        if (end < begin) {
            end = (x.length > y.length) ? y.length : x.length;
        }
        int npoints = end - begin;
        double[] X = new double[npoints];
        double[] Y = new double[npoints];
        for (int n = 0; n < npoints; n++) {
            X[n] = x[n + begin] * getWidth();
            Y[n] = y[n + begin] * getHeight();
        }
        if (getFill().getColor() != null) {
            _graphics.setFill(getFill().getColor());
            _graphics.fillPolygon(X, Y, npoints);
        }
        if (getLine().getColor() != null) {
            _graphics.setStroke(getLine().getColor());
            _graphics.setLineWidth(getLine().getWidth());
            _graphics.strokePolygon(X, Y, npoints);
        }
    }

    @Override
    public void drawMarker(double x, double y) {
        double X = x * getWidth();
        double Y = y * getHeight();
        double R = getMarkerSize() * getWidth() / 50;
        if (getLine().getColor() != null) {
            _graphics.setFill(getLine().getColor());
            _graphics.fillOval(X - R / 2, Y - R / 2, R, R);
        }
    }

    @Override
    public void drawRect(double x, double y, double width, double height) {
        double X = x * getWidth();
        double Y = y * getHeight();
        double W = width * getWidth();
        double H = height * getHeight();
        if (W == 0 && width > 0) {
            W = 1;
        }
        if (H == 0 && height > 0) {
            H = 1;
        }
        if (getFill().getColor() != null) {
            _graphics.setFill(getFill().getColor());
            _graphics.fillRect(X, Y, W, H);
        }
        if (getLine().getColor() != null) {
            _graphics.setStroke(getLine().getColor());
            _graphics.setLineWidth(getLine().getWidth());
            _graphics.strokeRect(X, Y, W, H);
        }
    }

    @Override
    public void drawString(String str, double x, double y, String align) {
        drawString(str, x, y, align, 0);
    }

    @Override
    public void drawString(String str, double x, double y, String align, double angle) {
        if (getFont().getColor() != null) {
            Affine t = _graphics.getTransform();
            _graphics.setFill(getFont().getColor());
            Font f = _graphics.getFont();
            _graphics.setFont(_j_font);
            double X = x * getWidth();
            double Y = y * getHeight();
            _graphics.translate(X, Y);
            _graphics.rotate(angle);
            if (align.matches(".*right.*")) {
                _graphics.setTextAlign(TextAlignment.RIGHT);
            } else if (align.matches(".*center.*")) {
                _graphics.setTextAlign(TextAlignment.CENTER);
            } else {
                _graphics.setTextAlign(TextAlignment.LEFT);
            }
            if (align.matches(".*top.*")) {
                _graphics.setTextBaseline(VPos.TOP);
            } else if (align.matches(".*middle.*")) {
                _graphics.setTextBaseline(VPos.CENTER);
            } else {
                _graphics.setTextBaseline(VPos.BOTTOM);
            }
            
            _graphics.fillText(str, 0, 0);
            _graphics.setFont(f);
            _graphics.setTransform(t);
        }
    }

    @Override
    public void drawCircle(double x, double y, double r_x, double r_y) {
        if (getFill().getColor() != null) {
            _graphics.setFill(getFill().getColor());
            _graphics.fillOval((x - r_x) * getWidth(),
                    (y - r_y) * getHeight(),
                    r_x * getWidth() * 2, r_y * getHeight() * 2);
        }
        if (getLine().getColor() != null) {
            _graphics.setStroke(getLine().getColor());
            _graphics.setLineWidth(getLine().getWidth());
            _graphics.strokeOval((x - r_x) * getWidth(),
                    (y - r_y) * getHeight(),
                    r_x * getWidth() * 2, r_y * getHeight() * 2);
        }
    }

    @Override
    public String getTime(long time, String format) {
        DateFormat df = new SimpleDateFormat(format);
        return df.format(new Date(time));
    }

    @Override
    public String getTime(String format) {
        DateFormat df = new SimpleDateFormat(format);
        return df.format(new Date());
    }

    @Override
    public void drawCircle(double x, double y, double r_x, double r_y,
            double angle0, double angle1) {
        if (getFill().getColor() != null) {
            _graphics.setFill(getFill().getColor());
            _graphics.fillArc((x - r_x) * getWidth(),
                    (y - r_y) * getHeight(),
                    r_x * getWidth() * 2, r_y * getHeight() * 2,
                    angle0, angle1, ArcType.CHORD);
        }
        if (getLine().getColor() != null) {
            _graphics.setStroke(getLine().getColor());
            _graphics.setLineWidth(getLine().getWidth());
            _graphics.strokeArc((x - r_x) * getWidth(),
                    (y - r_y) * getHeight(),
                    r_x * getWidth() * 2, r_y * getHeight() * 2,
                    angle0, angle1, ArcType.CHORD);
        }
    }

}
