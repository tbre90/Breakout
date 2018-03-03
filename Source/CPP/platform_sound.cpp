#include <Xaudio2.h>
#include <Objbase.h>
#include <string.h>

#include <vector>
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

#include "..\..\Include\platform_api.h"

static HRESULT
find_chunk(HANDLE file, DWORD fourcc, DWORD &chunk_size, DWORD &chunk_data_position);

static HRESULT
read_chunk_data(HANDLE file, void *buffer, DWORD buffer_size, DWORD buffer_offset);

struct sound
{
    IXAudio2SourceVoice *source_voice;
    XAUDIO2_BUFFER *buffer;
    int currently_playing;
};

static IXAudio2 *g_audio = NULL;
static IXAudio2MasteringVoice *g_mastering_voice = NULL;

static std::vector<struct sound*> g_sounds;

extern "C"
int
init_sound_system(void)
{
    if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK)
    {
        return 0;
    }

    if (FAILED(XAudio2Create(&g_audio, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        return 0;
    }

    if (FAILED(g_audio->CreateMasteringVoice(&g_mastering_voice)))
    {
        return 0;
    }

    return 1;
}

extern "C"
int
load_sound(char const * const file)
{
    HANDLE sound_file =
        CreateFileA(
            file,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

    if (sound_file == INVALID_HANDLE_VALUE) { return -1; }

    struct sound *new_sound = new struct sound();
    new_sound->buffer = new XAUDIO2_BUFFER();
    int index_return = static_cast<int>(g_sounds.size());

    WAVEFORMATEXTENSIBLE wfx = {0};

    DWORD chunk_size = 0;
    DWORD chunk_position = 0;
    find_chunk(sound_file, fourccRIFF, chunk_size, chunk_position);

    DWORD file_type = 0;
    read_chunk_data(sound_file, &file_type, sizeof(DWORD), chunk_position);
    if (file_type != fourccWAVE)
    {
        return -2;
    }

    find_chunk(sound_file, fourccFMT, chunk_size, chunk_position);
    read_chunk_data(sound_file, &wfx, chunk_size, chunk_position);

    find_chunk(sound_file, fourccDATA, chunk_size, chunk_position);
    
    BYTE *data_buffer = new BYTE[chunk_size];

    read_chunk_data(sound_file, data_buffer, chunk_size, chunk_position);

    new_sound->buffer->AudioBytes = chunk_size;
    new_sound->buffer->pAudioData = data_buffer;
    new_sound->buffer->Flags = XAUDIO2_END_OF_STREAM;

    if (FAILED(g_audio->CreateSourceVoice(&(new_sound->source_voice), (WAVEFORMATEX*)&wfx)))
    { return -3; }

    g_sounds.push_back(new_sound);

    CloseHandle(sound_file);

    return index_return;
}

extern "C"
void
play_sound(int sound)
{
    if (sound > g_sounds.size() || sound < 0)
    { return; }

    struct sound *s = g_sounds[sound];

    if (s->currently_playing)
    {
        s->source_voice->Stop(0, XAUDIO2_COMMIT_NOW);
        s->source_voice->FlushSourceBuffers();
        s->currently_playing = 0;
    }

    HRESULT hr = s->source_voice->SubmitSourceBuffer(s->buffer);
    if (FAILED(hr))
    { return; }

    hr = s->source_voice->Start(0);
    if (FAILED(hr))
    { return; }
    else
    { s->currently_playing = 1; }
}

static HRESULT
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

    while (hr == S_OK)
    {
        DWORD read = 0;
        if (ReadFile(file, &chunk_type, sizeof(DWORD), &read, NULL) == 0)
        { hr = HRESULT_FROM_WIN32(GetLastError()); }

        if (ReadFile(file, &chunk_data_size, sizeof(DWORD), &read, NULL) == 0)
        { hr = HRESULT_FROM_WIN32(GetLastError());  }

        switch (chunk_type)
        {
            case fourccRIFF:
            {
                riff_data_size = chunk_data_size;
                chunk_data_size = 4;
                if (ReadFile(file, &file_type, sizeof(DWORD), &read, NULL))
                { hr = HRESULT_FROM_WIN32(GetLastError()); }
            } break;
            default:
            {
                if (SetFilePointer(file, chunk_data_size, NULL, FILE_CURRENT) == 0)
                { hr = HRESULT_FROM_WIN32(GetLastError()); goto err_return; }
            } break;
        }

        offset += sizeof(DWORD) * 2;

        if (chunk_type == fourcc)
        {
            chunk_size = chunk_data_size;
            chunk_data_position = offset;
            goto success_return;
        }

        offset += chunk_data_size;
        
        if (bytes_read >= riff_data_size)
        { return S_FALSE; }
    }

err_return:
    return hr;

success_return:
    return S_OK;
}

static HRESULT
read_chunk_data(HANDLE file, void *buffer, DWORD buffer_size, DWORD buffer_offset)
{
    HRESULT hr = S_OK;
    if (SetFilePointer(file, buffer_offset, NULL, FILE_BEGIN) == 0)
    { return HRESULT_FROM_WIN32(GetLastError()); }

    DWORD read = 0;
    if (ReadFile(file, buffer, buffer_size, &read, NULL) == 0)
    { hr = HRESULT_FROM_WIN32(GetLastError()); }

    return hr;
}
