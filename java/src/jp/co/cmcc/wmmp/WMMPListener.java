package jp.co.cmcc.wmmp;

public interface WMMPListener {
    //void onDataReceived(byte[] data);
    void onDataReceived(int cmd,WMMPDataItem[] ditem);
}
