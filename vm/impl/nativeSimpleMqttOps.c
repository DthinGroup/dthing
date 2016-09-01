#include <utfstring.h>
#include <vm_common.h>
#include "nativeSimpleMqttOps.h"

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    subscribe0
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIZ)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_connect0(const u4* args, JValue* pResult) {
    StringObject * topicObj = (StringObject *) args[1];
    const jchar* topic = dvmGetStringData(topicObj);
//    const char* topic = dvmCreateCstrFromString(topicObj);
    int topicLen = dvmGetStringLength(topicObj);
    jint qos = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    subscribe0
 * Signature: (Ljava/lang/String;I)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_subscribe0(const u4* args, JValue* pResult) {
    StringObject * topicObj = (StringObject *) args[1];
    const jchar* topic = dvmGetStringData(topicObj);
//    const char* topic = dvmCreateCstrFromString(topicObj);
    int topicLen = dvmGetStringLength(topicObj);
    jint qos = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}