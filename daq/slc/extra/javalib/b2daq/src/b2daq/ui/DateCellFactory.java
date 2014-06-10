/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.ui;

/**
 *
 * @author tkonno
 */
import java.text.Format;
import java.text.SimpleDateFormat;
import java.util.Date;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.util.Callback;

public class DateCellFactory<S, T> implements Callback<TableColumn<S, T>, TableCell<S, T>> {

    private Date date = new Date();
    private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");

    public Date getDate() {
        return date;
    }

    public void setDate(Date date) {
        this.date = date;
    }

    @Override
    public TableCell<S, T> call(TableColumn<S, T> p) {
        TableCell<S, T> cell = new TableCell<S, T>() {

            @Override
            public void updateItem(Object item, boolean empty) {
                super.updateItem((T) item, empty);
                if (!empty) {
                    setText(dateformat.format(item));
                } else {
                    setText(null);
                }
            }
        };
        return cell;
    }
}
