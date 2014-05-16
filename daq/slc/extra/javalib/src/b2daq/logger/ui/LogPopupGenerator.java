package b2daq.logger.ui;

import b2daq.logger.core.LogMessage;
import b2daq.logger.core.SystemLogLevel;

public class LogPopupGenerator {
	public LogPopup getPopup(LogMessage message) {
		int level = message.getPriority().getLevel();
		if (level == SystemLogLevel.DEBUG.getLevel() ) {
			return new LogInfoPopup(message);
		} else if (level == SystemLogLevel.INFO.getLevel() ) {
			return new LogInfoPopup(message);
		} else if (level == SystemLogLevel.NOTICE.getLevel() ) {
			return new LogNoticePopup(message);
		} else if (level == SystemLogLevel.WARNING.getLevel() ) {
			return new LogWarningPopup(message);
		} else if (level == SystemLogLevel.ERROR.getLevel() ) {
			return new LogErrorPopup(message);
		} else if (level == SystemLogLevel.FATAL.getLevel() ) {
			return new LogFatalPopup(message);
		} else {
			return new LogInfoPopup(message);
		}
	}
}
