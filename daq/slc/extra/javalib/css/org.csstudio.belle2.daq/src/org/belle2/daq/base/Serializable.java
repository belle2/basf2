package org.belle2.daq.base;

import java.io.IOException;

public interface Serializable {

	public void readObject(Reader reader) throws IOException ;
	public void writeObject(Writer writer) throws IOException ;

}
