#include <Xaudio2.h>
#include <string.h>

#include <cstdlib>

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

#include "..\Include\platform_api.h"

extern "C"
int
init_sound_system(void)
{
    IXAudio2 *audio = NULL;

    if (FAILED(XAudio2Create(&audio, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        return 0;
    }

    IXAudio2MasteringVoice *voice = NULL;
    if (FAILED(audio->CreateMasteringVoice(&voice)))
    {
        return 0;
    }

    return 1;
}

extern "C"
void
test_sound(char const * const file)
{
    wchar_t wfname[PATH_MAX];
    std::mbstowcs(wfname, file, PATH_MAX); 

    HANDLE sound_file =
        CreateFileW(
            wfname,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0,
            NULL
        );

    if (sound_file == INVALID_HANDLE_VALUE) { return; }
}

HRESULT
find_chunk(HANDLE file, DWORD fourcc, DWORD &chunk_size, DWORD &chunk_data_position)
{
    HRESULT hr = S_OK;
    if (SetFilePointer(file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD chunk_type = 0;
    DWORD chunk_data_size = 0;
    DWORD riff_data_size = 0;
    DWORD file_type = 0;
    DWORD bytes_read = 0;
    DWORD offset = 0;


}
