package com.yarlungsoft.ams;

public class AppletListNode {
	
	AppletContent curAppContent;
	AppletListNode    next;

	AppletListNode(AppletContent ac) {
		curAppContent = ac;
		next = null;
	}

	AppletListNode () {
		this(null);
	}

	void setAppletContentInfo(AppletContent ac) {
		curAppContent = ac;
	}

	AppletContent getAppletContentInfo () {
		return curAppContent;
	}
	
	void setAppletListNextNode(AppletListNode nextNode) {
		next = nextNode;
	}
	
	AppletListNode getAppletListNextNode() {
		return next;
	}
	
	
	
}
