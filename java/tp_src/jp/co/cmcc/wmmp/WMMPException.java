package jp.co.cmcc.wmmp;

import java.io.IOException;

public class WMMPException extends IOException {
	public WMMPException()
	{
		super();
	}
	
	public WMMPException(String msg)
	{
		super(msg);
	}
}
