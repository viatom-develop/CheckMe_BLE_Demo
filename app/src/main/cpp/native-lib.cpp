#include <jni.h>
#include <string>
#include "EcgInterface.h"


extern "C"
JNIEXPORT void JNICALL
Java_com_vaca_ecg_1respiration_MainActivity_initEcgRespiration(JNIEnv *env, jobject thiz) {
    ECG_ALG_CONFIG alg_config;
    char ecg_alg_version[30] = {0};
    alg_config.sOrgSampleRate = 250;
    alg_config.cWorkMode = 2;
    alg_config.nCeilingValue = 12;
    alg_config.bJapanVersion = false;
    EcgAlgInitialize();
    EcgAlgSetup(alg_config, ecg_alg_version);
    EcgAlgGetResult(NULL,true);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_vaca_ecg_1respiration_MainActivity_inputEcgPoint(JNIEnv *env, jobject thiz, jint data) {
    EcgAlgAnalysis(data);
    ECG_ALG_RESULT alg_result={0};
    EcgAlgGetResult(&alg_result,0);
    return alg_result.RespRate;
}