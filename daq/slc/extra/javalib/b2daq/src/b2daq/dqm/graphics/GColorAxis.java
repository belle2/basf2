package b2daq.dqm.graphics;

import b2daq.dqm.core.Axis;
import b2daq.graphics.GraphicsDrawer;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;

public class GColorAxis extends GAxisY {

    private final static Color[] COLOR_LIST = {
        Color.rgb(153, 0, 255), Color.rgb(102, 0, 255),
        Color.rgb(51, 0, 255), Color.rgb(0, 0, 255),
        Color.rgb(0, 51, 255), Color.rgb(0, 102, 255),
        Color.rgb(0, 153, 255), Color.rgb(0, 204, 255),
        Color.rgb(0, 255, 255), Color.rgb(0, 255, 204),
        Color.rgb(0, 255, 153), Color.rgb(0, 255, 102),
        Color.rgb(0, 255, 51), Color.rgb(0, 255, 0),
        Color.rgb(51, 255, 0), Color.rgb(102, 255, 0),
        Color.rgb(153, 255, 00), Color.rgb(204, 255, 0),
        Color.rgb(255, 255, 0), Color.rgb(255, 204, 0),
        Color.rgb(255, 153, 00), Color.rgb(255, 102, 0),
        Color.rgb(255, 51, 0), Color.rgb(255, 0, 0),
        Color.rgb(255, 0, 51)
    };

    private ObservableList<Paint> colorPattern = FXCollections.observableArrayList();
    
    public GColorAxis() {
        super();
    }

    public GColorAxis(Axis axis) {
        super(axis);
    }

    public GColorAxis(int nbin, double min, double max, String label,
            boolean fixMin, boolean fixMax, boolean logon) {
        super(nbin, min, max, label, fixMin, fixMax, logon);
        setHand(HAND_L);
        getLabelsFont().setSize(0.90);
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        if (!isVisible()) {
            return;
        }
        canvas.setFont(getLabelsFont());
        canvas.setLine(null);
        if (colorPattern.size() == 0) {
            colorPattern.setAll(COLOR_LIST);
        }
        double height = length / colorPattern.size();
        double X = x0 - padding_x;
        double Y = y0;
        double px_height = height;
        if (height < 0.005) {
            px_height = 0.005;
        }
        for (Paint color : colorPattern) {
            Y -= height;
            canvas.getFill().setColor(color);
            canvas.drawRect(X, Y, padding_x * 1.05, px_height * 1.05);
        }
        super.draw(canvas);
    }

    public boolean hit(double x, double y) {
        if (!visible) {
            return false;
        }
        return x >= x0 - padding_x && x <= x0 + padding_x
                && y <= y0 && y >= y0 - length;
    }

    public ObservableList<Paint> getColorPattern() {
        return colorPattern;
    }

    public void setColorPattern(Paint... pattern) {
        colorPattern.clear();
        colorPattern.setAll(pattern);
    }

    public void setColorPattern(String pattern) {
        String [] str = pattern.split(",");
        Paint paint [] = new Paint[str.length];
        for (int i = 0; i < str.length; i++) {
            paint[i] = Color.web(str[i]);
        }
        colorPattern.clear();
        colorPattern.setAll(paint);
    }
    
}
