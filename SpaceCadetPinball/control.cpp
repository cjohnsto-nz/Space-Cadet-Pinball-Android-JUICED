#include "pch.h"
#include "control.h"

#include <cmath>

#ifdef __ANDROID__
#include <jni.h>
#include "../app/src/main/cpp/SpaceCadetPinballJNI.h"
#endif

#include "TimerMode.h"
#include "options.h"
#include "pb.h"
#include "pinball.h"
#include "TBlocker.h"
#include "TBumper.h"
#include "TComponentGroup.h"
#include "TFlagSpinner.h"
#include "TLight.h"
#include "TLightBargraph.h"
#include "TLightGroup.h"
#include "TLightRollover.h"
#include "TPinballTable.h"
#include "TPopupTarget.h"
#include "TSink.h"
#include "TSoloTarget.h"
#include "TSound.h"
#include "TTripwire.h"
#include "TDrain.h"
#include "TFlipper.h"
#include "TGate.h"
#include "THole.h"
#include "TKickback.h"
#include "TKickout.h"
#include "TOneway.h"
#include "TRamp.h"
#include "TPlunger.h"
#include "HDRLightOverlay.h"
#include "HDRConfig.h"
#include "TWall.h"
#include "TTextBox.h"

int control_bump_scores1[] = {500, 1000, 1500, 2000};
int control_roll_scores1[] = {2000};
int control_bump_scores2[] = {1500, 2500, 3500, 4500};
int control_roll_scores2[] = {500};
int control_rebo_score1[] = {500};
int control_oneway4_score1[6] = {15000, 30000, 75000, 30000, 15000, 7500};
int control_ramp_score1[1] = {5000};
int control_roll_score1[1] = {20000};
int control_roll_score2[2] = {5000, 25000};
int control_roll_score3[1] = {10000};
int control_roll_score4[1] = {500};
int control_flag_score1[2] = {500, 2500};
int control_kickout_score1[5] = {10000, 0, 20000, 50000, 150000};
int control_sink_score1[3] = {2500, 5000, 7500};
int control_target_score1[2] = {500, 5000};
int control_target_score2[3] = {1500, 10000, 50000};
int control_target_score3[2] = {500, 1500};
int control_target_score4[1] = {750};
int control_target_score5[1] = {1000};
int control_target_score6[1] = {750};
int control_target_score7[1] = {750};
int control_roll_score5[1] = {10000};
int control_kickout_score2[1] = {20000};
int control_kickout_score3[1] = {50000};


component_tag<TComponentGroup> control_attack_bump_tag = {"attack_bumpers", nullptr};
component_tag<TComponentGroup> control_launch_bump_tag = {"launch_bumpers", nullptr};
component_tag<TBlocker> control_block1_tag = {"v_bloc1", nullptr};
component_tag<TBumper> control_bump1_tag = {"a_bump1", nullptr};
component_tag<TBumper> control_bump2_tag = {"a_bump2", nullptr};
component_tag<TBumper> control_bump3_tag = {"a_bump3", nullptr};
component_tag<TBumper> control_bump4_tag = {"a_bump4", nullptr};
component_tag<TBumper> control_bump5_tag = {"a_bump5", nullptr};
component_tag<TBumper> control_bump6_tag = {"a_bump6", nullptr};
component_tag<TBumper> control_bump7_tag = {"a_bump7", nullptr};
component_tag<TDrain> control_drain_tag = {"drain", nullptr};
component_tag<TFlagSpinner> control_flag1_tag = {"a_flag1", nullptr};
component_tag<TFlagSpinner> control_flag2_tag = {"a_flag2", nullptr};
component_tag<TFlipper> control_flip1_tag = {"a_flip1", nullptr};
component_tag<TFlipper> control_flip2_tag = {"a_flip2", nullptr};
component_tag<TLightBargraph> control_fuel_bargraph_tag = {"fuel_bargraph", nullptr};
component_tag<TGate> control_gate1_tag = {"v_gate1", nullptr};
component_tag<TGate> control_gate2_tag = {"v_gate2", nullptr};
component_tag<TTextBox> control_info_text_box_tag = {"info_text_box", nullptr};
component_tag<TKickback> control_kicker1_tag = {"a_kick1", nullptr};
component_tag<TKickback> control_kicker2_tag = {"a_kick2", nullptr};
component_tag<TKickout> control_kickout1_tag = {"a_kout1", nullptr};
component_tag<TKickout> control_kickout2_tag = {"a_kout2", nullptr};
component_tag<TKickout> control_kickout3_tag = {"a_kout3", nullptr};
component_tag<TLight> control_lite1_tag = {"lite1", nullptr};
component_tag<TLight> control_lite2_tag = {"lite2", nullptr};
component_tag<TLight> control_lite3_tag = {"lite3", nullptr};
component_tag<TLight> control_lite4_tag = {"lite4", nullptr};
component_tag<TLight> control_lite5_tag = {"lite5", nullptr};
component_tag<TLight> control_lite6_tag = {"lite6", nullptr};
component_tag<TLight> control_lite7_tag = {"lite7", nullptr};
component_tag<TLight> control_lite8_tag = {"lite8", nullptr};
component_tag<TLight> control_lite9_tag = {"lite9", nullptr};
component_tag<TLight> control_lite10_tag = {"lite10", nullptr};
component_tag<TLight> control_lite11_tag = {"lite11", nullptr};
component_tag<TLight> control_lite12_tag = {"lite12", nullptr};
component_tag<TLight> control_lite13_tag = {"lite13", nullptr};
component_tag<TLight> control_lite16_tag = {"lite16", nullptr};
component_tag<TLight> control_lite17_tag = {"lite17", nullptr};
component_tag<TLight> control_lite18_tag = {"lite18", nullptr};
component_tag<TLight> control_lite19_tag = {"lite19", nullptr};
component_tag<TLight> control_lite20_tag = {"lite20", nullptr};
component_tag<TLight> control_lite21_tag = {"lite21", nullptr};
component_tag<TLight> control_lite22_tag = {"lite22", nullptr};
component_tag<TLight> control_lite23_tag = {"lite23", nullptr};
component_tag<TLight> control_lite24_tag = {"lite24", nullptr};
component_tag<TLight> control_lite25_tag = {"lite25", nullptr};
component_tag<TLight> control_lite26_tag = {"lite26", nullptr};
component_tag<TLight> control_lite27_tag = {"lite27", nullptr};
component_tag<TLight> control_lite28_tag = {"lite28", nullptr};
component_tag<TLight> control_lite29_tag = {"lite29", nullptr};
component_tag<TLight> control_lite30_tag = {"lite30", nullptr};
component_tag<TLight> control_lite54_tag = {"lite54", nullptr};
component_tag<TLight> control_lite55_tag = {"lite55", nullptr};
component_tag<TLight> control_lite56_tag = {"lite56", nullptr};
component_tag<TLight> control_lite58_tag = {"lite58", nullptr};
component_tag<TLight> control_lite59_tag = {"lite59", nullptr};
component_tag<TLight> control_lite60_tag = {"lite60", nullptr};
component_tag<TLight> control_lite61_tag = {"lite61", nullptr};
component_tag<TLight> control_lite62_tag = {"lite62", nullptr};
component_tag<TLight> control_lite67_tag = {"lite67", nullptr};
component_tag<TLight> control_lite68_tag = {"lite68", nullptr};
component_tag<TLight> control_lite69_tag = {"lite69", nullptr};
component_tag<TLight> control_lite70_tag = {"lite70", nullptr};
component_tag<TLight> control_lite71_tag = {"lite71", nullptr};
component_tag<TLight> control_lite72_tag = {"lite72", nullptr};
component_tag<TLight> control_lite77_tag = {"lite77", nullptr};
component_tag<TLight> control_lite84_tag = {"lite84", nullptr};
component_tag<TLight> control_lite85_tag = {"lite85", nullptr};
component_tag<TLight> control_lite101_tag = {"lite101", nullptr};
component_tag<TLight> control_lite102_tag = {"lite102", nullptr};
component_tag<TLight> control_lite103_tag = {"lite103", nullptr};
component_tag<TLight> control_lite104_tag = {"lite104", nullptr};
component_tag<TLight> control_lite105_tag = {"lite105", nullptr};
component_tag<TLight> control_lite106_tag = {"lite106", nullptr};
component_tag<TLight> control_lite107_tag = {"lite107", nullptr};
component_tag<TLight> control_lite108_tag = {"lite108", nullptr};
component_tag<TLight> control_lite109_tag = {"lite109", nullptr};
component_tag<TLight> control_lite110_tag = {"lite110", nullptr};
component_tag<TLight> control_lite130_tag = {"lite130", nullptr};
component_tag<TLight> control_lite131_tag = {"lite131", nullptr};
component_tag<TLight> control_lite132_tag = {"lite132", nullptr};
component_tag<TLight> control_lite133_tag = {"lite133", nullptr};
component_tag<TLight> control_lite169_tag = {"lite169", nullptr};
component_tag<TLight> control_lite170_tag = {"lite170", nullptr};
component_tag<TLight> control_lite171_tag = {"lite171", nullptr};
component_tag<TLight> control_lite195_tag = {"lite195", nullptr};
component_tag<TLight> control_lite196_tag = {"lite196", nullptr};
component_tag<TLight> control_lite198_tag = {"lite198", nullptr};
component_tag<TLight> control_lite199_tag = {"lite199", nullptr};
component_tag<TLight> control_lite200_tag = {"lite200", nullptr};
component_tag<TLight> control_lite300_tag = {"lite300", nullptr};
component_tag<TLight> control_lite301_tag = {"lite301", nullptr};
component_tag<TLight> control_lite302_tag = {"lite302", nullptr};
component_tag<TLight> control_lite303_tag = {"lite303", nullptr};
component_tag<TLight> control_lite304_tag = {"lite304", nullptr};
component_tag<TLight> control_lite305_tag = {"lite305", nullptr};
component_tag<TLight> control_lite306_tag = {"lite306", nullptr};
component_tag<TLight> control_lite307_tag = {"lite307", nullptr};
component_tag<TLight> control_lite308_tag = {"lite308", nullptr};
component_tag<TLight> control_lite309_tag = {"lite309", nullptr};
component_tag<TLight> control_lite310_tag = {"lite310", nullptr};
component_tag<TLight> control_lite311_tag = {"lite311", nullptr};
component_tag<TLight> control_lite312_tag = {"lite312", nullptr};
component_tag<TLight> control_lite313_tag = {"lite313", nullptr};
component_tag<TLight> control_lite314_tag = {"lite314", nullptr};
component_tag<TLight> control_lite315_tag = {"lite315", nullptr};
component_tag<TLight> control_lite316_tag = {"lite316", nullptr};
component_tag<TLight> control_lite317_tag = {"lite317", nullptr};
component_tag<TLight> control_lite318_tag = {"lite318", nullptr};
component_tag<TLight> control_lite319_tag = {"lite319", nullptr};
component_tag<TLight> control_lite320_tag = {"lite320", nullptr};
component_tag<TLight> control_lite321_tag = {"lite321", nullptr};
component_tag<TLight> control_lite322_tag = {"lite322", nullptr};
component_tag<TLight> control_literoll179_tag = {"literoll179", nullptr};
component_tag<TLight> control_literoll180_tag = {"literoll180", nullptr};
component_tag<TLight> control_literoll181_tag = {"literoll181", nullptr};
component_tag<TLight> control_literoll182_tag = {"literoll182", nullptr};
component_tag<TLight> control_literoll183_tag = {"literoll183", nullptr};
component_tag<TLight> control_literoll184_tag = {"literoll184", nullptr};
component_tag<TLightGroup> control_middle_circle_tag = {"middle_circle", nullptr};
component_tag<TLightGroup> control_lchute_tgt_lights_tag = {"lchute_tgt_lights", nullptr};
component_tag<TLightGroup> control_l_trek_lights_tag = {"l_trek_lights", nullptr};
component_tag<TLightGroup> control_goal_lights_tag = {"goal_lights", nullptr};
component_tag<TLightGroup> control_hyper_lights_tag = {"hyperspace_lights", nullptr};
component_tag<TLightGroup> control_bmpr_inc_lights_tag = {"bmpr_inc_lights", nullptr};
component_tag<TLightGroup> control_bpr_solotgt_lights_tag = {"bpr_solotgt_lights", nullptr};
component_tag<TLightGroup> control_bsink_arrow_lights_tag = {"bsink_arrow_lights", nullptr};
component_tag<TLightGroup> control_bumber_target_lights_tag = {"bumper_target_lights", nullptr};
component_tag<TLightGroup> control_outer_circle_tag = {"outer_circle", nullptr};
component_tag<TLightGroup> control_r_trek_lights_tag = {"r_trek_lights", nullptr};
component_tag<TLightGroup> control_ramp_bmpr_inc_lights_tag = {"ramp_bmpr_inc_lights", nullptr};
component_tag<TLightGroup> control_ramp_tgt_lights_tag = {"ramp_tgt_lights", nullptr};
component_tag<TLightGroup> control_skill_shot_lights_tag = {"skill_shot_lights", nullptr};
component_tag<TLightGroup> control_top_circle_tgt_lights_tag = {"top_circle_tgt_lights", nullptr};
component_tag<TLightGroup> control_top_target_lights_tag = {"top_target_lights", nullptr};
component_tag<TLightGroup> control_worm_hole_lights_tag = {"worm_hole_lights", nullptr};
component_tag<TTextBox> control_mission_text_box_tag = {"mission_text_box", nullptr};
component_tag<TOneway> control_oneway1_tag = {"s_onewy1", nullptr};
component_tag<TOneway> control_oneway4_tag = {"s_onewy4", nullptr};
component_tag<TOneway> control_oneway10_tag = {"s_onewy10", nullptr};
component_tag<TPlunger> control_plunger_tag = {"plunger", nullptr};
component_tag<THole> control_ramp_hole_tag = {"ramp_hole", nullptr};
component_tag<TRamp> control_ramp_tag = {"ramp", nullptr};
component_tag<TWall> control_rebo1_tag = {"v_rebo1", nullptr};
component_tag<TWall> control_rebo2_tag = {"v_rebo2", nullptr};
component_tag<TWall> control_rebo3_tag = {"v_rebo3", nullptr};
component_tag<TWall> control_rebo4_tag = {"v_rebo4", nullptr};
component_tag<TRollover> control_roll1_tag = {"a_roll1", nullptr};
component_tag<TRollover> control_roll2_tag = {"a_roll2", nullptr};
component_tag<TRollover> control_roll3_tag = {"a_roll3", nullptr};
component_tag<TRollover> control_roll4_tag = {"a_roll4", nullptr};
component_tag<TRollover> control_roll5_tag = {"a_roll5", nullptr};
component_tag<TRollover> control_roll6_tag = {"a_roll6", nullptr};
component_tag<TRollover> control_roll7_tag = {"a_roll7", nullptr};
component_tag<TRollover> control_roll8_tag = {"a_roll8", nullptr};
component_tag<TLightRollover> control_roll9_tag = {"a_roll9", nullptr};
component_tag<TRollover> control_roll110_tag = {"a_roll110", nullptr};
component_tag<TRollover> control_roll111_tag = {"a_roll111", nullptr};
component_tag<TRollover> control_roll112_tag = {"a_roll112", nullptr};
component_tag<TRollover> control_roll179_tag = {"a_roll179", nullptr};
component_tag<TRollover> control_roll180_tag = {"a_roll180", nullptr};
component_tag<TRollover> control_roll181_tag = {"a_roll181", nullptr};
component_tag<TRollover> control_roll182_tag = {"a_roll182", nullptr};
component_tag<TRollover> control_roll183_tag = {"a_roll183", nullptr};
component_tag<TRollover> control_roll184_tag = {"a_roll184", nullptr};
component_tag<TSink> control_sink1_tag = {"v_sink1", nullptr};
component_tag<TSink> control_sink2_tag = {"v_sink2", nullptr};
component_tag<TSink> control_sink3_tag = {"v_sink3", nullptr};
component_tag<TSink> control_sink7_tag = {"v_sink7", nullptr};
component_tag<TSound> control_soundwave3_tag = {"soundwave3", nullptr};
component_tag<TSound> control_soundwave7_tag = {"soundwave7", nullptr};
component_tag<TSound> control_soundwave8_tag = {"soundwave8", nullptr};
component_tag<TSound> control_soundwave9_tag = {"soundwave9", nullptr};
component_tag<TSound> control_soundwave10_tag = {"soundwave10", nullptr};
component_tag<TSound> control_soundwave14_1_tag = {"soundwave14", nullptr};
component_tag<TSound> control_soundwave14_2_tag = {"soundwave14", nullptr};
component_tag<TSound> control_soundwave21_tag = {"soundwave21", nullptr};
component_tag<TSound> control_soundwave23_tag = {"soundwave23", nullptr};
component_tag<TSound> control_soundwave24_tag = {"soundwave24", nullptr};
component_tag<TSound> control_soundwave25_tag = {"soundwave25", nullptr};
component_tag<TSound> control_soundwave26_tag = {"soundwave26", nullptr};
component_tag<TSound> control_soundwave27_tag = {"soundwave27", nullptr};
component_tag<TSound> control_soundwave28_tag = {"soundwave28", nullptr};
component_tag<TSound> control_soundwave30_tag = {"soundwave30", nullptr};
component_tag<TSound> control_soundwave35_1_tag = {"soundwave35", nullptr};
component_tag<TSound> control_soundwave35_2_tag = {"soundwave35", nullptr};
component_tag<TSound> control_soundwave36_1_tag = {"soundwave36", nullptr};
component_tag<TSound> control_soundwave36_2_tag = {"soundwave36", nullptr};
component_tag<TSound> control_soundwave38_tag = {"soundwave38", nullptr};
component_tag<TSound> control_soundwave39_tag = {"soundwave39", nullptr};
component_tag<TSound> control_soundwave40_tag = {"soundwave40", nullptr};
component_tag<TSound> control_soundwave41_tag = {"soundwave41", nullptr};
component_tag<TSound> control_soundwave44_tag = {"soundwave44", nullptr};
component_tag<TSound> control_soundwave45_tag = {"soundwave45", nullptr};
component_tag<TSound> control_soundwave46_tag = {"soundwave46", nullptr};
component_tag<TSound> control_soundwave47_tag = {"soundwave47", nullptr};
component_tag<TSound> control_soundwave48_tag = {"soundwave48", nullptr};
component_tag<TSound> control_soundwave49D_tag = {"soundwave49D", nullptr};
component_tag<TSound> control_soundwave50_1_tag = {"soundwave50", nullptr};
component_tag<TSound> control_soundwave50_2_tag = {"soundwave50", nullptr};
component_tag<TSound> control_soundwave52_tag = {"soundwave52", nullptr};
component_tag<TSound> control_soundwave59_tag = {"soundwave59", nullptr};
component_tag<TPopupTarget> control_target1_tag = {"a_targ1", nullptr};
component_tag<TPopupTarget> control_target2_tag = {"a_targ2", nullptr};
component_tag<TPopupTarget> control_target3_tag = {"a_targ3", nullptr};
component_tag<TPopupTarget> control_target4_tag = {"a_targ4", nullptr};
component_tag<TPopupTarget> control_target5_tag = {"a_targ5", nullptr};
component_tag<TPopupTarget> control_target6_tag = {"a_targ6", nullptr};
component_tag<TPopupTarget> control_target7_tag = {"a_targ7", nullptr};
component_tag<TPopupTarget> control_target8_tag = {"a_targ8", nullptr};
component_tag<TPopupTarget> control_target9_tag = {"a_targ9", nullptr};
component_tag<TSoloTarget> control_target10_tag = {"a_targ10", nullptr};
component_tag<TSoloTarget> control_target11_tag = {"a_targ11", nullptr};
component_tag<TSoloTarget> control_target12_tag = {"a_targ12", nullptr};
component_tag<TSoloTarget> control_target13_tag = {"a_targ13", nullptr};
component_tag<TSoloTarget> control_target14_tag = {"a_targ14", nullptr};
component_tag<TSoloTarget> control_target15_tag = {"a_targ15", nullptr};
component_tag<TSoloTarget> control_target16_tag = {"a_targ16", nullptr};
component_tag<TSoloTarget> control_target17_tag = {"a_targ17", nullptr};
component_tag<TSoloTarget> control_target18_tag = {"a_targ18", nullptr};
component_tag<TSoloTarget> control_target19_tag = {"a_targ19", nullptr};
component_tag<TSoloTarget> control_target20_tag = {"a_targ20", nullptr};
component_tag<TSoloTarget> control_target21_tag = {"a_targ21", nullptr};
component_tag<TSoloTarget> control_target22_tag = {"a_targ22", nullptr};
component_tag<TTripwire> control_trip1_tag = {"s_trip1", nullptr};
component_tag<TTripwire> control_trip2_tag = {"s_trip2", nullptr};
component_tag<TTripwire> control_trip3_tag = {"s_trip3", nullptr};
component_tag<TTripwire> control_trip4_tag = {"s_trip4", nullptr};
component_tag<TTripwire> control_trip5_tag = {"s_trip5", nullptr};


TPinballTable* control::TableG;
component_info control::score_components[88]
{
	component_info{&control_bump1_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_bump2_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_bump3_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_bump4_tag, {BumperControl, 4, control_bump_scores1}},
	component_info{&control_roll3_tag, {ReentryLanesRolloverControl, 1, control_roll_scores1}},
	component_info{&control_roll2_tag, {ReentryLanesRolloverControl, 1, control_roll_scores1}},
	component_info{&control_roll1_tag, {ReentryLanesRolloverControl, 1, control_roll_scores1}},
	component_info{&control_attack_bump_tag, {BumperGroupControl, 0, nullptr}},
	component_info{&control_bump5_tag, {BumperControl, 4, control_bump_scores2}},
	component_info{&control_bump6_tag, {BumperControl, 4, control_bump_scores2}},
	component_info{&control_bump7_tag, {BumperControl, 4, control_bump_scores2}},
	component_info{&control_roll112_tag, {LaunchLanesRolloverControl, 1, control_roll_scores2}},
	component_info{&control_roll111_tag, {LaunchLanesRolloverControl, 1, control_roll_scores2}},
	component_info{&control_roll110_tag, {LaunchLanesRolloverControl, 1, control_roll_scores2}},
	component_info{&control_launch_bump_tag, {BumperGroupControl, 0, nullptr}},
	component_info{&control_rebo1_tag, {FlipperRebounderControl1, 1, control_rebo_score1}},
	component_info{&control_rebo2_tag, {FlipperRebounderControl2, 1, control_rebo_score1}},
	component_info{&control_rebo3_tag, {RebounderControl, 1, control_rebo_score1}},
	component_info{&control_rebo4_tag, {RebounderControl, 1, control_rebo_score1}},
	component_info{&control_kicker1_tag, {LeftKickerControl, 0, nullptr}},
	component_info{&control_kicker2_tag, {RightKickerControl, 0, nullptr}},
	component_info{&control_gate1_tag, {LeftKickerGateControl, 0, nullptr}},
	component_info{&control_gate2_tag, {RightKickerGateControl, 0, nullptr}},
	component_info{&control_oneway4_tag, {DeploymentChuteToEscapeChuteOneWayControl, 6, control_oneway4_score1}},
	component_info{&control_oneway10_tag, {DeploymentChuteToTableOneWayControl, 0, nullptr}},
	component_info{&control_block1_tag, {DrainBallBlockerControl, 0, nullptr}},
	component_info{&control_ramp_tag, {LaunchRampControl, 1, control_ramp_score1}},
	component_info{&control_ramp_hole_tag, {LaunchRampHoleControl, 0, nullptr}},
	component_info{&control_roll4_tag, {OutLaneRolloverControl, 1, control_roll_score1}},
	component_info{&control_roll8_tag, {OutLaneRolloverControl, 1, control_roll_score1}},
	component_info{&control_lite17_tag, {ExtraBallLightControl, 0, nullptr}},
	component_info{&control_roll6_tag, {ReturnLaneRolloverControl, 2, control_roll_score2}},
	component_info{&control_roll7_tag, {ReturnLaneRolloverControl, 2, control_roll_score2}},
	component_info{&control_roll5_tag, {BonusLaneRolloverControl, 1, control_roll_score3}},
	component_info{&control_roll179_tag, {FuelRollover1Control, 1, control_roll_score4}},
	component_info{&control_roll180_tag, {FuelRollover2Control, 1, control_roll_score4}},
	component_info{&control_roll181_tag, {FuelRollover3Control, 1, control_roll_score4}},
	component_info{&control_roll182_tag, {FuelRollover4Control, 1, control_roll_score4}},
	component_info{&control_roll183_tag, {FuelRollover5Control, 1, control_roll_score4}},
	component_info{&control_roll184_tag, {FuelRollover6Control, 1, control_roll_score4}},
	component_info{&control_flag1_tag, {FlagControl, 2, control_flag_score1}},
	component_info{&control_kickout2_tag, {HyperspaceKickOutControl, 5, control_kickout_score1}},
	component_info{&control_hyper_lights_tag, {HyperspaceLightGroupControl, 0, nullptr}},
	component_info{&control_flag2_tag, {FlagControl, 2, control_flag_score1}},
	component_info{&control_sink1_tag, {WormHoleControl, 3, control_sink_score1}},
	component_info{&control_sink2_tag, {WormHoleControl, 3, control_sink_score1}},
	component_info{&control_sink3_tag, {WormHoleControl, 3, control_sink_score1}},
	component_info{&control_flip1_tag, {LeftFlipperControl, 0, nullptr}},
	component_info{&control_flip2_tag, {RightFlipperControl, 0, nullptr}},
	component_info{&control_plunger_tag, {PlungerControl, 0, nullptr}},
	component_info{&control_target1_tag, {BoosterTargetControl, 2, control_target_score1}},
	component_info{&control_target2_tag, {BoosterTargetControl, 2, control_target_score1}},
	component_info{&control_target3_tag, {BoosterTargetControl, 2, control_target_score1}},
	component_info{&control_lite60_tag, {JackpotLightControl, 0, nullptr}},
	component_info{&control_lite59_tag, {BonusLightControl, 0, nullptr}},
	component_info{&control_target6_tag, {MedalTargetControl, 3, control_target_score2}},
	component_info{&control_target5_tag, {MedalTargetControl, 3, control_target_score2}},
	component_info{&control_target4_tag, {MedalTargetControl, 3, control_target_score2}},
	component_info{&control_bumber_target_lights_tag, {MedalLightGroupControl, 0, nullptr}},
	component_info{&control_target9_tag, {MultiplierTargetControl, 2, control_target_score3}},
	component_info{&control_target8_tag, {MultiplierTargetControl, 2, control_target_score3}},
	component_info{&control_target7_tag, {MultiplierTargetControl, 2, control_target_score3}},
	component_info{&control_top_target_lights_tag, {MultiplierLightGroupControl, 0, nullptr}},
	component_info{&control_target10_tag, {FuelSpotTargetControl, 1, control_target_score4}},
	component_info{&control_target11_tag, {FuelSpotTargetControl, 1, control_target_score4}},
	component_info{&control_target12_tag, {FuelSpotTargetControl, 1, control_target_score4}},
	component_info{&control_target13_tag, {MissionSpotTargetControl, 1, control_target_score5}},
	component_info{&control_target14_tag, {MissionSpotTargetControl, 1, control_target_score5}},
	component_info{&control_target15_tag, {MissionSpotTargetControl, 1, control_target_score5}},
	component_info{&control_target16_tag, {LeftHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target17_tag, {LeftHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target18_tag, {LeftHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target19_tag, {RightHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target20_tag, {RightHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target21_tag, {RightHazardSpotTargetControl, 1, control_target_score6}},
	component_info{&control_target22_tag, {WormHoleDestinationControl, 1, control_target_score7}},
	component_info{&control_roll9_tag, {SpaceWarpRolloverControl, 1, control_roll_score5}},
	component_info{&control_kickout3_tag, {BlackHoleKickoutControl, 1, control_kickout_score2}},
	component_info{&control_kickout1_tag, {GravityWellKickoutControl, 1, control_kickout_score3}},
	component_info{&control_drain_tag, {BallDrainControl, 0, nullptr}},
	component_info{&control_oneway1_tag, {SkillShotGate1Control, 0, nullptr}},
	component_info{&control_trip1_tag, {SkillShotGate2Control, 0, nullptr}},
	component_info{&control_trip2_tag, {SkillShotGate3Control, 0, nullptr}},
	component_info{&control_trip3_tag, {SkillShotGate4Control, 0, nullptr}},
	component_info{&control_trip4_tag, {SkillShotGate5Control, 0, nullptr}},
	component_info{&control_trip5_tag, {SkillShotGate6Control, 0, nullptr}},
	component_info{&control_lite200_tag, {ShootAgainLightControl, 0, nullptr}},
	component_info{&control_sink7_tag, {EscapeChuteSinkControl, 0, nullptr}},
};


component_tag_base* control::simple_components[142]
{
	&control_lite8_tag,
	&control_lite9_tag,
	&control_lite10_tag,
	&control_bmpr_inc_lights_tag,
	&control_lite171_tag,
	&control_lite170_tag,
	&control_lite169_tag,
	&control_ramp_bmpr_inc_lights_tag,
	&control_lite30_tag,
	&control_lite29_tag,
	&control_lite1_tag,
	&control_lite54_tag,
	&control_lite55_tag,
	&control_lite56_tag,
	&control_lite18_tag,
	&control_lite27_tag,
	&control_lite28_tag,
	&control_lite16_tag,
	&control_lite21_tag,
	&control_lite22_tag,
	&control_lite23_tag,
	&control_lite24_tag,
	&control_lite25_tag,
	&control_lite26_tag,
	&control_lite130_tag,
	&control_lite5_tag,
	&control_lite6_tag,
	&control_lite7_tag,
	&control_worm_hole_lights_tag,
	&control_lite4_tag,
	&control_lite2_tag,
	&control_lite3_tag,
	&control_bsink_arrow_lights_tag,
	&control_l_trek_lights_tag,
	&control_r_trek_lights_tag,
	&control_literoll179_tag,
	&control_literoll180_tag,
	&control_literoll181_tag,
	&control_literoll182_tag,
	&control_literoll183_tag,
	&control_literoll184_tag,
	&control_fuel_bargraph_tag,
	&control_lite20_tag,
	&control_lite19_tag,
	&control_lite61_tag,
	&control_lite58_tag,
	&control_lite11_tag,
	&control_lite12_tag,
	&control_lite13_tag,
	&control_lite70_tag,
	&control_lite71_tag,
	&control_lite72_tag,
	&control_top_circle_tgt_lights_tag,
	&control_lite101_tag,
	&control_lite102_tag,
	&control_lite103_tag,
	&control_ramp_tgt_lights_tag,
	&control_lite104_tag,
	&control_lite105_tag,
	&control_lite106_tag,
	&control_lite107_tag,
	&control_lite108_tag,
	&control_lite109_tag,
	&control_lchute_tgt_lights_tag,
	&control_bpr_solotgt_lights_tag,
	&control_lite110_tag,
	&control_lite62_tag,
	&control_lite67_tag,
	&control_lite68_tag,
	&control_lite69_tag,
	&control_lite131_tag,
	&control_lite132_tag,
	&control_lite133_tag,
	&control_skill_shot_lights_tag,
	&control_lite77_tag,
	&control_lite198_tag,
	&control_middle_circle_tag,
	&control_outer_circle_tag,
	&control_soundwave9_tag,
	&control_soundwave10_tag,
	&control_soundwave21_tag,
	&control_soundwave23_tag,
	&control_soundwave24_tag,
	&control_soundwave30_tag,
	&control_soundwave28_tag,
	&control_soundwave50_1_tag,
	&control_soundwave8_tag,
	&control_soundwave40_tag,
	&control_soundwave41_tag,
	&control_soundwave36_1_tag,
	&control_soundwave50_2_tag,
	&control_soundwave35_1_tag,
	&control_soundwave36_2_tag,
	&control_soundwave35_2_tag,
	&control_soundwave38_tag,
	&control_soundwave39_tag,
	&control_soundwave44_tag,
	&control_soundwave45_tag,
	&control_soundwave46_tag,
	&control_soundwave47_tag,
	&control_soundwave48_tag,
	&control_soundwave52_tag,
	&control_soundwave14_1_tag,
	&control_soundwave59_tag,
	&control_lite199_tag,
	&control_lite196_tag,
	&control_lite195_tag,
	&control_info_text_box_tag,
	&control_mission_text_box_tag,
	&control_soundwave27_tag,
	&control_lite84_tag,
	&control_lite85_tag,
	&control_soundwave14_2_tag,
	&control_soundwave3_tag,
	&control_soundwave26_tag,
	&control_soundwave49D_tag,
	&control_lite300_tag,
	&control_lite301_tag,
	&control_lite302_tag,
	&control_lite303_tag,
	&control_lite304_tag,
	&control_lite305_tag,
	&control_lite306_tag,
	&control_lite307_tag,
	&control_lite308_tag,
	&control_lite309_tag,
	&control_lite310_tag,
	&control_lite311_tag,
	&control_lite312_tag,
	&control_lite313_tag,
	&control_lite314_tag,
	&control_lite315_tag,
	&control_lite316_tag,
	&control_lite317_tag,
	&control_lite318_tag,
	&control_lite319_tag,
	&control_lite320_tag,
	&control_lite321_tag,
	&control_lite322_tag,
	&control_goal_lights_tag,
	&control_soundwave25_tag,
	&control_soundwave7_tag
};

int control::waiting_deployment_flag;
bool control::table_unlimited_balls = false;
int control::extraball_light_flag;
int control::RankRcArray[9] = {84, 85, 86, 87, 88, 89, 90, 91, 92};
int control::MissionRcArray[17] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};
int control::mission_select_scores[17] =
{
	10000,
	10000,
	10000,
	10000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	20000,
	30000,
	30000,
	30000,
	30000
};
component_tag_base* control::wormhole_tag_array1[3] =
{
	&control_sink1_tag, &control_sink2_tag, &control_sink3_tag
};

component_tag_base* control::wormhole_tag_array2[3] =
{
	&control_lite5_tag, &control_lite6_tag, &control_lite7_tag
};

component_tag_base* control::wormhole_tag_array3[3] =
{
	&control_lite4_tag, &control_lite2_tag, &control_lite3_tag
};


void control::make_links(TPinballTable* table)
{
	TableG = table;
	
	// Get peak nits for all light configurations
	float peakNits = HDR::GetMaxDisplayNits();

	for (int index = 0; index < 88; index++)
	{
		auto compPtr = &score_components[index];
		TPinballComponent* comp = make_component_link(compPtr->Tag);
		if (comp)
		{
			comp->Control = &compPtr->Control;
			for (int scoreId = 0; scoreId < compPtr->Control.ScoreCount; scoreId++)
			{
				comp->put_scoring(scoreId, compPtr->Control.Scores[scoreId]);
			}
		}
	}

	for (int i = 0; i < 142; ++i)
		make_component_link(simple_components[i]);
	
	// Register HDR light overlays if HDR is active
	if (HDR::IsHDRActive())
	{
		// Register skill shot lights group
		if (control_skill_shot_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("skill_shot_lights", control_skill_shot_lights_tag.Component);
			
			// Add light configurations for each light in the skill shot group
			// These positions are approximate - will need tuning based on actual game coordinates
			// The skill shot lights are on the right side of the plunger ramp
			// Positions are normalized 0-1 relative to game texture
			
			// Skill shot light 1 (bottom)
			HDRLightOverlay::AddLightConfig({
				"skill_shot_lights", 0,
				0.518366f, 0.575480f,      // X, Y position (normalized)
				0.04f, 0.04f,      // Width, Height
				1.0f, 0.7f, 0.1f,  // R, G, B (yellow-orange)
				600.0f,            // Intensity when on (nits)
				1000.0f,           // Intensity when flashing (nits)
				1.0f,              // Glow radius
				true,              // AboveBall (not occluded)
				true               // Locked
			});
			
			// Skill shot light 2
			HDRLightOverlay::AddLightConfig({
				"skill_shot_lights", 1,
				0.504916f, 0.453200f,
				0.04f, 0.04f,
				1.0f, 0.7f, 0.1f,  // Yellow-orange
				peakNits, peakNits, 1.0f, true, true
			});
			
			// Skill shot light 3
			HDRLightOverlay::AddLightConfig({
				"skill_shot_lights", 2,
				0.493206f, 0.338865f,
				0.04f, 0.04f,
				1.0f, 0.7f, 0.1f,  // Yellow-orange
				peakNits, peakNits, 1.0f, true, true
			});
			
			// Skill shot light 4
			HDRLightOverlay::AddLightConfig({
				"skill_shot_lights", 3,
				0.484177f, 0.238625f,
				0.04f, 0.04f,
				1.0f, 0.7f, 0.1f,  // Yellow-orange
				peakNits, peakNits, 1.0f, true, true
			});
			
			// Skill shot light 5
			HDRLightOverlay::AddLightConfig({
				"skill_shot_lights", 4,
				0.460645f, 0.164781f,
				0.04f, 0.04f,
				1.0f, 0.7f, 0.1f,  // Yellow-orange
				peakNits, peakNits, 1.0f, false, true
			});
			
			// Skill shot light 6 (top)
			HDRLightOverlay::AddLightConfig({
				"skill_shot_lights", 5,
				0.418464f, 0.108107f,
				0.04f, 0.04f,
				1.0f, 0.7f, 0.1f,  // Yellow-orange
				700.0f, 1200.0f, 1.0f, true, true
			});
		}
		
		// Register middle circle (inner ring of score lights - orange)
		if (control_middle_circle_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("middle_circle", control_middle_circle_tag.Component);
			
			// The middle circle has 9 lights arranged in a ring
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 0,
				0.294936f, 0.578540f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 1,
				0.328999f, 0.584519f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 2,
				0.350836f, 0.625003f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 3,
				0.353777f, 0.663306f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 4,
				0.331721f, 0.704472f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 5,
				0.294855f, 0.711025f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 6,
				0.260959f, 0.682270f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 7,
				0.255760f, 0.635767f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"middle_circle", 8,
				0.266496f, 0.597461f,
				0.04f, 0.065574f,
				1.0f, 0.3f, 0.0f,  // Orange
				peakNits, peakNits, 1.0f, false, true
			});
		}
		
		// Register outer circle (outer ring of lights - blue)
		if (control_outer_circle_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("outer_circle", control_outer_circle_tag.Component);
			
			// The outer circle has 18 lights arranged in a larger ring around the middle circle
			float peakNits = HDR::GetMaxDisplayNits();
			
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 0,
				0.302717f, 0.546538f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 1,
				0.329714f, 0.552617f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 2,
				0.351085f, 0.569523f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 3,
				0.367402f, 0.591606f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 4,
				0.378241f, 0.621817f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 5,
				0.380017f, 0.657040f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 6,
				0.372434f, 0.690967f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 7,
				0.357523f, 0.723301f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 8,
				0.332970f, 0.743283f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 9,
				0.304408f, 0.750900f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 10,
				0.275846f, 0.739695f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 11,
				0.250932f, 0.722020f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 12,
				0.235790f, 0.690199f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 13,
				0.228799f, 0.655888f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 14,
				0.231315f, 0.619511f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 15,
				0.242219f, 0.591606f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 16,
				0.256400f, 0.565423f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"outer_circle", 17,
				0.278954f, 0.551592f,
				0.04f, 0.065574f,
				0.02f, 0.02f, 1.0f,  // Blue
				peakNits, peakNits, 1.0f, false, true
			});
		}
		
		// Register re-entry lane rollover lights (small yellow lights at top center)
		// lite8, lite9, lite10 - three lights in a row
		{
			float peakNits = HDR::GetMaxDisplayNits();
			float lightW = 0.03f;
			float lightH = 0.03f / 0.61f;
			
			if (control_lite8_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("lite8", control_lite8_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"lite8", 0,
					0.332482f, 0.125415f,      // Left rollover light
					lightW, lightH,
					1.0f, 0.8f, 0.0f,  // Yellow
					peakNits, peakNits, 1.0f, false, true
				});
			}
			
			if (control_lite9_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("lite9", control_lite9_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"lite9", 0,
					0.304704f, 0.121314f,     // Center rollover light
					lightW, lightH,
					1.0f, 0.8f, 0.0f,  // Yellow
					peakNits, peakNits, 1.0f, false, true
				});
			}
			
			if (control_lite10_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("lite10", control_lite10_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"lite10", 0,
					0.276408f, 0.127208f,      // Right rollover light
					lightW, lightH,
					1.0f, 0.8f, 0.0f,  // Yellow
					peakNits, peakNits, 1.0f, false, true
				});
			}
		}
		
		// Register fuel ramp lights
		// literoll179-184 - six lights along the fuel ramp
		// From top to bottom: Blue, (not visible), Purple, Red, Orange, Yellow
		{
			float peakNits = HDR::GetMaxDisplayNits();
			float lightW = 0.03f;
			float lightH = 0.03f / 0.61f;
			
			// literoll184 (top) - Blue
			if (control_literoll184_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("literoll184", control_literoll184_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"literoll184", 0,
					0.128693f, 0.314390f,
					lightW, lightH,
					0.0f, 0.0f, 1.0f,  // Blue
					peakNits, peakNits, 1.0f, false, true
				});
			}
			
			// literoll183 - Not visible (skip)
			
			// literoll182 - Purple
			if (control_literoll182_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("literoll182", control_literoll182_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"literoll182", 0,
					0.142458f, 0.379814f,
					lightW, lightH,
					0.8f, 0.0f, 1.0f,  // Purple
					peakNits, peakNits, 1.0f, false, true
				});
			}
			
			// literoll181 - Red
			if (control_literoll181_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("literoll181", control_literoll181_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"literoll181", 0,
					0.156074f, 0.410092f,
					lightW, lightH,
					1.0f, 0.0f, 0.0f,  // Red
					peakNits, peakNits, 1.0f, false, true
				});
			}
			
			// literoll180 - Orange
			if (control_literoll180_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("literoll180", control_literoll180_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"literoll180", 0,
					0.170874f, 0.438833f,
					lightW, lightH,
					1.0f, 0.3f, 0.0f,  // Orange
					peakNits, peakNits, 1.0f, false, true
				});
			}
			
			// literoll179 (bottom) - Yellow
			if (control_literoll179_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("literoll179", control_literoll179_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"literoll179", 0,
					0.187303f, 0.471161f,
					lightW, lightH,
					1.0f, 0.7f, 0.1f,  // Yellow
					peakNits, peakNits, 1.0f, false, true
				});
			}
		}
		
		// Register left trek lights (purple mission ramp on left side)
		// l_trek_lights - three lights in the ramp, bottom one hidden
		if (control_l_trek_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("l_trek_lights", control_l_trek_lights_tag.Component);
			float peakNits = HDR::GetMaxDisplayNits();
			float lightW = 0.03f;
			float lightH = 0.03f / 0.61f;
			
			// Two lights arranged vertically in the left ramp (bottom one skipped)
			HDRLightOverlay::AddLightConfig({
				"l_trek_lights", 0,
				0.116448f, 0.185458f,  // Bottom light
				lightW, lightH,
				1.0f, 0.7f, 0.1f,  // Bright white-yellow
				peakNits, peakNits, 1.0f, false, true
			});
			HDRLightOverlay::AddLightConfig({
				"l_trek_lights", 1,
				0.139240f, 0.135261f,  // Top light
				lightW, lightH,
				1.0f, 0.7f, 0.1f,  // Bright white-yellow
				peakNits, peakNits, 1.0f, false, true
			});
		}
		
		// Register launch lane rollover lights (three lights above bumpers in left mission ramp)
		// lite169, lite170, lite171 - very small yellow lights
		{
			float peakNits = HDR::GetMaxDisplayNits();
			float lightW = 0.02f;  // Even smaller
			float lightH = 0.02f / 0.61f;
			
			// lite169 (left rollover)
			if (control_lite169_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("lite169", control_lite169_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"lite169", 0,
					0.094898f, 0.445022f,      // Left position above bumpers
					lightW, lightH,
					0.5f, 0.4f, 0.0f,  // Yellow
					peakNits, peakNits, 0.5f, false, true  // Much smaller glow
				});
			}
			
			// lite170 (middle rollover)
			if (control_lite170_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("lite170", control_lite170_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"lite170", 0,
					0.120413f, 0.452198f,      // Middle position above bumpers
					lightW, lightH,
					1.0f, 0.8f, 0.0f,  // Yellow
					peakNits, peakNits, 0.5f, false, true  // Much smaller glow
				});
			}
			
			// lite171 (right rollover)
			if (control_lite171_tag.Component)
			{
				HDRLightOverlay::RegisterIndividualLight("lite171", control_lite171_tag.Component);
				HDRLightOverlay::AddLightConfig({
					"lite171", 0,
					0.148000f, 0.461680f,      // Right position above bumpers
					lightW, lightH,
					1.0f, 0.8f, 0.0f,  // Yellow
					peakNits, peakNits, 0.5f, false, true  // Much smaller glow
				});
			}
		}
		
		// Register exit ramp lights (red lights below paddles)
		// lite199 = Replay indicator (left side)
		// lite200 = Shoot Again / grace period light (right side)
		if (control_lite199_tag.Component)
		{
			HDRLightOverlay::RegisterIndividualLight("lite199", control_lite199_tag.Component);
			float peakNits = HDR::GetMaxDisplayNits();
			HDRLightOverlay::AddLightConfig({
				"lite199", 0,
				0.192027f, 0.922579f,      // X, Y position (left exit ramp, below left paddle)
				0.04f, 0.065574f,
				1.0f, 0.0f, 0.0f,  // Pure red
				peakNits, peakNits, 0.5f, false, true  // Reduced glow
			});
		}
		
		if (control_lite200_tag.Component)
		{
			HDRLightOverlay::RegisterIndividualLight("lite200", control_lite200_tag.Component);
			float peakNits = HDR::GetMaxDisplayNits();
			HDRLightOverlay::AddLightConfig({
				"lite200", 0,
				0.414273f, 0.925398f,      // X, Y position (right exit ramp, below right paddle)
				0.04f, 0.065574f,
				1.0f, 0.0f, 0.0f,  // Pure red
				peakNits, peakNits, 0.5f, false, true  // Reduced glow
			});
		}
		
		// Register ALL remaining individual lights for HDR overlay
		// These are mission lights, status lights, and other game indicators
		// All unlocked lights are placed in a single column on the left side, below the debug menu
		{
			float peakNits = HDR::GetMaxDisplayNits();
			float defaultW = 0.02f;
			float defaultH = 0.02f;
			float startY = 0.35f;  // Start below debug menu
			float spacing = 0.015f;  // Tight spacing for single column
			int row = 0;
			
			// lite1-7: Wormhole and sink related lights
			if (control_lite1_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite1", control_lite1_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite1", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite2_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite2", control_lite2_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite2", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite3_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite3", control_lite3_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite3", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite4_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite4", control_lite4_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite4", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite5_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite5", control_lite5_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite5", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite6_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite6", control_lite6_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite6", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite7_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite7", control_lite7_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite7", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// lite11-13: Additional lights
			if (control_lite11_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite11", control_lite11_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite11", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite12_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite12", control_lite12_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite12", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite13_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite13", control_lite13_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite13", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// lite16-30: Various game lights
			if (control_lite16_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite16", control_lite16_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite16", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite17_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite17", control_lite17_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite17", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite18_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite18", control_lite18_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite18", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite19_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite19", control_lite19_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite19", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite20_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite20", control_lite20_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite20", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite21_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite21", control_lite21_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite21", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite22_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite22", control_lite22_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite22", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite23_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite23", control_lite23_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite23", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite24_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite24", control_lite24_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite24", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite25_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite25", control_lite25_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite25", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite26_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite26", control_lite26_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite26", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite27_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite27", control_lite27_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite27", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite28_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite28", control_lite28_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite28", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite29_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite29", control_lite29_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite29", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite30_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite30", control_lite30_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite30", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// lite54-72: More game lights (continue single column)
			if (control_lite54_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite54", control_lite54_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite54", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite55_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite55", control_lite55_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite55", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite56_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite56", control_lite56_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite56", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite58_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite58", control_lite58_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite58", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite59_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite59", control_lite59_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite59", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite60_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite60", control_lite60_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite60", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite61_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite61", control_lite61_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite61", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite62_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite62", control_lite62_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite62", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite67_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite67", control_lite67_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite67", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite68_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite68", control_lite68_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite68", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite69_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite69", control_lite69_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite69", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite70_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite70", control_lite70_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite70", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite71_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite71", control_lite71_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite71", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite72_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite72", control_lite72_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite72", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite77_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite77", control_lite77_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite77", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite84_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite84", control_lite84_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite84", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite85_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite85", control_lite85_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite85", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.5f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// lite101-133: Mission and status lights (continue single column)
			if (control_lite101_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite101", control_lite101_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite101", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite102_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite102", control_lite102_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite102", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite103_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite103", control_lite103_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite103", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite104_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite104", control_lite104_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite104", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite105_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite105", control_lite105_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite105", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite106_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite106", control_lite106_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite106", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite107_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite107", control_lite107_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite107", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite108_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite108", control_lite108_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite108", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite109_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite109", control_lite109_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite109", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite110_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite110", control_lite110_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite110", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.5f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite130_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite130", control_lite130_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite130", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite131_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite131", control_lite131_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite131", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite132_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite132", control_lite132_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite132", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite133_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite133", control_lite133_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite133", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 0.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// lite195-198: Status lights (continue single column)
			if (control_lite195_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite195", control_lite195_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite195", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite196_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite196", control_lite196_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite196", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite198_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite198", control_lite198_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite198", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 1.0f, 1.0f, 0.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// lite300-322: Mission indicator lights (continue single column)
			if (control_lite300_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite300", control_lite300_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite300", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite301_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite301", control_lite301_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite301", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite302_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite302", control_lite302_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite302", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite303_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite303", control_lite303_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite303", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite304_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite304", control_lite304_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite304", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite305_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite305", control_lite305_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite305", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite306_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite306", control_lite306_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite306", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite307_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite307", control_lite307_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite307", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite308_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite308", control_lite308_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite308", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite309_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite309", control_lite309_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite309", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite310_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite310", control_lite310_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite310", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite311_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite311", control_lite311_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite311", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite312_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite312", control_lite312_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite312", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite313_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite313", control_lite313_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite313", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite314_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite314", control_lite314_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite314", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite315_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite315", control_lite315_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite315", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite316_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite316", control_lite316_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite316", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite317_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite317", control_lite317_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite317", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite318_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite318", control_lite318_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite318", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite319_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite319", control_lite319_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite319", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite320_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite320", control_lite320_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite320", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite321_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite321", control_lite321_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite321", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			if (control_lite322_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("lite322", control_lite322_tag.Component);
				HDRLightOverlay::AddLightConfig({"lite322", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.0f, 1.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
			
			// literoll183 (not yet registered)
			if (control_literoll183_tag.Component) {
				HDRLightOverlay::RegisterIndividualLight("literoll183", control_literoll183_tag.Component);
				HDRLightOverlay::AddLightConfig({"literoll183", 0, 0.02f, startY + row++ * spacing, defaultW, defaultH, 0.5f, 0.0f, 1.0f, peakNits, peakNits, 1.0f, false, false});
			}
		}
		
		// Register bumpers with dynamic color based on upgrade level
		// Bumpers change color as they upgrade: blue -> green -> yellow -> red
		// Attack bumpers (bump1-4) are on the left side
		// Launch bumpers (bump5-7) are on the right side
		{
			float peakNits = HDR::GetMaxDisplayNits();
			float bumperW = 0.04f;   // Bigger
			float bumperH = 0.04f / 0.61f;
			
			// Color progression for bumpers (upgrade levels 0-3)
			// Level 0: Blue (same as outer circle), Level 1: Forest green, Level 2: Yellow, Level 3: Red
			float colors[4][3] = {
				{0.02f, 0.02f, 1.0f}, // Blue - same as outer circle (level 0)
				{0.0f, 0.5f, 0.0f},   // Forest green (level 1)
				{1.0f, 0.8f, 0.0f},   // Yellow (level 2)
				{1.0f, 0.0f, 0.0f}    // Red (level 3)
			};
			
			// Attack bumpers (bump1-4) - left side cluster
			if (control_bump1_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump1", control_bump1_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump1",
					0.303826f, 0.284623f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
			
			if (control_bump2_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump2", control_bump2_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump2",
					0.338015f, 0.200081f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
			
			if (control_bump3_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump3", control_bump3_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump3",
					0.272301f, 0.218432f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
			
			if (control_bump4_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump4", control_bump4_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump4",
					0.168699f, 0.088914f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
			
			// Launch bumpers (bump5-7) - right side cluster (above launch lanes)
			if (control_bump5_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump5", control_bump5_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump5",
					0.139542f, 0.532802f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
			
			if (control_bump6_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump6", control_bump6_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump6",
					0.083967f, 0.512456f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
			
			if (control_bump7_tag.Component)
			{
				HDRLightOverlay::RegisterBumper("bump7", control_bump7_tag.Component);
				HDRLightOverlay::AddBumperConfig({
					"bump7",
					0.100321f, 0.570529f,
					bumperW, bumperH,
					{{colors[0][0], colors[0][1], colors[0][2]},
					 {colors[1][0], colors[1][1], colors[1][2]},
					 {colors[2][0], colors[2][1], colors[2][2]},
					 {colors[3][0], colors[3][1], colors[3][2]}},
					peakNits, 1.0f, true
				});
			}
		}
		
		// Add initial configurations for new light groups (unlocked for positioning)
		
		// Left chute target lights - 3 lights in left chute
		if (control_lchute_tgt_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"lchute_tgt_lights", i,
					0.02f, 0.02f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 1.0f, 0.0f,  // Yellow
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Goal lights - 3 lights for goal targets
		if (control_goal_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"goal_lights", i,
					0.02f, 0.08f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					0.2f, 0.3f, 0.5f,  // Light blue
					300, 600, 2.5f, false, true  // Locked
				});
			}
		}
		
		// Hyperspace lights - 4 lights for hyperspace (game only has 4, not 5)
		if (control_hyper_lights_tag.Component)
		{
			for (int i = 0; i < 4; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"hyperspace_lights", i,
					0.02f, 0.14f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					0.02f, 0.02f, 1.0f,  // Blue (same as outer circle)
					peakNits, peakNits, 1.5f, true, true  // Locked
				});
			}
		}
		
		// Bumper increment lights - 3 lights for bumper progress (game only has 3, not 4)
		if (control_bmpr_inc_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"bmpr_inc_lights", i,
					0.02f, 0.24f + i * 0.02f,  // Left column positions
					0.025f, 0.025f,
					1.0f, 0.5f, 0.0f,  // Orange
					peakNits, peakNits, 0.8f, false, true  // Locked
				});
			}
		}
		
		// Solo target lights - 3 lights
		if (control_bpr_solotgt_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"bpr_solotgt_lights", i,
					0.02f, 0.32f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 1.0f, 0.0f,  // Yellow (like bumper targets)
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Ball sink arrow lights - 3 arrow lights
		if (control_bsink_arrow_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"bsink_arrow_lights", i,
					0.02f, 0.38f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 1.0f, 1.0f,  // White
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Bumper target lights - 3 lights (actual game only has 3, not 6)
		if (control_bumber_target_lights_tag.Component)
		{
			// Top light - Blue (like outer circle)
			HDRLightOverlay::AddLightConfig({
				"bumper_target_lights", 0,
				0.02f, 0.44f,  // Left column positions
				0.025f, 0.025f,
				0.02f, 0.02f, 1.0f,  // Blue (same as outer circle)
				peakNits, peakNits, 1.2f, false, true  // Locked
			});
			
			// Second light - Orange (like fuel ramp)
			HDRLightOverlay::AddLightConfig({
				"bumper_target_lights", 1,
				0.02f, 0.46f,  // Left column positions
				0.025f, 0.025f,
				1.0f, 0.3f, 0.0f,  // Orange (same as literoll180)
				peakNits, peakNits, 1.2f, false, true  // Locked
			});
			
			// Third light - Purple (like fuel ramp)
			HDRLightOverlay::AddLightConfig({
				"bumper_target_lights", 2,
				0.02f, 0.48f,  // Left column positions
				0.025f, 0.025f,
				0.8f, 0.0f, 1.0f,  // Purple (same as literoll182)
				peakNits, peakNits, 1.2f, false, true  // Locked
			});
		}
		
		// Right trek lights - 2 lights in right ramp
		if (control_r_trek_lights_tag.Component)
		{
			for (int i = 0; i < 2; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"r_trek_lights", i,
					0.02f, 0.56f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 0.7f, 0.1f,  // Yellow-orange
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Ramp bumper increment lights - 3 lights (game only has 3, not 4)
		if (control_ramp_bmpr_inc_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"ramp_bmpr_inc_lights", i,
					0.02f, 0.60f + i * 0.02f,  // Left column positions
					0.025f, 0.025f,
					1.0f, 1.0f, 0.0f,  // Yellow (like bumper targets)
					peakNits, peakNits, 0.8f, false, true  // Locked
				});
			}
		}
		
		// Ramp target lights - 3 lights
		if (control_ramp_tgt_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"ramp_tgt_lights", i,
					0.02f, 0.68f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 1.0f, 0.0f,  // Yellow (like bumper targets)
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Top circle target lights - 3 lights in square (game only has 3, not 4)
		if (control_top_circle_tgt_lights_tag.Component)
		{
			for (int i = 0; i < 3; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"top_circle_tgt_lights", i,
					0.02f, 0.74f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 1.0f, 0.0f,  // Yellow (like bumper targets)
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Top target lights - 4 lights (game only has 4, not 5)
		if (control_top_target_lights_tag.Component)
		{
			for (int i = 0; i < 4; i++)
			{
				HDRLightOverlay::AddLightConfig({
					"top_target_lights", i,
					0.02f, 0.82f + i * 0.02f,  // Left column positions
					0.03f, 0.03f,
					1.0f, 1.0f, 0.0f,  // Yellow
					peakNits, peakNits, 1.0f, false, true  // Locked
				});
			}
		}
		
		// Worm hole lights - 3 lights
		if (control_worm_hole_lights_tag.Component)
		{
			// Light 0 - Red
			HDRLightOverlay::AddLightConfig({
				"worm_hole_lights", 0,
				0.02f, 0.92f,  // Left column positions
				0.03f, 0.03f,
				1.0f, 0.0f, 0.0f,  // Red
				peakNits, peakNits, 1.0f, false, true  // Locked
			});
			
			// Light 1 - Green
			HDRLightOverlay::AddLightConfig({
				"worm_hole_lights", 1,
				0.02f, 0.94f,  // Left column positions
				0.03f, 0.03f,
				0.0f, 1.0f, 0.0f,  // Green
				peakNits, peakNits, 1.0f, false, true  // Locked
			});
			
			// Light 2 - Yellow (unchanged)
			HDRLightOverlay::AddLightConfig({
				"worm_hole_lights", 2,
				0.02f, 0.96f,  // Left column positions
				0.03f, 0.03f,
				1.0f, 1.0f, 0.0f,  // Yellow (like bumper targets)
				peakNits, peakNits, 1.0f, false, true  // Locked
			});
		}
		
		// Register additional light groups
		// Left chute target lights
		if (control_lchute_tgt_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("lchute_tgt_lights", control_lchute_tgt_lights_tag.Component);
		}
		
		// Goal lights
		if (control_goal_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("goal_lights", control_goal_lights_tag.Component);
		}
		
		// Hyperspace lights
		if (control_hyper_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("hyperspace_lights", control_hyper_lights_tag.Component);
		}
		
		// Bumper increment lights
		if (control_bmpr_inc_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("bmpr_inc_lights", control_bmpr_inc_lights_tag.Component);
		}
		
		// Solo target lights
		if (control_bpr_solotgt_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("bpr_solotgt_lights", control_bpr_solotgt_lights_tag.Component);
		}
		
		// Ball sink arrow lights
		if (control_bsink_arrow_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("bsink_arrow_lights", control_bsink_arrow_lights_tag.Component);
		}
		
		// Bumper target lights
		if (control_bumber_target_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("bumper_target_lights", control_bumber_target_lights_tag.Component);
		}
		
		// Right trek lights
		if (control_r_trek_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("r_trek_lights", control_r_trek_lights_tag.Component);
		}
		
		// Ramp bumper increment lights
		if (control_ramp_bmpr_inc_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("ramp_bmpr_inc_lights", control_ramp_bmpr_inc_lights_tag.Component);
		}
		
		// Ramp target lights
		if (control_ramp_tgt_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("ramp_tgt_lights", control_ramp_tgt_lights_tag.Component);
		}
		
		// Top circle target lights
		if (control_top_circle_tgt_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("top_circle_tgt_lights", control_top_circle_tgt_lights_tag.Component);
		}
		
		// Top target lights
		if (control_top_target_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("top_target_lights", control_top_target_lights_tag.Component);
		}
		
		// Worm hole lights
		if (control_worm_hole_lights_tag.Component)
		{
			HDRLightOverlay::RegisterLightGroup("worm_hole_lights", control_worm_hole_lights_tag.Component);
		}
		
		// Load light positions from config file with fallback to default
		if (!HDRLightOverlay::LoadLightPositions("/data/data/com.juiced.spacecadetpinball/files/light_positions.cfg")) {
			// Try to load default config from assets if user config doesn't exist
			HDRLightOverlay::LoadLightPositions("/data/data/com.juiced.spacecadetpinball/files/light_positions_default.cfg");
		}
	}
}

void control::ClearLinks()
{
	TableG = nullptr;
	for (auto& component : score_components)
		component.Tag->SetComponent(nullptr);
	for (auto& component : simple_components)
		component->SetComponent(nullptr);
}

TPinballComponent* control::make_component_link(component_tag_base* tag)
{
	if (tag->GetComponent())
		return tag->GetComponent();

	for (auto component: TableG->ComponentList)
	{
		if (component->GroupName)
		{
			if (!strcmp(component->GroupName, tag->Name))
			{
				tag->SetComponent(component);
				return component;
			}
		}
	}

	return nullptr;
}

void control::handler(int code, TPinballComponent* cmp)
{
	component_control* control = cmp->Control;
	
	if (control)
	{
		if (code == 1019)
		{
			for (auto scoreInd = 0; scoreInd < control->ScoreCount; ++scoreInd)
			{
				cmp->put_scoring(scoreInd, control->Scores[scoreInd]);
			}
		}
		control->ControlFunc(code, cmp);
	}
	MissionControl(code, cmp);
}

void control::pbctrl_bdoor_controller(char key)
{
	// Buffer large enough for longest cheat + null
	static char cheatBuffer[11 + 1]{};
	static char* bufferEnd = &cheatBuffer[11];
	static const char* quotes[8]
	{
		"Hey, is that a screen saver?",
		"I guess it has been a good week",
		"She may already be a glue bottle",
		"If you don't come in Saturday,\n...\n",
		"don't even bother coming in Sunday.",
		"Tomorrow already sucks",
		"I knew it worked too good to be right.",
		"World's most expensive flippers"
	};

	// Original allowed to enter cheats only before the first launch.
	std::memmove(&cheatBuffer[0], &cheatBuffer[1], 10);
	cheatBuffer[10] = key;

	if (strcmp(bufferEnd - 11, "hidden test") == 0)
	{
		pb::cheat_mode ^= true;
	}
	else if (strcmp(bufferEnd - 4, "gmax") == 0)
	{
		GravityWellKickoutControl(64, nullptr);
	}
	else if (strcmp(bufferEnd - 4, "1max") == 0)
	{
		table_add_extra_ball(2.0);
	}
	else if (strcmp(bufferEnd - 4, "bmax") == 0)
	{
		table_unlimited_balls ^= true;
	}
	else if (strcmp(bufferEnd - 4, "rmax") == 0)
	{
		cheat_bump_rank();
	}
	else if (pb::FullTiltMode && strcmp(bufferEnd - 5, "quote") == 0)
	{
		// A sad developer easter egg type 'cheat' from Full Tilt 
		float time = 0;
		for (auto quote : quotes)
			control_mission_text_box_tag.Component->Display(quote, time += 3, 1);
		return;
	}
	else
	{
		return;
	}

	TableG->CheatsUsed = 1;
    SpaceCadetPinballJNI::cheatsUsed();
}

bool control::check_cheats()
{
    if (TableG == nullptr) return false;
    else return TableG->CheatsUsed == 1;
}

void control::table_add_extra_ball(float count)
{
	++TableG->ExtraBalls;
	control_soundwave28_tag.Component->Play();
	auto msg = pinball::get_rc_string(9, 0);
	control_info_text_box_tag.Component->Display(msg, count, 2);
}

void control::table_set_bonus_hold()
{
	control_lite58_tag.Component->Message(19, 0.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(52, 0), 2.0, 2);
}

void control::table_set_bonus()
{
	TableG->ScoreSpecial2Flag = 1;
	control_lite59_tag.Component->Message(9, 60.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(4, 0), 2.0, 2);
}

void control::table_set_jackpot()
{
	TableG->ScoreSpecial3Flag = 1;
	control_lite60_tag.Component->Message(9, 60.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(15, 0), 2.0, 2);
}

void control::table_set_flag_lights()
{
	control_lite20_tag.Component->Message(9, 60.0);
	control_lite19_tag.Component->Message(9, 60.0);
	control_lite61_tag.Component->Message(9, 60.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(51, 0), 2.0, 2);
}

void control::table_set_multiball()
{
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(16, 0), 2.0, 2);
}

void control::table_bump_ball_sink_lock()
{
	if (TableG->BallLockedCounter == 2)
	{
		table_set_multiball();
		TableG->BallLockedCounter = 0;
	}
	else
	{
		TableG->BallLockedCounter = TableG->BallLockedCounter + 1;
		control_soundwave44_tag.Component->Play();
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(1, 0), 2.0, 2);
		TableG->Plunger->Message(1016, 0.0);
	}
}

void control::table_set_replay(float value)
{
	control_lite199_tag.Component->Message(19, 0.0);
	control_info_text_box_tag.Component->Display(pinball::get_rc_string(0, 0), value, 2);
}

void control::cheat_bump_rank()
{
	char Buffer[64]{};

	auto rank = control_middle_circle_tag.Component->Message(37, 0.0);
	if (rank < 9)
	{
		control_middle_circle_tag.Component->Message(41, 2.0f);
		auto rankText = pinball::get_rc_string(RankRcArray[rank], 1);
		snprintf(Buffer,sizeof Buffer, pinball::get_rc_string(83, 0), rankText);
		control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
		control_soundwave10_tag.Component->Play();
	}
}

bool control::light_on(component_tag<TLight>* tag)
{
	auto light = tag->Component;
	return light->BmpIndex1 || light->FlasherFlag2 || light->FlasherActive;
}

int control::SpecialAddScore(int score)
{
	int prevFlag1 = TableG->ScoreSpecial3Flag;
	TableG->ScoreSpecial3Flag = 0;
	int prevFlag2 = TableG->ScoreSpecial2Flag;
	TableG->ScoreSpecial2Flag = 0;
	int prevMult = TableG->ScoreMultiplier;
	TableG->ScoreMultiplier = 0;

	int addedScore = TableG->AddScore(score);
	TableG->ScoreSpecial2Flag = prevFlag2;
	TableG->ScoreMultiplier = prevMult;
	TableG->ScoreSpecial3Flag = prevFlag1;
	return addedScore;
}

int control::GetPlayerRank()
{
	if (control_middle_circle_tag.Component)
		return control_middle_circle_tag.Component->Message(37, 0.0);
	return 0;
}

int control::GetOuterCircleProgress()
{
	if (control_outer_circle_tag.Component)
		return control_outer_circle_tag.Component->Message(37, 0.0);
	return 0;
}

int control::GetOuterCircleTotal()
{
	if (control_outer_circle_tag.Component)
		return control_outer_circle_tag.Component->Message(38, 0.0);
	return 0;
}

int control::AddRankProgress(int rank)
{
	char Buffer[64];
	int result = 0;

	control_lite16_tag.Component->Message(19, 0.0);
	TPinballComponent* outerCircle = control_outer_circle_tag.Component;
	for (int index = rank; index; --index)
	{
		outerCircle->Message(41, 2.0);
	}

	int activeCount = outerCircle->Message(37, 0.0);
	int totalCount = outerCircle->Message(38, 0.0);
	if (activeCount == totalCount)
	{
		result = 1;
		outerCircle->Message(16, 5.0);
		TPinballComponent* middleCircle = control_middle_circle_tag.Component;
		control_middle_circle_tag.Component->Message(34, 0.0);
		int midActiveCount = middleCircle->Message(37, 0.0);
		if (midActiveCount < 9)
		{
			middleCircle->Message(41, 5.0);
			auto rankText = pinball::get_rc_string(RankRcArray[midActiveCount], 1);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(83, 0), rankText);
			control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
			control_soundwave10_tag.Component->Play();
		}
	}
	else if (activeCount >= 3 * totalCount / 4)
	{
		control_middle_circle_tag.Component->Message(27, -1.0);
	}
	return result;
}

void control::AdvanceWormHoleDestination(int flag)
{
	int lite198Msg = control_lite198_tag.Component->MessageField;
	if (lite198Msg != 16 && lite198Msg != 22 && lite198Msg != 23)
	{
		int lite4Msg = control_lite4_tag.Component->MessageField;
		if (flag || lite4Msg)
		{
			int val1 = lite4Msg + 1;
			int val2 = val1;
			if (val1 == 4)
			{
				val1 = 1;
				val2 = 1;
			}
			control_bsink_arrow_lights_tag.Component->Message(23, static_cast<float>(val2));
			control_bsink_arrow_lights_tag.Component->Message(11, static_cast<float>(3 - val1));
			if (!light_on(&control_lite4_tag))
			{
				control_worm_hole_lights_tag.Component->Message(19, 0.0);
				control_bsink_arrow_lights_tag.Component->Message(19, 0.0);
			}
		}
	}
}

void control::FlipperRebounderControl1(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite84_tag.Component->Message(9, 0.1f);
		auto score = caller->get_scoring(0);
		TableG->AddScore(score);
	}
}

void control::FlipperRebounderControl2(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite85_tag.Component->Message(9, 0.1f);
		int score = caller->get_scoring(0);
		TableG->AddScore(score);
	}
}

void control::RebounderControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::BumperControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		TableG->AddScore(caller->get_scoring(static_cast<TBumper*>(caller)->BmpIndex));
	}
}

void control::LeftKickerControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		control_gate1_tag.Component->Message(54, 0.0);
}

void control::RightKickerControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		control_gate2_tag.Component->Message(54, 0.0);
}

void control::LeftKickerGateControl(int code, TPinballComponent* caller)
{
	if (code == 53)
	{
		control_lite30_tag.Component->Message(15, 5.0);
		control_lite196_tag.Component->Message(7, 5.0);
	}
	else if (code == 54)
	{
		control_lite30_tag.Component->Message(20, 0.0);
		control_lite196_tag.Component->Message(20, 0.0);
	}
}

void control::RightKickerGateControl(int code, TPinballComponent* caller)
{
	if (code == 53)
	{
		control_lite29_tag.Component->Message(15, 5.0);
		control_lite195_tag.Component->Message(7, 5.0);
	}
	else if (code == 54)
	{
		control_lite29_tag.Component->Message(20, 0.0);
		control_lite195_tag.Component->Message(20, 0.0);
	}
}

void control::DeploymentChuteToEscapeChuteOneWayControl(int code, TPinballComponent* caller)
{
	char Buffer[64];
	if (code == 63)
	{
		int count = control_skill_shot_lights_tag.Component->Message(37, 0.0);
		if (count)
		{
			control_soundwave3_tag.Component->Play();
			int score = TableG->AddScore(caller->get_scoring(count - 1));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(21, 0), score);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			if (!light_on(&control_lite56_tag))
			{
				control_l_trek_lights_tag.Component->Message(34, 0.0);
				control_l_trek_lights_tag.Component->Message(20, 0.0);
				control_r_trek_lights_tag.Component->Message(34, 0.0);
				control_r_trek_lights_tag.Component->Message(20, 0.0);
			}
			control_skill_shot_lights_tag.Component->Message(44, 1.0);
		}
	}
}

void control::DeploymentChuteToTableOneWayControl(int code, TPinballComponent* caller)
{
	if (code == 63)
		control_skill_shot_lights_tag.Component->Message(20, 0.0);
}

void control::DrainBallBlockerControl(int code, TPinballComponent* caller)
{
	int msgCode;
	float msgValue;

	auto block = static_cast<TBlocker*>(caller);
	if (code == 52)
	{
		block->MessageField = 1;
		block->Message(52, static_cast<float>(block->TurnOnMsgValue));
		msgValue = static_cast<float>(block->TurnOnMsgValue);
		msgCode = 9;
	}
	else
	{
		if (code != 60)
			return;
		if (block->MessageField != 1)
		{
			block->MessageField = 0;
			block->Message(51, 0.0);
			return;
		}
		block->MessageField = 2;
		block->Message(59, static_cast<float>(block->TurnOffMsgValue));
		msgValue = static_cast<float>(block->TurnOffMsgValue);
		msgCode = 7;
	}
	control_lite1_tag.Component->Message(msgCode, msgValue);
}

void control::LaunchRampControl(int code, TPinballComponent* caller)
{
	TSound* sound;
	char Buffer[64];

	if (code == 63)
	{
		int someFlag = 0;
		if (light_on(&control_lite54_tag))
		{
			someFlag = 1;
			int addedScore = SpecialAddScore(TableG->ScoreSpecial1);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(10, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
		}
		if (light_on(&control_lite55_tag))
			someFlag |= 2u;
		if (light_on(&control_lite56_tag))
			someFlag |= 4u;
		if (someFlag)
		{
			if (someFlag == 1)
			{
				sound = control_soundwave21_tag.Component;
			}
			else if (someFlag < 1 || someFlag > 3)
			{
				sound = control_soundwave24_tag.Component;
			}
			else
			{
				sound = control_soundwave23_tag.Component;
			}
		}
		else
		{
			TableG->AddScore(caller->get_scoring(0));
			sound = control_soundwave30_tag.Component;
		}
		sound->Play();
	}
}

void control::LaunchRampHoleControl(int code, TPinballComponent* caller)
{
	if (code == 58)
		control_lite54_tag.Component->Message(7, 5.0);
}

void control::SpaceWarpRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		control_lite27_tag.Component->Message(19, 0.0);
		control_lite28_tag.Component->Message(19, 0.0);
	}
}

void control::ReentryLanesRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (!light_on(&control_lite56_tag) && control_l_trek_lights_tag.Component->Message(39, 0.0))
		{
			control_l_trek_lights_tag.Component->Message(34, 0.0);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(34, 0.0);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
		}

		TLight* light;
		if (control_roll3_tag.Component == caller)
		{
			light = control_lite8_tag.Component;
		}
		else
		{
			light = control_lite9_tag.Component;
			if (control_roll2_tag.Component != caller)
				light = control_lite10_tag.Component;
		}
		if (!light->FlasherActive)
		{
			if (light->BmpIndex1)
			{
				light->Message(20, 0.0);
			}
			else
			{
				light->Message(19, 0.0);
				int activeCount = control_bmpr_inc_lights_tag.Component->Message(37, 0.0);
				if (activeCount == control_bmpr_inc_lights_tag.Component->Message(38, 0.0))
				{
					control_bmpr_inc_lights_tag.Component->Message(7, 5.0);
					control_bmpr_inc_lights_tag.Component->Message(0, 0.0);
					if (control_bump1_tag.Component->BmpIndex < 3)
					{
						control_attack_bump_tag.Component->Message(12, 0.0);
						control_info_text_box_tag.Component->Display(pinball::get_rc_string(5, 0), 2.0, 2);
					}
					control_attack_bump_tag.Component->Message(48, 60.0);
				}
			}
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::BumperGroupControl(int code, TPinballComponent* caller)
{
	if (code == 61)
	{
		/*Bug in the original. Caller (TComponentGroup) is accessed beyond bounds at 0x4E*/
		if (static_cast<TBumper*>(caller)->BmpIndex)
		{
			caller->Message(48, 60.0);
			caller->Message(13, 0.0);
		}
	}
}

void control::LaunchLanesRolloverControl(int code, TPinballComponent* caller)
{
	TLight* light;

	if (code == 63)
	{
		if (control_roll112_tag.Component == caller)
		{
			light = control_lite171_tag.Component;
		}
		else
		{
			light = control_lite170_tag.Component;
			if (control_roll111_tag.Component != caller)
				light = control_lite169_tag.Component;
		}
		if (!light->FlasherActive)
		{
			if (light->BmpIndex1)
			{
				light->Message(20, 0.0);
			}
			else
			{
				light->Message(19, 0.0);
				int msg1 = control_ramp_bmpr_inc_lights_tag.Component->Message(37, 0.0);
				if (msg1 == control_ramp_bmpr_inc_lights_tag.Component->Message(38, 0.0))
				{
					control_ramp_bmpr_inc_lights_tag.Component->Message(7, 5.0);
					control_ramp_bmpr_inc_lights_tag.Component->Message(0, 0.0);
					if (control_bump5_tag.Component->BmpIndex < 3)
					{
						control_launch_bump_tag.Component->Message(12, 0.0);
						control_info_text_box_tag.Component->Display(pinball::get_rc_string(6, 0), 2.0, 2);
					}
					control_launch_bump_tag.Component->Message(48, 60.0);
				}
			}
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::OutLaneRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite17_tag) || light_on(&control_lite18_tag))
		{
			table_add_extra_ball(2.0);
			control_lite17_tag.Component->Message(20, 0.0);
			control_lite18_tag.Component->Message(20, 0.0);
		}
		else
		{
			control_soundwave26_tag.Component->Play();
		}
		if (control_roll4_tag.Component == caller)
		{
			if (light_on(&control_lite30_tag))
			{
				control_lite30_tag.Component->Message(4, 0.0);
				control_lite196_tag.Component->Message(4, 0.0);
			}
		}
		else if (light_on(&control_lite29_tag))
		{
			control_lite29_tag.Component->Message(4, 0.0);
			control_lite195_tag.Component->Message(4, 0.0);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::ExtraBallLightControl(int code, TPinballComponent* caller)
{
	if (code == 19)
	{
		control_lite17_tag.Component->Message(9, 55.0);
		control_lite18_tag.Component->Message(9, 55.0);
		extraball_light_flag = 1;
	}
	else if (code == 60)
	{
		if (extraball_light_flag)
		{
			control_lite17_tag.Component->Message(7, 5.0);
			control_lite18_tag.Component->Message(7, 5.0);
			extraball_light_flag = 0;
		}
	}
}

void control::ReturnLaneRolloverControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_roll6_tag.Component == caller)
		{
			if (light_on(&control_lite27_tag))
			{
				control_lite59_tag.Component->Message(20, 0.0);
				control_lite27_tag.Component->Message(20, 0.0);
				TableG->AddScore(caller->get_scoring(1));
			}
			else
				TableG->AddScore(caller->get_scoring(0));
		}
		else if (control_roll7_tag.Component == caller)
		{
			if (light_on(&control_lite28_tag))
			{
				control_lite59_tag.Component->Message(20, 0.0);
				control_lite28_tag.Component->Message(20, 0.0);
				TableG->AddScore(caller->get_scoring(1));
			}
			else
				TableG->AddScore(caller->get_scoring(0));
		}
	}
}

void control::BonusLaneRolloverControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		if (light_on(&control_lite16_tag))
		{
			int addedScore = SpecialAddScore(TableG->ScoreSpecial2);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(3, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			control_lite16_tag.Component->Message(20, 0.0);
			control_soundwave50_1_tag.Component->Play();
		}
		else
		{
			TableG->AddScore(caller->get_scoring(0));
			control_soundwave25_tag.Component->Play();
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		control_fuel_bargraph_tag.Component->Message(45, 11.0);
	}
}

void control::FuelRollover1Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 1)
		{
			control_literoll179_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 1.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover2Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 3)
		{
			control_literoll180_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 3.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover3Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 5)
		{
			control_literoll181_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 5.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover4Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 7)
		{
			control_literoll182_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 7.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover5Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 9)
		{
			control_literoll183_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 9.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::FuelRollover6Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (control_fuel_bargraph_tag.Component->Message(37, 0.0) > 11)
		{
			control_literoll184_tag.Component->Message(8, 0.05f);
		}
		else
		{
			control_fuel_bargraph_tag.Component->Message(45, 11.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::HyperspaceLightGroupControl(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 0:
		caller->Message(0, 0.0);
		break;
	case 41:
		caller->Message(41, 2.0);
		caller->Message(43, 60.0);
		break;
	case 61:
		caller->Message(33, 0.0);
		if (caller->Message(37, 0.0))
			caller->Message(43, 60.0);
		break;
	default: break;
	}
}

void control::WormHoleControl(int code, TPinballComponent* caller)
{
	int sinkFlag2;
	TSink* sink = static_cast<TSink*>(caller);

	if (code == 63)
	{
		// Timer mode: if waiting for final sink, eject ball to drain
		if (TimerMode::IsWaitingForFinalSink())
		{
			// Eject ball so it drains - OnFinalSink called from drain code 60
			caller->Message(56, 0.0);
			return;
		}
		int sinkFlag = 0;
		if (control_sink1_tag.Component != sink)
		{
			sinkFlag = control_sink2_tag.Component != sink;
			++sinkFlag;
		}

		int lite4Msg = control_lite4_tag.Component->MessageField;
		if (lite4Msg)
		{
			control_lite4_tag.Component->MessageField = 0;
			control_worm_hole_lights_tag.Component->Message(20, 0.0);
			control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
			control_lite110_tag.Component->Message(20, 0.0);
			if (lite4Msg == sinkFlag + 1)
			{
				if (TableG->MultiballFlag)
				{
					table_bump_ball_sink_lock();
					TableG->AddScore(10000);
				}
				else
				{
					control_info_text_box_tag.Component->Display(pinball::get_rc_string(49, 0), 2.0, 2);
					table_set_replay(4.0);
					TableG->AddScore(sink->get_scoring(1));
					wormhole_tag_array2[sinkFlag]->GetComponent()->Message(16, sink->TimerTime);
					wormhole_tag_array3[sinkFlag]->GetComponent()->Message(11, static_cast<float>(2 - sinkFlag));
					wormhole_tag_array3[sinkFlag]->GetComponent()->Message(16, sink->TimerTime);
					wormhole_tag_array1[sinkFlag]->GetComponent()->Message(56, sink->TimerTime);
				}
				return;
			}
			TableG->AddScore(sink->get_scoring(2));
			sinkFlag2 = lite4Msg - 1;
		}
		else
		{
			TableG->AddScore(sink->get_scoring(0));
			sinkFlag2 = sinkFlag;
		}

		wormhole_tag_array2[sinkFlag2]->GetComponent()->Message(16, sink->TimerTime);
		wormhole_tag_array3[sinkFlag2]->GetComponent()->Message(11, static_cast<float>(2 - sinkFlag2));
		wormhole_tag_array3[sinkFlag2]->GetComponent()->Message(16, sink->TimerTime);
		wormhole_tag_array1[sinkFlag2]->GetComponent()->Message(56, sink->TimerTime);
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(49, 0), 2.0, 2);
	}
}

void control::LeftFlipperControl(int code, TPinballComponent* caller)
{
	if (code == 1)
	{
		control_bmpr_inc_lights_tag.Component->Message(24, 0.0);
		control_ramp_bmpr_inc_lights_tag.Component->Message(24, 0.0);
	}
}

void control::RightFlipperControl(int code, TPinballComponent* caller)
{
	if (code == 1)
	{
		control_bmpr_inc_lights_tag.Component->Message(25, 0.0);
		control_ramp_bmpr_inc_lights_tag.Component->Message(25, 0.0);
	}
}

void control::JackpotLightControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		TableG->ScoreSpecial3Flag = 0;
}

void control::BonusLightControl(int code, TPinballComponent* caller)
{
	if (code == 60)
		TableG->ScoreSpecial2Flag = 0;
}

void control::BoosterTargetControl(int code, TPinballComponent* caller)
{
	TSound* sound = nullptr;

	if (code == 63 && !caller->MessageField)
	{
		caller->MessageField = 1;
		if (control_target1_tag.Component->MessageField
			+ control_target2_tag.Component->MessageField
			+ control_target3_tag.Component->MessageField != 3)
		{
			TableG->AddScore(caller->get_scoring(0));
			return;
		}
		if (light_on(&control_lite61_tag))
		{
			if (light_on(&control_lite60_tag))
			{
				if (light_on(&control_lite59_tag))
				{
					if (light_on(&control_lite58_tag))
					{
						TableG->AddScore(caller->get_scoring(1));
					}
					else
					{
						table_set_bonus_hold();
					}
					sound = control_soundwave48_tag.Component;
				}
				else
				{
					table_set_bonus();
					sound = control_soundwave46_tag.Component;
				}
			}
			else
			{
				table_set_jackpot();
				sound = control_soundwave45_tag.Component;
			}
		}
		else
		{
			int msg = control_lite198_tag.Component->MessageField;
			if (msg != 15 && msg != 29)
			{
				table_set_flag_lights();
				sound = control_soundwave47_tag.Component;
			}
		}
		if (sound)
			sound->Play();

		control_target1_tag.Component->MessageField = 0;
		control_target1_tag.Component->Message(50, 0.0);
		control_target2_tag.Component->MessageField = 0;
		control_target2_tag.Component->Message(50, 0.0);
		control_target3_tag.Component->MessageField = 0;
		control_target3_tag.Component->Message(50, 0.0);
		TableG->AddScore(caller->get_scoring(1));
	}
}

void control::MedalLightGroupControl(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 0:
		caller->Message(0, 0.0);
		break;
	case 41:
		caller->Message(41, 2.0);
		caller->Message(43, 30.0);
		break;
	case 61:
		caller->Message(33, 0.0);
		if (caller->Message(37, 0.0))
			caller->Message(43, 30.0);
		break;
	default: break;
	}
}

void control::MultiplierLightGroupControl(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 0:
		caller->Message(0, 0.0);
		break;
	case 41:
		caller->Message(41, 2.0);
		caller->Message(43, 30.0);
		break;
	case 61:
		if (TableG->ScoreMultiplier)
			TableG->ScoreMultiplier = TableG->ScoreMultiplier - 1;
		caller->Message(33, 0.0);
		if (caller->Message(37, 0.0))
			caller->Message(43, 30.0);
		break;
	case 64:
		TableG->ScoreMultiplier = 4;
		caller->Message(19, 0.0);
		caller->Message(43, 30.0);
		// Only show multiplier text in classic mode
		if (!TimerMode::IsTimerMode())
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(59, 0), 2.0, 2);
		break;
	case 65:
		TableG->ScoreMultiplier = 0;
		caller->Message(20, 0.0);
		caller->Message(43, -1.0);
		break;
	default:
		break;
	}
}

void control::FuelSpotTargetControl(int code, TPinballComponent* caller)
{
	TPinballComponent* liteComp;

	if (code == 63)
	{
		if (control_target10_tag.Component == caller)
		{
			liteComp = control_lite70_tag.Component;
		}
		else
		{
			liteComp = control_lite71_tag.Component;
			if (control_target11_tag.Component != caller)
				liteComp = control_lite72_tag.Component;
		}
		liteComp->Message(15, 2.0);
		TableG->AddScore(caller->get_scoring(0));
		if (control_top_circle_tgt_lights_tag.Component->Message(37, 0.0) == 3)
		{
			control_top_circle_tgt_lights_tag.Component->Message(16, 2.0);
			control_fuel_bargraph_tag.Component->Message(45, 11.0);
			control_soundwave25_tag.Component->Play();
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(44, 0), 2.0, 2);
		}
		else
		{
			control_soundwave49D_tag.Component->Play();
		}
	}
}

void control::MissionSpotTargetControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		TPinballComponent* lite;
		if (control_target13_tag.Component == caller)
		{
			control_lite101_tag.Component->MessageField |= 1u;
			lite = control_lite101_tag.Component;
		}
		else if (control_target14_tag.Component == caller)
		{
			control_lite101_tag.Component->MessageField |= 2u;
			lite = control_lite102_tag.Component;
		}
		else
		{
			control_lite101_tag.Component->MessageField |= 4u;
			lite = control_lite103_tag.Component;
		}
		lite->Message(15, 2.0);

		TSound* sound;
		if (!light_on(&control_lite198_tag) || control_lite198_tag.Component->FlasherActive)
		{
			sound = control_soundwave52_tag.Component;
		}
		else
			sound = control_soundwave49D_tag.Component;
		sound->Play();
		TableG->AddScore(caller->get_scoring(0));
		if (control_ramp_tgt_lights_tag.Component->Message(37, 0.0) == 3)
			control_ramp_tgt_lights_tag.Component->Message(16, 2.0);
	}
}

void control::LeftHazardSpotTargetControl(int code, TPinballComponent* caller)
{
	TPinballComponent* lite;

	if (code == 63)
	{
		if (control_target16_tag.Component == caller)
		{
			control_lite104_tag.Component->MessageField |= 1u;
			lite = control_lite104_tag.Component;
		}
		else if (control_target17_tag.Component == caller)
		{
			control_lite104_tag.Component->MessageField |= 2u;
			lite = control_lite105_tag.Component;
		}
		else
		{
			control_lite104_tag.Component->MessageField |= 4u;
			lite = control_lite106_tag.Component;
		}
		lite->Message(15, 2.0);
		TableG->AddScore(caller->get_scoring(0));
		if (control_lchute_tgt_lights_tag.Component->Message(37, 0.0) == 3)
		{
			control_soundwave14_1_tag.Component->Play();
			control_gate1_tag.Component->Message(53, 0.0);
			control_lchute_tgt_lights_tag.Component->Message(16, 2.0);
		}
		else
		{
			control_soundwave49D_tag.Component->Play();
		}
	}
}

void control::RightHazardSpotTargetControl(int code, TPinballComponent* caller)
{
	TPinballComponent* light;

	if (code == 63)
	{
		if (control_target19_tag.Component == caller)
		{
			control_lite107_tag.Component->MessageField |= 1u;
			light = control_lite107_tag.Component;
		}
		else if (control_target20_tag.Component == caller)
		{
			control_lite107_tag.Component->MessageField |= 2u;
			light = control_lite108_tag.Component;
		}
		else
		{
			control_lite107_tag.Component->MessageField |= 4u;
			light = control_lite109_tag.Component;
		}
		light->Message(15, 2.0);
		TableG->AddScore(caller->get_scoring(0));
		if (control_bpr_solotgt_lights_tag.Component->Message(37, 0.0) == 3)
		{
			control_soundwave14_1_tag.Component->Play();
			control_gate2_tag.Component->Message(53, 0.0);
			control_bpr_solotgt_lights_tag.Component->Message(16, 2.0);
		}
		else
		{
			control_soundwave49D_tag.Component->Play();
		}
	}
}

void control::WormHoleDestinationControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (!light_on(&control_lite110_tag))
		{
			control_lite110_tag.Component->Message(15, 3.0);
			control_info_text_box_tag.Component->Display(pinball::get_rc_string(93, 0), 2.0, 2);
		}
		TableG->AddScore(caller->get_scoring(0));
		AdvanceWormHoleDestination(1);
	}
}

void control::BlackHoleKickoutControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		// Timer mode: if waiting for final sink, eject ball to drain
		if (TimerMode::IsWaitingForFinalSink())
		{
			// Eject ball so it drains - OnFinalSink called from drain code 60
			caller->Message(55, -1.0);
			return;
		}
		int addedScore = TableG->AddScore(caller->get_scoring(0));
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(80, 0), addedScore);
		control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
		caller->Message(55, -1.0);
	}
}

void control::FlagControl(int code, TPinballComponent* caller)
{
	if (code == 62)
	{
		AdvanceWormHoleDestination(0);
	}
	else if (code == 63)
	{
		int score = caller->get_scoring(light_on(&control_lite20_tag));
		TableG->AddScore(score);
	}
}

void control::GravityWellKickoutControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	switch (code)
	{
	case 63:
		{
			// Timer mode: if waiting for final sink, eject ball to drain
			if (TimerMode::IsWaitingForFinalSink())
			{
				// Eject ball so it drains - OnFinalSink called from drain code 60
				caller->Message(55, 0.0);
				return;
			}
			auto addedScore = TableG->AddScore(caller->get_scoring(0));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(81, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			control_lite62_tag.Component->Message(20, 0.0);
			caller->ActiveFlag = 0;
			auto duration = control_soundwave7_tag.Component->Play();
			caller->Message(55, duration);
			break;
		}
	case 64:
		{
			auto score = reinterpret_cast<size_t>(caller);
			if (score)
			{
				snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(82, 0), score);
			}
			else
			{
				snprintf(Buffer, sizeof Buffer, "%s", pinball::get_rc_string(45, 0));
			}
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			control_lite62_tag.Component->Message(4, 0.0);
			control_kickout1_tag.Component->ActiveFlag = 1;
			break;
		}
	case 1024:
		control_kickout1_tag.Component->ActiveFlag = 0;
		break;
	}
}

void control::SkillShotGate1Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		SpaceCadetPinballJNI::setBallInPlunger(false);
		control_lite200_tag.Component->Message(9, 5.0);
		if (light_on(&control_lite67_tag))
		{
			control_skill_shot_lights_tag.Component->Message(34, 0.0);
			control_skill_shot_lights_tag.Component->Message(20, 0.0);
			control_lite67_tag.Component->Message(19, 0.0);
			control_lite54_tag.Component->Message(7, 5.0);
			control_lite25_tag.Component->Message(7, 5.0);
			control_fuel_bargraph_tag.Component->Message(45, 11.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate2Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite68_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate3Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite69_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate4Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite131_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate5Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite132_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::SkillShotGate6Control(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		if (light_on(&control_lite67_tag))
		{
			control_lite133_tag.Component->Message(19, 0.0);
			control_soundwave14_2_tag.Component->Play();
		}
	}
}

void control::ShootAgainLightControl(int code, TPinballComponent* caller)
{
	if (code == 60)
	{
		if (caller->MessageField)
		{
			caller->MessageField = 0;
		}
		else
		{
			caller->Message(16, 5.0);
			caller->MessageField = 1;
		}
	}
}

void control::EscapeChuteSinkControl(int code, TPinballComponent* caller)
{
	if (code == 63)
	{
		// Timer mode: if waiting for final sink, eject ball to drain
		if (TimerMode::IsWaitingForFinalSink())
		{
			// Eject ball so it drains - OnFinalSink called from drain code 60
			caller->Message(56, 0.0);
			return;
		}
		caller->Message(56, static_cast<TSink*>(caller)->TimerTime);
	}
}

// Track previous mission state for audio notifications
static int s_previousMissionState = 0;

void control::MissionControl(int code, TPinballComponent* caller)
{
	if (!control_lite198_tag.Component)
		return;

	int lite198Msg = control_lite198_tag.Component->MessageField;
	
#ifdef __ANDROID__
	// Notify Java layer when mission state changes (for mission music track)
	// Mission is "active" when state >= 2 (actual mission in progress, not just selecting)
	bool wasActive = s_previousMissionState >= 2;
	bool isActive = lite198Msg >= 2;
	if (wasActive != isActive) {
		SpaceCadetPinballJNI::setMissionActive(isActive);
	}
	s_previousMissionState = lite198Msg;
#endif
	switch (code)
	{
	case 47:
		if (control_fuel_bargraph_tag.Component == caller && lite198Msg > 1)
		{
			control_l_trek_lights_tag.Component->Message(34, 0.0);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(34, 0.0);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(109, 0), 4.0, 1);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
		}
		break;
	case 60:
		if (control_fuel_bargraph_tag.Component == caller && lite198Msg)
		{
			if (control_fuel_bargraph_tag.Component->Message(37, 0.0) == 1)
			{
				control_mission_text_box_tag.Component->Display(pinball::get_rc_string(116, 0), 4.0, 1);
			}
			break;
		}
		if (control_mission_text_box_tag.Component == caller)
			code = 67;
		break;
	case 1009:
		code = 67;
		break;
	default:
		break;
	}

	switch (lite198Msg)
	{
	case 0:
		WaitingDeploymentController(code, caller);
		break;
	case 1:
		SelectMissionController(code, caller);
		break;
	case 2:
		PracticeMissionController(code, caller);
		break;
	case 3:
		LaunchTrainingController(code, caller);
		break;
	case 4:
		ReentryTrainingController(code, caller);
		break;
	case 5:
		ScienceMissionController(code, caller);
		break;
	case 6:
		StrayCometController(code, caller);
		break;
	case 7:
		BlackHoleThreatController(code, caller);
		break;
	case 8:
		SpaceRadiationController(code, caller);
		break;
	case 9:
		BugHuntController(code, caller);
		break;
	case 10:
		AlienMenaceController(code, caller);
		break;
	case 11:
		RescueMissionController(code, caller);
		break;
	case 12:
		SatelliteController(code, caller);
		break;
	case 13:
		ReconnaissanceController(code, caller);
		break;
	case 14:
		DoomsdayMachineController(code, caller);
		break;
	case 15:
		CosmicPlagueController(code, caller);
		break;
	case 16:
		SecretMissionYellowController(code, caller);
		break;
	case 17:
		TimeWarpController(code, caller);
		break;
	case 18:
		MaelstromController(code, caller);
		break;
	case 20:
		AlienMenacePartTwoController(code, caller);
		break;
	case 21:
		CosmicPlaguePartTwoController(code, caller);
		break;
	case 22:
		SecretMissionRedController(code, caller);
		break;
	case 23:
		SecretMissionGreenController(code, caller);
		break;
	case 24:
		TimeWarpPartTwoController(code, caller);
		break;
	case 25:
		MaelstromPartTwoController(code, caller);
		break;
	case 26:
		MaelstromPartThreeController(code, caller);
		break;
	case 27:
		MaelstromPartFourController(code, caller);
		break;
	case 28:
		MaelstromPartFiveController(code, caller);
		break;
	case 29:
		MaelstromPartSixController(code, caller);
		break;
	case 30:
		MaelstromPartSevenController(code, caller);
		break;
	case 31:
		MaelstromPartEightController(code, caller);
		break;
	case 32:
		GameoverController(code, caller);
		break;
	default:
		UnselectMissionController(code, caller);
		break;
	}
}

void control::HyperspaceKickOutControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
		return;

	auto activeCount = control_hyper_lights_tag.Component->Message(37, 0.0);
	HyperspaceLightGroupControl(41, control_hyper_lights_tag.Component);
	switch (activeCount)
	{
	case 0:
		{
			auto addedScore = TableG->AddScore(caller->get_scoring(0));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(12, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			break;
		}
	case 1:
		{
			auto addedScore = SpecialAddScore(TableG->ScoreSpecial3);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(14, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			TableG->ScoreSpecial3 = 20000;
			break;
		}
	case 2:
		{
			DrainBallBlockerControl(52, control_block1_tag.Component);
			auto addedScore = TableG->AddScore(caller->get_scoring(2));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(2, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			break;
		}
	case 3:
		{
			ExtraBallLightControl(19, nullptr);
			auto addedScore = TableG->AddScore(caller->get_scoring(3));
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(8, 0), addedScore);
			control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
			break;
		}
	case 4:
		{
			control_hyper_lights_tag.Component->Message(0, 0.0);
			size_t addedScore = TableG->AddScore(caller->get_scoring(4));
			GravityWellKickoutControl(64, reinterpret_cast<TPinballComponent*>(addedScore));
			break;
		}
	default:
		break;
	}

	int someFlag = 0;
	if (light_on(&control_lite25_tag))
	{
		someFlag = 1;
		auto addedScore = SpecialAddScore(TableG->ScoreSpecial1);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(10, 0), addedScore);
		control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
	}
	if (light_on(&control_lite26_tag))
		someFlag |= 2u;
	if (light_on(&control_lite130_tag))
	{
		someFlag |= 4u;
		control_lite130_tag.Component->Message(20, 0.0);
		MultiplierLightGroupControl(64, control_top_target_lights_tag.Component);
		control_bumber_target_lights_tag.Component->Message(19, 0.0);
		table_set_jackpot();
		table_set_bonus();
		table_set_flag_lights();
		table_set_bonus_hold();
		control_lite27_tag.Component->Message(19, 0.0);
		control_lite28_tag.Component->Message(19, 0.0);
		ExtraBallLightControl(19, nullptr);
		DrainBallBlockerControl(52, control_block1_tag.Component);

		if (TableG->MultiballFlag)
		{
			table_set_multiball();
		}
		if (TableG->ScoreSpecial3 < 100000)
			TableG->ScoreSpecial3 = 100000;
		if (TableG->ScoreSpecial2 < 100000)
			TableG->ScoreSpecial2 = 100000;
		GravityWellKickoutControl(64, nullptr);
	}

	TSound* sound;
	if (someFlag)
	{
		if (someFlag == 1)
		{
			sound = control_soundwave21_tag.Component;
		}
		else
		{
			if (someFlag < 1 || someFlag > 3)
			{
				auto duration = control_soundwave41_tag.Component->Play();
				control_soundwave36_1_tag.Component->Play();
				control_soundwave50_2_tag.Component->Play();
				control_lite25_tag.Component->Message(7, 5.0);
				caller->Message(55, duration);
				return;
			}
			sound = control_soundwave40_tag.Component;
		}
	}
	else
	{
		switch (activeCount)
		{
		case 1:
			sound = control_soundwave36_2_tag.Component;
			break;
		case 2:
			sound = control_soundwave35_2_tag.Component;
			break;
		case 3:
			sound = control_soundwave38_tag.Component;
			break;
		case 4:
			sound = control_soundwave39_tag.Component;
			break;
		default:
			sound = control_soundwave35_1_tag.Component;
			break;
		}
	}
	auto duration = sound->Play();
	control_lite25_tag.Component->Message(7, 5.0);
	caller->Message(55, duration);
}

void control::PlungerControl(int code, TPinballComponent* caller)
{
	if (code == 1015)
	{
		MissionControl(67, nullptr);
	}
	else if (code == 1016)
	{
		table_unlimited_balls = false;
		if (!control_middle_circle_tag.Component->Message(37, 0.0))
			control_middle_circle_tag.Component->Message(32, 0.0);
		if (!light_on(&control_lite200_tag))
		{
			control_skill_shot_lights_tag.Component->Message(20, 0.0);
			control_lite67_tag.Component->Message(19, 0.0);
			control_skill_shot_lights_tag.Component->Message(26, 0.25f);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_l_trek_lights_tag.Component->Message(32, 0.2f);
			control_l_trek_lights_tag.Component->Message(26, 0.2f);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(32, 0.2f);
			control_r_trek_lights_tag.Component->Message(26, 0.2f);
			TableG->ScoreSpecial1 = 25000;
			MultiplierLightGroupControl(65, control_top_target_lights_tag.Component);
			control_fuel_bargraph_tag.Component->Message(19, 0.0);
			control_lite200_tag.Component->Message(19, 0.0);
			control_gate1_tag.Component->Message(53, 0.0);
			control_gate2_tag.Component->Message(53, 0.0);
		}
		control_lite200_tag.Component->MessageField = 0;
	}
}

void control::MedalTargetControl(int code, TPinballComponent* caller)
{
	if (code == 63 && !caller->MessageField)
	{
		caller->MessageField = 1;
		if (control_target6_tag.Component->MessageField
			+ control_target5_tag.Component->MessageField
			+ control_target4_tag.Component->MessageField == 3)
		{
			MedalLightGroupControl(41, control_bumber_target_lights_tag.Component);
			int activeCount = control_bumber_target_lights_tag.Component->Message(37, 0.0) - 1;
			char* text;
			switch (activeCount)
			{
			case 0:
				TableG->AddScore(caller->get_scoring(1));
				text = pinball::get_rc_string(53, 0);
				break;
			case 1:
				TableG->AddScore(caller->get_scoring(2));
				text = pinball::get_rc_string(54, 0);
				break;
			default:
				table_add_extra_ball(4.0);
				text = pinball::get_rc_string(55, 0);
				break;
			}
			control_info_text_box_tag.Component->Display(text, 2.0, 2);
			control_target6_tag.Component->MessageField = 0;
			control_target6_tag.Component->Message(50, 0.0);
			control_target5_tag.Component->MessageField = 0;
			control_target5_tag.Component->Message(50, 0.0);
			control_target4_tag.Component->MessageField = 0;
			control_target4_tag.Component->Message(50, 0.0);
			return;
		}
		TableG->AddScore(caller->get_scoring(0));
	}
}

void control::MultiplierTargetControl(int code, TPinballComponent* caller)
{
	if (code == 63 && !caller->MessageField)
	{
		caller->MessageField = 1;
		if (control_target9_tag.Component->MessageField
			+ control_target8_tag.Component->MessageField
			+ control_target7_tag.Component->MessageField == 3)
		{
			TableG->AddScore(caller->get_scoring(1));
			MultiplierLightGroupControl(41, control_top_target_lights_tag.Component);
			int activeCount = control_top_target_lights_tag.Component->Message(37, 0.0);
			char* text;
			switch (activeCount)
			{
			case 1:
				TableG->ScoreMultiplier = 1;
				text = pinball::get_rc_string(56, 0);
				break;
			case 2:
				TableG->ScoreMultiplier = 2;
				text = pinball::get_rc_string(57, 0);
				break;
			case 3:
				TableG->ScoreMultiplier = 3;
				text = pinball::get_rc_string(58, 0);
				break;
			default:
				TableG->ScoreMultiplier = 4;
				text = pinball::get_rc_string(59, 0);
				break;
			}

			// Only show multiplier text in classic mode
			if (!TimerMode::IsTimerMode())
				control_info_text_box_tag.Component->Display(text, 2.0, 2);
			control_target9_tag.Component->MessageField = 0;
			control_target9_tag.Component->Message(50, 0.0);
			control_target8_tag.Component->MessageField = 0;
			control_target8_tag.Component->Message(50, 0.0);
			control_target7_tag.Component->MessageField = 0;
			control_target7_tag.Component->Message(50, 0.0);
		}
		else
		{
			TableG->AddScore(caller->get_scoring(0));
		}
	}
}

void control::BallDrainControl(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 60)
	{
		if (TimerMode::IsWaitingForFinalSink())
		{
			TimerMode::OnFinalSink();
			return;
		}
		if (TimerMode::IsTimerMode())
		{
			return;
		}

		if (control_lite199_tag.Component->MessageField)
		{
			TableG->Message(1022, 0.0);
			if (pb::chk_highscore())
			{
				control_soundwave3_tag.Component->Play();
				TableG->LightGroup->Message(16, 3.0);
				char* v11 = pinball::get_rc_string(177, 0);
				control_mission_text_box_tag.Component->Display(v11, -1.0, 1);
			}
		}
		else
		{
			control_plunger_tag.Component->Message(1016, 0.0);
		}
	}
	else if (code == 63)
	{
		if (TimerMode::IsWaitingForFinalSink())
		{
			return;
		}

		if (table_unlimited_balls)
		{
			control_drain_tag.Component->Message(1024, 0.0);
			control_sink3_tag.Component->Message(56, 0.0);
		}
		else if (TimerMode::IsTimerMode())
		{
			if (light_on(&control_lite199_tag))
			{
				control_lite199_tag.Component->Message(20, 0.0);
				control_lite200_tag.Component->Message(19, 0.0);
				control_soundwave59_tag.Component->Play();
				control_info_text_box_tag.Component->Display("Replay Ball - No Penalty!", 2.0, 2);
			}
			else if (TableG->ExtraBalls)
			{
				TableG->ExtraBalls--;
				control_soundwave59_tag.Component->Play();
				control_info_text_box_tag.Component->Display("Extra Ball - No Penalty!", 2.0, 2);
			}
			else if (!TableG->ReplayActiveFlag)
			{
				if (light_on(&control_lite200_tag))
				{
					TimerMode::OnBallCrash(true);
					control_lite200_tag.Component->Message(20, 0.0);
					control_info_text_box_tag.Component->Display("Grace Period - Half Penalty!", 2.0, 2);
				}
				else
				{
					TimerMode::OnBallCrash(false);
				}
			}

			if (TimerMode::IsWaitingForFinalSink())
			{
				return;
			}

			if (TableG->TiltLockFlag)
			{
				pb::tilt_no_more();
				control_info_text_box_tag.Component->Display("Tilt Reset", 2.0, 2);
				control_fuel_bargraph_tag.Component->Message(45, 23.0f);
				control_lite198_tag.Component->MessageField = 1;
				MissionControl(66, nullptr);
			}

			control_skill_shot_lights_tag.Component->Message(20, 0.0);
			control_lite67_tag.Component->Message(19, 0.0);
			control_gate1_tag.Component->Message(53, 0.0);
			control_gate2_tag.Component->Message(53, 0.0);

			control_drain_tag.Component->Message(1024, 0.0);
			int randomSink = rand() % 3;
			if (randomSink == 0)
				control_sink1_tag.Component->Message(56, 0.0);
			else if (randomSink == 1)
				control_sink2_tag.Component->Message(56, 0.0);
			else
				control_sink3_tag.Component->Message(56, 0.0);
		}
		else
		{
			if (TableG->TiltLockFlag)
			{
				control_lite200_tag.Component->Message(20, 0.0);
				control_lite199_tag.Component->Message(20, 0.0);
			}
			if (light_on(&control_lite200_tag))
			{
				control_soundwave27_tag.Component->Play();
				control_lite200_tag.Component->Message(19, 0.0);
				control_info_text_box_tag.Component->Display(pinball::get_rc_string(96, 0), -1.0, 2);
				control_soundwave59_tag.Component->Play();
			}
			else if (light_on(&control_lite199_tag))
			{
				control_soundwave27_tag.Component->Play();
				control_lite199_tag.Component->Message(20, 0.0);
				control_lite200_tag.Component->Message(19, 0.0);
				control_info_text_box_tag.Component->Display(pinball::get_rc_string(95, 0), 2.0, 2);
				control_soundwave59_tag.Component->Play();
				--TableG->UnknownP78;
			}
			else if (TableG->UnknownP75)
			{
				control_soundwave27_tag.Component->Play();
				--TableG->UnknownP75;
			}
			else
			{
				if (!TableG->TiltLockFlag)
				{
					int time = SpecialAddScore(TableG->ScoreSpecial2);
					snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(94, 0), time);
					control_info_text_box_tag.Component->Display(Buffer, 2.0, 2);
				}
				if (TableG->ExtraBalls)
				{
					TableG->ExtraBalls--;

					char* shootAgainText;
					control_soundwave59_tag.Component->Play();
					switch (TableG->CurrentPlayer)
					{
					case 0:
						shootAgainText = pinball::get_rc_string(97, 0);
						break;
					case 1:
						shootAgainText = pinball::get_rc_string(98, 0);
						break;
					case 2:
						shootAgainText = pinball::get_rc_string(99, 0);
						break;
					default:
					case 3:
						shootAgainText = pinball::get_rc_string(100, 0);
						break;
					}
					control_info_text_box_tag.Component->Display(shootAgainText, -1.0, 2);
				}
				else
				{
					TableG->ChangeBallCount(TableG->BallCount - 1);
					if (TableG->CurrentPlayer + 1 != TableG->PlayerCount || TableG->BallCount)
					{
						TableG->Message(1021, 0.0);
						control_lite199_tag.Component->MessageField = 0;
					}
					else
					{
						control_lite199_tag.Component->MessageField = 1;
					}
					control_soundwave27_tag.Component->Play();
				}
				control_bmpr_inc_lights_tag.Component->Message(20, 0.0);
				control_ramp_bmpr_inc_lights_tag.Component->Message(20, 0.0);
				control_lite30_tag.Component->Message(20, 0.0);
				control_lite29_tag.Component->Message(20, 0.0);
				control_lite1_tag.Component->Message(20, 0.0);
				control_lite54_tag.Component->Message(20, 0.0);
				control_lite55_tag.Component->Message(20, 0.0);
				control_lite56_tag.Component->Message(20, 0.0);
				control_lite17_tag.Component->Message(20, 0.0);
				control_lite18_tag.Component->Message(20, 0.0);
				control_lite27_tag.Component->Message(20, 0.0);
				control_lite28_tag.Component->Message(20, 0.0);
				control_lite16_tag.Component->Message(20, 0.0);
				control_lite20_tag.Component->Message(20, 0.0);
				control_hyper_lights_tag.Component->Message(20, 0.0);
				control_lite25_tag.Component->Message(20, 0.0);
				control_lite26_tag.Component->Message(20, 0.0);
				control_lite130_tag.Component->Message(20, 0.0);
				control_lite19_tag.Component->Message(20, 0.0);
				control_worm_hole_lights_tag.Component->Message(20, 0.0);
				control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
				control_l_trek_lights_tag.Component->Message(20, 0.0);
				control_r_trek_lights_tag.Component->Message(20, 0.0);
				control_lite60_tag.Component->Message(20, 0.0);
				control_lite59_tag.Component->Message(20, 0.0);
				control_lite61_tag.Component->Message(20, 0.0);
				control_bumber_target_lights_tag.Component->Message(20, 0.0);
				control_top_target_lights_tag.Component->Message(20, 0.0);
				control_top_circle_tgt_lights_tag.Component->Message(20, 0.0);
				control_ramp_tgt_lights_tag.Component->Message(20, 0.0);
				control_lchute_tgt_lights_tag.Component->Message(20, 0.0);
				control_bpr_solotgt_lights_tag.Component->Message(20, 0.0);
				control_lite110_tag.Component->Message(20, 0.0);
				control_skill_shot_lights_tag.Component->Message(20, 0.0);
				control_lite77_tag.Component->Message(20, 0.0);
				control_lite198_tag.Component->Message(20, 0.0);
				control_lite196_tag.Component->Message(20, 0.0);
				control_lite195_tag.Component->Message(20, 0.0);
				control_fuel_bargraph_tag.Component->Message(20, 0.0);
				control_fuel_bargraph_tag.Component->Message(1024, 0.0);
				GravityWellKickoutControl(1024, nullptr);
				control_lite62_tag.Component->Message(20, 0.0);
				control_lite4_tag.Component->MessageField = 0;
				control_lite101_tag.Component->MessageField = 0;
				control_lite102_tag.Component->MessageField = 0;
				control_lite103_tag.Component->MessageField = 0;
				control_ramp_tgt_lights_tag.Component->MessageField = 0;
				control_outer_circle_tag.Component->Message(34, 0.0);
				control_middle_circle_tag.Component->Message(34, 0.0);
				control_attack_bump_tag.Component->Message(1024, 0.0);
				control_launch_bump_tag.Component->Message(1024, 0.0);
				control_gate1_tag.Component->Message(1024, 0.0);
				control_gate2_tag.Component->Message(1024, 0.0);
				control_block1_tag.Component->Message(1024, 0.0);
				control_target1_tag.Component->Message(1024, 0.0);
				control_target2_tag.Component->Message(1024, 0.0);
				control_target3_tag.Component->Message(1024, 0.0);
				control_target6_tag.Component->Message(1024, 0.0);
				control_target5_tag.Component->Message(1024, 0.0);
				control_target4_tag.Component->Message(1024, 0.0);
				control_target9_tag.Component->Message(1024, 0.0);
				control_target8_tag.Component->Message(1024, 0.0);
				control_target7_tag.Component->Message(1024, 0.0);
				if (control_lite199_tag.Component->MessageField)
					control_lite198_tag.Component->MessageField = 32;
				else
					control_lite198_tag.Component->MessageField = 0;
				MissionControl(66, nullptr);
				TableG->Message(1012, 0.0);
				if (light_on(&control_lite58_tag))
					control_lite58_tag.Component->Message(20, 0.0);
				else
					TableG->ScoreSpecial2 = 25000;
			}
		}
	}
}
void control::table_control_handler(int code)
{
	if (code == 1011)
	{
		table_unlimited_balls = false;
		control_lite77_tag.Component->Message(7, 0.0);
	}
}

void control::reset_tilt_light()
{
	// Turn off the tilt light (lite77)
	if (control_lite77_tag.Component)
	{
		control_lite77_tag.Component->Message(20, 0.0);
	}
}

void control::AlienMenaceController(int code, TPinballComponent* caller)
{
	if (code != 11)
	{
		if (code == 66)
		{
			control_attack_bump_tag.Component->Message(11, 0.0);
			TPinballComponent* lTrekLight = control_l_trek_lights_tag.Component;
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			lTrekLight->Message(32, 0.2f);
			lTrekLight->Message(26, 0.2f);
			TPinballComponent* rTrekLight = control_r_trek_lights_tag.Component;
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			rTrekLight->Message(32, 0.2f);
			rTrekLight->Message(26, 0.2f);
			control_lite307_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(175, 0), -1.0, 1);
		return;
	}
	if (control_bump1_tag.Component == caller)
	{
		if (control_bump1_tag.Component->BmpIndex)
		{
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 20;
			MissionControl(66, nullptr);
		}
	}
}

void control::AlienMenacePartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 8;
			control_l_trek_lights_tag.Component->Message(34, 0.0);
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			control_r_trek_lights_tag.Component->Message(34, 0.0);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite311_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(107, 0),
		          control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_bump1_tag.Component == caller
		|| control_bump2_tag.Component == caller
		|| control_bump3_tag.Component == caller
		|| control_bump4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite311_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(130, 0), 4.0, 1);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(7))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::BlackHoleThreatController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 11)
	{
		if (control_bump5_tag.Component == caller)
			MissionControl(67, caller);
	}
	else if (code == 63)
	{
		if (control_kickout3_tag.Component == caller
			&& control_bump5_tag.Component->BmpIndex)
		{
			if (light_on(&control_lite316_tag))
				control_lite316_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite314_tag))
				control_lite314_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(124, 0), 4.0, 1);
			int addedScore = SpecialAddScore(1000000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(8))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
	else
	{
		if (code == 66)
		{
			control_launch_bump_tag.Component->Message(11, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		if (control_bump5_tag.Component->BmpIndex)
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(123, 0), -1.0, 1);
			if (light_on(&control_lite316_tag))
				control_lite316_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite314_tag))
			{
				control_lite314_tag.Component->Message(7, 0.0);
			}
		}
		else
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(122, 0), -1.0, 1);
			if (light_on(&control_lite314_tag))
				control_lite314_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite316_tag))
			{
				control_lite316_tag.Component->Message(7, 0.0);
			}
		}
	}
}

void control::BugHuntController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 15;
			control_target1_tag.Component->MessageField = 0;
			control_target1_tag.Component->Message(50, 0.0);
			control_target2_tag.Component->MessageField = 0;
			control_target2_tag.Component->Message(50, 0.0);
			control_target3_tag.Component->MessageField = 0;
			control_target3_tag.Component->Message(50, 0.0);
			control_target6_tag.Component->MessageField = 0;
			control_target6_tag.Component->Message(50, 0.0);
			control_target5_tag.Component->MessageField = 0;
			control_target5_tag.Component->Message(50, 0.0);
			control_target4_tag.Component->MessageField = 0;
			control_target4_tag.Component->Message(50, 0.0);
			control_target9_tag.Component->MessageField = 0;
			control_target9_tag.Component->Message(50, 0.0);
			control_target8_tag.Component->MessageField = 0;
			control_target8_tag.Component->Message(50, 0.0);
			control_target7_tag.Component->MessageField = 0;
			control_target7_tag.Component->Message(50, 0.0);
			control_top_circle_tgt_lights_tag.Component->Message(20, 0.0);
			control_ramp_tgt_lights_tag.Component->Message(20, 0.0);
			control_lchute_tgt_lights_tag.Component->Message(20, 0.0);
			control_bpr_solotgt_lights_tag.Component->Message(20, 0.0);
			control_lite306_tag.Component->Message(7, 0.0);
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite310_tag.Component->Message(7, 0.0);
			control_lite313_tag.Component->Message(7, 0.0);
			control_lite319_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(125, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_target1_tag.Component == caller
		|| control_target2_tag.Component == caller
		|| control_target3_tag.Component == caller
		|| control_target6_tag.Component == caller
		|| control_target5_tag.Component == caller
		|| control_target4_tag.Component == caller
		|| control_target9_tag.Component == caller
		|| control_target8_tag.Component == caller
		|| control_target7_tag.Component == caller
		|| control_target10_tag.Component == caller
		|| control_target11_tag.Component == caller
		|| control_target12_tag.Component == caller
		|| control_target13_tag.Component == caller
		|| control_target14_tag.Component == caller
		|| control_target15_tag.Component == caller
		|| control_target16_tag.Component == caller
		|| control_target17_tag.Component == caller
		|| control_target18_tag.Component == caller
		|| control_target19_tag.Component == caller
		|| control_target20_tag.Component == caller
		|| control_target21_tag.Component == caller
		|| control_target22_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite306_tag.Component->Message(20, 0.0);
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite310_tag.Component->Message(20, 0.0);
			control_lite313_tag.Component->Message(20, 0.0);
			control_lite319_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(126, 0), 4.0, 1);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(7))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::CosmicPlagueController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 75;
			control_lite20_tag.Component->Message(19, 0.0);
			control_lite19_tag.Component->Message(19, 0.0);
			control_lite305_tag.Component->Message(7, 0.0);
			control_lite312_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(139, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_flag1_tag.Component == caller || control_flag2_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite305_tag.Component->Message(20, 0.0);
			control_lite312_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 21;
			MissionControl(66, nullptr);
			control_lite20_tag.Component->Message(20, 0.0);
			control_lite19_tag.Component->Message(20, 0.0);
		}
	}
}

void control::CosmicPlaguePartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite310_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(140, 0), -1.0, 1);
		return;
	}
	if (control_roll9_tag.Component == caller)
	{
		control_lite310_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 1;
		MissionControl(66, nullptr);
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(141, 0), 4.0, 1);
		int addedScore = SpecialAddScore(1750000);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
		if (!AddRankProgress(11))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
			control_soundwave9_tag.Component->Play();
		}
	}
}

void control::DoomsdayMachineController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite301_tag.Component->Message(7, 0.0);
			control_lite320_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(137, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_roll4_tag.Component == caller || control_roll8_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite301_tag.Component->Message(20, 0.0);
			control_lite320_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(138, 0), 4.0, 1);
			int addedScore = SpecialAddScore(1250000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::GameoverController(int code, TPinballComponent* caller)
{
	char Buffer[128];

	if (code == 66)
	{
		control_goal_lights_tag.Component->Message(20, 0.0);
		pb::mode_change(GameModes::GameOver);
		control_flip1_tag.Component->Message(1022, 0.0);
		control_flip2_tag.Component->Message(1022, 0.0);
		control_mission_text_box_tag.Component->MessageField = 0;
		return;
	}
	if (code != 67)
		return;

	int missionMsg = control_mission_text_box_tag.Component->MessageField;
	if (missionMsg & 0x100)
	{
		int playerId = missionMsg % 4;
		int playerScore = TableG->PlayerScores[playerId].ScoreStruct->Score;
		auto nextPlayerId = playerId + 1;
		if (playerScore >= 0)
		{
			const char* playerNScoreText = nullptr;
			switch (nextPlayerId)
			{
			case 1:
				playerNScoreText = pinball::get_rc_string(180, 0);
				break;
			case 2:
				playerNScoreText = pinball::get_rc_string(181, 0);
				break;
			case 3:
				playerNScoreText = pinball::get_rc_string(182, 0);
				break;
			case 4:
				playerNScoreText = pinball::get_rc_string(183, 0);
				break;
			default:
				break;
			}
			if (playerNScoreText != nullptr)
			{
				snprintf(Buffer, sizeof Buffer, playerNScoreText, playerScore);
				control_mission_text_box_tag.Component->Display(Buffer, 3.0, 1);
				int msgField = nextPlayerId == TableG->PlayerCount ? 0x200 : nextPlayerId | 0x100;
				control_mission_text_box_tag.Component->MessageField = msgField;
				return;
			}
		}
		control_mission_text_box_tag.Component->MessageField = 0x200;
	}

	if (missionMsg & 0x200)
	{
		int highscoreId = missionMsg % 5;
		int highScore = pb::highscore_table[highscoreId].Score;
		auto nextHidhscoreId = highscoreId + 1;
		if (highScore > 0)
		{
			const char* highScoreNText = nullptr;
			switch (nextHidhscoreId)
			{
			case 1:
				highScoreNText = pinball::get_rc_string(184, 0);
				break;
			case 2:
				highScoreNText = pinball::get_rc_string(185, 0);
				break;
			case 3:
				highScoreNText = pinball::get_rc_string(186, 0);
				break;
			case 4:
				highScoreNText = pinball::get_rc_string(187, 0);
				break;
			case 5:
				highScoreNText = pinball::get_rc_string(188, 0);
				break;
			default:
				break;
			}
			if (highScoreNText != nullptr)
			{
				snprintf(Buffer, sizeof Buffer, highScoreNText, highScore);
				control_mission_text_box_tag.Component->Display(Buffer, 3.0, 1);
				int msgField = nextHidhscoreId == 5 ? 0 : nextHidhscoreId | 0x200;
				control_mission_text_box_tag.Component->MessageField = msgField;
				return;
			}
		}
	}

	control_mission_text_box_tag.Component->MessageField = 0x100;
	control_mission_text_box_tag.Component->Display(pinball::get_rc_string(172, 0), 10.0, 1);
}

void control::LaunchTrainingController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite317_tag.Component->Message(7, 0.0);
			control_lite56_tag.Component->MessageField = 3;
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(110, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_ramp_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite317_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(111, 0), 4.0, 1);
			int addedScore = SpecialAddScore(500000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(6))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::MaelstromController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite303_tag.Component->Message(7, 0.0);
			control_lite309_tag.Component->Message(7, 0.0);
			control_lite315_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(148, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_target1_tag.Component == caller
		|| control_target2_tag.Component == caller
		|| control_target3_tag.Component == caller
		|| control_target6_tag.Component == caller
		|| control_target5_tag.Component == caller
		|| control_target4_tag.Component == caller
		|| control_target9_tag.Component == caller
		|| control_target8_tag.Component == caller
		|| control_target7_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite303_tag.Component->Message(20, 0.0);
			control_lite309_tag.Component->Message(20, 0.0);
			control_lite315_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 25;
			MissionControl(66, nullptr);
		}
	}
}

void control::MaelstromPartEightController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite130_tag.Component->Message(19, 0.0);
			control_lite304_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(155, 0), -1.0, 1);
		return;
	}
	if (control_kickout2_tag.Component == caller)
	{
		control_lite304_tag.Component->Message(20, 0.0);
		control_lite130_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 1;
		MissionControl(66, nullptr);
		int addedScore = SpecialAddScore(5000000);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
		control_info_text_box_tag.Component->Display(pinball::get_rc_string(48, 0), 4.0, 2);
		if (!AddRankProgress(18))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
			control_soundwave9_tag.Component->Play();
		}
	}
}

void control::MaelstromPartFiveController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite317_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(152, 0), -1.0, 1);
		return;
	}
	if (control_ramp_tag.Component == caller)
	{
		control_lite317_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 29;
		MissionControl(66, nullptr);
	}
}

void control::MaelstromPartFourController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 0;
			control_lite318_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(151, 0), -1.0, 1);
		return;
	}
	if (control_roll184_tag.Component == caller)
	{
		control_lite318_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 28;
		MissionControl(66, nullptr);
	}
}

void control::MaelstromPartSevenController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			AdvanceWormHoleDestination(1);
			control_sink1_tag.Component->Message(7, 0.0);
			control_sink2_tag.Component->Message(7, 0.0);
			control_sink3_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(154, 0), -1.0, 1);
		return;
	}
	if (control_sink1_tag.Component == caller
		|| control_sink2_tag.Component == caller
		|| control_sink3_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 31;
		MissionControl(66, nullptr);
	}
}

void control::MaelstromPartSixController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite20_tag.Component->Message(19, 0.0);
			control_lite19_tag.Component->Message(19, 0.0);
			control_lite305_tag.Component->Message(7, 0.0);
			control_lite312_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(153, 0), -1.0, 1);
		return;
	}
	if (control_flag1_tag.Component == caller || control_flag2_tag.Component == caller)
	{
		control_lite305_tag.Component->Message(20, 0.0);
		control_lite312_tag.Component->Message(20, 0.0);
		control_lite198_tag.Component->MessageField = 30;
		MissionControl(66, nullptr);
		control_lite20_tag.Component->Message(20, 0.0);
		control_lite19_tag.Component->Message(20, 0.0);
	}
}

void control::MaelstromPartThreeController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 5;
			control_lite301_tag.Component->Message(7, 0.0);
			control_lite302_tag.Component->Message(7, 0.0);
			control_lite307_tag.Component->Message(7, 0.0);
			control_lite316_tag.Component->Message(7, 0.0);
			control_lite320_tag.Component->Message(7, 0.0);
			control_lite321_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(150, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_roll3_tag.Component == caller
		|| control_roll2_tag.Component == caller
		|| control_roll1_tag.Component == caller
		|| control_roll112_tag.Component == caller
		|| control_roll111_tag.Component == caller
		|| control_roll110_tag.Component == caller
		|| control_roll4_tag.Component == caller
		|| control_roll8_tag.Component == caller
		|| control_roll6_tag.Component == caller
		|| control_roll7_tag.Component == caller
		|| control_roll5_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite301_tag.Component->Message(20, 0.0);
			control_lite302_tag.Component->Message(20, 0.0);
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite316_tag.Component->Message(20, 0.0);
			control_lite320_tag.Component->Message(20, 0.0);
			control_lite321_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 27;
			MissionControl(66, nullptr);
		}
	}
}

void control::MaelstromPartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite306_tag.Component->Message(7, 0.0);
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite310_tag.Component->Message(7, 0.0);
			control_lite313_tag.Component->Message(7, 0.0);
			control_lite319_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(149, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_target10_tag.Component == caller
		|| control_target11_tag.Component == caller
		|| control_target12_tag.Component == caller
		|| control_target13_tag.Component == caller
		|| control_target14_tag.Component == caller
		|| control_target15_tag.Component == caller
		|| control_target16_tag.Component == caller
		|| control_target17_tag.Component == caller
		|| control_target18_tag.Component == caller
		|| control_target19_tag.Component == caller
		|| control_target20_tag.Component == caller
		|| control_target21_tag.Component == caller
		|| control_target22_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite306_tag.Component->Message(20, 0.0);
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite310_tag.Component->Message(20, 0.0);
			control_lite313_tag.Component->Message(20, 0.0);
			control_lite319_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 26;
			MissionControl(66, nullptr);
		}
	}
}

void control::PracticeMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite308_tag.Component->Message(7, 0.0);
			control_lite311_tag.Component->Message(7, 0.0);
			control_lite56_tag.Component->MessageField = 8;
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(107, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}

	if (control_bump1_tag.Component == caller
		|| control_bump2_tag.Component == caller
		|| control_bump3_tag.Component == caller
		|| control_bump4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite311_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(108, 0), 4.0, 1);
			int addedScore = SpecialAddScore(500000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(6))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::ReconnaissanceController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 15;
			control_lite301_tag.Component->Message(7, 0.0);
			control_lite302_tag.Component->Message(7, 0.0);
			control_lite307_tag.Component->Message(7, 0.0);
			control_lite316_tag.Component->Message(7, 0.0);
			control_lite320_tag.Component->Message(7, 0.0);
			control_lite321_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(134, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_roll3_tag.Component == caller
		|| control_roll2_tag.Component == caller
		|| control_roll1_tag.Component == caller
		|| control_roll112_tag.Component == caller
		|| control_roll111_tag.Component == caller
		|| control_roll110_tag.Component == caller
		|| control_roll4_tag.Component == caller
		|| control_roll8_tag.Component == caller
		|| control_roll6_tag.Component == caller
		|| control_roll7_tag.Component == caller
		|| control_roll5_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, nullptr);
		}
		else
		{
			control_lite301_tag.Component->Message(20, 0.0);
			control_lite302_tag.Component->Message(20, 0.0);
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite316_tag.Component->Message(20, 0.0);
			control_lite320_tag.Component->Message(20, 0.0);
			control_lite321_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(136, 0), 4.0, 1);
			int addedScore = SpecialAddScore(1250000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::ReentryTrainingController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_l_trek_lights_tag.Component->Message(20, 0.0);
			static_cast<TPinballComponent*>(control_l_trek_lights_tag.Component)->Message(32, 0.2f);
			static_cast<TPinballComponent*>(control_l_trek_lights_tag.Component)->Message(26, 0.2f);
			control_r_trek_lights_tag.Component->Message(20, 0.0);
			static_cast<TPinballComponent*>(control_r_trek_lights_tag.Component)->Message(32, 0.2f);
			static_cast<TPinballComponent*>(control_r_trek_lights_tag.Component)->Message(26, 0.2f);
			control_lite307_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(112, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_roll3_tag.Component == caller
		|| control_roll2_tag.Component == caller
		|| control_roll1_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite307_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(113, 0), 4.0, 1);
			int addedScore = SpecialAddScore(500000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(6))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::RescueMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	switch (code)
	{
	case 63:
		{
			if (control_target1_tag.Component == caller
				|| control_target2_tag.Component == caller
				|| control_target3_tag.Component == caller)
			{
				MissionControl(67, caller);
				return;
			}
			if (control_kickout2_tag.Component != caller || !light_on(&control_lite20_tag))
				return;
			control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
			if (control_lite56_tag.Component->MessageField)
			{
				MissionControl(67, caller);
				return;
			}
			if (light_on(&control_lite303_tag))
				control_lite303_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite304_tag))
				control_lite304_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(129, 0), 4.0, 1);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(7))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
			break;
		}
	case 66:
		control_lite20_tag.Component->Message(20, 0.0);
		control_lite19_tag.Component->Message(20, 0.0);
		control_lite56_tag.Component->MessageField = 1;
		break;
	case 67:
		if (light_on(&control_lite20_tag))
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(128, 0), -1.0, 1);
			if (light_on(&control_lite303_tag))
				control_lite303_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite304_tag))
			{
				control_lite304_tag.Component->Message(7, 0.0);
			}
		}
		else
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(127, 0), -1.0, 1);
			if (light_on(&control_lite304_tag))
				control_lite304_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite303_tag))
			{
				control_lite303_tag.Component->Message(7, 0.0);
			}
		}
		break;
	default:
		break;
	}
}

void control::SatelliteController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 3;
			control_lite308_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(132, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_bump4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite308_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(133, 0), 4.0, 1);
			int addedScore = SpecialAddScore(1250000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::ScienceMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 9;
			control_target1_tag.Component->MessageField = 0;
			control_target1_tag.Component->Message(50, 0.0);
			control_target2_tag.Component->MessageField = 0;
			control_target2_tag.Component->Message(50, 0.0);
			control_target3_tag.Component->MessageField = 0;
			control_target3_tag.Component->Message(50, 0.0);
			control_target6_tag.Component->MessageField = 0;
			control_target6_tag.Component->Message(50, 0.0);
			control_target5_tag.Component->MessageField = 0;
			control_target5_tag.Component->Message(50, 0.0);
			control_target4_tag.Component->MessageField = 0;
			control_target4_tag.Component->Message(50, 0.0);
			control_target9_tag.Component->MessageField = 0;
			control_target9_tag.Component->Message(50, 0.0);
			control_target8_tag.Component->MessageField = 0;
			control_target8_tag.Component->Message(50, 0.0);
			control_target7_tag.Component->MessageField = 0;
			control_target7_tag.Component->Message(50, 0.0);
			control_lite303_tag.Component->Message(7, 0.0);
			control_lite309_tag.Component->Message(7, 0.0);
			control_lite315_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(114, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_target1_tag.Component == caller
		|| control_target2_tag.Component == caller
		|| control_target3_tag.Component == caller
		|| control_target6_tag.Component == caller
		|| control_target5_tag.Component == caller
		|| control_target4_tag.Component == caller
		|| control_target9_tag.Component == caller
		|| control_target8_tag.Component == caller
		|| control_target7_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite303_tag.Component->Message(20, 0.0);
			control_lite309_tag.Component->Message(20, 0.0);
			control_lite315_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(115, 0), 4.0, 1);
			int addedScore = SpecialAddScore(750000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(9))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
}

void control::SecretMissionGreenController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite6_tag.Component->Message(19, 0.0);
			control_lite2_tag.Component->Message(11, 1.0);
			control_lite2_tag.Component->Message(19, 0.0);
			control_lite2_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		char* v2 = pinball::get_rc_string(144, 0);
		control_mission_text_box_tag.Component->Display(v2, -1.0, 1);
		return;
	}
	if (control_sink2_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 1;
		MissionControl(66, nullptr);
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(145, 0), 4.0, 1);
		int addedScore = SpecialAddScore(1500000);
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
		if (!AddRankProgress(10))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
			control_soundwave9_tag.Component->Play();
		}
	}
}

void control::SecretMissionRedController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_lite5_tag.Component->Message(19, 0.0);
			control_lite4_tag.Component->Message(11, 2.0);
			control_lite4_tag.Component->Message(19, 0.0);
			control_lite4_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(143, 0), -1.0, 1);
		return;
	}
	if (control_sink1_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 23;
		MissionControl(66, nullptr);
	}
}

void control::SecretMissionYellowController(int code, TPinballComponent* caller)
{
	if (code != 63)
	{
		if (code == 66)
		{
			control_worm_hole_lights_tag.Component->Message(20, 0.0);
			control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
			control_bsink_arrow_lights_tag.Component->Message(23, 0.0);
			control_lite110_tag.Component->Message(20, 0.0);
			control_lite7_tag.Component->Message(19, 0.0);
			control_lite3_tag.Component->Message(11, 0.0);
			control_lite3_tag.Component->Message(19, 0.0);
			control_lite3_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(142, 0), -1.0, 1);
		return;
	}
	if (control_sink3_tag.Component == caller)
	{
		control_lite198_tag.Component->MessageField = 22;
		MissionControl(66, nullptr);
	}
}

void control::SelectMissionController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	switch (code)
	{
	case 45:
	case 47:
		if (control_fuel_bargraph_tag.Component != caller)
			return;
		MissionControl(67, caller);
		return;
	case 63:
		{
			int missionLevel = 0;
			if (control_target13_tag.Component == caller)
				missionLevel = 1;
			if (control_target14_tag.Component == caller)
				missionLevel = 2;
			if (control_target15_tag.Component == caller)
				missionLevel = 3;
			if (!missionLevel)
			{
				if (control_ramp_tag.Component == caller
					&& light_on(&control_lite56_tag)
					&& control_fuel_bargraph_tag.Component->Message(37, 0.0))
				{
					control_lite56_tag.Component->Message(20, 0.0);
					control_lite198_tag.Component->Message(19, 0.0);
					control_outer_circle_tag.Component->Message(26, -1.0);
					if (light_on(&control_lite317_tag))
						control_lite317_tag.Component->Message(20, 0.0);
					if (light_on(&control_lite318_tag))
						control_lite318_tag.Component->Message(20, 0.0);
					if (light_on(&control_lite319_tag))
						control_lite319_tag.Component->Message(20, 0.0);
					control_lite198_tag.Component->MessageField = control_lite56_tag.Component->MessageField;
					auto scoreId = control_lite56_tag.Component->MessageField - 2;
					MissionControl(66, nullptr);
					int addedScore = SpecialAddScore(mission_select_scores[scoreId]);
					snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(77, 0), addedScore);
					control_mission_text_box_tag.Component->Display(Buffer, 4.0, 1);
				}
				return;
			}

			if (control_lite101_tag.Component->MessageField == 7)
			{
				control_lite101_tag.Component->MessageField = 0;
				missionLevel = 4;
			}

			int missionId;
			auto activeCount = control_middle_circle_tag.Component->Message(37, 0.0);
			switch (activeCount)
			{
			case 1:
				switch (missionLevel)
				{
				case 1:
					missionId = 3;
					break;
				case 2:
					missionId = 4;
					break;
				case 3:
					missionId = 2;
					break;
				default:
					missionId = 5;
					break;
				}
				break;
			case 2:
			case 3:
				switch (missionLevel)
				{
				case 1:
					missionId = 9;
					break;
				case 2:
					missionId = 11;
					break;
				case 3:
					missionId = 10;
					break;
				default:
					missionId = 16;
					break;
				}
				break;
			case 4:
			case 5:
				switch (missionLevel)
				{
				case 1:
					missionId = 6;
					break;
				case 2:
					missionId = 8;
					break;
				case 3:
					missionId = 7;
					break;
				default:
					missionId = 15;
					break;
				}
				break;
			case 6:
			case 7:
				switch (missionLevel)
				{
				case 1:
					missionId = 12;
					break;
				case 2:
					missionId = 13;
					break;
				case 3:
					missionId = 14;
					break;
				default:
					missionId = 17;
					break;
				}
				break;
			case 8:
			case 9:
				switch (missionLevel)
				{
				case 1:
					missionId = 15;
					break;
				case 2:
					missionId = 16;
					break;
				case 3:
					missionId = 17;
					break;
				default:
					missionId = 18;
					break;
				}
				break;
			default:
				return;
			}
			control_lite56_tag.Component->MessageField = missionId;
			control_lite56_tag.Component->Message(15, 2.0);
			control_lite198_tag.Component->Message(4, 0.0);
			MissionControl(67, caller);
			return;
		}
	case 66:
		control_lite198_tag.Component->Message(20, 0.0);
		control_outer_circle_tag.Component->Message(34, 0.0);
		control_ramp_tgt_lights_tag.Component->Message(20, 0.0);
		control_lite56_tag.Component->MessageField = 0;
		control_lite101_tag.Component->MessageField = 0;
		control_l_trek_lights_tag.Component->Message(34, 0.0);
		control_l_trek_lights_tag.Component->Message(20, 0.0);
		control_r_trek_lights_tag.Component->Message(34, 0.0);
		control_r_trek_lights_tag.Component->Message(20, 0.0);
		control_goal_lights_tag.Component->Message(20, 0.0);
		control_worm_hole_lights_tag.Component->Message(20, 0.0);
		control_bsink_arrow_lights_tag.Component->Message(20, 0.0);
		break;
	case 67:
		break;
	default:
		return;
	}

	if (control_fuel_bargraph_tag.Component->Message(37, 0.0))
	{
		if (light_on(&control_lite56_tag))
		{
			auto missionText = pinball::
				get_rc_string(MissionRcArray[control_lite56_tag.Component->MessageField - 2], 1);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(106, 0), missionText);
			control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
			if (light_on(&control_lite318_tag))
				control_lite318_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite319_tag))
				control_lite319_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite317_tag))
				control_lite317_tag.Component->Message(7, 0.0);
		}
		else
		{
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(104, 0), -1.0, 1);
			if (light_on(&control_lite317_tag))
				control_lite317_tag.Component->Message(20, 0.0);
			if (light_on(&control_lite318_tag))
				control_lite318_tag.Component->Message(20, 0.0);
			if (!light_on(&control_lite319_tag))
			{
				control_lite319_tag.Component->Message(7, 0.0);
			}
		}
	}
	else
	{
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(105, 0), -1.0, 1);
		if (light_on(&control_lite317_tag))
			control_lite317_tag.Component->Message(20, 0.0);
		if (light_on(&control_lite319_tag))
			control_lite319_tag.Component->Message(20, 0.0);
		if (!light_on(&control_lite318_tag))
		{
			control_lite318_tag.Component->Message(7, 0.0);
		}
	}
}

void control::SpaceRadiationController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		if (control_target16_tag.Component == caller
			|| control_target17_tag.Component == caller
			|| control_target18_tag.Component == caller)
		{
			if (control_lite104_tag.Component->MessageField == 7)
			{
				control_lite104_tag.Component->MessageField = 15;
				control_bsink_arrow_lights_tag.Component->Message(7, 0.0);
				control_lite313_tag.Component->Message(20, 0.0);
				MissionControl(67, caller);
				AdvanceWormHoleDestination(1);
			}
		}
		else if ((control_sink1_tag.Component == caller
				|| control_sink2_tag.Component == caller
				|| control_sink3_tag.Component == caller)
			&& control_lite104_tag.Component->MessageField == 15)
		{
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(121, 0), 4.0, 1);
			int addedScore = SpecialAddScore(1000000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(8))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
	else
	{
		if (code == 66)
		{
			control_lchute_tgt_lights_tag.Component->Message(20, 0.0);
			control_lite104_tag.Component->MessageField = 0;
			control_lite313_tag.Component->Message(7, 0.0);
		}
		else if (code == 67)
		{
			char* text;
			if (control_lite104_tag.Component->MessageField == 15)
				text = pinball::get_rc_string(120, 0);
			else
				text = pinball::get_rc_string(176, 0);
			control_mission_text_box_tag.Component->Display(text, -1.0, 1);
		}
	}
}

void control::StrayCometController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code == 63)
	{
		if (control_target19_tag.Component == caller
			|| control_target20_tag.Component == caller
			|| control_target21_tag.Component == caller)
		{
			if (control_lite107_tag.Component->MessageField == 7)
			{
				control_lite306_tag.Component->Message(20, 0.0);
				control_lite304_tag.Component->Message(7, 0.0);
				control_lite107_tag.Component->MessageField = 15;
				MissionControl(67, caller);
			}
		}
		else if (control_kickout2_tag.Component == caller && control_lite107_tag.Component->MessageField == 15)
		{
			control_lite304_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
			control_mission_text_box_tag.Component->Display(pinball::get_rc_string(119, 0), 4.0, 1);
			int addedScore = SpecialAddScore(1000000);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(78, 0), addedScore);
			if (!AddRankProgress(8))
			{
				control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
				control_soundwave9_tag.Component->Play();
			}
		}
	}
	else
	{
		if (code == 66)
		{
			control_bpr_solotgt_lights_tag.Component->Message(20, 0.0);
			control_lite107_tag.Component->MessageField = 0;
			control_lite306_tag.Component->Message(7, 0.0);
		}
		else if (code == 67)
		{
			char* text;
			if (control_lite107_tag.Component->MessageField == 15)
				text = pinball::get_rc_string(118, 0);
			else
				text = pinball::get_rc_string(117, 0);
			control_mission_text_box_tag.Component->Display(text, -1.0, 1);
		}
	}
}

void control::TimeWarpController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite56_tag.Component->MessageField = 25;
			control_lite300_tag.Component->Message(7, 0.0);
			control_lite322_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(146, 0), control_lite56_tag.Component->MessageField);
		control_mission_text_box_tag.Component->Display(Buffer, -1.0, 1);
		return;
	}
	if (control_rebo1_tag.Component == caller
		|| control_rebo2_tag.Component == caller
		|| control_rebo3_tag.Component == caller
		|| control_rebo4_tag.Component == caller)
	{
		control_lite56_tag.Component->MessageField = control_lite56_tag.Component->MessageField - 1;
		if (control_lite56_tag.Component->MessageField)
		{
			MissionControl(67, caller);
		}
		else
		{
			control_lite300_tag.Component->Message(20, 0.0);
			control_lite322_tag.Component->Message(20, 0.0);
			control_lite198_tag.Component->MessageField = 24;
			MissionControl(66, nullptr);
		}
	}
}

void control::TimeWarpPartTwoController(int code, TPinballComponent* caller)
{
	char Buffer[64];

	if (code != 63)
	{
		if (code == 66)
		{
			control_lite55_tag.Component->Message(7, -1.0);
			control_lite26_tag.Component->Message(7, -1.0);
			control_lite304_tag.Component->Message(7, 0.0);
			control_lite317_tag.Component->Message(7, 0.0);
		}
		else if (code != 67)
		{
			return;
		}
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(147, 0), -1.0, 1);
		return;
	}
	if (control_kickout2_tag.Component == caller)
	{
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(47, 0), 4.0, 1);
		if (control_middle_circle_tag.Component->Message(37, 0.0) > 1)
		{
			control_middle_circle_tag.Component->Message(33, 5.0);
			int rank = control_middle_circle_tag.Component->Message(37, 0.0);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(174, 0), pinball::get_rc_string(RankRcArray[rank - 1], 1));
			control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
		}
	}
	else
	{
		if (control_ramp_tag.Component != caller)
			return;
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(46, 0), 4.0, 1);
		if (control_middle_circle_tag.Component->Message(37, 0.0) < 9)
		{
			int rank = control_middle_circle_tag.Component->Message(37, 0.0);
			control_middle_circle_tag.Component->Message(41, 5.0);
			snprintf(Buffer, sizeof Buffer, pinball::get_rc_string(173, 0), pinball::get_rc_string(RankRcArray[rank], 1));
		}
		if (!AddRankProgress(12))
		{
			control_mission_text_box_tag.Component->Display(Buffer, 8.0, 1);
			control_soundwave10_tag.Component->Play();
		}
	}
	SpecialAddScore(2000000);
	control_lite55_tag.Component->Message(20, 0.0);
	control_lite26_tag.Component->Message(20, 0.0);
	control_lite304_tag.Component->Message(20, 0.0);
	control_lite317_tag.Component->Message(20, 0.0);
	control_lite198_tag.Component->MessageField = 1;
	MissionControl(66, nullptr);
	// SpecialAddScore sets the score dirty flag. So next tick it will be redrawn.
}

void control::UnselectMissionController(int code, TPinballComponent* caller)
{
	control_lite198_tag.Component->MessageField = 1;
	MissionControl(66, nullptr);
}

void control::WaitingDeploymentController(int code, TPinballComponent* caller)
{
	switch (code)
	{
	case 63:
		if (control_oneway4_tag.Component == caller || control_oneway10_tag.Component == caller)
		{
			control_lite198_tag.Component->MessageField = 1;
			MissionControl(66, nullptr);
		}
		break;
	case 66:
		control_mission_text_box_tag.Component->Clear(1);
		waiting_deployment_flag = 0;
		break;
	case 67:
		control_mission_text_box_tag.Component->Display(pinball::get_rc_string(50, 0), -1.0, 1);
		break;
	default:
		break;
	}
}

// Light Debug Mode Implementation
static bool g_lightDebugMode = false;
static int g_currentLightIndex = 0;
static std::vector<std::pair<std::string, int>> g_lightList;

// Initialize the list of all available lights for debugging
void InitializeLightDebugList() {
    if (!g_lightList.empty()) return;
    
    // Add all light groups with their indices
    // This should match the light groups defined in make_links and HDR configs
    
    // Skill shot lights - 6 lights in plunger lane
    g_lightList.push_back({"skill_shot_lights", 0});
    g_lightList.push_back({"skill_shot_lights", 1});
    g_lightList.push_back({"skill_shot_lights", 2});
    g_lightList.push_back({"skill_shot_lights", 3});
    g_lightList.push_back({"skill_shot_lights", 4});
    g_lightList.push_back({"skill_shot_lights", 5});
    
    // Middle circle - 9 lights (inner orange ring)
    for (int i = 0; i < 9; i++) {
        g_lightList.push_back({"middle_circle", i});
    }
    
    // Outer circle - 18 lights (outer blue ring)
    for (int i = 0; i < 18; i++) {
        g_lightList.push_back({"outer_circle", i});
    }
    
    // Left trek lights - 2 lights in left ramp
    g_lightList.push_back({"l_trek_lights", 0});
    g_lightList.push_back({"l_trek_lights", 1});
    
    // Right trek lights - 2 lights in right ramp
    g_lightList.push_back({"r_trek_lights", 0});
    g_lightList.push_back({"r_trek_lights", 1});
    
    // Left chute target lights - 3 lights
    g_lightList.push_back({"lchute_tgt_lights", 0});
    g_lightList.push_back({"lchute_tgt_lights", 1});
    g_lightList.push_back({"lchute_tgt_lights", 2});
    
    // Goal lights - 3 lights
    g_lightList.push_back({"goal_lights", 0});
    g_lightList.push_back({"goal_lights", 1});
    g_lightList.push_back({"goal_lights", 2});
    
    // Hyperspace lights - 4 lights
    g_lightList.push_back({"hyperspace_lights", 0});
    g_lightList.push_back({"hyperspace_lights", 1});
    g_lightList.push_back({"hyperspace_lights", 2});
    g_lightList.push_back({"hyperspace_lights", 3});
    
    // Bumper increment lights - 3 lights
    g_lightList.push_back({"bmpr_inc_lights", 0});
    g_lightList.push_back({"bmpr_inc_lights", 1});
    g_lightList.push_back({"bmpr_inc_lights", 2});
    
    // Solo target lights - 3 lights
    g_lightList.push_back({"bpr_solotgt_lights", 0});
    g_lightList.push_back({"bpr_solotgt_lights", 1});
    g_lightList.push_back({"bpr_solotgt_lights", 2});
    
    // Ball sink arrow lights - 3 lights
    g_lightList.push_back({"bsink_arrow_lights", 0});
    g_lightList.push_back({"bsink_arrow_lights", 1});
    g_lightList.push_back({"bsink_arrow_lights", 2});
    
    // Bumper target lights - 3 lights
    g_lightList.push_back({"bumper_target_lights", 0});
    g_lightList.push_back({"bumper_target_lights", 1});
    g_lightList.push_back({"bumper_target_lights", 2});
    
    // Ramp bumper increment lights - 3 lights
    g_lightList.push_back({"ramp_bmpr_inc_lights", 0});
    g_lightList.push_back({"ramp_bmpr_inc_lights", 1});
    g_lightList.push_back({"ramp_bmpr_inc_lights", 2});
    
    // Ramp target lights - 3 lights
    g_lightList.push_back({"ramp_tgt_lights", 0});
    g_lightList.push_back({"ramp_tgt_lights", 1});
    g_lightList.push_back({"ramp_tgt_lights", 2});
    
    // Top circle target lights - 3 lights
    g_lightList.push_back({"top_circle_tgt_lights", 0});
    g_lightList.push_back({"top_circle_tgt_lights", 1});
    g_lightList.push_back({"top_circle_tgt_lights", 2});
    
    // Top target lights - 4 lights
    g_lightList.push_back({"top_target_lights", 0});
    g_lightList.push_back({"top_target_lights", 1});
    g_lightList.push_back({"top_target_lights", 2});
    g_lightList.push_back({"top_target_lights", 3});
    
    // Worm hole lights - 3 lights
    g_lightList.push_back({"worm_hole_lights", 0});
    g_lightList.push_back({"worm_hole_lights", 1});
    g_lightList.push_back({"worm_hole_lights", 2});
    
    // Individual lights - lite1-7
    g_lightList.push_back({"lite1", 0});
    g_lightList.push_back({"lite2", 0});
    g_lightList.push_back({"lite3", 0});
    g_lightList.push_back({"lite4", 0});
    g_lightList.push_back({"lite5", 0});
    g_lightList.push_back({"lite6", 0});
    g_lightList.push_back({"lite7", 0});
    g_lightList.push_back({"lite8", 0});
    g_lightList.push_back({"lite9", 0});
    g_lightList.push_back({"lite10", 0});
    g_lightList.push_back({"lite11", 0});
    g_lightList.push_back({"lite12", 0});
    g_lightList.push_back({"lite13", 0});
    g_lightList.push_back({"lite16", 0});
    g_lightList.push_back({"lite17", 0});
    g_lightList.push_back({"lite18", 0});
    g_lightList.push_back({"lite19", 0});
    g_lightList.push_back({"lite20", 0});
    g_lightList.push_back({"lite21", 0});
    g_lightList.push_back({"lite22", 0});
    g_lightList.push_back({"lite23", 0});
    g_lightList.push_back({"lite24", 0});
    g_lightList.push_back({"lite25", 0});
    g_lightList.push_back({"lite26", 0});
    g_lightList.push_back({"lite27", 0});
    g_lightList.push_back({"lite28", 0});
    g_lightList.push_back({"lite29", 0});
    g_lightList.push_back({"lite30", 0});
    g_lightList.push_back({"lite54", 0});
    g_lightList.push_back({"lite55", 0});
    g_lightList.push_back({"lite56", 0});
    g_lightList.push_back({"lite58", 0});
    g_lightList.push_back({"lite59", 0});
    g_lightList.push_back({"lite60", 0});
    g_lightList.push_back({"lite61", 0});
    g_lightList.push_back({"lite62", 0});
    g_lightList.push_back({"lite67", 0});
    g_lightList.push_back({"lite68", 0});
    g_lightList.push_back({"lite69", 0});
    g_lightList.push_back({"lite70", 0});
    g_lightList.push_back({"lite71", 0});
    g_lightList.push_back({"lite72", 0});
    g_lightList.push_back({"lite77", 0});
    g_lightList.push_back({"lite84", 0});
    g_lightList.push_back({"lite85", 0});
    g_lightList.push_back({"lite101", 0});
    g_lightList.push_back({"lite102", 0});
    g_lightList.push_back({"lite103", 0});
    g_lightList.push_back({"lite104", 0});
    g_lightList.push_back({"lite105", 0});
    g_lightList.push_back({"lite106", 0});
    g_lightList.push_back({"lite107", 0});
    g_lightList.push_back({"lite108", 0});
    g_lightList.push_back({"lite109", 0});
    g_lightList.push_back({"lite110", 0});
    g_lightList.push_back({"lite130", 0});
    g_lightList.push_back({"lite131", 0});
    g_lightList.push_back({"lite132", 0});
    g_lightList.push_back({"lite133", 0});
    g_lightList.push_back({"lite169", 0});
    g_lightList.push_back({"lite170", 0});
    g_lightList.push_back({"lite171", 0});
    g_lightList.push_back({"lite195", 0});
    g_lightList.push_back({"lite196", 0});
    g_lightList.push_back({"lite198", 0});
    g_lightList.push_back({"lite199", 0});
    g_lightList.push_back({"lite200", 0});
    g_lightList.push_back({"lite300", 0});
    g_lightList.push_back({"lite301", 0});
    g_lightList.push_back({"lite302", 0});
    g_lightList.push_back({"lite303", 0});
    g_lightList.push_back({"lite304", 0});
    g_lightList.push_back({"lite305", 0});
    g_lightList.push_back({"lite306", 0});
    g_lightList.push_back({"lite307", 0});
    g_lightList.push_back({"lite308", 0});
    g_lightList.push_back({"lite309", 0});
    g_lightList.push_back({"lite310", 0});
    g_lightList.push_back({"lite311", 0});
    g_lightList.push_back({"lite312", 0});
    g_lightList.push_back({"lite313", 0});
    g_lightList.push_back({"lite314", 0});
    g_lightList.push_back({"lite315", 0});
    g_lightList.push_back({"lite316", 0});
    g_lightList.push_back({"lite317", 0});
    g_lightList.push_back({"lite318", 0});
    g_lightList.push_back({"lite319", 0});
    g_lightList.push_back({"lite320", 0});
    g_lightList.push_back({"lite321", 0});
    g_lightList.push_back({"lite322", 0});
    g_lightList.push_back({"literoll179", 0});
    g_lightList.push_back({"literoll180", 0});
    g_lightList.push_back({"literoll181", 0});
    g_lightList.push_back({"literoll182", 0});
    g_lightList.push_back({"literoll183", 0});
    g_lightList.push_back({"literoll184", 0});
}

// Per-frame enforcement of light debug mode - turns off all NON-selected/toggled lights
void control_EnforceLightDebugMode() {
    if (!g_lightDebugMode || g_lightList.empty()) return;
    
    auto& selectedLight = g_lightList[g_currentLightIndex];
    std::string selectedGroup = selectedLight.first;
    int selectedIndex = selectedLight.second;
    
    // Map group names to components
    std::vector<std::pair<std::string, TPinballComponent*>> allGroups = {
        {"skill_shot_lights", control_skill_shot_lights_tag.Component},
        {"middle_circle", control_middle_circle_tag.Component},
        {"outer_circle", control_outer_circle_tag.Component},
        {"l_trek_lights", control_l_trek_lights_tag.Component},
        {"r_trek_lights", control_r_trek_lights_tag.Component},
        {"lchute_tgt_lights", control_lchute_tgt_lights_tag.Component},
        {"goal_lights", control_goal_lights_tag.Component},
        {"hyperspace_lights", control_hyper_lights_tag.Component},
        {"bmpr_inc_lights", control_bmpr_inc_lights_tag.Component},
        {"bpr_solotgt_lights", control_bpr_solotgt_lights_tag.Component},
        {"bsink_arrow_lights", control_bsink_arrow_lights_tag.Component},
        {"bumper_target_lights", control_bumber_target_lights_tag.Component},
        {"ramp_bmpr_inc_lights", control_ramp_bmpr_inc_lights_tag.Component},
        {"ramp_tgt_lights", control_ramp_tgt_lights_tag.Component},
        {"top_circle_tgt_lights", control_top_circle_tgt_lights_tag.Component},
        {"top_target_lights", control_top_target_lights_tag.Component},
        {"worm_hole_lights", control_worm_hole_lights_tag.Component}
    };
    
    for (auto& groupPair : allGroups) {
        std::string groupName = groupPair.first;
        TPinballComponent* comp = groupPair.second;
        if (!comp) continue;
        TLightGroup* group = dynamic_cast<TLightGroup*>(comp);
        if (!group) continue;
        
        // Turn off lights in this group EXCEPT if they match the selected light
        for (size_t i = 0; i < group->List.size(); i++) {
            TLight* light = group->List[i];
            if (!light) continue;
            
            // Skip if this is the currently selected light
            if (groupName == selectedGroup && static_cast<int>(i) == selectedIndex) {
                continue;  // Don't turn off the selected light
            }
            
            light->Message(0, 0.0);  // Turn off non-selected lights
        }
    }
}

// Check if light debug mode is active
bool control_IsLightDebugModeActive() {
    return g_lightDebugMode;
}

// Flag to temporarily allow light toggle commands to bypass the block
static bool g_lightDebugToggleAllowed = false;

bool control_IsLightDebugToggleAllowed() {
    return g_lightDebugToggleAllowed;
}

void control_SetLightDebugToggleAllowed(bool allowed) {
    g_lightDebugToggleAllowed = allowed;
}

// Get current selected light info for HDR overlay enforcement
void control_GetSelectedLightInfo(std::string& outGroupName, int& outLightIndex) {
    if (!g_lightDebugMode || g_lightList.empty()) {
        outGroupName = "";
        outLightIndex = -1;
        return;
    }
    auto& selectedLight = g_lightList[g_currentLightIndex];
    outGroupName = selectedLight.first;
    outLightIndex = selectedLight.second;
}

#ifdef __ANDROID__
extern "C" {
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_Settings_setLightDebugModeNative(JNIEnv* env, jobject obj, jboolean enabled) {
        g_lightDebugMode = enabled;
        if (enabled) {
            InitializeLightDebugList();
            g_currentLightIndex = 0;
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_Settings_nextLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        g_currentLightIndex = (g_currentLightIndex + 1) % g_lightList.size();
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_Settings_previousLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        g_currentLightIndex = (g_currentLightIndex - 1 + g_lightList.size()) % g_lightList.size();
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_Settings_toggleTableLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        // Find the corresponding light component and toggle it
        if (groupName == "lchute_tgt_lights" && control_lchute_tgt_lights_tag.Component) {
            control_lchute_tgt_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "goal_lights" && control_goal_lights_tag.Component) {
            control_goal_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "hyperspace_lights" && control_hyper_lights_tag.Component) {
            control_hyper_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "bmpr_inc_lights" && control_bmpr_inc_lights_tag.Component) {
            control_bmpr_inc_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "bpr_solotgt_lights" && control_bpr_solotgt_lights_tag.Component) {
            control_bpr_solotgt_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "bsink_arrow_lights" && control_bsink_arrow_lights_tag.Component) {
            control_bsink_arrow_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "bumper_target_lights" && control_bumber_target_lights_tag.Component) {
            control_bumber_target_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "l_trek_lights" && control_l_trek_lights_tag.Component) {
            control_l_trek_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "r_trek_lights" && control_r_trek_lights_tag.Component) {
            control_r_trek_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "ramp_bmpr_inc_lights" && control_ramp_bmpr_inc_lights_tag.Component) {
            control_ramp_bmpr_inc_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "ramp_tgt_lights" && control_ramp_tgt_lights_tag.Component) {
            control_ramp_tgt_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "top_circle_tgt_lights" && control_top_circle_tgt_lights_tag.Component) {
            control_top_circle_tgt_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "top_target_lights" && control_top_target_lights_tag.Component) {
            control_top_target_lights_tag.Component->Message(1, 0.0);
        } else if (groupName == "worm_hole_lights" && control_worm_hole_lights_tag.Component) {
            control_worm_hole_lights_tag.Component->Message(1, 0.0);
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_Settings_toggleHDRLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        // Toggle the corresponding HDR light using HDRLightOverlay
        HDRLightOverlay::ToggleDebugLight(groupName.c_str(), lightIndex);
    }
    
    JNIEXPORT jstring JNICALL Java_com_juiced_spacecadetpinball_Settings_getCurrentLightInfoNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) {
            return env->NewStringUTF("");
        }
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string info = currentLight.first + "[" + std::to_string(currentLight.second) + "]";
        return env->NewStringUTF(info.c_str());
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_Settings_setParticlesEnabledNative(JNIEnv* env, jobject obj, jboolean enabled) {
        options::Options.ParticlesEnabled = enabled;
    }

    // MainActivity versions of the same functions
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setLightDebugModeNative(JNIEnv* env, jobject obj, jboolean enabled) {
        g_lightDebugMode = enabled;
        if (enabled) {
            InitializeLightDebugList();
            g_currentLightIndex = 0;
        } else {
            // Clear debug toggled lights when exiting debug mode
            HDRLightOverlay::ClearDebugToggledLights();
        }
    }
    
    // Forward declarations for helper functions used by next/prev
    static TLightGroup* GetLightGroupByName(const std::string& groupName);
    static TLight* GetIndividualLightByName(const std::string& lightName);
    static bool g_tableLightDebugOn = false;
    
    // Helper to turn on the current light (both table and HDR)
    static void TurnOnCurrentLight() {
        if (g_lightList.empty()) return;
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        // Turn on HDR light
        HDRLightOverlay::ClearDebugToggledLights();
        HDRLightOverlay::ToggleDebugLight(groupName.c_str(), lightIndex);
        
        // Turn on table light
        TLight* light = nullptr;
        TLightGroup* group = GetLightGroupByName(groupName);
        if (group && lightIndex >= 0 && lightIndex < static_cast<int>(group->List.size())) {
            light = group->List[lightIndex];
        } else {
            light = GetIndividualLightByName(groupName);
        }
        if (light) {
            control_SetLightDebugToggleAllowed(true);
            light->Message(1, 0.0);  // Turn on
            control_SetLightDebugToggleAllowed(false);
            g_tableLightDebugOn = true;
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_nextLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        g_currentLightIndex = (g_currentLightIndex + 1) % g_lightList.size();
        TurnOnCurrentLight();
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_previousLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        g_currentLightIndex = (g_currentLightIndex - 1 + g_lightList.size()) % g_lightList.size();
        TurnOnCurrentLight();
    }
    
    // Helper to get TLightGroup by name
    static TLightGroup* GetLightGroupByName(const std::string& groupName) {
        if (groupName == "skill_shot_lights") return dynamic_cast<TLightGroup*>(control_skill_shot_lights_tag.Component);
        if (groupName == "middle_circle") return dynamic_cast<TLightGroup*>(control_middle_circle_tag.Component);
        if (groupName == "outer_circle") return dynamic_cast<TLightGroup*>(control_outer_circle_tag.Component);
        if (groupName == "l_trek_lights") return dynamic_cast<TLightGroup*>(control_l_trek_lights_tag.Component);
        if (groupName == "r_trek_lights") return dynamic_cast<TLightGroup*>(control_r_trek_lights_tag.Component);
        if (groupName == "lchute_tgt_lights") return dynamic_cast<TLightGroup*>(control_lchute_tgt_lights_tag.Component);
        if (groupName == "goal_lights") return dynamic_cast<TLightGroup*>(control_goal_lights_tag.Component);
        if (groupName == "hyperspace_lights") return dynamic_cast<TLightGroup*>(control_hyper_lights_tag.Component);
        if (groupName == "bmpr_inc_lights") return dynamic_cast<TLightGroup*>(control_bmpr_inc_lights_tag.Component);
        if (groupName == "bpr_solotgt_lights") return dynamic_cast<TLightGroup*>(control_bpr_solotgt_lights_tag.Component);
        if (groupName == "bsink_arrow_lights") return dynamic_cast<TLightGroup*>(control_bsink_arrow_lights_tag.Component);
        if (groupName == "bumper_target_lights") return dynamic_cast<TLightGroup*>(control_bumber_target_lights_tag.Component);
        if (groupName == "ramp_bmpr_inc_lights") return dynamic_cast<TLightGroup*>(control_ramp_bmpr_inc_lights_tag.Component);
        if (groupName == "ramp_tgt_lights") return dynamic_cast<TLightGroup*>(control_ramp_tgt_lights_tag.Component);
        if (groupName == "top_circle_tgt_lights") return dynamic_cast<TLightGroup*>(control_top_circle_tgt_lights_tag.Component);
        if (groupName == "top_target_lights") return dynamic_cast<TLightGroup*>(control_top_target_lights_tag.Component);
        if (groupName == "worm_hole_lights") return dynamic_cast<TLightGroup*>(control_worm_hole_lights_tag.Component);
        return nullptr;
    }
    
    // Helper to get individual TLight by name
    static TLight* GetIndividualLightByName(const std::string& lightName) {
        if (lightName == "lite1") return control_lite1_tag.Component;
        if (lightName == "lite2") return control_lite2_tag.Component;
        if (lightName == "lite3") return control_lite3_tag.Component;
        if (lightName == "lite4") return control_lite4_tag.Component;
        if (lightName == "lite5") return control_lite5_tag.Component;
        if (lightName == "lite6") return control_lite6_tag.Component;
        if (lightName == "lite7") return control_lite7_tag.Component;
        if (lightName == "lite8") return control_lite8_tag.Component;
        if (lightName == "lite9") return control_lite9_tag.Component;
        if (lightName == "lite10") return control_lite10_tag.Component;
        if (lightName == "lite11") return control_lite11_tag.Component;
        if (lightName == "lite12") return control_lite12_tag.Component;
        if (lightName == "lite13") return control_lite13_tag.Component;
        if (lightName == "lite16") return control_lite16_tag.Component;
        if (lightName == "lite17") return control_lite17_tag.Component;
        if (lightName == "lite18") return control_lite18_tag.Component;
        if (lightName == "lite19") return control_lite19_tag.Component;
        if (lightName == "lite20") return control_lite20_tag.Component;
        if (lightName == "lite21") return control_lite21_tag.Component;
        if (lightName == "lite22") return control_lite22_tag.Component;
        if (lightName == "lite23") return control_lite23_tag.Component;
        if (lightName == "lite24") return control_lite24_tag.Component;
        if (lightName == "lite25") return control_lite25_tag.Component;
        if (lightName == "lite26") return control_lite26_tag.Component;
        if (lightName == "lite27") return control_lite27_tag.Component;
        if (lightName == "lite28") return control_lite28_tag.Component;
        if (lightName == "lite29") return control_lite29_tag.Component;
        if (lightName == "lite30") return control_lite30_tag.Component;
        if (lightName == "lite54") return control_lite54_tag.Component;
        if (lightName == "lite55") return control_lite55_tag.Component;
        if (lightName == "lite56") return control_lite56_tag.Component;
        if (lightName == "lite58") return control_lite58_tag.Component;
        if (lightName == "lite59") return control_lite59_tag.Component;
        if (lightName == "lite60") return control_lite60_tag.Component;
        if (lightName == "lite61") return control_lite61_tag.Component;
        if (lightName == "lite62") return control_lite62_tag.Component;
        if (lightName == "lite67") return control_lite67_tag.Component;
        if (lightName == "lite68") return control_lite68_tag.Component;
        if (lightName == "lite69") return control_lite69_tag.Component;
        if (lightName == "lite70") return control_lite70_tag.Component;
        if (lightName == "lite71") return control_lite71_tag.Component;
        if (lightName == "lite72") return control_lite72_tag.Component;
        if (lightName == "lite77") return control_lite77_tag.Component;
        if (lightName == "lite84") return control_lite84_tag.Component;
        if (lightName == "lite85") return control_lite85_tag.Component;
        if (lightName == "lite101") return control_lite101_tag.Component;
        if (lightName == "lite102") return control_lite102_tag.Component;
        if (lightName == "lite103") return control_lite103_tag.Component;
        if (lightName == "lite104") return control_lite104_tag.Component;
        if (lightName == "lite105") return control_lite105_tag.Component;
        if (lightName == "lite106") return control_lite106_tag.Component;
        if (lightName == "lite107") return control_lite107_tag.Component;
        if (lightName == "lite108") return control_lite108_tag.Component;
        if (lightName == "lite109") return control_lite109_tag.Component;
        if (lightName == "lite110") return control_lite110_tag.Component;
        if (lightName == "lite130") return control_lite130_tag.Component;
        if (lightName == "lite131") return control_lite131_tag.Component;
        if (lightName == "lite132") return control_lite132_tag.Component;
        if (lightName == "lite133") return control_lite133_tag.Component;
        if (lightName == "lite169") return control_lite169_tag.Component;
        if (lightName == "lite170") return control_lite170_tag.Component;
        if (lightName == "lite171") return control_lite171_tag.Component;
        if (lightName == "lite195") return control_lite195_tag.Component;
        if (lightName == "lite196") return control_lite196_tag.Component;
        if (lightName == "lite198") return control_lite198_tag.Component;
        if (lightName == "lite199") return control_lite199_tag.Component;
        if (lightName == "lite200") return control_lite200_tag.Component;
        if (lightName == "lite300") return control_lite300_tag.Component;
        if (lightName == "lite301") return control_lite301_tag.Component;
        if (lightName == "lite302") return control_lite302_tag.Component;
        if (lightName == "lite303") return control_lite303_tag.Component;
        if (lightName == "lite304") return control_lite304_tag.Component;
        if (lightName == "lite305") return control_lite305_tag.Component;
        if (lightName == "lite306") return control_lite306_tag.Component;
        if (lightName == "lite307") return control_lite307_tag.Component;
        if (lightName == "lite308") return control_lite308_tag.Component;
        if (lightName == "lite309") return control_lite309_tag.Component;
        if (lightName == "lite310") return control_lite310_tag.Component;
        if (lightName == "lite311") return control_lite311_tag.Component;
        if (lightName == "lite312") return control_lite312_tag.Component;
        if (lightName == "lite313") return control_lite313_tag.Component;
        if (lightName == "lite314") return control_lite314_tag.Component;
        if (lightName == "lite315") return control_lite315_tag.Component;
        if (lightName == "lite316") return control_lite316_tag.Component;
        if (lightName == "lite317") return control_lite317_tag.Component;
        if (lightName == "lite318") return control_lite318_tag.Component;
        if (lightName == "lite319") return control_lite319_tag.Component;
        if (lightName == "lite320") return control_lite320_tag.Component;
        if (lightName == "lite321") return control_lite321_tag.Component;
        if (lightName == "lite322") return control_lite322_tag.Component;
        if (lightName == "literoll179") return control_literoll179_tag.Component;
        if (lightName == "literoll180") return control_literoll180_tag.Component;
        if (lightName == "literoll181") return control_literoll181_tag.Component;
        if (lightName == "literoll182") return control_literoll182_tag.Component;
        if (lightName == "literoll183") return control_literoll183_tag.Component;
        if (lightName == "literoll184") return control_literoll184_tag.Component;
        return nullptr;
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_toggleTableLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        TLight* light = nullptr;
        
        // First try to get as a light group
        TLightGroup* group = GetLightGroupByName(groupName);
        if (group && lightIndex >= 0 && lightIndex < static_cast<int>(group->List.size())) {
            light = group->List[lightIndex];
        } else {
            // Try as individual light
            light = GetIndividualLightByName(groupName);
        }
        
        if (light) {
            // Toggle: check current state and flip it
            // Message(2, 0.0) returns BmpIndex1 (current on/off state)
            int currentState = light->Message(2, 0.0);
            // Temporarily allow the toggle command to bypass the block
            control_SetLightDebugToggleAllowed(true);
            // Message(0, 0.0) turns off, Message(1, 0.0) turns on
            light->Message(currentState ? 0 : 1, 0.0);
            control_SetLightDebugToggleAllowed(false);
            g_tableLightDebugOn = !currentState;
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_toggleHDRLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        HDRLightOverlay::ToggleDebugLight(groupName.c_str(), lightIndex);
    }
    
    JNIEXPORT jstring JNICALL Java_com_juiced_spacecadetpinball_MainActivity_getCurrentLightInfoNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) {
            return env->NewStringUTF("");
        }
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string info = currentLight.first + "[" + std::to_string(currentLight.second) + "]";
        return env->NewStringUTF(info.c_str());
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_turnOffAllLightsNative(JNIEnv* env, jobject obj) {
        // Turn off all light groups using Message(0, 0.0) which turns lights off
        // Message code 0 sets BmpIndex1 = 0 on each light in the group
        if (control_lchute_tgt_lights_tag.Component) control_lchute_tgt_lights_tag.Component->Message(0, 0.0);
        if (control_goal_lights_tag.Component) control_goal_lights_tag.Component->Message(0, 0.0);
        if (control_hyper_lights_tag.Component) control_hyper_lights_tag.Component->Message(0, 0.0);
        if (control_bmpr_inc_lights_tag.Component) control_bmpr_inc_lights_tag.Component->Message(0, 0.0);
        if (control_bpr_solotgt_lights_tag.Component) control_bpr_solotgt_lights_tag.Component->Message(0, 0.0);
        if (control_bsink_arrow_lights_tag.Component) control_bsink_arrow_lights_tag.Component->Message(0, 0.0);
        if (control_bumber_target_lights_tag.Component) control_bumber_target_lights_tag.Component->Message(0, 0.0);
        if (control_l_trek_lights_tag.Component) control_l_trek_lights_tag.Component->Message(0, 0.0);
        if (control_r_trek_lights_tag.Component) control_r_trek_lights_tag.Component->Message(0, 0.0);
        if (control_ramp_bmpr_inc_lights_tag.Component) control_ramp_bmpr_inc_lights_tag.Component->Message(0, 0.0);
        if (control_ramp_tgt_lights_tag.Component) control_ramp_tgt_lights_tag.Component->Message(0, 0.0);
        if (control_top_circle_tgt_lights_tag.Component) control_top_circle_tgt_lights_tag.Component->Message(0, 0.0);
        if (control_top_target_lights_tag.Component) control_top_target_lights_tag.Component->Message(0, 0.0);
        if (control_worm_hole_lights_tag.Component) control_worm_hole_lights_tag.Component->Message(0, 0.0);
        if (control_skill_shot_lights_tag.Component) control_skill_shot_lights_tag.Component->Message(0, 0.0);
    }
    
    // Debug light repositioning - moves the currently selected HDR light to touch position
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_onDebugLightTouchDown(JNIEnv* env, jobject obj, jfloat screenX, jfloat screenY, jint viewportX, jint viewportY, jint viewportW, jint viewportH) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        // Convert screen coordinates to normalized (0-1) coordinates
        float normX = screenX / (float)viewportW;
        float normY = screenY / (float)viewportH;
        
        // Get the currently selected light
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        // Find the config index for this light and update its position
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        for (size_t i = 0; i < configs.size(); i++) {
            if (strcmp(configs[i].GroupName, groupName.c_str()) == 0 && configs[i].LightIndex == lightIndex) {
                HDRLightOverlay::UpdateLightPosition(static_cast<int>(i), normX, normY);
                break;
            }
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_onDebugLightTouchMove(JNIEnv* env, jobject obj, jfloat screenX, jfloat screenY, jint viewportX, jint viewportY, jint viewportW, jint viewportH) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        // Convert screen coordinates to normalized (0-1) coordinates
        float normX = screenX / (float)viewportW;
        float normY = screenY / (float)viewportH;
        
        // Get the currently selected light
        auto& currentLight = g_lightList[g_currentLightIndex];
        std::string groupName = currentLight.first;
        int lightIndex = currentLight.second;
        
        // Find the config index for this light and update its position
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        for (size_t i = 0; i < configs.size(); i++) {
            if (strcmp(configs[i].GroupName, groupName.c_str()) == 0 && configs[i].LightIndex == lightIndex) {
                HDRLightOverlay::UpdateLightPosition(static_cast<int>(i), normX, normY);
                break;
            }
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_onDebugLightTouchUp(JNIEnv* env, jobject obj) {
        // Nothing special needed on touch up - position is already updated
    }
    
    // ============== Preset Management JNI Functions ==============
    
    JNIEXPORT jint JNICALL Java_com_juiced_spacecadetpinball_MainActivity_getPresetCountNative(JNIEnv* env, jobject obj) {
        return HDRLightOverlay::GetPresetCount();
    }
    
    JNIEXPORT jstring JNICALL Java_com_juiced_spacecadetpinball_MainActivity_getPresetNameNative(JNIEnv* env, jobject obj, jint index) {
        const char* name = HDRLightOverlay::GetPresetNameByIndex(index);
        if (name) {
            return env->NewStringUTF(name);
        }
        return env->NewStringUTF("");
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_createPresetFromCurrentLightNative(JNIEnv* env, jobject obj, jstring presetName) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        const char* name = env->GetStringUTFChars(presetName, nullptr);
        if (!name) return;
        
        // Get the HDR config index for the current light
        auto& currentLight = g_lightList[g_currentLightIndex];
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        int configIndex = -1;
        for (size_t i = 0; i < configs.size(); i++) {
            if (configs[i].GroupName == currentLight.first && configs[i].LightIndex == currentLight.second) {
                configIndex = (int)i;
                break;
            }
        }
        
        if (configIndex >= 0) {
            HDRLightPreset preset = HDRLightOverlay::CreatePresetFromLight(configIndex, name);
            HDRLightOverlay::AddPreset(preset);
        }
        
        env->ReleaseStringUTFChars(presetName, name);
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_applyPresetToCurrentLightNative(JNIEnv* env, jobject obj, jstring presetName) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        const char* name = env->GetStringUTFChars(presetName, nullptr);
        if (!name) return;
        
        // Get the HDR config index for the current light
        auto& currentLight = g_lightList[g_currentLightIndex];
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        int configIndex = -1;
        for (size_t i = 0; i < configs.size(); i++) {
            if (configs[i].GroupName == currentLight.first && configs[i].LightIndex == currentLight.second) {
                configIndex = (int)i;
                break;
            }
        }
        
        if (configIndex >= 0) {
            HDRLightOverlay::AssignPresetToLight(configIndex, name);
        }
        
        env->ReleaseStringUTFChars(presetName, name);
    }
    
    JNIEXPORT jstring JNICALL Java_com_juiced_spacecadetpinball_MainActivity_getCurrentLightPresetNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) {
            return env->NewStringUTF("");
        }
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        for (const auto& config : configs) {
            if (config.GroupName == currentLight.first && config.LightIndex == currentLight.second) {
                return env->NewStringUTF(config.PresetName.c_str());
            }
        }
        return env->NewStringUTF("");
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_clearPresetFromCurrentLightNative(JNIEnv* env, jobject obj) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        for (size_t i = 0; i < configs.size(); i++) {
            if (configs[i].GroupName == currentLight.first && configs[i].LightIndex == currentLight.second) {
                HDRLightOverlay::ClearPresetFromLight((int)i);
                break;
            }
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_savePresetsNative(JNIEnv* env, jobject obj, jstring filepath) {
        const char* path = env->GetStringUTFChars(filepath, nullptr);
        if (path) {
            HDRLightOverlay::SavePresets(path);
            env->ReleaseStringUTFChars(filepath, path);
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_loadPresetsNative(JNIEnv* env, jobject obj, jstring filepath) {
        const char* path = env->GetStringUTFChars(filepath, nullptr);
        if (path) {
            HDRLightOverlay::LoadPresets(path);
            env->ReleaseStringUTFChars(filepath, path);
        }
    }
    
    // ============== Live Property Editing JNI Functions ==============
    
    JNIEXPORT jfloatArray JNICALL Java_com_juiced_spacecadetpinball_MainActivity_getCurrentLightPropertiesNative(JNIEnv* env, jobject obj) {
        // Returns: [r, g, b, width, height, intensityOn, intensityFlash, glowRadius, aboveBall, x, y, locked]
        jfloatArray result = env->NewFloatArray(12);
        if (!g_lightDebugMode || g_lightList.empty()) {
            float zeros[12] = {0};
            env->SetFloatArrayRegion(result, 0, 12, zeros);
            return result;
        }
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        for (const auto& config : configs) {
            if (config.GroupName == currentLight.first && config.LightIndex == currentLight.second) {
                float props[12] = {
                    config.R, config.G, config.B,
                    config.Width, config.Height,
                    config.IntensityOn, config.IntensityFlash,
                    config.GlowRadius,
                    config.AboveBall ? 1.0f : 0.0f,
                    config.X, config.Y,
                    config.Locked ? 1.0f : 0.0f
                };
                env->SetFloatArrayRegion(result, 0, 12, props);
                return result;
            }
        }
        
        float zeros[12] = {0};
        env->SetFloatArrayRegion(result, 0, 12, zeros);
        return result;
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setCurrentLightColorNative(JNIEnv* env, jobject obj, jfloat r, jfloat g, jfloat b) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        HDRLightOverlay::UpdateLightColor(currentLight.first.c_str(), currentLight.second, r, g, b);
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setCurrentLightSizeNative(JNIEnv* env, jobject obj, jfloat width, jfloat height) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        const auto& configs = HDRLightOverlay::GetLightConfigs();
        for (size_t i = 0; i < configs.size(); i++) {
            if (configs[i].GroupName == currentLight.first && configs[i].LightIndex == currentLight.second) {
                HDRLightOverlay::UpdateLightSize((int)i, width, height);
                break;
            }
        }
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setCurrentLightIntensityNative(JNIEnv* env, jobject obj, jfloat intensityOn, jfloat intensityFlash) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        HDRLightOverlay::UpdateLightIntensity(currentLight.first.c_str(), currentLight.second, intensityOn, intensityFlash);
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setCurrentLightGlowNative(JNIEnv* env, jobject obj, jfloat glowRadius) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        HDRLightOverlay::UpdateLightGlow(currentLight.first.c_str(), currentLight.second, glowRadius);
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_nudgeCurrentLightNative(JNIEnv* env, jobject obj, jfloat dx, jfloat dy) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        HDRLightOverlay::NudgeLight(currentLight.first.c_str(), currentLight.second, dx, dy);
    }
    
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setCurrentLightLockedNative(JNIEnv* env, jobject obj, jboolean locked) {
        if (!g_lightDebugMode || g_lightList.empty()) return;
        
        auto& currentLight = g_lightList[g_currentLightIndex];
        HDRLightOverlay::UpdateLightLocked(currentLight.first.c_str(), currentLight.second, locked);
    }
    
    // Audio ready flag - blocks game startup until audio is loaded
    JNIEXPORT void JNICALL Java_com_juiced_spacecadetpinball_MainActivity_setAudioReadyNative(JNIEnv* env, jobject obj, jboolean ready) {
        pb::audioReady = ready;
    }
}
#endif // __ANDROID__
