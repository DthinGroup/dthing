
package com.yarlungsoft.ams;

/**
 * Store and fetch information of applet list node.
 */
public class AppletListNode {

    private AppletContent mCurAppContent;
    private AppletListNode mNext;

    AppletListNode(AppletContent ac) {
        mCurAppContent = ac;
        mNext = null;
    }

    AppletListNode() {
        this(null);
    }

    protected void setAppletContentInfo(AppletContent ac) {
        mCurAppContent = ac;
    }

    protected AppletContent getAppletContentInfo() {
        return mCurAppContent;
    }

    protected void setAppletListNextNode(AppletListNode nextNode) {
        mNext = nextNode;
    }

    protected AppletListNode getAppletListNextNode() {
        return mNext;
    }
}
