package b2daq.dqm.ui.event;

import b2daq.dqm.graphics.GAxis;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.ui.InputDialog;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;

public class AxisRangeSettingMenu extends CleanableMenu {

    private static final long serialVersionUID = 1L;
    private GAxis _axis;
    private CanvasPanel _panel;

    public AxisRangeSettingMenu(String title, GAxis axis, CanvasPanel panel) {
        super(title);
        _axis = axis;
        _panel = panel;
        MenuItem item = new MenuItem("Set Maximum");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting axis", "Set range maximum",
                        ""+_axis.get().getMax());
                if (str != null) {
                    _axis.get().setFixedMax(false);
                    _axis.get().setMax(Double.parseDouble(str));
                    _axis.get().setFixedMax(true);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Minimum");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting axis", "Set range minimum",
                        ""+_axis.get().getMin());
                if (str != null) {
                    _axis.get().setFixedMin(false);
                    _axis.get().setMin(Double.parseDouble(str));
                    _axis.get().setFixedMin(true);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
    }

}
