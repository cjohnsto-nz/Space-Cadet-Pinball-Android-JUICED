#pragma once
class nudge
{
public:
	static void un_nudge_right(int timerId, void* caller);
	static void un_nudge_left(int timerId, void* caller);
	static void un_nudge_up(int timerId, void* caller);
	static void nudge_right();
	static void nudge_left();
	static void nudge_up();
	static void end_cooldown(int timerId, void* caller);
	static bool jolt(float x, float y);
	static void reset_jolt_state();

	static int nudged_left;
	static int nudged_right;
	static int nudged_up;
	static float nudge_count;
	static bool in_cooldown;
private:
	static void _nudge(float x, float y);
	static int timer;
	static int cooldown_timer;
	static constexpr float COOLDOWN_DURATION = 0.33f;
	static constexpr float JOLT_TILT_INCREMENT = 1.0f;
};
