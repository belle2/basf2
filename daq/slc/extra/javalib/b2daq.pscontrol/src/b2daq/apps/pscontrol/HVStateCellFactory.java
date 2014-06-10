/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

/**
 *
 * @author tkonno
 */
import b2daq.hvcontrol.core.HVState;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.util.Callback;

public class HVStateCellFactory<S, T> implements Callback<TableColumn<S, T>, TableCell<S, T>> {

    private HVState state = new HVState();

    public HVState getState() {
        return state;
    }

    public void setState(HVState state) {
        this.state = state;
    }

    @Override
    public TableCell<S, T> call(TableColumn<S, T> p) {
        TableCell<S, T> cell = new TableCell<S, T>() {

            @Override
            public void updateItem(Object item, boolean empty) {
                super.updateItem((T) item, empty);
                if (!empty) {
                    getStyleClass().removeAll("hvstate-off", "hvstate-peak",
                            "hvstate-stable", "hvstate-transition",
                            "hvstate-error");
                    HVState state = new HVState();
                    //state.copy(hvstatus.getState());
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

                    setText(state.getLabel());
                } else {
                    setText(null);
                }
            }
        };
        return cell;
    }
}
