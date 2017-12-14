package b2daq.core;

import java.io.IOException;

public interface Serializable {

	public void readObject(Reader reader) throws IOException ;
	public void writeObject(Writer writer) throws IOException ;

}
