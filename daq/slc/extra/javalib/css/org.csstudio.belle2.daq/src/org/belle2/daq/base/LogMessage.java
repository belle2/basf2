package org.belle2.daq.base;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 *
 * @author tkonno
 */
public class LogMessage {
    
    private String from;
    private LogLevel level;
    private Date date;
    private String message;
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd-MM-yyyy");
    
    static public final String getCSSPath() {
        return LogMessage.class.getResource("LogMessage.css").toExternalForm();
    }
    
    public LogMessage(String from, LogLevel level, Date date, String message) {
        this.from = from;
        this.level = level;
        this.date = date;
        this.message = message;
    }
    
    public LogMessage(String from, LogLevel level, String message) {
        this(from, level, new Date(), message);
    }
    
    public String getFrom() {
        return from;
    }

    public LogLevel getLevel() {
        return level;
    }

    public Date getDate() {
        return date;
    }

    public String getDateText() {
        return dateformat.format(getDate());
    }

    public String getMessage() {
        return message;
    }
    
}
