/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.database.FieldInfo;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeTableCell;
import javafx.scene.control.TreeTableColumn;
import javafx.scene.control.TreeTableView;
import javafx.scene.control.cell.TreeItemPropertyValueFactory;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.util.Callback;

/**
 *
 * @author tkonno
 */
public class ConfigViewPane extends VBox {

    TreeItem<ConfigParameter> m_root;

    public ConfigViewPane(ConfigObject obj) {
        set(obj);
    }

    private TreeItem<ConfigParameter> getTree(String name_in, ConfigObject obj) {
        TreeItem<ConfigParameter> root = new TreeItem<>(new ConfigParameter((name_in.length() > 0) ? name_in : obj.getName(), obj.getPath()));
        for (String name : obj.getFieldNames()) {
            if (obj.getProperty(name).getType() == FieldInfo.TEXT) {
                root.getChildren().add(new TreeItem<>(new ConfigParameter(name, obj.getText(name))));
            } else if (obj.getProperty(name).getType() != FieldInfo.OBJECT) {
                root.getChildren().add(new TreeItem<>(new ConfigParameter(name, obj.getValueText(name))));
            } else {
                int i = 0;
                int len = obj.getObjects(name).size();
                for (ConfigObject cobj : obj.getObjects(name)) {
                    root.getChildren().add(getTree((len > 1) ? name + "[" + i + "]" : "", cobj));
                    i++;
                }
            }
        }
        return root;
    }

    public final void set(ConfigObject obj) {
        m_root = getTree("", obj);
        TreeTableColumn<ConfigParameter, String> paramColumn = new TreeTableColumn<>("Parameter");
        paramColumn.setCellValueFactory(new TreeItemPropertyValueFactory<>("name"));
        paramColumn.setEditable(true);
        paramColumn.setSortable(false);

        TreeTableColumn<ConfigParameter, String> valueColumn = new TreeTableColumn<>("Value");
        valueColumn.setCellValueFactory(new TreeItemPropertyValueFactory<>("value"));
        valueColumn.setEditable(true);
        valueColumn.setSortable(false);

        valueColumn.setCellFactory(new Callback<TreeTableColumn<ConfigParameter, String>, TreeTableCell<ConfigParameter, String>>() {
            @Override
            public TreeTableCell<ConfigParameter, String> call(TreeTableColumn<ConfigParameter, String> p) {
                TreeTableCell<ConfigParameter, String> cell = new TreeTableCell<ConfigParameter, String>() {
                    @Override
                    protected void updateItem(String item, boolean empty) {
                        super.updateItem(item, empty);
                        if (item == null || empty) {
                            setText("");
                        } else {
                            setText(item);
                        }
                    }
                };
                cell.addEventFilter(MouseEvent.MOUSE_CLICKED, (MouseEvent event) -> {
                    if (event.getClickCount() > 1) {
                        System.out.println("double clicked!");
                        TreeTableCell c = (TreeTableCell) event.getSource();
                        System.out.println("Cell text: " + c.getText());
                    }
                });
                return cell;
            }
        });

        TreeTableView view = new TreeTableView<>(m_root);
        view.getColumns().setAll(paramColumn, valueColumn);
        paramColumn.prefWidthProperty().bind(view.widthProperty().divide(2));
        valueColumn.prefWidthProperty().bind(view.widthProperty().divide(2));
        ScrollPane scroll = new ScrollPane();
        scroll.setFitToWidth(true);
        scroll.setFitToHeight(true);
        scroll.setContent(view);
        TextField field = new TextField(obj.getName());
        getChildren().add(field);
        getChildren().add(scroll);
        VBox.setVgrow(scroll, Priority.ALWAYS);
    }
}
