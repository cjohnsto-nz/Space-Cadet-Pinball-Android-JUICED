#include "SpaceCadetPinballJNI.h"
#include "../../../../SpaceCadetPinball/winmain.h"
#include "../../../../SpaceCadetPinball/Sound.h"
#include "../../../../SpaceCadetPinball/pinball.h"
#include "../../../../SpaceCadetPinball/control.h"
#include "../../../../SpaceCadetPinball/HDRConfig.h"
#include "../../../../SpaceCadetPinball/pb.h"
#include <jni.h>
#include <android/log.h>

static JavaVM* g_JavaVM = nullptr;
static jclass clazz = nullptr;
static JNIEnv *env = nullptr;
static bool s_ballInPlunger = false;

void SpaceCadetPinballJNI::show_error_dialog(std::string title, std::string message) {
    __android_log_print(ANDROID_LOG_ERROR, "SpaceCadetPinballJNI", "Error: %s, %s", title.c_str(), message.c_str());
}

void SpaceCadetPinballJNI::notifyGameState(int state) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "setState", "(I)V");

    env->CallStaticVoidMethod(clazz, mid, state);
}

void SpaceCadetPinballJNI::setBallInPlunger(bool isInPlunger) {
    s_ballInPlunger = isInPlunger;
    
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "setBallInPlunger", "(Z)V");

    env->CallStaticVoidMethod(clazz, mid, isInPlunger);
}

bool SpaceCadetPinballJNI::isBallInPlunger() {
    return s_ballInPlunger;
}

void SpaceCadetPinballJNI::addHighScore(int score) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "addHighScore", "(I)V");

    env->CallStaticVoidMethod(clazz, mid, score);
}

int SpaceCadetPinballJNI::getHighScore() {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "getHighScore", "()I");

    return env->CallStaticIntMethod(clazz, mid);
}

void SpaceCadetPinballJNI::displayText(const char* text, int type) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "printString", "(Ljava/lang/String;I)V");

    jstring str = env->NewStringUTF(text);

    env->CallStaticVoidMethod(clazz, mid, str, type);
}

void SpaceCadetPinballJNI::postRemainingBalls(int balls) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "setRemainingBalls", "(I)V");

    env->CallStaticVoidMethod(clazz, mid, balls);
}

void SpaceCadetPinballJNI::clearText(int type) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "clearText", "(I)V");

    env->CallStaticVoidMethod(clazz, mid, type);
}

void SpaceCadetPinballJNI::postScore(int score) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "postScore", "(I)V");

    env->CallStaticVoidMethod(clazz, mid, score);
}

void SpaceCadetPinballJNI::postBallCount(int count) {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "postBallCount", "(I)V");

    env->CallStaticVoidMethod(clazz, mid, count);
}

void SpaceCadetPinballJNI::cheatsUsed() {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "cheatsUsed", "()V");

    env->CallStaticVoidMethod(clazz, mid);
}

void SpaceCadetPinballJNI::gameReady() {
    if (env == nullptr) g_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_6);

    if (clazz == nullptr) clazz = env->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    jmethodID mid = env->GetStaticMethodID(clazz, "gameIsReady", "()V");

    env->CallStaticVoidMethod(clazz, mid);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_initNative(JNIEnv *env, jobject thiz,
        jstring data_path) {
winmain::BasePath = (char *) env->GetStringUTFChars(data_path, nullptr);
    env->GetJavaVM(&g_JavaVM);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_setVolume(JNIEnv *env, jobject thiz, jint vol) {
    Sound::SetVolume(vol);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_putString(JNIEnv *env, jobject thiz, jint id, jstring str) {
    LPCSTR mstr = (*env).GetStringUTFChars(str, nullptr);
    pinball::set_rc_string(id, mstr);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_checkCheatsUsed(JNIEnv *env, jobject thiz) {
    return control::check_cheats();
}

void SpaceCadetPinballJNI::triggerHapticFeedback(float intensity) {
    JNIEnv* jniEnv = nullptr;
    if (g_JavaVM == nullptr) return;
    g_JavaVM->GetEnv((void **) &jniEnv, JNI_VERSION_1_6);
    if (jniEnv == nullptr) return;

    jclass jniClass = jniEnv->FindClass("com/fexed/spacecadetpinball/JNIEntryPoint");
    if (jniClass == nullptr) return;
    
    jmethodID mid = jniEnv->GetStaticMethodID(jniClass, "triggerHapticFeedback", "(F)V");
    if (mid == nullptr) return;

    jniEnv->CallStaticVoidMethod(jniClass, mid, intensity);
}

// HDR Support Functions
bool SpaceCadetPinballJNI::queryHDRSupport() {
    return HDR::g_hdrCapabilities.isSupported;
}

float SpaceCadetPinballJNI::getMaxDisplayLuminance() {
    return HDR::g_hdrCapabilities.maxLuminanceNits;
}

void SpaceCadetPinballJNI::setHDRCapabilities(bool supported, bool bt2020, bool pq, bool scrgb,
                                              bool fp16, float maxNits, float minNits) {
    HDR::g_hdrCapabilities.isSupported = supported;
    HDR::g_hdrCapabilities.isBT2020Supported = bt2020;
    HDR::g_hdrCapabilities.isPQSupported = pq;
    HDR::g_hdrCapabilities.isScRGBSupported = scrgb;
    HDR::g_hdrCapabilities.isFP16Supported = fp16;
    HDR::g_hdrCapabilities.maxLuminanceNits = maxNits;
    HDR::g_hdrCapabilities.minLuminanceNits = minNits;
    
    __android_log_print(ANDROID_LOG_INFO, "SpaceCadetPinballJNI", 
        "HDR Capabilities: supported=%d, bt2020=%d, pq=%d, scrgb=%d, fp16=%d, maxNits=%.1f, minNits=%.4f",
        supported, bt2020, pq, scrgb, fp16, maxNits, minNits);
    
    HDR::InitHDR();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_setHDRCapabilities(JNIEnv *env, jobject thiz,
        jboolean supported, jboolean bt2020, jboolean pq, jboolean scrgb,
        jboolean fp16, jfloat maxNits, jfloat minNits) {
    SpaceCadetPinballJNI::setHDRCapabilities(supported, bt2020, pq, scrgb, fp16, maxNits, minNits);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_isHDRActive(JNIEnv *env, jobject thiz) {
    return HDR::IsHDRActive();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_setHDREnabled(JNIEnv *env, jobject thiz, jboolean enabled) {
    HDR::SetHDREnabled(enabled);
}

// Light editor JNI functions
#include "../../../../SpaceCadetPinball/HDRLightOverlay.h"
#include "../../../../SpaceCadetPinball/HDRRenderer.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_setLightEditMode(JNIEnv *env, jobject thiz, jboolean enabled) {
    HDRLightOverlay::SetEditMode(enabled);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_getLightEditMode(JNIEnv *env, jobject thiz) {
    return HDRLightOverlay::GetEditMode();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_onLightTouchDown(JNIEnv *env, jobject thiz, 
        jfloat screenX, jfloat screenY, jint viewportX, jint viewportY, jint viewportW, jint viewportH) {
    // Use viewport from HDRRenderer instead of passed values (more accurate)
    int vx = HDRRenderer::GetViewportX();
    int vy = HDRRenderer::GetViewportY();
    int vw = HDRRenderer::GetViewportW();
    int vh = HDRRenderer::GetViewportH();
    if (vw > 0 && vh > 0) {
        HDRLightOverlay::OnTouchDown(screenX, screenY, vx, vy, vw, vh);
    } else {
        HDRLightOverlay::OnTouchDown(screenX, screenY, viewportX, viewportY, viewportW, viewportH);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_onLightTouchMove(JNIEnv *env, jobject thiz,
        jfloat screenX, jfloat screenY, jint viewportX, jint viewportY, jint viewportW, jint viewportH) {
    // Use viewport from HDRRenderer instead of passed values (more accurate)
    int vx = HDRRenderer::GetViewportX();
    int vy = HDRRenderer::GetViewportY();
    int vw = HDRRenderer::GetViewportW();
    int vh = HDRRenderer::GetViewportH();
    if (vw > 0 && vh > 0) {
        HDRLightOverlay::OnTouchMove(screenX, screenY, vx, vy, vw, vh);
    } else {
        HDRLightOverlay::OnTouchMove(screenX, screenY, viewportX, viewportY, viewportW, viewportH);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_onLightTouchUp(JNIEnv *env, jobject thiz) {
    HDRLightOverlay::OnTouchUp();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_hasLightSelection(JNIEnv *env, jobject thiz) {
    return HDRLightOverlay::HasSelection();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_saveLightPositions(JNIEnv *env, jobject thiz, jstring filepath) {
    const char* path = env->GetStringUTFChars(filepath, nullptr);
    bool result = HDRLightOverlay::SaveLightPositions(path);
    env->ReleaseStringUTFChars(filepath, path);
    return result;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_loadLightPositions(JNIEnv *env, jobject thiz, jstring filepath) {
    const char* path = env->GetStringUTFChars(filepath, nullptr);
    bool result = HDRLightOverlay::LoadLightPositions(path);
    env->ReleaseStringUTFChars(filepath, path);
    return result;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_getSelectedLightIndex(JNIEnv *env, jobject thiz) {
    return HDRLightOverlay::GetSelectedLightIndex();
}

// Settings activity save function
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fexed_spacecadetpinball_Settings_saveLightPositionsNative(JNIEnv *env, jobject thiz, jstring filepath) {
    const char* path = env->GetStringUTFChars(filepath, nullptr);
    bool result = HDRLightOverlay::SaveLightPositions(path);
    env->ReleaseStringUTFChars(filepath, path);
    return result;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_fexed_spacecadetpinball_Settings_resetOutOfBoundsLightsNative(JNIEnv *env, jobject thiz) {
    return HDRLightOverlay::ResetOutOfBoundsLights();
}

// Flag to request demo mode toggle from game thread
static bool s_requestDemoToggle = false;

bool SpaceCadetPinballJNI::shouldToggleDemo() {
    if (s_requestDemoToggle) {
        s_requestDemoToggle = false;
        return true;
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fexed_spacecadetpinball_MainActivity_triggerDemoMode(JNIEnv *env, jobject thiz) {
    // Set flag to be processed on game thread
    s_requestDemoToggle = true;
}