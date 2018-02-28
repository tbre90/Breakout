#include <Xaudio2.h>
#include <Objbase.h>
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

#include "..\..\Include\platform_api.h"

HRESULT
find_chunk(HANDLE file, DWORD fourcc, DWORD &chunk_size, DWORD &chunk_data_position);

HRESULT
read_chunk_data(HANDLE file, void *buffer, DWORD buffer_size, DWORD buffer_offset);

IXAudio2 *audio = NULL;
IXAudio2MasteringVoice *voice = NULL;
IXAudio2SourceVoice *source_voice = NULL;

extern "C"
int
init_sound_system(void)
{
    if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK)
    {
        return 0;
    }

    if (FAILED(XAudio2Create(&audio, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        return 0;
    }

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

    if (sound_file == INVALID_HANDLE_VALUE) { return; }

    WAVEFORMATEXTENSIBLE wfx = {0};
    XAUDIO2_BUFFER buffer = {0};

    DWORD chunk_size = 0;
    DWORD chunk_position = 0;
    find_chunk(sound_file, fourccRIFF, chunk_size, chunk_position);

    DWORD file_type = 0;
    read_chunk_data(sound_file, &file_type, sizeof(DWORD), chunk_position);
    if (file_type != fourccWAVE)
    {
        return;
    }

    find_chunk(sound_file, fourccFMT, chunk_size, chunk_position);
    read_chunk_data(sound_file, &wfx, chunk_size, chunk_position);

    find_chunk(sound_file, fourccDATA, chunk_size, chunk_position);
    
    BYTE *data_buffer = new BYTE[chunk_size];
    read_chunk_data(sound_file, data_buffer, chunk_size, chunk_position);

    buffer.AudioBytes = chunk_size;
    buffer.pAudioData = data_buffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    if (FAILED(audio->CreateSourceVoice(&source_voice, (WAVEFORMATEX*)&wfx)))
    { return; }

    if (FAILED(source_voice->SubmitSourceBuffer(&buffer)))
    { return; }

    if (FAILED(source_voice->Start(0)))
    { return; }
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

HRESULT
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
