#include "pch.h"
#include "loader.h"
#include "GroupData.h"
#include "pb.h"
#include "pinball.h"
#include "Sound.h"
#include "zdrv.h"
#include "options.h"

// Original sound durations (in seconds) for game timing when using enhanced audio
// Enhanced audio files may be longer (due to reverb, etc.) but game logic should use original timing
static const std::map<std::string, float> originalSoundDurations = {
	{"SOUND1.WAV", 5.020408f},
	{"SOUND3.WAV", 2.060590f},
	{"SOUND4.WAV", 1.484807f},
	{"SOUND5.WAV", 0.265215f},
	{"SOUND6.WAV", 0.380136f},
	{"SOUND7.WAV", 2.385669f},
	{"SOUND8.WAV", 0.177868f},
	{"SOUND9.WAV", 1.810249f},
	{"SOUND12.WAV", 0.376961f},
	{"SOUND13.WAV", 0.712018f},
	{"SOUND14.WAV", 0.259592f},
	{"SOUND16.WAV", 0.082177f},
	{"SOUND17.WAV", 0.176871f},
	{"SOUND18.WAV", 0.348753f},
	{"SOUND19.WAV", 0.461587f},
	{"SOUND20.WAV", 0.771791f},
	{"SOUND21.WAV", 0.821224f},
	{"SOUND22.WAV", 0.656236f},
	{"SOUND24.WAV", 1.085351f},
	{"SOUND25.WAV", 2.318730f},
	{"SOUND26.WAV", 0.649977f},
	{"SOUND27.WAV", 1.823311f},
	{"SOUND28.WAV", 0.771882f},
	{"SOUND29.WAV", 0.927347f},
	{"SOUND30.WAV", 2.034467f},
	{"SOUND34.WAV", 0.125170f},
	{"SOUND35.WAV", 1.755828f},
	{"SOUND36.WAV", 3.057324f},
	{"SOUND38.WAV", 1.168617f},
	{"SOUND39.WAV", 2.552472f},
	{"SOUND42.WAV", 2.630295f},
	{"SOUND43.WAV", 2.054966f},
	{"SOUND45.WAV", 0.873469f},
	{"SOUND49.WAV", 0.157460f},
	{"SOUND49D.WAV", 0.289252f},
	{"SOUND50.WAV", 1.082449f},
	{"SOUND53.WAV", 0.801633f},
	{"SOUND54.WAV", 1.642630f},
	{"SOUND55.WAV", 1.972698f},
	{"SOUND57.WAV", 2.753832f},
	{"SOUND58.WAV", 0.289705f},
	{"SOUND65.WAV", 1.586485f},
	{"SOUND68.WAV", 2.924082f},
	{"SOUND104.WAV", 0.098503f},
	{"SOUND105.WAV", 0.165805f},
	{"SOUND108.WAV", 0.680091f},
	{"SOUND111.WAV", 0.063946f},
	{"SOUND112.WAV", 0.058050f},
	{"SOUND131.WAV", 0.104308f},
	{"SOUND136.WAV", 1.732245f},
	{"SOUND181.WAV", 2.472744f},
	{"SOUND240.WAV", 1.307483f},
	{"SOUND243.WAV", 1.861950f},
	{"SOUND528.WAV", 0.793469f},
	{"SOUND560.WAV", 2.612789f},
	{"SOUND563.WAV", 2.176236f},
	{"SOUND713.WAV", 1.305578f},
	{"SOUND735.WAV", 2.455692f},
	{"SOUND827.WAV", 4.267211f},
	{"SOUND999.WAV", 0.603447f},
};

errorMsg loader::loader_errors[] =
{
	errorMsg{0, "Bad Handle"},
	errorMsg{1, "No Type Field"},
	errorMsg{2, "No Attributes Field"},
	errorMsg{3, "Wrong Type: MATERIAL Expected"},
	errorMsg{4, "Wrong Type: KICKER Expected"},
	errorMsg{5, "Wrong Type: AN_OBJECT Expected"},
	errorMsg{6, "Wrong Type: A_STATE Expected"},
	errorMsg{7, "STATES (re)defined in a state"},
	errorMsg{9, "Unrecognized Attribute"},
	errorMsg{0x0A, "Unrecognized float Attribute"},
	errorMsg{0x0B, "No float Attributes Field"},
	errorMsg{0x0D, "float Attribute not found"},
	errorMsg{0x0C, "state_index out of range"},
	errorMsg{0x0F, "loader_material() reports failure"},
	errorMsg{0x0E, "loader_kicker() reports failure"},
	errorMsg{0x10, "loader_state_id() reports failure"},
	errorMsg{0x8, "# walls doesn't match data size"},
	errorMsg{0x11, "loader_query_visual_states()"},
	errorMsg{0x12, "loader_query_visual()"},
	errorMsg{0x15, "loader_material()"},
	errorMsg{0x14, "loader_kicker()"},
	errorMsg{0x16, "loader_query_attribute()"},
	errorMsg{0x17, "loader_query_iattribute()"},
	errorMsg{0x13, "loader_query_name()"},
	errorMsg{0x18, "loader_state_id()"},
	errorMsg{0x19, "loader_get_sound_id()"},
	errorMsg{0x1A, "sound reference is not A_SOUND record"},
	errorMsg{-1, "Unknown"},
};

int loader::sound_count = 1;
int loader::loader_sound_count;
DatFile* loader::loader_table;
DatFile* loader::sound_record_table;
soundListStruct loader::sound_list[65];

int loader::error(int errorCode, int captionCode)
{
	auto curCode = loader_errors;
	const char *errorText = nullptr, *errorCaption = nullptr;
	auto index = 0;
	while (curCode->Code >= 0)
	{
		if (errorCode == curCode->Code)
			errorText = curCode->Message;
		if (captionCode == curCode->Code)
			errorCaption = curCode->Message;
		curCode++;
		index++;
	}

	if (!errorText)
		errorText = loader_errors[index].Message;
	SpaceCadetPinballJNI::show_error_dialog(errorCaption, errorText);
	return -1;
}

void loader::default_vsi(visualStruct* visual)
{
	visual->CollisionGroup = 0;
	visual->Kicker.Threshold = 8.9999999e10f;
	visual->Kicker.HardHitSoundId = 0;
	visual->Smoothness = 0.94999999f;
	visual->Elasticity = 0.60000002f;
	visual->FloatArrCount = 0;
	visual->SoftHitSoundId = 0;
	visual->Bitmap = nullptr;
	visual->ZMap = nullptr;
	visual->SoundIndex3 = 0;
	visual->SoundIndex4 = 0;
}

void loader::loadfrom(DatFile* datFile)
{
	loader_table = datFile;
	sound_record_table = loader_table;

	for (auto groupIndex = 0; groupIndex < static_cast<int>(datFile->Groups.size()); ++groupIndex)
	{
		auto value = reinterpret_cast<int16_t*>(datFile->field(groupIndex, FieldTypes::ShortValue));
		if (value && *value == 202)
		{
			if (sound_count < 65)
			{
				sound_list[sound_count] = {nullptr, groupIndex, 0, 0};
				sound_count++;
			}
		}
	}
	loader_sound_count = sound_count;
}

void loader::unload()
{
	for (int index = 1; index < sound_count; ++index)
	{
		Sound::FreeSound(sound_list[index].WavePtr);
		sound_list[index] = {};
	}

	sound_count = 1;
}

int loader::get_sound_id(int groupIndex)
{
	int16_t soundIndex = 1;
	if (sound_count <= 1)
	{
		error(25, 26);
		return -1;
	}

	while (sound_list[soundIndex].GroupIndex != groupIndex)
	{
		++soundIndex;
		if (soundIndex >= sound_count)
		{
			error(25, 26);
			return -1;
		}
	}

	if (!sound_list[soundIndex].Loaded && !sound_list[soundIndex].WavePtr)
	{
		int soundGroupId = sound_list[soundIndex].GroupIndex;
		sound_list[soundIndex].Duration = 0.0;
		if (soundGroupId > 0 && !pinball::quickFlag)
		{
			auto value = reinterpret_cast<int16_t*>(loader_table->field(soundGroupId,
			                                                            FieldTypes::ShortValue));
			if (value && *value == 202)
			{
				std::string fileName = loader_table->field(soundGroupId, FieldTypes::String);

				// File name is in lower case, while game data is in upper case.				
				std::transform(fileName.begin(), fileName.end(), fileName.begin(),
				               [](unsigned char c) { return std::toupper(c); });
				// if (pb::FullTiltMode)
				// {
				// 	// FT sounds are in SOUND subfolder
				// 	fileName.insert(0, 1, PathSeparator);
				// 	fileName.insert(0, "SOUND");
				// }

				float duration = -1;
				std::string filePath;
				
				SDL_Log("Loading sound: %s, EnhancedAudio option: %d", fileName.c_str(), options::Options.EnhancedAudio ? 1 : 0);
				
				// Check for enhanced audio if option is enabled
				if (options::Options.EnhancedAudio)
				{
					// Try enhanced version first: enhanced/SOUNDXX_enhanced.wav
					std::string baseName = fileName.substr(0, fileName.find_last_of('.'));
					std::string enhancedFileName = "enhanced" + std::string(1, PathSeparator) + baseName + "_enhanced.wav";
					auto enhancedPath = pinball::make_path_name(enhancedFileName);
					auto enhancedFile = fopen(enhancedPath.c_str(), "rb");
					if (enhancedFile)
					{
						fclose(enhancedFile);
						filePath = enhancedPath;
						SDL_Log("Enhanced audio loaded: %s", enhancedPath.c_str());
					}
					else
					{
						SDL_Log("Enhanced audio not found: %s (using original)", enhancedPath.c_str());
					}
				}
				
				// Fall back to original file if enhanced not found or not enabled
				if (filePath.empty())
				{
					filePath = pinball::make_path_name(fileName);
				}
				
				auto file = fopen(filePath.c_str(), "rb");
				if (file)
				{
					// Robust WAV parsing: find fmt and data chunks dynamically
					// This handles WAV files with extra chunks (JUNK, LIST, etc.) before fmt/data
					unsigned char header[12];
					if (fread(header, 1, 12, file) == 12 &&
					    memcmp(header, "RIFF", 4) == 0 &&
					    memcmp(header + 8, "WAVE", 4) == 0)
					{
						unsigned short channels = 0;
						unsigned int sample_rate = 0;
						unsigned short bits_per_sample = 0;
						unsigned int data_size = 0;
						bool found_fmt = false;
						bool found_data = false;
						
						// Parse chunks until we find both fmt and data
						while (!found_fmt || !found_data)
						{
							unsigned char chunkHeader[8];
							if (fread(chunkHeader, 1, 8, file) != 8)
								break;
							
							unsigned int chunkSize = chunkHeader[4] | (chunkHeader[5] << 8) | 
							                         (chunkHeader[6] << 16) | (chunkHeader[7] << 24);
							
							if (memcmp(chunkHeader, "fmt ", 4) == 0)
							{
								// Read fmt chunk data
								unsigned char fmtData[16];
								if (fread(fmtData, 1, 16, file) == 16)
								{
									channels = fmtData[2] | (fmtData[3] << 8);
									sample_rate = fmtData[4] | (fmtData[5] << 8) | 
									             (fmtData[6] << 16) | (fmtData[7] << 24);
									bits_per_sample = fmtData[14] | (fmtData[15] << 8);
									found_fmt = true;
								}
								// Skip remaining fmt chunk data if any
								if (chunkSize > 16)
									fseek(file, chunkSize - 16, SEEK_CUR);
							}
							else if (memcmp(chunkHeader, "data", 4) == 0)
							{
								data_size = chunkSize;
								found_data = true;
								// Don't need to read actual audio data
								break;
							}
							else
							{
								// Skip unknown chunk (JUNK, LIST, etc.)
								// Chunks are word-aligned, so round up to even size
								fseek(file, (chunkSize + 1) & ~1, SEEK_CUR);
							}
						}
						
						if (found_fmt && found_data && channels > 0 && sample_rate > 0 && bits_per_sample > 0)
						{
							auto sampleCount = data_size / (channels * (bits_per_sample / 8.0));
							duration = static_cast<float>(sampleCount / sample_rate);
						}
					}
					fclose(file);
				}

				// When using enhanced audio, use original duration for game timing
				// This ensures game events happen at the right time even if enhanced audio is longer
				if (options::Options.EnhancedAudio)
				{
					auto it = originalSoundDurations.find(fileName);
					if (it != originalSoundDurations.end())
					{
						duration = it->second;
						SDL_Log("Using original duration %.3fs for %s (enhanced audio)", duration, fileName.c_str());
					}
				}
				
				sound_list[soundIndex].Duration = duration;
				sound_list[soundIndex].WavePtr = Sound::LoadWaveFile(filePath);
			}
		}
	}

	++sound_list[soundIndex].Loaded;
	return soundIndex;
}


int loader::query_handle(LPCSTR lpString)
{
	return loader_table->record_labeled(lpString);
}

short loader::query_visual_states(int groupIndex)
{
	short result;
	if (groupIndex < 0)
		return error(0, 17);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortArray));
	if (shortArr && *shortArr == 100)
		result = shortArr[1];
	else
		result = 1;
	return result;
}

char* loader::query_name(int groupIndex)
{
	if (groupIndex < 0)
	{
		error(0, 19);
		return nullptr;
	}

	return loader_table->field(groupIndex, FieldTypes::GroupName);
}

int16_t* loader::query_iattribute(int groupIndex, int firstValue, int* arraySize)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return nullptr;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto shortArr = reinterpret_cast<int16_t*>(loader_table->field_nth(groupIndex,
		                                                                   FieldTypes::ShortArray, skipIndex));
		if (!shortArr)
			break;
		if (*shortArr == firstValue)
		{
			*arraySize = loader_table->field_size(groupIndex, FieldTypes::ShortArray) / 2 - 1;
			return shortArr + 1;
		}
	}

	error(2, 23);
	*arraySize = 0;
	return nullptr;
}

float* loader::query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return nullptr;
	}

	int stateId = state_id(groupIndex, groupIndexOffset);
	if (stateId < 0)
	{
		error(16, 22);
		return nullptr;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto floatArr = reinterpret_cast<float*>(loader_table->field_nth(stateId, FieldTypes::FloatArray,
		                                                                 skipIndex));
		if (!floatArr)
			break;
		if (static_cast<int16_t>(floor(*floatArr)) == firstValue)
			return floatArr + 1;
	}

	error(13, 22);
	return nullptr;
}

float loader::query_float_attribute(int groupIndex, int groupIndexOffset, int firstValue, float defVal)
{
	if (groupIndex < 0)
	{
		error(0, 22);
		return NAN;
	}

	int stateId = state_id(groupIndex, groupIndexOffset);
	if (stateId < 0)
	{
		error(16, 22);
		return NAN;
	}

	for (auto skipIndex = 0;; ++skipIndex)
	{
		auto floatArr = reinterpret_cast<float*>(loader_table->field_nth(stateId,
		                                                                 FieldTypes::FloatArray, skipIndex));
		if (!floatArr)
			break;
		if (static_cast<int16_t>(floor(*floatArr)) == firstValue)
			return floatArr[1];
	}

	if (!isnan(defVal))
		return defVal;
	error(13, 22);
	return NAN;
}

int loader::material(int groupIndex, visualStruct* visual)
{
	if (groupIndex < 0)
		return error(0, 21);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 21);
	if (*shortArr != 300)
		return error(3, 21);
	auto floatArr = reinterpret_cast<float*>(loader_table->field(groupIndex, FieldTypes::FloatArray));
	if (!floatArr)
		return error(11, 21);

	int floatArrLength = loader_table->field_size(groupIndex, FieldTypes::FloatArray) / 4;
	for (auto index = 0; index < floatArrLength; index += 2)
	{
		switch (static_cast<int>(floor(floatArr[index])))
		{
		case 301:
			visual->Smoothness = floatArr[index + 1];
			break;
		case 302:
			visual->Elasticity = floatArr[index + 1];
			break;
		case 304:
			visual->SoftHitSoundId = get_sound_id(static_cast<int>(floor(floatArr[index + 1])));
			break;
		default:
			return error(9, 21);
		}
	}
	return 0;
}


float loader::play_sound(int soundIndex)
{
	if (soundIndex <= 0)
		return 0.0;
	Sound::PlaySound(sound_list[soundIndex].WavePtr, pb::time_ticks);
	return sound_list[soundIndex].Duration;
}

int loader::state_id(int groupIndex, int groupIndexOffset)
{
	auto visualState = query_visual_states(groupIndex);
	if (visualState <= 0)
		return error(12, 24);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 200)
		return error(5, 24);
	if (groupIndexOffset > visualState)
		return error(12, 24);
	if (!groupIndexOffset)
		return groupIndex;

	groupIndex += groupIndexOffset;
	shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 24);
	if (*shortArr != 201)
		return error(6, 24);
	return groupIndex;
}

int loader::kicker(int groupIndex, visualKickerStruct* kicker)
{
	if (groupIndex < 0)
		return error(0, 20);
	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(groupIndex, FieldTypes::ShortValue));
	if (!shortArr)
		return error(1, 20);
	if (*shortArr != 400)
		return error(4, 20);
	auto floatArr = reinterpret_cast<float*>(loader_table->field(groupIndex, FieldTypes::FloatArray));
	if (!floatArr)
		return error(11, 20);
	int floatArrLength = loader_table->field_size(groupIndex, FieldTypes::FloatArray) / 4;
	if (floatArrLength <= 0)
		return 0;

	for (auto index = 0; index < floatArrLength;)
	{
		int floorVal = static_cast<int>(floor(*floatArr++));
		switch (floorVal)
		{
		case 401:
			kicker->Threshold = *floatArr;
			break;
		case 402:
			kicker->Boost = *floatArr;
			break;
		case 403:
			kicker->ThrowBallMult = *floatArr;
			break;
		case 404:
			kicker->ThrowBallAcceleration = *reinterpret_cast<vector3*>(floatArr);
			floatArr += 3;
			index += 4;
			break;
		case 405:
			kicker->ThrowBallAngleMult = *floatArr;
			break;
		case 406:
			kicker->HardHitSoundId = get_sound_id(static_cast<int>(floor(*floatArr)));
			break;
		default:
			return error(10, 20);
		}
		if (floorVal != 404)
		{
			floatArr++;
			index += 2;
		}
	}
	return 0;
}


int loader::query_visual(int groupIndex, int groupIndexOffset, visualStruct* visual)
{
	default_vsi(visual);
	if (groupIndex < 0)
		return error(0, 18);
	auto stateId = state_id(groupIndex, groupIndexOffset);
	if (stateId < 0)
		return error(16, 18);

	visual->Bitmap = loader_table->GetBitmap(stateId);
	visual->ZMap = loader_table->GetZMap(stateId);

	auto shortArr = reinterpret_cast<int16_t*>(loader_table->field(stateId, FieldTypes::ShortArray));
	if (shortArr)
	{
		unsigned int shortArrSize = loader_table->field_size(stateId, FieldTypes::ShortArray);
		for (auto index = 0u; index < shortArrSize / 2;)
		{
			switch (shortArr[0])
			{
			case 100:
				if (groupIndexOffset)
					return error(7, 18);
				break;
			case 300:
				if (material(shortArr[1], visual))
					return error(15, 18);
				break;
			case 304:
				visual->SoftHitSoundId = get_sound_id(shortArr[1]);
				break;
			case 400:
				if (kicker(shortArr[1], &visual->Kicker))
					return error(14, 18);
				break;
			case 406:
				visual->Kicker.HardHitSoundId = get_sound_id(shortArr[1]);
				break;
			case 602:
				visual->CollisionGroup |= 1 << shortArr[1];
				break;
			case 1100:
				visual->SoundIndex4 = get_sound_id(shortArr[1]);
				break;
			case 1101:
				visual->SoundIndex3 = get_sound_id(shortArr[1]);
				break;
			case 1500:
				shortArr += 7;
				index += 7;
				break;
			default:
				return error(9, 18);
			}
			shortArr += 2;
			index += 2;
		}
	}

	if (!visual->CollisionGroup)
		visual->CollisionGroup = 1;
	auto floatArr = reinterpret_cast<float*>(loader_table->field(stateId, FieldTypes::FloatArray));
	if (!floatArr)
		return 0;
	if (*floatArr != 600.0f)
		return 0;

	visual->FloatArrCount = loader_table->field_size(stateId, FieldTypes::FloatArray) / 4 / 2 - 2;
	auto floatVal = static_cast<int>(floor(floatArr[1]) - 1.0f);
	switch (floatVal)
	{
	case 0:
		visual->FloatArrCount = 1;
		break;
	case 1:
		visual->FloatArrCount = 2;
		break;
	default:
		if (floatVal != visual->FloatArrCount)
			return error(8, 18);
		break;
	}

	visual->FloatArr = floatArr + 2;
	return 0;
}
