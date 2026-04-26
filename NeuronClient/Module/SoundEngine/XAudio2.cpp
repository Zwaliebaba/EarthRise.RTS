#include "Module/SoundEngine.h"
#include "Game/Camera.h"
#include "Game/Object.h"
#include "LTE/Array.h"
#include "LTE/AutoPtr.h"
#include "LTE/Location.h"
#include "LTE/Map.h"
#include "LTE/Math.h"
#include "LTE/ProgramLog.h"
#include "LTE/Vector.h"
#include <objbase.h>
#include <cstring>
#include <xaudio2.h>
#include <x3daudio.h>

namespace
{
  constexpr uint kProceduralSampleRate = 44100;
  const String kDefaultSoundPath = "sound\\";

  struct SoundEngineXAudio2Impl;

  struct SoundXAudio2Impl;

  struct DecodedWaveData
  {
    Array<float> samples;
    uint channels;
    uint sampleRate;
    uint sampleFrames;

    DecodedWaveData()
      : channels(1),
        sampleRate(kProceduralSampleRate),
        sampleFrames(0) {}
  };

  uint ReadLE16(const Array<uchar>& data, size_t offset)
  {
    return static_cast<uint>(data[offset]) | (static_cast<uint>(data[offset + 1]) << 8);
  }

  uint ReadLE32(const Array<uchar>& data, size_t offset)
  {
    return static_cast<uint>(data[offset]) | (static_cast<uint>(data[offset + 1]) << 8) | (static_cast<uint>(data[offset + 2]) << 16) | (
      static_cast<uint>(data[offset + 3]) << 24);
  }

  bool DecodeWave(const String& filename, const Array<uchar>& fileData, DecodedWaveData& decoded)
  {
    if (fileData.size() < 12)
    {
      Log_Warning(Stringize() | "WAV file <" | filename | "> is too small");
      return false;
    }

    if (memcmp(fileData.data(), "RIFF", 4) || memcmp(fileData.data() + 8, "WAVE", 4))
    {
      Log_Warning(Stringize() | "WAV file <" | filename | "> is missing a RIFF/WAVE header");
      return false;
    }

    uint formatTag = 0;
    uint bitsPerSample = 0;
    bool foundFormat = false;
    bool foundData = false;
    size_t offset = 12;
    while (offset + 8 <= fileData.size())
    {
      const char* chunkId = (const char*)fileData.data() + offset;
      uint chunkSize = ReadLE32(fileData, offset + 4);
      size_t chunkDataOffset = offset + 8;
      size_t chunkEnd = chunkDataOffset + chunkSize;
      if (chunkEnd > fileData.size())
      {
        Log_Warning(Stringize() | "WAV file <" | filename | "> has a truncated chunk");
        return false;
      }

      if (!memcmp(chunkId, "fmt ", 4))
      {
        if (chunkSize < 16)
        {
          Log_Warning(Stringize() | "WAV file <" | filename | "> has an invalid format chunk");
          return false;
        }

        formatTag = ReadLE16(fileData, chunkDataOffset + 0);
        decoded.channels = ReadLE16(fileData, chunkDataOffset + 2);
        decoded.sampleRate = ReadLE32(fileData, chunkDataOffset + 4);
        bitsPerSample = ReadLE16(fileData, chunkDataOffset + 14);

        if (formatTag != WAVE_FORMAT_PCM && formatTag != WAVE_FORMAT_IEEE_FLOAT)
        {
          Log_Warning(Stringize() | "WAV file <" | filename | "> uses unsupported format tag " | formatTag);
          return false;
        }

        if (decoded.channels < 1 || decoded.channels > 2)
        {
          Log_Warning(Stringize() | "WAV file <" | filename | "> uses unsupported channel count " | decoded.channels);
          return false;
        }

        if ((formatTag == WAVE_FORMAT_PCM && bitsPerSample != 16) || (formatTag == WAVE_FORMAT_IEEE_FLOAT && bitsPerSample != 32))
        {
          Log_Warning(Stringize() | "WAV file <" | filename | "> uses unsupported bit depth " | bitsPerSample);
          return false;
        }

        foundFormat = true;
      }
      else if (!memcmp(chunkId, "data", 4))
      {
        foundData = true;
        if (!foundFormat)
          continue;

        uint bytesPerSample = bitsPerSample / 8;
        uint totalSamples = chunkSize / bytesPerSample;
        if (!decoded.channels || (totalSamples % decoded.channels) != 0)
        {
          Log_Warning(Stringize() | "WAV file <" | filename | "> has misaligned sample data");
          return false;
        }

        decoded.sampleFrames = totalSamples / decoded.channels;
        decoded.samples.resize(totalSamples);
        if (formatTag == WAVE_FORMAT_PCM)
        {
          for (uint i = 0; i < totalSamples; ++i)
          {
            short sample = static_cast<short>(ReadLE16(fileData, chunkDataOffset + i * 2));
            decoded.samples[i] = static_cast<float>(sample) / 32768.0f;
          }
        }
        else
          memcpy(decoded.samples.data(), fileData.data() + chunkDataOffset, chunkSize);
      }

      offset = chunkEnd + (chunkSize & 1);
    }

    if (!foundFormat || !foundData || !decoded.sampleFrames)
    {
      Log_Warning(Stringize() | "WAV file <" | filename | "> is missing playable sample data");
      return false;
    }

    return true;
  }

  DecodedWaveData DownmixToMono(const DecodedWaveData& decoded)
  {
    if (decoded.channels == 1)
      return decoded;

    DecodedWaveData mono;
    mono.channels = 1;
    mono.sampleRate = decoded.sampleRate;
    mono.sampleFrames = decoded.sampleFrames;
    mono.samples.resize(decoded.sampleFrames);

    for (uint frame = 0; frame < decoded.sampleFrames; ++frame)
    {
      float sample = 0;
      for (uint channel = 0; channel < decoded.channels; ++channel)
        sample += decoded.samples[frame * decoded.channels + channel];
      mono.samples[frame] = sample / static_cast<float>(decoded.channels);
    }

    return mono;
  }

  X3DAUDIO_VECTOR ToX3D(const V3& v)
  {
    X3DAUDIO_VECTOR result;
    result.x = static_cast<FLOAT>(v.x);
    result.y = static_cast<FLOAT>(v.y);
    result.z = static_cast<FLOAT>(v.z);
    return result;
  }

  V3 SafeNormalize(const V3& v, const V3& fallback)
  {
    float length = Length(v);
    return length > 0.0001f ? v / length : fallback;
  }

  struct SoundXAudio2Callback : IXAudio2VoiceCallback
  {
    SoundXAudio2Impl* owner;

    SoundXAudio2Callback(SoundXAudio2Impl* owner)
      : owner(owner) {}

    STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) {}
    STDMETHOD_(void, OnVoiceProcessingPassEnd)() {}
    STDMETHOD_(void, OnStreamEnd)() {}
    STDMETHOD_(void, OnBufferStart)(void*) {}
    STDMETHOD_(void, OnLoopEnd)(void*) {}

    STDMETHOD_(void, OnBufferEnd)(void*);
    STDMETHOD_(void, OnVoiceError)(void*, HRESULT);
  };

  struct SoundXAudio2Impl : SoundT
  {
    SoundEngineXAudio2Impl* engine;
    SoundXAudio2Callback callback;
    Array<float> samples;
    uint channels;
    uint sampleRate;
    uint sampleFrames;
    IXAudio2SourceVoice* sourceVoice;
    Object carrier;
    V3 offset;
    float distanceScale;
    bool deleted;
    bool finished;
    bool looped;
    bool playing;
    bool spatial;
    float duration;
    float pan;
    float pitch;
    float volume;
    Vector<float> matrix;

    SoundXAudio2Impl(SoundEngineXAudio2Impl* engine, bool looped = false)
      : engine(engine),
        callback(this),
        channels(1),
        sampleRate(kProceduralSampleRate),
        sampleFrames(0),
        sourceVoice(nullptr),
        offset(0),
        distanceScale(1000),
        deleted(false),
        finished(true),
        looped(looped),
        playing(false),
        spatial(false),
        duration(0),
        pan(0),
        pitch(1),
        volume(1) {}

    ~SoundXAudio2Impl() override { Shutdown(); }

    bool Initialize(const Array<float>& buffer);
    bool Initialize(const DecodedWaveData& decoded);
    void ConfigureSpatial(const Object& carrier, const V3& offset, float distanceDiv);
    void Shutdown();
    bool SubmitBuffer(uint playBeginFrames);
    void ApplyPan();
    void ApplyPitch(float ratio);
    void RefreshFinishedState();
    void UpdateSpatial();
    bool ShouldCollect() const { return deleted || finished; }

    void Delete() override
    {
      deleted = true;
      if (sourceVoice)
      {
        sourceVoice->Stop(0);
        sourceVoice->FlushSourceBuffers();
      }
    }

    bool IsFinished() const override { return finished; }

    bool IsLooped() const override { return looped; }

    float GetDuration() const override { return duration; }

    float GetPan() const override { return pan; }

    float GetPitch() const override { return pitch; }

    float GetVolume() const override { return volume; }

    void SetCursor(float position) override
    {
      if (!sourceVoice || !sampleFrames)
        return;

      uint playBeginFrames = static_cast<uint>(position * sampleRate / 1000.0f);
      if (sampleFrames)
        playBeginFrames %= sampleFrames;

      bool restart = playing;
      sourceVoice->Stop(0);
      sourceVoice->FlushSourceBuffers();
      finished = false;
      if (!SubmitBuffer(playBeginFrames))
      {
        finished = true;
        return;
      }

      if (restart)
        sourceVoice->Start(0);
    }

    void SetPan(float pan) override
    {
      if (pan < -1.0f)
        pan = -1.0f;
      if (pan > 1.0f)
        pan = 1.0f;
      this->pan = pan;
      ApplyPan();
    }

    void SetPlaying(bool playing) override
    {
      this->playing = playing;
      if (!sourceVoice)
        return;
      if (playing && !finished)
        sourceVoice->Start(0);
      else
        sourceVoice->Stop(0);
    }

    void SetPitch(float pitch) override
    {
      if (pitch <= 0.01f)
        pitch = 0.01f;
      if (pitch > XAUDIO2_MAX_FREQ_RATIO)
        pitch = XAUDIO2_MAX_FREQ_RATIO;
      this->pitch = pitch;
      if (spatial)
        UpdateSpatial();
      else
        ApplyPitch(pitch);
    }

    void SetVolume(float volume) override
    {
      if (volume < 0)
        volume = 0;
      this->volume = volume;
      if (sourceVoice)
        sourceVoice->SetVolume(volume);
    }
  };

  struct SoundEngineXAudio2Impl : SoundEngine
  {
    IXAudio2* xaudio;
    IXAudio2MasteringVoice* masteringVoice;
    X3DAUDIO_HANDLE x3d;
    bool comInitialized;
    bool ready;
    bool spatialReady;
    uint outputChannels;
    Vector<Sound> activeSounds;
    Map<String, DecodedWaveData> waveCache;

    SoundEngineXAudio2Impl()
      : xaudio(nullptr),
        masteringVoice(nullptr),
        comInitialized(false),
        ready(false),
        spatialReady(false),
        outputChannels(0)
    {
      memset(x3d, 0, sizeof(x3d));

      HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      if (result == S_OK || result == S_FALSE)
        comInitialized = true;
      else if (result != RPC_E_CHANGED_MODE)
        Log_Warning(Stringize() | "CoInitializeEx failed for XAudio2 (" | static_cast<int>(result) | ")");

      result = XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
      if (FAILED(result))
      {
        Log_Warning(Stringize() | "XAudio2Create failed (" | static_cast<int>(result) | ")");
        return;
      }

      result = xaudio->CreateMasteringVoice(&masteringVoice);
      if (FAILED(result))
      {
        Log_Warning(Stringize() | "CreateMasteringVoice failed (" | static_cast<int>(result) | ")");
        xaudio->Release();
        xaudio = nullptr;
        return;
      }

      XAUDIO2_VOICE_DETAILS details;
      masteringVoice->GetVoiceDetails(&details);
      outputChannels = details.InputChannels ? details.InputChannels : 1;

      DWORD channelMask = 0;
      if (SUCCEEDED(masteringVoice->GetChannelMask(&channelMask)) && channelMask)
      {
        result = X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3d);
        if (SUCCEEDED(result))
          spatialReady = true;
        else
          Log_Warning(Stringize() | "X3DAudioInitialize failed (" | static_cast<int>(result) | ")");
      }

      result = xaudio->StartEngine();
      if (FAILED(result))
      {
        Log_Warning(Stringize() | "XAudio2 StartEngine failed (" | static_cast<int>(result) | ")");
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
        xaudio->Release();
        xaudio = nullptr;
        return;
      }

      ready = true;
    }

    ~SoundEngineXAudio2Impl() override
    {
      for (size_t i = 0; i < activeSounds.size(); ++i)
      {
        SoundXAudio2Impl* sound = activeSounds[i].Cast<SoundXAudio2Impl>();
        sound->Shutdown();
      }
      activeSounds.clear();

      if (masteringVoice)
      {
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
      }

      if (xaudio)
      {
        xaudio->StopEngine();
        xaudio->Release();
        xaudio = nullptr;
      }

      if (comInitialized)
        CoUninitialize();
    }

    bool IsReady() const { return ready; }

    const DecodedWaveData* LoadWave(const String& filename)
    {
      DecodedWaveData* cached = waveCache.get(filename);
      if (cached)
        return cached;

      std::unique_ptr<Array<uchar>> fileData(Location_Resource(kDefaultSoundPath + filename)->Read().release());
      if (!fileData)
        return nullptr;

      DecodedWaveData decoded;
      if (!DecodeWave(filename, *fileData, decoded))
        return nullptr;

      waveCache[filename] = decoded;
      return waveCache.get(filename);
    }

    const char* GetName() const override { return "SoundEngine (XAudio2)"; }

    Sound Play(const Array<float>& buffer) override
    {
      Sound sound = new SoundXAudio2Impl(this, false);
      SoundXAudio2Impl* impl = sound.Cast<SoundXAudio2Impl>();
      if (!impl->Initialize(buffer))
        return sound;

      impl->SetPlaying(true);
      activeSounds << sound;
      return sound;
    }

    Sound Play2D(const String& filename, float volume, bool looped) override
    {
      Sound sound = new SoundXAudio2Impl(this, looped);
      SoundXAudio2Impl* impl = sound.Cast<SoundXAudio2Impl>();
      const DecodedWaveData* decoded = LoadWave(filename);
      if (!decoded || !impl->Initialize(*decoded))
      {
        sound->SetVolume(volume);
        return sound;
      }

      impl->SetVolume(volume);
      impl->SetPlaying(true);
      activeSounds << sound;
      return sound;
    }

    Sound Play3D(const String& filename, const Object& carrier, const V3& offset, float volume, float distanceDiv, bool looped) override
    {
      Sound sound = new SoundXAudio2Impl(this, looped);
      SoundXAudio2Impl* impl = sound.Cast<SoundXAudio2Impl>();
      const DecodedWaveData* decoded = LoadWave(filename);
      if (!decoded)
      {
        sound->SetVolume(volume);
        return sound;
      }

      DecodedWaveData mono = DownmixToMono(*decoded);
      impl->ConfigureSpatial(carrier, offset, distanceDiv);
      if (!impl->Initialize(mono))
      {
        sound->SetVolume(volume);
        return sound;
      }

      impl->SetVolume(volume);
      impl->SetPlaying(true);
      activeSounds << sound;
      return sound;
    }

    void Update() override
    {
      for (size_t i = 0; i < activeSounds.size(); ++i)
      {
        SoundXAudio2Impl* sound = activeSounds[i].Cast<SoundXAudio2Impl>();
        sound->UpdateSpatial();
        sound->RefreshFinishedState();
        if (!sound->ShouldCollect())
          continue;

        sound->Shutdown();
        activeSounds.eraseIndex(i);
        --i;
      }
    }
  };

  STDMETHODIMP_(void) SoundXAudio2Callback::OnBufferEnd(void*)
  {
    owner->finished = true;
    owner->playing = false;
  }

  STDMETHODIMP_(void) SoundXAudio2Callback::OnVoiceError(void*, HRESULT Fatal)
  {
    owner->finished = true;
    owner->playing = false;
    Log_Warning(Stringize() | "XAudio2 voice Fatal (" | static_cast<int>(Fatal) | ")");
  }

  bool SoundXAudio2Impl::Initialize(const Array<float>& buffer)
  {
    samples = buffer;
    channels = 1;
    sampleRate = kProceduralSampleRate;
    sampleFrames = static_cast<uint>(samples.size());
    duration = sampleFrames ? (1000.0f * static_cast<float>(sampleFrames)) / static_cast<float>(sampleRate) : 0;

    if (!engine || !engine->IsReady() || !sampleFrames)
    {
      finished = true;
      return false;
    }

    WAVEFORMATEX format;
    format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    format.nChannels = static_cast<WORD>(channels);
    format.nSamplesPerSec = sampleRate;
    format.nAvgBytesPerSec = sampleRate * channels * sizeof(float);
    format.nBlockAlign = static_cast<WORD>(channels * sizeof(float));
    format.wBitsPerSample = 8 * sizeof(float);
    format.cbSize = 0;

    HRESULT result = engine->xaudio->CreateSourceVoice(&sourceVoice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callback);
    if (FAILED(result))
    {
      Log_Warning(Stringize() | "CreateSourceVoice failed (" | static_cast<int>(result) | ")");
      sourceVoice = nullptr;
      finished = true;
      return false;
    }

    if (!SubmitBuffer(0))
    {
      Shutdown();
      finished = true;
      return false;
    }

    finished = false;
    SetVolume(volume);
    SetPitch(pitch);
    if (spatial)
      UpdateSpatial();
    else
      SetPan(pan);
    return true;
  }

  bool SoundXAudio2Impl::Initialize(const DecodedWaveData& decoded)
  {
    samples = decoded.samples;
    channels = decoded.channels;
    sampleRate = decoded.sampleRate;
    sampleFrames = decoded.sampleFrames;
    duration = sampleFrames ? (1000.0f * static_cast<float>(sampleFrames)) / static_cast<float>(sampleRate) : 0;

    if (!engine || !engine->IsReady() || !sampleFrames)
    {
      finished = true;
      return false;
    }

    WAVEFORMATEX format;
    format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    format.nChannels = static_cast<WORD>(channels);
    format.nSamplesPerSec = sampleRate;
    format.nAvgBytesPerSec = sampleRate * channels * sizeof(float);
    format.nBlockAlign = static_cast<WORD>(channels * sizeof(float));
    format.wBitsPerSample = 8 * sizeof(float);
    format.cbSize = 0;

    HRESULT result = engine->xaudio->CreateSourceVoice(&sourceVoice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callback);
    if (FAILED(result))
    {
      Log_Warning(Stringize() | "CreateSourceVoice failed (" | static_cast<int>(result) | ")");
      sourceVoice = nullptr;
      finished = true;
      return false;
    }

    if (!SubmitBuffer(0))
    {
      Shutdown();
      finished = true;
      return false;
    }

    finished = false;
    SetVolume(volume);
    SetPitch(pitch);
    if (spatial)
      UpdateSpatial();
    else
      SetPan(pan);
    return true;
  }

  void SoundXAudio2Impl::ConfigureSpatial(const Object& carrier, const V3& offset, float distanceDiv)
  {
    this->carrier = carrier;
    this->offset = offset;
    this->distanceScale = Max(1.0f, distanceDiv * 1000.0f);
    this->spatial = true;
  }

  void SoundXAudio2Impl::Shutdown()
  {
    if (sourceVoice)
    {
      sourceVoice->Stop(0);
      sourceVoice->FlushSourceBuffers();
      sourceVoice->DestroyVoice();
      sourceVoice = nullptr;
    }
    engine = nullptr;
  }

  bool SoundXAudio2Impl::SubmitBuffer(uint playBeginFrames)
  {
    if (!sourceVoice || !samples.size())
      return false;

    if (playBeginFrames >= sampleFrames)
      playBeginFrames = sampleFrames - 1;

    XAUDIO2_BUFFER buffer;
    memset(&buffer, 0, sizeof(buffer));
    buffer.AudioBytes = static_cast<UINT32>(samples.size() * sizeof(float));
    buffer.pAudioData = (const BYTE*)samples.data();
    buffer.PlayBegin = playBeginFrames;
    buffer.LoopCount = looped ? XAUDIO2_LOOP_INFINITE : 0;
    buffer.Flags = looped ? 0 : XAUDIO2_END_OF_STREAM;

    HRESULT result = sourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(result))
    {
      Log_Warning(Stringize() | "SubmitSourceBuffer failed (" | static_cast<int>(result) | ")");
      return false;
    }
    return true;
  }

  void SoundXAudio2Impl::ApplyPan()
  {
    if (!sourceVoice || !engine || !engine->masteringVoice || !engine->outputChannels)
      return;

    if (channels == 1 && engine->outputChannels == 1)
    {
      float matrix[1] = {1.0f};
      sourceVoice->SetOutputMatrix(engine->masteringVoice, 1, 1, matrix);
      return;
    }

    if (channels == 1 && engine->outputChannels == 2)
    {
      float matrix[2] = {pan > 0 ? 1.0f - pan : 1.0f, pan < 0 ? 1.0f + pan : 1.0f};
      sourceVoice->SetOutputMatrix(engine->masteringVoice, 1, 2, matrix);
      return;
    }

    if (channels == 2 && engine->outputChannels == 2)
    {
      float leftGain = pan > 0 ? 1.0f - pan : 1.0f;
      float rightGain = pan < 0 ? 1.0f + pan : 1.0f;
      float matrix[4] = {leftGain, 0.0f, 0.0f, rightGain};
      sourceVoice->SetOutputMatrix(engine->masteringVoice, 2, 2, matrix);
      return;
    }

    Vector<float> matrix;
    matrix.resize(channels * engine->outputChannels, 0.0f);
    for (uint channel = 0; channel < channels && channel < engine->outputChannels; ++channel)
      matrix[channel * engine->outputChannels + channel] = 1.0f;
    sourceVoice->SetOutputMatrix(engine->masteringVoice, channels, engine->outputChannels, matrix.data());
  }

  void SoundXAudio2Impl::ApplyPitch(float ratio)
  {
    if (!sourceVoice)
      return;

    if (ratio <= 0.01f)
      ratio = 0.01f;
    if (ratio > XAUDIO2_MAX_FREQ_RATIO)
      ratio = XAUDIO2_MAX_FREQ_RATIO;
    sourceVoice->SetFrequencyRatio(ratio);
  }

  void SoundXAudio2Impl::RefreshFinishedState()
  {
    if (!sourceVoice || looped || finished)
      return;

    XAUDIO2_VOICE_STATE state;
    sourceVoice->GetState(&state);
    if (!state.BuffersQueued)
    {
      finished = true;
      playing = false;
    }
  }

  void SoundXAudio2Impl::UpdateSpatial()
  {
    if (!spatial || !sourceVoice || !engine)
      return;

    if (carrier && carrier->IsDeleted())
    {
      Delete();
      return;
    }

    if (!engine->spatialReady || !engine->masteringVoice || !engine->outputChannels)
    {
      ApplyPan();
      ApplyPitch(pitch);
      return;
    }

    Camera camera = Camera_Get();
    V3 listenerPos = camera ? static_cast<V3>(camera->GetPos()) : V3(0);
    V3 listenerVelocity = camera ? camera->GetVelocity() : V3(0);
    V3 listenerLook = camera ? camera->GetLook() : V3(0, 0, 1);
    V3 listenerUp = camera ? camera->GetUp() : V3(0, 1, 0);

    V3 emitterPos = offset;
    auto emitterVelocity = V3(0);
    if (carrier)
    {
      emitterPos = static_cast<V3>(carrier->GetPos()) + carrier->GetRight() * offset.x + carrier->GetUp() * offset.y + carrier->GetLook() *
        offset.z;
      emitterVelocity = carrier->GetVelocity();
    }

    X3DAUDIO_LISTENER listener;
    memset(&listener, 0, sizeof(listener));
    listener.Position = ToX3D(listenerPos);
    listener.Velocity = ToX3D(listenerVelocity);
    listener.OrientFront = ToX3D(SafeNormalize(listenerLook, V3(0, 0, 1)));
    listener.OrientTop = ToX3D(SafeNormalize(listenerUp, V3(0, 1, 0)));

    X3DAUDIO_EMITTER emitter;
    memset(&emitter, 0, sizeof(emitter));
    emitter.Position = ToX3D(emitterPos);
    emitter.Velocity = ToX3D(emitterVelocity);
    emitter.OrientFront = ToX3D(V3(0, 0, 1));
    emitter.OrientTop = ToX3D(V3(0, 1, 0));
    emitter.ChannelCount = channels;
    emitter.CurveDistanceScaler = distanceScale;
    emitter.DopplerScaler = 1.0f;

    matrix.resize(channels * engine->outputChannels, 0.0f);

    X3DAUDIO_DSP_SETTINGS dsp;
    memset(&dsp, 0, sizeof(dsp));
    dsp.SrcChannelCount = channels;
    dsp.DstChannelCount = engine->outputChannels;
    dsp.pMatrixCoefficients = matrix.data();

    X3DAudioCalculate(engine->x3d, &listener, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp);

    sourceVoice->SetOutputMatrix(engine->masteringVoice, channels, engine->outputChannels, matrix.data());
    ApplyPitch(pitch * dsp.DopplerFactor);
  }
}

SoundEngine* SoundEngine_XAudio2()
{
  auto engine = new SoundEngineXAudio2Impl;
  if (engine->IsReady())
    return engine;

  delete engine;
  Log_Warning("XAudio2 sound backend failed to initialize; falling back to Null");
  return SoundEngine_Null();
}
