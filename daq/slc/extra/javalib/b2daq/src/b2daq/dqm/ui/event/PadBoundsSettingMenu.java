package b2daq.dqm.ui.event;

import b2daq.dqm.graphics.Pad;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.ui.InputDialog;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.MenuItem;

public class PadBoundsSettingMenu extends CleanableMenu {

    private static final long serialVersionUID = 1L;
    private Pad _pad;
    private CanvasPanel _panel;

    public PadBoundsSettingMenu(String title, Pad pad, CanvasPanel panel) {
        super(title);
        _pad = pad;
        _panel = panel;
        MenuItem item = new MenuItem("Set X");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting pad", "Set X", ""+_pad.getX());
                if (str != null) {
                    _pad.setX(Double.parseDouble(str));
                    _panel.getCanvas().resetPadding();
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Y");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting pad", "Set Y", ""+_pad.getY());
                if (str != null) {
                    _pad.setY(Double.parseDouble(str));
                    _panel.getCanvas().resetPadding();
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Width");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting pad", "Set width", ""+_pad.getWidth());
                if (str != null) {
                    _pad.setWidth(Double.parseDouble(str));
                    _panel.getCanvas().resetPadding();
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Height");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting pad", "Set height", ""+_pad.getHeight());
                if (str != null) {
                    _pad.setHeight(Double.parseDouble(str));
                    _panel.getCanvas().resetPadding();
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
    }

}
