package b2daq.dqm.ui.event;

import b2daq.dqm.ui.CanvasPanel;
import b2daq.graphics.FontProperty;
import b2daq.ui.InputDialog;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.control.MenuItem;
import javafx.scene.paint.Color;

public class FontSettingMenu extends CleanableMenu {

    private FontProperty _font;
    private CanvasPanel _panel;

    public FontSettingMenu(String title, FontProperty font, CanvasPanel panel) {
        super(title);
        _font = font;
        _panel = panel;
        MenuItem item = new MenuItem("Set Color");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting font", "Set font color", 
                        _font.getColor()!=null?_font.getColor().toString():"");
                if (str != null) {
                    _font.setColor(Color.web(str));
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Size");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting font", "Set font size", ""+_font.getSize());
                if (str != null) {
                    _font.setSize(Double.parseDouble(str));
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Family");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting font", "Set font family", ""+_font.getFamily());
                if (str != null) {
                    _font.setFamily(str);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
    }

}
