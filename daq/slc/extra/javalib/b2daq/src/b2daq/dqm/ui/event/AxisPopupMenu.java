package b2daq.dqm.ui.event;

import b2daq.dqm.graphics.GAxis;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.ui.InputDialog;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.ContextMenu;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;

public class AxisPopupMenu extends ContextMenu {

    private static final long serialVersionUID = 1L;
    private GAxis _axis;
    private CanvasPanel _panel;

    public AxisPopupMenu(String name, GAxis axis, CanvasPanel panel) {
        _axis = axis;
        _panel = panel;
        getItems().add(new Menu(name));
        Menu menu = new AxisRangeSettingMenu("Set Range", axis, panel);
        getItems().add(menu);
        menu = new FontSettingMenu("Set Title", axis.getFont(), panel);
        MenuItem item = new MenuItem("Set Position");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting axis", "Set title position", 
                        ""+_axis.getTitlePosition());
                if (str != null) {
                    _axis.setTitlePosition(str);
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        item = new MenuItem("Set Text");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting axis", "Set title", 
                        _axis.get().getTitle());
                if (str != null) {
                    _axis.get().setTitle(str);
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        getItems().add(menu);
        menu = new FontSettingMenu("Set Labels", axis.getLabelsFont(), panel);
        item = new MenuItem("Set Alignment");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting axis", "Set label aligment", 
                        _axis.getLabelAlignment());
                if (str != null) {
                    _axis.setLabelAlignment(str);
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        getItems().add(menu);
        menu = new LineSettingMenu("Set Ticks", axis.getLine(), panel);
        item = new MenuItem("Set Length");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting axis", "Set titck length", 
                        ""+_axis.getTickLength());
                if (str != null) {
                    _axis.setTickLength(Double.parseDouble(str));
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        item = new MenuItem("Set Ndivision");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting axis", "Set ndivision", 
                        ""+_axis.getNdivisions());
                if (str != null) {
                    _axis.setNdivisions(Integer.parseInt(str));
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        getItems().add(menu);
        if (_axis.get().getMin() > 0) {
            if (!_axis.isLogScale()) {
                item = new MenuItem("Set Log Scale");
            } else {
                item = new MenuItem("Unset Log Scale");
            }
            item.setOnAction(new EventHandler<ActionEvent>() {
                @Override
                public void handle(ActionEvent event) {
                    hide();
                    _axis.setLogScale(!_axis.isLogScale());
                    _panel.repaint();
                }
            });
            getItems().add(item);
        }
    }

}
