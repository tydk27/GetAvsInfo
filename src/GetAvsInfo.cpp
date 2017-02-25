#include <stdio.h>
#include <iostream>
#include <Windows.h>

#include "avisynth.h"

const AVS_Linkage *AVS_linkage = 0;

using namespace std;

int main(int argc, const char* argv[])
{
	if (argc != 2) {
		cout << "Usage: GetAvsInfo.exe [avs file]" << endl;
		return 2;
	}

	const char *input = NULL;
	input = argv[1];

	try {
		HMODULE dll = NULL;
		dll = LoadLibraryEx("avisynth", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (!dll) {
			cerr << "Error: failed to load avisynth.dll" << endl;
			return 1;
		}

		typedef IScriptEnvironment* (__stdcall *DLLFUNC)(int);
		IScriptEnvironment* env;

		DLLFUNC CreateEnv = (DLLFUNC)GetProcAddress(dll, "CreateScriptEnvironment");
		if (!CreateEnv) {
			cerr << "Error: failed to load CreateScriptEnvironment" << endl;
			FreeLibrary(dll);
			return 1;
		}

		env = CreateEnv(AVISYNTH_INTERFACE_VERSION);

		AVS_linkage = env->GetAVSLinkage();
		AVSValue arg(input);
		AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));
		if (!res.IsClip()) {
			cerr << "Error: " << input << " didn't return a video clip." << endl;
			FreeLibrary(dll);
			return 1;
		}

		PClip clip = res.AsClip();

		if (clip->GetVersion() < 5) {
			cout << "Warning: using old avisynth. recommend to install v2.60 or later." << endl;
		}

		VideoInfo vi = clip->GetVideoInfo();

		if (!vi.HasVideo()) {
			cerr << "Error: " << input << " didn't have a video clip." << endl;
			FreeLibrary(dll);
			return 1;
		}

		cout << input << endl;
		cout << vi.width << "x" << vi.height << endl;
		cout << vi.fps_numerator << "/" << vi.fps_denominator << "fps" << endl;
		cout << vi.num_frames << "frames" << endl;

		env->DeleteScriptEnvironment();
		FreeLibrary(dll);
		AVS_linkage = 0;

	} catch (AvisynthError err) {
		cerr << "Error: " << err.msg << endl;
		return 1;
	}

	return 0;
}