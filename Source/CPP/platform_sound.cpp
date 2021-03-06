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

struct embedded_sound
{
    char const * const byte_array;
    size_t read_offset;
    size_t size;
};

static HRESULT
find_chunk(HANDLE file, DWORD fourcc, DWORD &chunk_size, DWORD &chunk_data_position);

static HRESULT
read_chunk_data(HANDLE file, void *buffer, DWORD buffer_size, DWORD buffer_offset);

struct sound
{


    IXAudio2SourceVoice **source_voices;
    int* voice_is_playing;

    XAUDIO2_BUFFER *buffer;

    int max_voices;
    int current_voice;
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

    new_sound->current_voice = 0;
    new_sound->max_voices = 2;
    new_sound->voice_is_playing = new int[new_sound->max_voices];
    new_sound->source_voices = new IXAudio2SourceVoice*[new_sound->max_voices];
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

    for (int i = 0; i < new_sound->max_voices; i++)
    {
        if (FAILED(g_audio->CreateSourceVoice((&new_sound->source_voices[0]), (WAVEFORMATEX*)&wfx)))
        { return -3; }
        new_sound->voice_is_playing[i] = 0;
    }

    g_sounds.push_back(new_sound);

    CloseHandle(sound_file);

    return index_return;
}

static size_t
read_embedded(void *dst, embedded_sound *src, size_t size)
{
    if (src->read_offset + size >= src->size)
    { return 0; }

    memcpy(dst, src->byte_array + src->read_offset, size);
    src->read_offset += size;

    return size;
}

static int
find_chunk_embedded(embedded_sound *file, DWORD fourcc, DWORD &chunk_size, DWORD &chunk_data_position)
{
    file->read_offset = 0;

    DWORD chunk_type = 0;
    DWORD chunk_data_size = 0;
    DWORD riff_data_size = 0;
    DWORD file_type = 0;
    DWORD bytes_read = 0;
    DWORD offset = 0;

    int read_result = 1;

    while (read_result)
    {
        read_result = static_cast<int>(read_embedded(&chunk_type, file, sizeof(DWORD)));
        read_result = static_cast<int>(read_embedded(&chunk_data_size, file, sizeof(DWORD)));

        switch (chunk_type)
        {
            case fourccRIFF:
            {
                riff_data_size = chunk_data_size;
                chunk_data_size = 4;
                read_result = static_cast<int>(read_embedded(&file_type, file, sizeof(DWORD)));
            } break;
            default:
            {
                file->read_offset += chunk_data_size;
                if (file->read_offset >= file->size)
                { goto err_return; }
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
        { return 0; }
    }

err_return:
    return 0;

success_return:
    return 1;
}

static int
read_chunk_data_embedded(embedded_sound *file, void *buffer, DWORD buffer_size, DWORD buffer_offset)
{
    file->read_offset = buffer_offset;
    if (read_embedded(buffer, file, buffer_size) == 0)
    { return 0; }

    return 1;
}

extern "C"
int
load_sound_embedded(char const * const byte_array, size_t data_size)
{
    struct sound *new_sound = new struct sound();
    new_sound->buffer = new XAUDIO2_BUFFER();
    new_sound->current_voice = 0;
    new_sound->max_voices = 5; // 5 voices seems to be enough
    new_sound->voice_is_playing = new int[new_sound->max_voices];
    new_sound->source_voices = new IXAudio2SourceVoice*[new_sound->max_voices];
    new_sound->buffer = new XAUDIO2_BUFFER();

    int index_return = static_cast<int>(g_sounds.size());

    embedded_sound es = { byte_array, 0, data_size };

    WAVEFORMATEXTENSIBLE wfx = {0};

    DWORD chunk_size = 0;
    DWORD chunk_position = 0;
    find_chunk_embedded(&es, fourccRIFF, chunk_size, chunk_position); 

    DWORD file_type = 0;
    read_chunk_data_embedded(&es, &file_type, chunk_size, chunk_position);
    if (file_type != fourccWAVE)
    {
        return -1;
    }

    find_chunk_embedded(&es, fourccFMT, chunk_size, chunk_position);
    read_chunk_data_embedded(&es, &wfx, chunk_size, chunk_position);

    find_chunk_embedded(&es, fourccDATA, chunk_size, chunk_position);

    BYTE *data_buffer = new BYTE[chunk_size];

    read_chunk_data_embedded(&es, data_buffer, chunk_size, chunk_position);

    new_sound->buffer->AudioBytes = chunk_size;
    new_sound->buffer->pAudioData = data_buffer;
    new_sound->buffer->Flags = XAUDIO2_END_OF_STREAM;


    for (int i = 0; i < new_sound->max_voices; i++)
    {
        if (FAILED(g_audio->CreateSourceVoice((&new_sound->source_voices[i]), (WAVEFORMATEX*)&wfx)))
        { return -3; }
        new_sound->voice_is_playing[i] = 0;
    }

    g_sounds.push_back(new_sound);

    return index_return;
}

extern "C"
void
play_sound(int sound)
{
    if (sound > g_sounds.size() || sound < 0)
    { return; }

    struct sound *s = g_sounds[sound];

    if (s->voice_is_playing[s->current_voice])
    {
        //s->source_voice->Stop(0, XAUDIO2_COMMIT_NOW);
        //s->source_voice->FlushSourceBuffers();

        /*
            microsoft docs :
            A voice stopped with the XAUDIO2_PLAY_TAILS flag
            stops consuming source buffers, but continues to process its
            effects and send audio to its destination voices.

            XAUDIO2_PLAY_TAILS keeps the sound from being choppy when starting/stopping,
            and having at least 5 voices means there are enough voices for a new sound
            to immediately be started
        */
        s->source_voices[s->current_voice]->Stop(0, XAUDIO2_PLAY_TAILS);
        s->source_voices[s->current_voice]->FlushSourceBuffers();

        s->voice_is_playing[s->current_voice] = 0;
    }

    s->current_voice++;
    if (s->current_voice >= s->max_voices) s->current_voice = 0;

#ifdef TESTING_BUILD
    char buffer[128];
    snprintf(buffer, 128, "Playing buffer [%d] - %p\n", s->current_voice, s->source_voices[s->current_voice]);
    OutputDebugStringA(buffer);
#endif

    HRESULT hr = s->source_voices[s->current_voice]->SubmitSourceBuffer(s->buffer);
    if (FAILED(hr))
    { return; }

    hr = s->source_voices[s->current_voice]->Start(0);
    if (FAILED(hr))
    { return; }
    else
    { s->voice_is_playing[s->current_voice] = 1; }
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
