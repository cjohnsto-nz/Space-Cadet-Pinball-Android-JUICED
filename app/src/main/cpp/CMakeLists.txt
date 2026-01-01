cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
project(SpaceCadetPinball)

# Fetch Oboe for real-time audio processing
include(FetchContent)
FetchContent_Declare(
    oboe
    GIT_REPOSITORY https://github.com/google/oboe.git
    GIT_TAG 1.8.0
)
FetchContent_MakeAvailable(oboe)

set(SOURCE_FILES
        ../../../../SpaceCadetPinball/control.cpp
        ../../../../SpaceCadetPinball/control.h
        ../../../../SpaceCadetPinball/EmbeddedData.cpp
        ../../../../SpaceCadetPinball/EmbeddedData.h
        ../../../../SpaceCadetPinball/fullscrn.cpp
        ../../../../SpaceCadetPinball/fullscrn.h
        ../../../../SpaceCadetPinball/gdrv.cpp
        ../../../../SpaceCadetPinball/gdrv.h
        ../../../../SpaceCadetPinball/GroupData.cpp
        ../../../../SpaceCadetPinball/GroupData.h
        ../../../../SpaceCadetPinball/high_score.cpp
        ../../../../SpaceCadetPinball/high_score.h
        ../../../../SpaceCadetPinball/loader.cpp
        ../../../../SpaceCadetPinball/loader.h
        ../../../../SpaceCadetPinball/maths.cpp
        ../../../../SpaceCadetPinball/maths.h
        ../../../../SpaceCadetPinball/midi.cpp
        ../../../../SpaceCadetPinball/midi.h
        ../../../../SpaceCadetPinball/nudge.cpp
        ../../../../SpaceCadetPinball/nudge.h
        ../../../../SpaceCadetPinball/options.cpp
        ../../../../SpaceCadetPinball/options.h
        ../../../../SpaceCadetPinball/partman.cpp
        ../../../../SpaceCadetPinball/partman.h
        ../../../../SpaceCadetPinball/pb.cpp
        ../../../../SpaceCadetPinball/pb.h
        ../../../../SpaceCadetPinball/pch.h
        ../../../../SpaceCadetPinball/pinball.cpp
        ../../../../SpaceCadetPinball/pinball.h
        ../../../../SpaceCadetPinball/proj.cpp
        ../../../../SpaceCadetPinball/proj.h
        ../../../../SpaceCadetPinball/render.cpp
        ../../../../SpaceCadetPinball/render.h
        ../../../../SpaceCadetPinball/score.cpp
        ../../../../SpaceCadetPinball/score.h
        ../../../../SpaceCadetPinball/Sound.cpp
        ../../../../SpaceCadetPinball/Sound.h
        ../../../../SpaceCadetPinball/SpaceCadetPinball.cpp
        ../../../../SpaceCadetPinball/TBall.cpp
        ../../../../SpaceCadetPinball/TBall.h
        ../../../../SpaceCadetPinball/TBlocker.cpp
        ../../../../SpaceCadetPinball/TBlocker.h
        ../../../../SpaceCadetPinball/TBumper.cpp
        ../../../../SpaceCadetPinball/TBumper.h
        ../../../../SpaceCadetPinball/TCircle.cpp
        ../../../../SpaceCadetPinball/TCircle.h
        ../../../../SpaceCadetPinball/TCollisionComponent.cpp
        ../../../../SpaceCadetPinball/TCollisionComponent.h
        ../../../../SpaceCadetPinball/TComponentGroup.cpp
        ../../../../SpaceCadetPinball/TComponentGroup.h
        ../../../../SpaceCadetPinball/TDrain.cpp
        ../../../../SpaceCadetPinball/TDrain.h
        ../../../../SpaceCadetPinball/TEdgeBox.h
        ../../../../SpaceCadetPinball/TEdgeManager.cpp
        ../../../../SpaceCadetPinball/TEdgeManager.h
        ../../../../SpaceCadetPinball/TEdgeSegment.cpp
        ../../../../SpaceCadetPinball/TEdgeSegment.h
        ../../../../SpaceCadetPinball/TFlagSpinner.cpp
        ../../../../SpaceCadetPinball/TFlagSpinner.h
        ../../../../SpaceCadetPinball/TFlipper.cpp
        ../../../../SpaceCadetPinball/TFlipper.h
        ../../../../SpaceCadetPinball/TFlipperEdge.cpp
        ../../../../SpaceCadetPinball/TFlipperEdge.h
        ../../../../SpaceCadetPinball/TGate.cpp
        ../../../../SpaceCadetPinball/TGate.h
        ../../../../SpaceCadetPinball/THole.cpp
        ../../../../SpaceCadetPinball/THole.h
        ../../../../SpaceCadetPinball/timer.cpp
        ../../../../SpaceCadetPinball/timer.h
        ../../../../SpaceCadetPinball/TKickback.cpp
        ../../../../SpaceCadetPinball/TKickback.h
        ../../../../SpaceCadetPinball/TKickout.cpp
        ../../../../SpaceCadetPinball/TKickout.h
        ../../../../SpaceCadetPinball/TLight.cpp
        ../../../../SpaceCadetPinball/TLight.h
        ../../../../SpaceCadetPinball/TLightBargraph.cpp
        ../../../../SpaceCadetPinball/TLightBargraph.h
        ../../../../SpaceCadetPinball/TLightGroup.cpp
        ../../../../SpaceCadetPinball/TLightGroup.h
        ../../../../SpaceCadetPinball/TLightRollover.cpp
        ../../../../SpaceCadetPinball/TLightRollover.h
        ../../../../SpaceCadetPinball/TLine.cpp
        ../../../../SpaceCadetPinball/TLine.h
        ../../../../SpaceCadetPinball/TOneway.cpp
        ../../../../SpaceCadetPinball/TOneway.h
        ../../../../SpaceCadetPinball/TPinballComponent.cpp
        ../../../../SpaceCadetPinball/TPinballComponent.h
        ../../../../SpaceCadetPinball/TPinballTable.cpp
        ../../../../SpaceCadetPinball/TPinballTable.h
        ../../../../SpaceCadetPinball/TPlunger.cpp
        ../../../../SpaceCadetPinball/TPlunger.h
        ../../../../SpaceCadetPinball/TPopupTarget.cpp
        ../../../../SpaceCadetPinball/TPopupTarget.h
        ../../../../SpaceCadetPinball/TRamp.cpp
        ../../../../SpaceCadetPinball/TRamp.h
        ../../../../SpaceCadetPinball/TRollover.cpp
        ../../../../SpaceCadetPinball/TRollover.h
        ../../../../SpaceCadetPinball/TSink.cpp
        ../../../../SpaceCadetPinball/TSink.h
        ../../../../SpaceCadetPinball/TSoloTarget.cpp
        ../../../../SpaceCadetPinball/TSoloTarget.h
        ../../../../SpaceCadetPinball/TSound.cpp
        ../../../../SpaceCadetPinball/TSound.h
        ../../../../SpaceCadetPinball/TTableLayer.cpp
        ../../../../SpaceCadetPinball/TTableLayer.h
        ../../../../SpaceCadetPinball/TTextBox.cpp
        ../../../../SpaceCadetPinball/TTextBox.h
        ../../../../SpaceCadetPinball/TTextBoxMessage.cpp
        ../../../../SpaceCadetPinball/TTextBoxMessage.h
        ../../../../SpaceCadetPinball/TTimer.cpp
        ../../../../SpaceCadetPinball/TTimer.h
        ../../../../SpaceCadetPinball/TTripwire.cpp
        ../../../../SpaceCadetPinball/TTripwire.h
        ../../../../SpaceCadetPinball/TWall.cpp
        ../../../../SpaceCadetPinball/TWall.h
        ../../../../SpaceCadetPinball/winmain.cpp
        ../../../../SpaceCadetPinball/winmain.h
        ../../../../SpaceCadetPinball/zdrv.cpp
        ../../../../SpaceCadetPinball/zdrv.h
        ../../../../SpaceCadetPinball/HDRConfig.cpp
        ../../../../SpaceCadetPinball/HDRConfig.h
        ../../../../SpaceCadetPinball/HDRRenderer.cpp
        ../../../../SpaceCadetPinball/HDRRenderer.h
        ../../../../SpaceCadetPinball/HDRLightOverlay.cpp
        ../../../../SpaceCadetPinball/HDRLightOverlay.h
        ../../../../SpaceCadetPinball/TimerMode.cpp
        ../../../../SpaceCadetPinball/TimerMode.h
        OboeMusicPlayer.cpp
        OboeMusicPlayer.h
        SpaceCadetPinballJNI.cpp
        SpaceCadetPinballJNI.h)

add_library(SpaceCadetPinball SHARED ${SOURCE_FILES})

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/include
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL_mixer/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL_mixer)

find_library(log-lib log)
find_library(gles3-lib GLESv3)
find_library(egl-lib EGL)
find_library(android-lib android)
find_library(media-ndk-lib mediandk)

target_link_libraries(SpaceCadetPinball SDL2 SDL2_mixer oboe ${log-lib} ${gles3-lib} ${egl-lib} ${android-lib} ${media-ndk-lib})