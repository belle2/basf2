/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.database.ConfigObject;
import b2daq.hvcontrol.core.HVState;
import java.net.URL;
import java.util.Comparator;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.Node;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.util.Callback;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerSupplyCrateStatusTableController implements Initializable {

    @FXML
    private TableView table;
    @FXML
    private TableColumn col_state;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        col_state.setComparator(new Comparator<HVState>() {
            @Override
            public int compare(HVState t, HVState t1) {
                return (t.getId() > t1.getId()) ? 1 : 0;
            }
        });
        col_state.setCellFactory(new Callback<TableColumn<HVStatusProperty, HVState>, TableCell<HVStatusProperty, HVState>>() {
            @Override
            public TableCell<HVStatusProperty, HVState> call(TableColumn<HVStatusProperty, HVState> param) {
                return new TableCell<HVStatusProperty, HVState>() {
                    @Override
                    protected void updateItem(HVState item, boolean empty) {
                        super.updateItem(item, empty);
                        getStyleClass().removeAll("hvstate-off", "hvstate-peak",
                                "hvstate-stable", "hvstate-transition",
                                "hvstate-error");
                        if (!empty && item != null) {
                            HVState state = (HVState) item;
                            if (state.equals(HVState.OFF_S)) {
                                getStyleClass().add("hvstate-off");
                            } else if (state.equals(HVState.PEAK_S)) {
                                getStyleClass().add("hvstate-peak");
                            } else if (state.isStable()) {
                                getStyleClass().add("hvstate-stable");
                            } else if (state.isTransition()) {
                                getStyleClass().add("hvstate-transition");
                            } else if (state.isError()) {
                                getStyleClass().add("hvstate-error");
                            }
                            setText(item.getLabel());
                        } else {
                            setText(null);
                        }
                    }
                };
            }
        });
        col_state.setCellValueFactory(new PropertyValueFactory<>("state"));

    }

    void setDB(ConfigObject obj) {
        table.getItems().add(new HVStatusProperty(obj.getIndex(),
                obj.getInt("slot"), obj.getInt("channel")));
    }
    
    public HVStatusProperty getStatus(int index) {
        return (HVStatusProperty)table.getItems().get(index);
    }

    Node getTable() {
        return table;
    }
}
