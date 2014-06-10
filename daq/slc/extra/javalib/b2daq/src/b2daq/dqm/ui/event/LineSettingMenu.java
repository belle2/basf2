package b2daq.dqm.ui.event;

import b2daq.dqm.ui.CanvasPanel;
import b2daq.graphics.LineProperty;
import b2daq.ui.InputDialog;
import javafx.event.EventHandler;
import javafx.scene.control.MenuItem;
import javafx.scene.paint.Color;

public class LineSettingMenu extends CleanableMenu {

    private static final long serialVersionUID = 1L;
    private LineProperty _line;
    private CanvasPanel _panel;

    public LineSettingMenu(String title, LineProperty line, CanvasPanel panel) {
        super(title);
        _line = line;
        _panel = panel;
        MenuItem item = new MenuItem("Set Line Color");
        item.setOnAction(new EventHandler<javafx.event.ActionEvent>() {
            @Override
            public void handle(javafx.event.ActionEvent event) {
                cleanup();
                String str;
                str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting line", "Set line color",
                        _line.getColor()!=null?_line.getColor().toString():"");
                if (str != null) {
                    _line.setColor(Color.web(str));
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Line Width");
        item.setOnAction(new EventHandler<javafx.event.ActionEvent>() {
            @Override
            public void handle(javafx.event.ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting line", "Set line width", ""+_line.getWidth());
                if (str != null) {
                    _line.setWidth((int) Double.parseDouble(str));
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Line Style");
        item.setOnAction(new EventHandler<javafx.event.ActionEvent>() {
            @Override
            public void handle(javafx.event.ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting line", "Set line style", ""+_line.getStyle());
                if (str != null) {
                    _line.setStyle(str);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
    }

}
