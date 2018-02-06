#include <jni.h>
/* Header for class de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding */

#ifndef IBNET_MSGRC_MSGRCJNIBINDING_H
#define IBNET_MSGRC_MSGRCJNIBINDING_H
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding
 * Method:    init
 * Signature: (Lde/hhu/bsinfo/net/ib/MsgrcJNIBinding/CallbackHandler;ZZISIIIIIIIJI)Z
 */
JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_init
    (JNIEnv *, jclass, jobject, jboolean, jboolean, jint, jshort, jint,
        jint, jint, jint, jint, jint, jint, jlong, jint);

/*
 * Class:     de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding
 * Method:    shutdown
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_shutdown
    (JNIEnv *, jclass);

/*
 * Class:     de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding
 * Method:    addNode
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_addNode
(JNIEnv *, jclass, jint);

/*
 * Class:     de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding
 * Method:    getSendBufferAddress
 * Signature: (S)J
 */
JNIEXPORT jlong JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_getSendBufferAddress
    (JNIEnv *, jclass, jshort);

/*
 * Class:     de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding
 * Method:    returnRecvBuffer
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_de_hhu_bsinfo_dxnet_ib_MsgrcJNIBinding_returnRecvBuffer
(JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif //IBNET_MSGRC_MSGRCJNIBINDING_H
