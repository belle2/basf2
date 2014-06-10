/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.hvcontrol.core.HVState;
import javafx.scene.control.TableRow;
import javafx.scene.control.TableView;
import javafx.util.Callback;

/**
 *
 * @author tkonno
 */
public class HVStatusRowCellFactory implements Callback<TableView<HVStatusProperty>, TableRow<HVStatusProperty>> {

    @Override
    public TableRow<HVStatusProperty> call(TableView<HVStatusProperty> tableView) {
        final TableRow<HVStatusProperty> row = new TableRow<HVStatusProperty>() {
            @Override
            protected void updateItem(HVStatusProperty hvstatus, boolean empty) {
                super.updateItem(hvstatus, empty);
                if (!isEmpty()) {
                    getStyleClass().removeAll("hvstate-off", "hvstate-peak",
                            "hvstate-stable", "hvstate-transition",
                            "hvstate-error");
                    HVState state = new HVState();
                    state.copy(hvstatus.getState());
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
                }
            }
        };
        return row;
    }
}
