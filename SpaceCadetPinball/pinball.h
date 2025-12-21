#pragma once

#include <unordered_map>
#include <string>

class TTextBox;

class pinball
{
public:
	static int quickFlag;
	static TTextBox* InfoTextBox;
	static TTextBox* MissTextBox;
	static int RightShift;
	static int LeftShift;
	static std::unordered_map<uint32_t, std::string> rc_strings;

	static char* get_rc_string(int uID, int a2);
	static void set_rc_string(int uID, LPCSTR str);
	static int get_rc_int(int uID, int* dst);
	static std::string make_path_name(const std::string& fileName);
	
	// Plunger control functions for drag-based system
	static void set_plunger_position(float position);
	static void set_plunger_launch_power(float power);

	// Getter functions for drag-based control (internal use)
	friend float get_plunger_position();
	friend float get_plunger_launch_power();
	friend bool is_using_drag_control();
	friend void reset_drag_control();
	
	// Direct visual update function
	static void update_plunger_visual(float position);
	
	// Power curve transformation function
	static float apply_power_curve(float position);
private:
	static char getRcBuffer[256 * 6];
	static int rc_string_slot;
};
