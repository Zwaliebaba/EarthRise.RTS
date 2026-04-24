#include "Module/SoundEngine.h"

#include "LTE/Array.h"
#include "LTE/AutoPtr.h"
#include "LTE/Location.h"
#include "LTE/Map.h"
#include "LTE/ProgramLog.h"
#include "LTE/Vector.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <objbase.h>
#include <cstring>
#include <xaudio2.h>

namespace {
  uint const kProceduralSampleRate = 44100;
  String const kDefaultSoundPath = "sound/";

  bool gLogged3DUnimplemented = false;

  struct SoundEngineXAudio2Impl;

  struct SoundXAudio2Impl;

  struct DecodedWaveData {
    Array<float> samples;
    uint channels;
    uint sampleRate;
    uint sampleFrames;

    DecodedWaveData() :
      channels(1),
      sampleRate(kProceduralSampleRate),
      sampleFrames(0)
      {}
  };

  uint ReadLE16(Array<uchar> const& data, size_t offset) {
    return (uint)data[offset] | ((uint)data[offset + 1] << 8);
  }

  uint ReadLE32(Array<uchar> const& data, size_t offset) {
    return (uint)data[offset]
      | ((uint)data[offset + 1] << 8)
      | ((uint)data[offset + 2] << 16)
      | ((uint)data[offset + 3] << 24);
  }

  bool DecodeWave(String const& filename, Array<uchar> const& fileData, DecodedWaveData& decoded) {
    if (fileData.size() < 12) {
      Log_Warning(Stringize() | "WAV file <" | filename | "> is too small");
      return false;
    }

    if (memcmp(fileData.data(), "RIFF", 4) || memcmp(fileData.data() + 8, "WAVE", 4)) {
      Log_Warning(Stringize() | "WAV file <" | filename | "> is missing a RIFF/WAVE header");
      return false;
    }

    uint formatTag = 0;
    uint bitsPerSample = 0;
    bool foundFormat = false;
    bool foundData = false;
    size_t offset = 12;
    while (offset + 8 <= fileData.size()) {
      char const* chunkId = (char const*)fileData.data() + offset;
      uint chunkSize = ReadLE32(fileData, offset + 4);
      size_t chunkDataOffset = offset + 8;
      size_t chunkEnd = chunkDataOffset + chunkSize;
      if (chunkEnd > fileData.size()) {
        Log_Warning(Stringize() | "WAV file <" | filename | "> has a truncated chunk");
        return false;
      }

      if (!memcmp(chunkId, "fmt ", 4)) {
        if (chunkSize < 16) {
          Log_Warning(Stringize() | "WAV file <" | filename | "> has an invalid format chunk");
          return false;
        }

        formatTag = ReadLE16(fileData, chunkDataOffset + 0);
        decoded.channels = ReadLE16(fileData, chunkDataOffset + 2);
        decoded.sampleRate = ReadLE32(fileData, chunkDataOffset + 4);
        bitsPerSample = ReadLE16(fileData, chunkDataOffset + 14);

        if (formatTag != WAVE_FORMAT_PCM && formatTag != WAVE_FORMAT_IEEE_FLOAT) {
          Log_Warning(Stringize() | "WAV file <" | filename | "> uses unsupported format tag " | formatTag);
          return false;
        }

        if (decoded.channels < 1 || decoded.channels > 2) {
          Log_Warning(Stringize() | "WAV file <" | filename | "> uses unsupported channel count " | decoded.channels);
          return false;
        }

        if ((formatTag == WAVE_FORMAT_PCM && bitsPerSample != 16)
          || (formatTag == WAVE_FORMAT_IEEE_FLOAT && bitsPerSample != 32))
        {
          Log_Warning(Stringize() | "WAV file <" | filename | "> uses unsupported bit depth " | bitsPerSample);
          return false;
        }

        foundFormat = true;
      } else if (!memcmp(chunkId, "data", 4)) {
        foundData = true;
        if (!foundFormat)
          continue;

        uint bytesPerSample = bitsPerSample / 8;
        uint totalSamples = chunkSize / bytesPerSample;
        if (!decoded.channels || (totalSamples % decoded.channels) != 0) {
          Log_Warning(Stringize() | "WAV file <" | filename | "> has misaligned sample data");
          return false;
        }

        decoded.sampleFrames = totalSamples / decoded.channels;
        decoded.samples.resize(totalSamples);
        if (formatTag == WAVE_FORMAT_PCM) {
          for (uint i = 0; i < totalSamples; ++i) {
            short sample = (short)ReadLE16(fileData, chunkDataOffset + i * 2);
            decoded.samples[i] = (float)sample / 32768.0f;
          }
        } else {
          memcpy(decoded.samples.data(), fileData.data() + chunkDataOffset, chunkSize);
        }
      }

      offset = chunkEnd + (chunkSize & 1);
    }

    if (!foundFormat || !foundData || !decoded.sampleFrames) {
      Log_Warning(Stringize() | "WAV file <" | filename | "> is missing playable sample data");
      return false;
    }

    return true;
  }

  struct SoundXAudio2Callback : public IXAudio2VoiceCallback {
    SoundXAudio2Impl* owner;

    SoundXAudio2Callback(SoundXAudio2Impl* owner) :
      owner(owner)
      {}

    STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) {}
    STDMETHOD_(void, OnVoiceProcessingPassEnd)() {}
    STDMETHOD_(void, OnStreamEnd)() {}
    STDMETHOD_(void, OnBufferStart)(void*) {}
    STDMETHOD_(void, OnLoopEnd)(void*) {}

    STDMETHOD_(void, OnBufferEnd)(void*);
    STDMETHOD_(void, OnVoiceError)(void*, HRESULT);
  };

  struct SoundXAudio2Impl : public SoundT {
    SoundEngineXAudio2Impl* engine;
    SoundXAudio2Callback callback;
    Array<float> samples;
    uint channels;
    uint sampleRate;
    uint sampleFrames;
    IXAudio2SourceVoice* sourceVoice;
    bool deleted;
    bool finished;
    bool looped;
    bool playing;
    float duration;
    float pan;
    float pitch;
    float volume;

    SoundXAudio2Impl(SoundEngineXAudio2Impl* engine, bool looped = false) :
      engine(engine),
      callback(this),
      channels(1),
      sampleRate(kProceduralSampleRate),
      sampleFrames(0),
      sourceVoice(nullptr),
      deleted(false),
      finished(true),
      looped(looped),
      playing(false),
      duration(0),
      pan(0),
      pitch(1),
      volume(1)
      {}

    ~SoundXAudio2Impl() {
      Shutdown();
    }

    bool Initialize(Array<float> const& buffer);
    bool Initialize(DecodedWaveData const& decoded);
    void Shutdown();
    bool SubmitBuffer(uint playBeginFrames);
    void ApplyPan();
    void RefreshFinishedState();
    bool ShouldCollect() const {
      return deleted || finished;
    }

    void Delete() {
      deleted = true;
      if (sourceVoice) {
        sourceVoice->Stop(0);
        sourceVoice->FlushSourceBuffers();
      }
    }

    bool IsFinished() const {
      return finished;
    }

    bool IsLooped() const {
      return looped;
    }

    float GetDuration() const {
      return duration;
    }

    float GetPan() const {
      return pan;
    }

    float GetPitch() const {
      return pitch;
    }

    float GetVolume() const {
      return volume;
    }

    void SetCursor(float position) {
      if (!sourceVoice || !sampleFrames)
        return;

      uint playBeginFrames = (uint)(position * sampleRate / 1000.0f);
      if (sampleFrames)
        playBeginFrames %= sampleFrames;

      bool restart = playing;
      sourceVoice->Stop(0);
      sourceVoice->FlushSourceBuffers();
      finished = false;
      if (!SubmitBuffer(playBeginFrames)) {
        finished = true;
        return;
      }

      if (restart)
        sourceVoice->Start(0);
    }

    void SetPan(float pan) {
      if (pan < -1.0f)
        pan = -1.0f;
      if (pan > 1.0f)
        pan = 1.0f;
      this->pan = pan;
      ApplyPan();
    }

    void SetPlaying(bool playing) {
      this->playing = playing;
      if (!sourceVoice)
        return;
      if (playing && !finished)
        sourceVoice->Start(0);
      else
        sourceVoice->Stop(0);
    }

    void SetPitch(float pitch) {
      if (pitch <= 0.01f)
        pitch = 0.01f;
      if (pitch > XAUDIO2_MAX_FREQ_RATIO)
        pitch = XAUDIO2_MAX_FREQ_RATIO;
      this->pitch = pitch;
      if (sourceVoice)
        sourceVoice->SetFrequencyRatio(pitch);
    }

    void SetVolume(float volume) {
      if (volume < 0)
        volume = 0;
      this->volume = volume;
      if (sourceVoice)
        sourceVoice->SetVolume(volume);
    }
  };

  struct SoundEngineXAudio2Impl : public SoundEngine {
    IXAudio2* xaudio;
    IXAudio2MasteringVoice* masteringVoice;
    bool comInitialized;
    bool ready;
    uint outputChannels;
    Vector<Sound> activeSounds;
    Map<String, DecodedWaveData> waveCache;

    SoundEngineXAudio2Impl() :
      xaudio(nullptr),
      masteringVoice(nullptr),
      comInitialized(false),
      ready(false),
      outputChannels(0)
    {
      HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      if (result == S_OK || result == S_FALSE)
        comInitialized = true;
      else if (result != RPC_E_CHANGED_MODE)
        Log_Warning(Stringize() | "CoInitializeEx failed for XAudio2 (" | (int)result | ")");

      result = XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
      if (FAILED(result)) {
        Log_Warning(Stringize() | "XAudio2Create failed (" | (int)result | ")");
        return;
      }

      result = xaudio->CreateMasteringVoice(&masteringVoice);
      if (FAILED(result)) {
        Log_Warning(Stringize() | "CreateMasteringVoice failed (" | (int)result | ")");
        xaudio->Release();
        xaudio = nullptr;
        return;
      }

      XAUDIO2_VOICE_DETAILS details;
      masteringVoice->GetVoiceDetails(&details);
      outputChannels = details.InputChannels ? details.InputChannels : 1;

      result = xaudio->StartEngine();
      if (FAILED(result)) {
        Log_Warning(Stringize() | "XAudio2 StartEngine failed (" | (int)result | ")");
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
        xaudio->Release();
        xaudio = nullptr;
        return;
      }

      ready = true;
    }

    ~SoundEngineXAudio2Impl() {
      for (size_t i = 0; i < activeSounds.size(); ++i) {
        SoundXAudio2Impl* sound = activeSounds[i].Cast<SoundXAudio2Impl>();
        sound->Shutdown();
      }
      activeSounds.clear();

      if (masteringVoice) {
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
      }

      if (xaudio) {
        xaudio->StopEngine();
        xaudio->Release();
        xaudio = nullptr;
      }

      if (comInitialized)
        CoUninitialize();
    }

    bool IsReady() const {
      return ready;
    }

    DecodedWaveData const* LoadWave(String const& filename) {
      DecodedWaveData* cached = waveCache.get(filename);
      if (cached)
        return cached;

      AutoPtr<Array<uchar> > fileData = Location_Resource(kDefaultSoundPath + filename)->Read();
      if (!fileData)
        return nullptr;

      DecodedWaveData decoded;
      if (!DecodeWave(filename, *fileData, decoded))
        return nullptr;

      waveCache[filename] = decoded;
      return waveCache.get(filename);
    }

    char const* GetName() const {
      return "SoundEngine (XAudio2)";
    }

    Sound Play(Array<float> const& buffer) {
      Sound sound = new SoundXAudio2Impl(this, false);
      SoundXAudio2Impl* impl = sound.Cast<SoundXAudio2Impl>();
      if (!impl->Initialize(buffer))
        return sound;

      impl->SetPlaying(true);
      activeSounds << sound;
      return sound;
    }

    Sound Play2D(String const& filename, float volume, bool looped) {
      Sound sound = new SoundXAudio2Impl(this, looped);
      SoundXAudio2Impl* impl = sound.Cast<SoundXAudio2Impl>();
      DecodedWaveData const* decoded = LoadWave(filename);
      if (!decoded || !impl->Initialize(*decoded)) {
        sound->SetVolume(volume);
        return sound;
      }

      impl->SetVolume(volume);
      impl->SetPlaying(true);
      activeSounds << sound;
      return sound;
    }

    Sound Play3D(
      String const& filename,
      Object const& carrier,
      V3 const& offset,
      float volume,
      float distanceDiv,
      bool looped)
    {
      if (!gLogged3DUnimplemented) {
        gLogged3DUnimplemented = true;
        Log_Warning("XAudio2 3D playback is not implemented yet; falling back to a silent sound");
      }

      Sound sound = new SoundXAudio2Impl(this, looped);
      sound->SetVolume(volume);
      return sound;
    }

    void Update() {
      for (size_t i = 0; i < activeSounds.size(); ++i) {
        SoundXAudio2Impl* sound = activeSounds[i].Cast<SoundXAudio2Impl>();
        sound->RefreshFinishedState();
        if (!sound->ShouldCollect())
          continue;

        sound->Shutdown();
        activeSounds.eraseIndex(i);
        --i;
      }
    }
  };

  STDMETHODIMP_(void) SoundXAudio2Callback::OnBufferEnd(void*) {
    owner->finished = true;
    owner->playing = false;
  }

  STDMETHODIMP_(void) SoundXAudio2Callback::OnVoiceError(void*, HRESULT error) {
    owner->finished = true;
    owner->playing = false;
    Log_Warning(Stringize() | "XAudio2 voice error (" | (int)error | ")");
  }

  bool SoundXAudio2Impl::Initialize(Array<float> const& buffer) {
    samples = buffer;
    channels = 1;
    sampleRate = kProceduralSampleRate;
    sampleFrames = (uint)samples.size();
    duration = sampleFrames
      ? (1000.0f * (float)sampleFrames) / (float)sampleRate
      : 0;

    if (!engine || !engine->IsReady() || !sampleFrames) {
      finished = true;
      return false;
    }

    WAVEFORMATEX format;
    format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    format.nChannels = (WORD)channels;
    format.nSamplesPerSec = sampleRate;
    format.nAvgBytesPerSec = sampleRate * channels * sizeof(float);
    format.nBlockAlign = (WORD)(channels * sizeof(float));
    format.wBitsPerSample = 8 * sizeof(float);
    format.cbSize = 0;

    HRESULT result = engine->xaudio->CreateSourceVoice(
      &sourceVoice,
      &format,
      0,
      XAUDIO2_DEFAULT_FREQ_RATIO,
      &callback);
    if (FAILED(result)) {
      Log_Warning(Stringize() | "CreateSourceVoice failed (" | (int)result | ")");
      sourceVoice = nullptr;
      finished = true;
      return false;
    }

    if (!SubmitBuffer(0)) {
      Shutdown();
      finished = true;
      return false;
    }

    finished = false;
    SetVolume(volume);
    SetPitch(pitch);
    SetPan(pan);
    return true;
  }

  bool SoundXAudio2Impl::Initialize(DecodedWaveData const& decoded) {
    samples = decoded.samples;
    channels = decoded.channels;
    sampleRate = decoded.sampleRate;
    sampleFrames = decoded.sampleFrames;
    duration = sampleFrames
      ? (1000.0f * (float)sampleFrames) / (float)sampleRate
      : 0;

    if (!engine || !engine->IsReady() || !sampleFrames) {
      finished = true;
      return false;
    }

    WAVEFORMATEX format;
    format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    format.nChannels = (WORD)channels;
    format.nSamplesPerSec = sampleRate;
    format.nAvgBytesPerSec = sampleRate * channels * sizeof(float);
    format.nBlockAlign = (WORD)(channels * sizeof(float));
    format.wBitsPerSample = 8 * sizeof(float);
    format.cbSize = 0;

    HRESULT result = engine->xaudio->CreateSourceVoice(
      &sourceVoice,
      &format,
      0,
      XAUDIO2_DEFAULT_FREQ_RATIO,
      &callback);
    if (FAILED(result)) {
      Log_Warning(Stringize() | "CreateSourceVoice failed (" | (int)result | ")");
      sourceVoice = nullptr;
      finished = true;
      return false;
    }

    if (!SubmitBuffer(0)) {
      Shutdown();
      finished = true;
      return false;
    }

    finished = false;
    SetVolume(volume);
    SetPitch(pitch);
    SetPan(pan);
    return true;
  }

  void SoundXAudio2Impl::Shutdown() {
    if (sourceVoice) {
      sourceVoice->Stop(0);
      sourceVoice->FlushSourceBuffers();
      sourceVoice->DestroyVoice();
      sourceVoice = nullptr;
    }
    engine = nullptr;
  }

  bool SoundXAudio2Impl::SubmitBuffer(uint playBeginFrames) {
    if (!sourceVoice || !samples.size())
      return false;

    if (playBeginFrames >= sampleFrames)
      playBeginFrames = sampleFrames - 1;

    XAUDIO2_BUFFER buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.AudioBytes = (UINT32)(samples.size() * sizeof(float));
    buffer.pAudioData = (BYTE const*)samples.data();
    buffer.PlayBegin = playBeginFrames;
    buffer.LoopCount = looped ? XAUDIO2_LOOP_INFINITE : 0;
    buffer.Flags = looped ? 0 : XAUDIO2_END_OF_STREAM;

    HRESULT result = sourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(result)) {
      Log_Warning(Stringize() | "SubmitSourceBuffer failed (" | (int)result | ")");
      return false;
    }
    return true;
  }

  void SoundXAudio2Impl::ApplyPan() {
    if (!sourceVoice || !engine || !engine->masteringVoice || !engine->outputChannels)
      return;

    if (channels == 1 && engine->outputChannels == 1) {
      float matrix[1] = {1.0f};
      sourceVoice->SetOutputMatrix(engine->masteringVoice, 1, 1, matrix);
      return;
    }

    if (channels == 1 && engine->outputChannels == 2) {
      float matrix[2] = {
        pan > 0 ? 1.0f - pan : 1.0f,
        pan < 0 ? 1.0f + pan : 1.0f };
      sourceVoice->SetOutputMatrix(engine->masteringVoice, 1, 2, matrix);
      return;
    }

    if (channels == 2 && engine->outputChannels == 2) {
      float leftGain = pan > 0 ? 1.0f - pan : 1.0f;
      float rightGain = pan < 0 ? 1.0f + pan : 1.0f;
      float matrix[4] = {
        leftGain, 0.0f,
        0.0f, rightGain };
      sourceVoice->SetOutputMatrix(engine->masteringVoice, 2, 2, matrix);
      return;
    }

    Vector<float> matrix;
    matrix.resize(channels * engine->outputChannels, 0.0f);
    for (uint channel = 0; channel < channels && channel < engine->outputChannels; ++channel)
      matrix[channel * engine->outputChannels + channel] = 1.0f;
    sourceVoice->SetOutputMatrix(
      engine->masteringVoice,
      channels,
      engine->outputChannels,
      matrix.data());
  }

  void SoundXAudio2Impl::RefreshFinishedState() {
    if (!sourceVoice || looped || finished)
      return;

    XAUDIO2_VOICE_STATE state;
    sourceVoice->GetState(&state);
    if (!state.BuffersQueued) {
      finished = true;
      playing = false;
    }
  }
}

SoundEngine* SoundEngine_XAudio2() {
  SoundEngineXAudio2Impl* engine = new SoundEngineXAudio2Impl;
  if (engine->IsReady())
    return engine;

  delete engine;
  Log_Warning("XAudio2 sound backend failed to initialize; falling back to Null");
  return SoundEngine_Null();
}