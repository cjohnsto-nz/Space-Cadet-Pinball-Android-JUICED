#include "pch.h"
#include "winmain.h"

#include "control.h"
#include "fullscrn.h"
#include "midi.h"
#include "pinball.h"
#include "options.h"
#include "pb.h"
#include "render.h"
#include "Sound.h"
#include "HDRConfig.h"
#include "HDRRenderer.h"
#include "HDRLightOverlay.h"
#include "TPinballTable.h"
#include "TBall.h"
#include "proj.h"
#include "../app/src/main/cpp/SpaceCadetPinballJNI.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

SDL_Window* winmain::MainWindow = nullptr;
SDL_Renderer* winmain::Renderer = nullptr;

int winmain::return_value = 0;
bool winmain::bQuit = false;
bool winmain::activated = false;
int winmain::DispFrameRate = 0;
int winmain::DispGRhistory = 0;
bool winmain::single_step = false;
bool winmain::has_focus = true;
int winmain::last_mouse_x;
int winmain::last_mouse_y;
int winmain::mouse_down;
bool winmain::no_time_loss = false;

bool winmain::restart = false;

gdrv_bitmap8* winmain::gfr_display = nullptr;
std::string winmain::DatFileName;
bool winmain::ShowAboutDialog = false;
bool winmain::ShowImGuiDemo = false;
bool winmain::ShowSpriteViewer = false;
bool winmain::LaunchBallEnabled = true;
bool winmain::HighScoresEnabled = true;
bool winmain::DemoActive = false;
char* winmain::BasePath;
int winmain::MainMenuHeight = 0; 
std::string winmain::FpsDetails;
double winmain::UpdateToFrameRatio;
winmain::DurationMs winmain::TargetFrameTime;
optionsStruct& winmain::Options = options::Options;

int winmain::WinMain(LPCSTR lpCmdLine)
{
	SpaceCadetPinballJNI::gameReady();
	restart = false;
	bQuit = false;

	std::set_new_handler(memalloc_failure);

	// SDL init
	SDL_SetMainReady();
	
	// Set HDR hint before SDL_Init if HDR is enabled
	if (HDR::IsHDRActive())
	{
		SDL_SetHint("SDL_VIDEO_EGL_HDR", "1");
		SDL_Log("HDR: Enabled SDL_VIDEO_EGL_HDR hint for HDR colorspace");
	}
	
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		SpaceCadetPinballJNI::show_error_dialog("Could not initialize SDL2", SDL_GetError());
		return 1;
	}

	pinball::quickFlag = strstr(lpCmdLine, "-quick") != nullptr;
	DatFileName = options::get_string("Pinball Data", pinball::get_rc_string(168, 0));

	/*Check for full tilt .dat file and switch to it automatically*/
	auto cadetFilePath = pinball::make_path_name("CADET.DAT");
	auto cadetDat = fopen(cadetFilePath.c_str(), "r");
	if (cadetDat)
	{
		fclose(cadetDat);
		DatFileName = "CADET.DAT";
		pb::FullTiltMode = true;
	}
		pb::FullTiltMode = true;
	// SDL window
	SDL_Window* window = SDL_CreateWindow
	(
		pinball::get_rc_string(38, 0),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		800, 556,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE
	);
	MainWindow = window;
	if (!window)
	{
		SpaceCadetPinballJNI::show_error_dialog("Could not create window", SDL_GetError());
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer
	(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
	);
	Renderer = renderer;
	if (!renderer)
	{
		SpaceCadetPinballJNI::show_error_dialog("Could not create renderer", SDL_GetError());
		return 1;
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	// PB init from message handler
	{
		options::init();
		if (!Sound::Init(Options.SoundChannels, Options.Sounds, Options.SoundVolume))
			Options.Sounds = false;

		if (!pinball::quickFlag && !midi::music_init(Options.MusicVolume))
			Options.Music = false;

		if (pb::init())
		{
			SpaceCadetPinballJNI::show_error_dialog("Could not load game data",
			                         "The .dat file is missing");
			return 1;
		}

		fullscrn::init();
	}

	pb::reset_table();
	pb::firsttime_setup();

	if (strstr(lpCmdLine, "-fullscreen"))
	{
		Options.FullScreen = true;
	}

	SDL_ShowWindow(window);
	fullscrn::set_screen_mode(Options.FullScreen);

	if (strstr(lpCmdLine, "-demo"))
		pb::toggle_demo();
	else
		pb::replay_level(false);

	unsigned dtHistoryCounter = 300u, updateCounter = 0, frameCounter = 0;

	auto frameStart = Clock::now();
	double UpdateToFrameCounter = 0;
	DurationMs sleepRemainder(0), frameDuration(TargetFrameTime);
	auto prevTime = frameStart;
	while (true)
	{
		if (DispFrameRate)
		{
			auto curTime = Clock::now();
			if (curTime - prevTime > DurationMs(1000))
			{
				char buf[60];
				auto elapsedSec = DurationMs(curTime - prevTime).count() * 0.001;
				snprintf(buf, sizeof buf, "Updates/sec = %02.02f Frames/sec = %02.02f ",
				         updateCounter / elapsedSec, frameCounter / elapsedSec);
				SDL_SetWindowTitle(window, buf);
				FpsDetails = buf;
				frameCounter = updateCounter = 0;
				prevTime = curTime;
			}
		}

		if (DispGRhistory)
		{
			if (!gfr_display)
			{
				auto plt = static_cast<ColorRgba*>(malloc(1024u));
				auto pltPtr = &plt[10]; // first 10 entries are system colors hardcoded in display_palette()
				for (int i1 = 0, i2 = 0; i1 < 256 - 10; ++i1, i2 += 8)
				{
					unsigned char blue = i2, redGreen = i2;
					if (i2 > 255)
					{
						blue = 255;
						redGreen = i1;
					}

					*pltPtr++ = ColorRgba{Rgba{redGreen, redGreen, blue, 0}};
				}
				gdrv::display_palette(plt);
				free(plt);
				gfr_display = new gdrv_bitmap8(400, 15, false);
			}

			if (!dtHistoryCounter)
			{
				dtHistoryCounter = 300;
				gdrv::copy_bitmap(render::vscreen, 300, 10, 0, 30, gfr_display, 0, 0);
				gdrv::fill_bitmap(gfr_display, 300, 10, 0, 0, 0);
			}
		}

		if (!ProcessWindowMessages() || bQuit)
			break;

		if (has_focus)
		{
			if (mouse_down)
			{
				int x, y, w, h;
				SDL_GetMouseState(&x, &y);
				SDL_GetWindowSize(window, &w, &h);
				float dx = static_cast<float>(last_mouse_x - x) / static_cast<float>(w);
				float dy = static_cast<float>(y - last_mouse_y) / static_cast<float>(h);
				pb::ballset(dx, dy);

				SDL_WarpMouseInWindow(window, last_mouse_x, last_mouse_y);

				// Mouse warp does not work over remote desktop or in some VMs
				//last_mouse_x = x;
				//last_mouse_y = y;
			}
			if (!single_step && !no_time_loss)
			{
				// Check for demo mode toggle request from UI thread
				if (SpaceCadetPinballJNI::shouldToggleDemo()) {
					pb::toggle_demo();
				}
				
				auto dt = static_cast<float>(frameDuration.count());
				auto dtWhole = static_cast<int>(std::round(dt));
				
				// Debug: Log frame timing every 30 frames for better resolution
				static int frameLogCounter = 0;
				static float maxDt = 0;
				static float minDt = 9999.0f;
				static float avgDt = 0;
				static float dtHistory[30];
				static int dtIndex = 0;
				
				// Track min/max/avg
				if (dt > maxDt) maxDt = dt;
				if (dt < minDt) minDt = dt;
				dtHistory[dtIndex] = dt;
				dtIndex = (dtIndex + 1) % 30;
				
				if (++frameLogCounter >= 30) {
					// Calculate average from history
					avgDt = 0;
					float targetMs = static_cast<float>(TargetFrameTime.count());
					float slowThreshold = targetMs * 1.5f;  // 50% over target
					float verySlowThreshold = targetMs * 2.0f;  // 100% over target
					int fastFrames = 0, okFrames = 0, slowFrames = 0, verySlowFrames = 0;
					for (int i = 0; i < 30; i++) {
						avgDt += dtHistory[i];
						if (dtHistory[i] <= targetMs) fastFrames++;
						else if (dtHistory[i] <= slowThreshold) okFrames++;
						else if (dtHistory[i] <= verySlowThreshold) slowFrames++;
						else verySlowFrames++;
					}
					avgDt /= 30.0f;
					
					// Always log for now to get detailed data
					__android_log_print(ANDROID_LOG_WARN, "FrameTiming", 
						"Frame timing - Avg: %.1fms, Min: %.1fms, Max: %.1fms (target: %.1fms)", 
						avgDt, minDt, maxDt, targetMs);
					
					// Log distribution using target-relative thresholds
					__android_log_print(ANDROID_LOG_WARN, "FrameDist", 
						"Frame distribution - <=%.1fms: %d, <=%.1fms: %d, <=%.1fms: %d, >%.1fms: %d", 
						targetMs, fastFrames, slowThreshold, okFrames, verySlowThreshold, slowFrames, verySlowThreshold, verySlowFrames);
					
					frameLogCounter = 0;
					maxDt = 0;
					minDt = 9999.0f;
				}
				
				pb::frame(dt);
				
				// Measure time spent in ball position tracking
				auto ballTrackStart = Clock::now();
				
				// Update debug ball position from first active ball in table
				if (pb::MainTable && !pb::MainTable->BallList.empty()) {
					TBall* activeBall = nullptr;
					for (auto* ball : pb::MainTable->BallList) {
						if (ball && ball->ActiveFlag) {
							activeBall = ball;
							break;
						}
					}
					
					if (activeBall) {
						// Convert 3D ball position to 2D screen coordinates using proj
						// Use float version for subpixel precision (smoother trail)
						float pos2D[2];
						proj::xform_to_2d_float(&activeBall->Position, pos2D);
						
						// Get vscreen dimensions for texture coordinate conversion
						float vscreenWidth = static_cast<float>(render::vscreen->Width);
						float vscreenHeight = static_cast<float>(render::vscreen->Height);
						
						// Ball position from proj is in table pixel coordinates
						// Normalize to texture coordinates (0-1 range of full vscreen/texture)
						float normX = pos2D[0] / vscreenWidth;
						float normY = pos2D[1] / vscreenHeight;
						
						// Debug logging disabled
						
						// Set debug ball position
						HDRLightOverlay::SetDebugBallPosition(normX, normY);
						HDRLightOverlay::SetBallValid(true);
					} else {
						// No active ball - ball is in teleporter or inactive
						HDRLightOverlay::SetBallValid(false);
						HDRLightOverlay::DecayTrail();  // Keep trail decaying even when ball inactive
					}
				} else {
					// No table - draw at center for testing
					HDRLightOverlay::SetDebugBallPosition(0.3f, 0.5f);
				}
				
				// Log ball tracking time periodically
				auto ballTrackEnd = Clock::now();
				static float maxBallTrackMs = 0;
				float ballTrackMs = DurationMs(ballTrackEnd - ballTrackStart).count();
				if (ballTrackMs > maxBallTrackMs) maxBallTrackMs = ballTrackMs;
				static int ballTrackLogCounter = 0;
				if (++ballTrackLogCounter >= 240) {
					if (maxBallTrackMs > 0.5f) {
						__android_log_print(ANDROID_LOG_WARN, "BallTrack", "Max ball tracking time: %.2fms", maxBallTrackMs);
					}
					ballTrackLogCounter = 0;
					maxBallTrackMs = 0;
				}
				
				if (gfr_display)
				{
					auto deltaTPal = dtWhole + 10;
					auto fillChar = static_cast<uint8_t>(deltaTPal);
					if (deltaTPal > 236)
					{
						fillChar = 1;
					}
					gdrv::fill_bitmap(gfr_display, 1, 10, 300 - dtHistoryCounter, 0, fillChar);
					--dtHistoryCounter;
				}
				updateCounter++;
			}
			no_time_loss = false;

			if (UpdateToFrameCounter >= UpdateToFrameRatio)
			{
				auto renderStart = Clock::now();
				
				SDL_RenderClear(renderer);
                // Alternative clear hack, clear might fail on some systems
                // Todo: remove original clear, if save for all platforms
                SDL_RenderFillRect(renderer, nullptr);
				render::PresentVScreen();

				auto presentStart = Clock::now();
				
				// When using HDR, we use raw GL and need to swap buffers ourselves
				// SDL_RenderPresent would overwrite our GL output
				if (HDRRenderer::ShouldUseHDR())
				{
					SDL_GL_SwapWindow(MainWindow);
				}
				else
				{
					SDL_RenderPresent(renderer);
				}
				
				auto swapEnd = Clock::now();
				
				// Log render timing periodically
				static float maxRenderMs = 0, maxSwapMs = 0;
				float renderMs = DurationMs(presentStart - renderStart).count();
				float swapMs = DurationMs(swapEnd - presentStart).count();
				if (renderMs > maxRenderMs) maxRenderMs = renderMs;
				if (swapMs > maxSwapMs) maxSwapMs = swapMs;
				static int renderLogCounter = 0;
				if (++renderLogCounter >= 120) {
					__android_log_print(ANDROID_LOG_WARN, "RenderTiming", 
						"Render: %.1fms max, Swap: %.1fms max", maxRenderMs, maxSwapMs);
					renderLogCounter = 0;
					maxRenderMs = 0;
					maxSwapMs = 0;
				}
				
				frameCounter++;
				UpdateToFrameCounter -= UpdateToFrameRatio;
			}

			auto sdlError = SDL_GetError();
			if (sdlError[0])
			{
				SDL_ClearError();
				printf("SDL Error: %s\n", sdlError);
			}

			auto updateEnd = Clock::now();
			auto targetTimeDelta = TargetFrameTime - DurationMs(updateEnd - frameStart) - sleepRemainder;

			TimePoint frameEnd;
			if (targetTimeDelta > DurationMs::zero() && !Options.UncappedUpdatesPerSecond)
			{
				std::this_thread::sleep_for(targetTimeDelta);
				frameEnd = Clock::now();
				sleepRemainder = DurationMs(frameEnd - updateEnd) - targetTimeDelta;
			}
			else
			{
				frameEnd = updateEnd;
				sleepRemainder = DurationMs(0);
			}

			// Limit duration to 2 * target time
			frameDuration = std::min<DurationMs>(DurationMs(frameEnd - frameStart), 2 * TargetFrameTime);
			frameStart = frameEnd;
			UpdateToFrameCounter++;
		}
	}

	delete gfr_display;
	options::uninit();
	midi::music_shutdown();
	pb::uninit();
	Sound::Close();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return return_value;
}

int winmain::event_handler(const SDL_Event* event)
{
	switch (event->type)
	{
	case SDL_QUIT:
		end_pause();
		bQuit = true;
		fullscrn::shutdown();
		return_value = 0;
		return 0;
	case SDL_KEYUP:
		pb::InputUp({InputTypes::Keyboard, event->key.keysym.sym});
		break;
	case SDL_KEYDOWN:
		if (!event->key.repeat)
			pb::InputDown({InputTypes::Keyboard, event->key.keysym.sym});
		switch (event->key.keysym.sym)
		{
		case SDLK_ESCAPE:
			if (Options.FullScreen)
				options::toggle(Menu1::Full_Screen);
			SDL_MinimizeWindow(MainWindow);
			break;
		case SDLK_F2:
			new_game();
			break;
		case SDLK_F3:
			pause();
			break;
		case SDLK_F4:
			options::toggle(Menu1::Full_Screen);
			break;
		case SDLK_F5:
			options::toggle(Menu1::Sounds);
			break;
		case SDLK_F6:
			options::toggle(Menu1::Music);
			break;
		case SDLK_F8:
			if (!single_step)
				pause();
			options::ShowControlDialog();
			break;
		case SDLK_F9:
			options::toggle(Menu1::Show_Menu);
			break;
		default:
			break;
		}

		if (!pb::cheat_mode)
			break;

		switch (event->key.keysym.sym)
		{
		case SDLK_g:
			DispGRhistory = 1;
			break;
		case SDLK_y:
			SDL_SetWindowTitle(MainWindow, "Pinball");
			DispFrameRate = DispFrameRate == 0;
			break;
		case SDLK_F1:
			pb::frame(10);
			break;
		case SDLK_F10:
			single_step ^= true;
			if (!single_step)
				no_time_loss = true;
			break;
		default:
			break;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		{
			bool noInput = false;
			switch (event->button.button)
			{
			case SDL_BUTTON_LEFT:
				if (pb::cheat_mode)
				{
					mouse_down = 1;
					last_mouse_x = event->button.x;
					last_mouse_y = event->button.y;
					SDL_SetWindowGrab(MainWindow, SDL_TRUE);
					noInput = true;
				}
				break;
			default:
				break;
			}

			if (!noInput)
				pb::InputDown({InputTypes::Mouse, event->button.button});
		}
		break;
	case SDL_MOUSEBUTTONUP:
		{
			bool noInput = false;
			switch (event->button.button)
			{
			case SDL_BUTTON_LEFT:
				if (mouse_down)
				{
					mouse_down = 0;
					SDL_SetWindowGrab(MainWindow, SDL_FALSE);
					noInput = true;
				}
				break;
			default:
				break;
			}

			if (!noInput)
				pb::InputUp({InputTypes::Mouse, event->button.button});
		}
		break;
	case SDL_WINDOWEVENT:
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_TAKE_FOCUS:
		case SDL_WINDOWEVENT_SHOWN:
			activated = true;
			Sound::Activate();
			if (Options.Music && !single_step)
				midi::play_pb_theme();
			no_time_loss = true;
			has_focus = true;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_HIDDEN:
			activated = false;
			fullscrn::activate(0);
			Options.FullScreen = false;
			Sound::Deactivate();
			midi::music_stop();
			has_focus = false;
			pb::loose_focus();
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED:
			fullscrn::window_size_changed();
			break;
		default: ;
		}
		break;
	case SDL_JOYDEVICEADDED:
		if (SDL_IsGameController(event->jdevice.which))
		{
			SDL_GameControllerOpen(event->jdevice.which);
		}
		break;
	case SDL_JOYDEVICEREMOVED:
		{
			SDL_GameController* controller = SDL_GameControllerFromInstanceID(event->jdevice.which);
			if (controller)
			{
				SDL_GameControllerClose(controller);
			}
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		pb::InputDown({InputTypes::GameController, event->cbutton.button});
		switch (event->cbutton.button)
		{
            case SDL_CONTROLLER_BUTTON_START:
                pause();
                break;
            case SDL_CONTROLLER_BUTTON_BACK:
                if (single_step)
                {
                    SDL_Event event{ SDL_QUIT };
                    SDL_PushEvent(&event);
                }
                break;
            default:;
		}
		break;
	case SDL_CONTROLLERBUTTONUP:
		pb::InputUp({InputTypes::GameController, event->cbutton.button});
		break;
	default: ;
	}

	return 1;
}

int winmain::ProcessWindowMessages()
{
	static auto idleWait = 0;
	SDL_Event event;
	if (has_focus && !single_step)
	{
		idleWait = static_cast<int>(TargetFrameTime.count());
		while (SDL_PollEvent(&event))
		{
			if (!event_handler(&event))
				return 0;
		}

		return 1;
	}

	// Progressively wait longer when transitioning to idle
	idleWait = std::min(idleWait + static_cast<int>(TargetFrameTime.count()), 500);
	if (SDL_WaitEventTimeout(&event, idleWait))
	{
		idleWait = static_cast<int>(TargetFrameTime.count());
		return event_handler(&event);
	}
	return 1;
}

void winmain::memalloc_failure()
{
	midi::music_stop();
	Sound::Close();
	char* caption = pinball::get_rc_string(170, 0);
	char* text = pinball::get_rc_string(179, 0);
	SpaceCadetPinballJNI::show_error_dialog(caption, text);
	std::exit(1);
}

void winmain::end_pause()
{
	if (single_step)
	{
		pb::pause_continue();
		no_time_loss = true;
	}
}

void winmain::new_game()
{
	end_pause();
	pb::replay_level(false);
}

void winmain::pause(bool toggle)
{
    if (toggle || !single_step)
    {
        pb::pause_continue();
        no_time_loss = true;
    }
}

void winmain::Restart()
{
	restart = true;
	SDL_Event event{SDL_QUIT};
	SDL_PushEvent(&event);
}

void winmain::UpdateFrameRate()
{
	// UPS >= FPS
	auto fps = Options.FramesPerSecond, ups = Options.UpdatesPerSecond;
	UpdateToFrameRatio = static_cast<double>(ups) / fps;
	TargetFrameTime = DurationMs(1000.0 / ups);
}
