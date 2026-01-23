#include "handmade.h"
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include <xinput.h>
#include <dsound.h>

typedef uint32_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t   u8;

typedef int32_t b32;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t   i8;

typedef float f32;
typedef double f64;

typedef struct 
{
    f32 x;
    f32 y;
} vec2;

typedef struct 
{
    f32 x;
    f32 y;
    f32 z;
} vec3;

#define Pi32 3.14159265359f
#define local_persist static
#define global_variable static
#define internal static

#include "handmade.cpp"



#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);




#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct win32_sound_output
{
    int ToneHz;
    int SamplesPerSecond;
    u32 RunningSampleIndex;
    i16 ToneVolume;
    int WavePeriod;
    int BytesPerSample;
    int SecondaryBufferSize;
    f32 tSine;
    int LatencySampleCount;
};

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimensions
{
    int Width;
    int Height;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


internal void
Win32LoadDSound(HWND Window, i32 SamplesPerSecond, i32 BufferSize)
{
    HMODULE DSoundLib = LoadLibrary("dsound.dll");
    if (DSoundLib) 
    {
	direct_sound_create *DirectSoundCreate = (direct_sound_create*) GetProcAddress(DSoundLib, "DirectSoundCreate");
	LPDIRECTSOUND DirectSound;
	if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
	{
	    WAVEFORMATEX WaveFormat = {};
	    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	    WaveFormat.nChannels = 2;
	    WaveFormat.nSamplesPerSec = SamplesPerSecond;
	    WaveFormat.wBitsPerSample = 16;
	    WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
	    WaveFormat.nAvgBytesPerSec =  WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
	    WaveFormat.cbSize = 0;

	    if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
	    {
		// Dicen que tenemos que zerear la memoria. 
		DSBUFFERDESC BufferDescription = {};
		BufferDescription.dwSize = sizeof(BufferDescription);
		BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

		LPDIRECTSOUNDBUFFER PrimaryBuffer;
		if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
		{
		    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
		    if (SUCCEEDED(Error))
		    {
			OutputDebugStringA("PrimaryBuffer Was set!");
		    }
		    else
		    {

		    }
		}
		else
		{

		}
	    }
	    else
	    {

	    }
	    // TODO: Create second buffer
	    DSBUFFERDESC BufferDescription = {};
	    BufferDescription.dwSize = sizeof(BufferDescription);
	    BufferDescription.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	    BufferDescription.dwBufferBytes = BufferSize;
	    BufferDescription.lpwfxFormat = &WaveFormat;

	    HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
	    if(SUCCEEDED(Error))
	    {
		OutputDebugStringA("SecondaryBuffer Was set!");
	    }
	    else
	    {

	    }

	    // TODO: Start playing
	}
    }
    else
    {


    }

}


internal void
Win32LoadXInput(void)
{
    HMODULE XInputLib = LoadLibrary("xinput1_4.dll");
    if (!XInputLib)
    {
	XInputLib = LoadLibrary("xinput1_3.dll");
    }
    if (XInputLib)
    {
	XInputGetState = (x_input_get_state*)GetProcAddress(XInputLib, "XInputGetState");
	XInputSetState = (x_input_set_state*)GetProcAddress(XInputLib, "XInputSetState");
    }
}


internal win32_window_dimensions
Win32GetWindowDimensions(HWND Window)
{
    win32_window_dimensions Dimensions;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);

    Dimensions.Width = ClientRect.right - ClientRect.left;
    Dimensions.Height = ClientRect.bottom - ClientRect.top;
    
    return Dimensions;
}


internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
	VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height; // This is negative so DIB will be top down.
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    

    int BitmapMemorySize = Buffer->BytesPerPixel * Width * Height;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}

internal void
Win32DisplayBufferWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer)
{
    StretchDIBits(
	DeviceContext, 
	0, 0, WindowWidth, WindowHeight,
	0, 0, Buffer.Width, Buffer.Height,
	Buffer.Memory, &Buffer.Info,
	DIB_RGB_COLORS, SRCCOPY
    );
}   


internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, 
				&Region1, &Region1Size, 
				&Region2, &Region2Size, 
				0)))
    {
	i16 *SampleOut = (i16*) Region1;
	DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
	for (DWORD SampleIndex = 0;
	    SampleIndex< Region1SampleCount;
	    ++SampleIndex)
	{
	    f32 SineValue = sinf(SoundOutput->tSine);
	    i16 SampleValue = (i32)(SineValue * SoundOutput->ToneVolume);
	    *SampleOut++=SampleValue;
	    *SampleOut++=SampleValue;

	    SoundOutput->tSine += 2.0f * Pi32 * 1.0 / (f32)SoundOutput->WavePeriod;
	    ++SoundOutput->RunningSampleIndex;
	}

	DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
	SampleOut = (i16*) Region2;
	for (DWORD SampleIndex = 0;
	    SampleIndex< Region2SampleCount;
	    ++SampleIndex)
	{
	    f32 SineValue = sinf(SoundOutput->tSine);
	    i16 SampleValue = (i32)(SineValue * SoundOutput->ToneVolume);
	    *SampleOut++=SampleValue;
	    *SampleOut++=SampleValue;

	    SoundOutput->tSine += 2.0f * Pi32 * 1.0 / (f32)SoundOutput->WavePeriod;
	    ++SoundOutput->RunningSampleIndex;
	}

	GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}	



LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
		   UINT Message,
		   WPARAM WParam,
		   LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
	case WM_SIZE:
	{
	} break;

	case WM_DESTROY:
	{
	    GlobalRunning = false;
	    OutputDebugStringA("WM_DESTROY\n");
	} break;
	
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
	    u32 VKCode = WParam;
	    bool WasDown = ((LParam & (1<<30)) != 0);
	    bool IsDown = ((LParam & (1<<31)) == 0);

	    if (WasDown != IsDown)
	    {
		if (VKCode == 'W')
		{
		}
		else if (VKCode == 'A')
		{
		}
		else if (VKCode == 'S')
		{
		}
		else if (VKCode == 'D')
		{
		}
		else if (VKCode == 'Q')
		{
		}
		else if (VKCode == 'E')
		{
		}
		else if (VKCode == VK_UP)
		{
		}
		else if (VKCode == VK_DOWN)
		{
		}
		else if (VKCode == VK_LEFT)
		{
		}
		else if (VKCode == VK_RIGHT)
		{
		}
		else if (VKCode == VK_DOWN)
		{
		}
		else if (VKCode == VK_ESCAPE)
		{
		    GlobalRunning = false;    	
		}
	    }


	    b32 AltKeyWasDown = (LParam & (1 << 29));
	    if (VKCode == VK_F4 && AltKeyWasDown)
	    {
		GlobalRunning = false;
	    }
	} break;

	case WM_CLOSE:
	{
	    GlobalRunning = false;
	    OutputDebugStringA("WM_CLOSE\n");
	} break;

	case WM_ACTIVATEAPP:
	{
	    OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

    	case WM_PAINT:
	{
	    PAINTSTRUCT Paint;
	    HDC DeviceContext = BeginPaint(Window, &Paint);
	    int X = Paint.rcPaint.left;
	    int Y = Paint.rcPaint.top;
	    // int Width = Paint.rcPaint.right - Paint.rcPaint.left;
	    // int Height =  Paint.rcPaint.bottom - Paint.rcPaint.top;

	    win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
	    // We draw here for System triggered Redraws, like on resize or stuff like that.
	    Win32DisplayBufferWindow(DeviceContext, Dimensions.Width, Dimensions.Height, GlobalBackbuffer);
	    EndPaint(Window, &Paint);
	} break;

	default:
	{
	    Result = DefWindowProc(Window, Message, WParam, LParam);
	} break;

    }

    return Result;
}


int CALLBACK
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
    Win32LoadXInput();


    WNDCLASS WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HANDMADE HERO CLASS!";

    Win32ResizeDIBSection(&GlobalBackbuffer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    LARGE_INTEGER LastCounter;
    QueryPerformanceCounter(&LastCounter);

    LARGE_INTEGER PerformanceCounterFrequency; // In Counts/Second.
    QueryPerformanceFrequency(&PerformanceCounterFrequency);
    
    i64 LastCycleCount = __rdtsc();
    GlobalRunning = true;
    if (RegisterClass(&WindowClass))
    {
	HWND Window = 
	    CreateWindowEx(
		0,
		WindowClass.lpszClassName,
		"HANDMADE HERO WINDOW",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0, 
		Instance,
		0
	    );

	if (Window)
	{
	    
	    HDC DeviceContext = GetDC(Window);

	    int XOffset = 0;
	    int YOffset = 0;

	    win32_sound_output SoundOutput = {};

	    SoundOutput.SamplesPerSecond = 48000;
	    SoundOutput.ToneHz = 256;
	    SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
	    SoundOutput.RunningSampleIndex = 0;
	    SoundOutput.ToneVolume = 600;
	    SoundOutput.BytesPerSample = sizeof(i16)*2;
	    SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
	    SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond/15; // How far we want to write after the cursor.

	    Win32LoadDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
	    Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);
	    GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

	    MSG Message;
	    while(GlobalRunning)
	    {
		// Difference with get message, get message will give power to OS if no messages are comming.
		while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
		    if (Message.message == WM_QUIT)
		    {
			GlobalRunning = false;
		    }


		    TranslateMessage(&Message);
		    DispatchMessage(&Message);
		}
    
		for(DWORD i=0; i< XUSER_MAX_COUNT; ++i)
		{
		    XINPUT_STATE ControllerState;
		    if(XInputGetState(i, &ControllerState)==ERROR_SUCCESS)
		    {
			XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
			bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
			bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
			bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
			bool LShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			bool RShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
			bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
			bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
			bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

			i16 StickX = Pad->sThumbLX;
			i16 StickY = Pad->sThumbLY;

			XINPUT_VIBRATION Vibration;
			if (Down)
			{
			    Vibration.wLeftMotorSpeed = 60000;
			    Vibration.wRightMotorSpeed = 60000;
			} 
			else
			{
			    Vibration.wLeftMotorSpeed = 0;
			    Vibration.wRightMotorSpeed = 0;
			}
			XInputSetState(i, &Vibration);
			
			XOffset += StickX >> 12;
			YOffset += StickY >> 13;


			SoundOutput.ToneHz = 512 + (int)256.0f *((f32)StickY / 30000.0f);
			SoundOutput.WavePeriod =  SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
		    }
		    
		}
		
		game_offscreen_buffer Buffer = {};
		Buffer.Memory = GlobalBackbuffer.Memory;
		Buffer.Width = GlobalBackbuffer.Width;
		Buffer.Height = GlobalBackbuffer.Height;
		Buffer.Pitch = GlobalBackbuffer.Pitch;
		GameUpdateAndRender(&Buffer, XOffset, YOffset);

		DWORD PlayCursor;
		DWORD WriteCursor;

		if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
		{
		    DWORD ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);

		    DWORD TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);
		    DWORD BytesToWrite;
		    if (ByteToLock > TargetCursor)
		    {
			BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
			BytesToWrite += TargetCursor;
		    }
		    else
		    {
			BytesToWrite = TargetCursor - ByteToLock;
		    }
		    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);
		}

		// We just draw always.
		win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
		Win32DisplayBufferWindow( DeviceContext, Dimensions.Width, 
					  Dimensions.Height, GlobalBackbuffer);


		i64 EndCycleCount = __rdtsc();

		LARGE_INTEGER EndCounter;
		QueryPerformanceCounter(&EndCounter);

		i64 CyclesElapsed = EndCycleCount - LastCycleCount;
		i64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;

		i64 MSPerFrame = (1000 * CounterElapsed) / PerformanceCounterFrequency.QuadPart;
		i32 FPS = PerformanceCounterFrequency.QuadPart / CounterElapsed;
		i32 MegaCyclesPerFrame =  (i32)(CyclesElapsed / (1000000));


		#if 0
		char Buffer[256];
		wsprintf(Buffer, "M%dms, %dF/s, %dmillionc/f\n", MSPerFrame, FPS, MegaCyclesPerFrame);
		OutputDebugStringA(Buffer);
		#endif
		LastCounter = EndCounter;
		LastCycleCount = EndCycleCount;


	    }
	}
	else
	{

	}

    }
    else
    {
	// TODO: Logg
    }
    return(0);
}


