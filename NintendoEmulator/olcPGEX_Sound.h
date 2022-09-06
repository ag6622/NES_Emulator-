#ifndef OLC_PGEX_SOUND_H
#define OLC_PGEX_SOUND_H

#include "olcPixelGameEngine.h"
#include <istream>
#include <cstring>
#include <climits>

#include <algorithm>
#undef min
#undef max

// Choose a default sound backend
#if !defined(USE_ALSA) && !defined(USE_OPENAL) && !defined(USE_WINDOWS)
#ifdef __linux__
#define USE_ALSA
#endif

#ifdef __EMSCRIPTEN__
#define USE_OPENAL
#endif

#ifdef _WIN32
#define USE_WINDOWS
#endif

#endif

#ifdef USE_ALSA
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#endif

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <queue>
#endif
#include <mutex>

#pragma pack(push, 1)
typedef struct {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
} OLC_WAVEFORMATEX;
#pragma pack(pop)

namespace olc
{
	// Container class for Advanced 2D Drawing functions
	class SOUND : public olc::PGEX
	{
		// A representation of an affine transform, used to rotate, scale, offset & shear space
	public:
		class AudioSample
		{
		public:
			AudioSample();
			AudioSample(std::string sWavFile, olc::ResourcePack *pack = nullptr);
			olc::rcode LoadFromFile(std::string sWavFile, olc::ResourcePack *pack = nullptr);

		public:
			OLC_WAVEFORMATEX wavHeader;
			float *fSample = nullptr;
			long nSamples = 0;
			int nChannels = 0;
			bool bSampleValid = false;
		};

		struct sCurrentlyPlayingSample
		{
			int nAudioSampleID = 0;
			long nSamplePosition = 0;
			bool bFinished = false;
			bool bLoop = false;
			bool bFlagForStop = false;
		};

		static std::list<sCurrentlyPlayingSample> listActiveSamples;

	public:
		static bool InitialiseAudio(unsigned int nSampleRate = 44100, unsigned int nChannels = 1, unsigned int nBlocks = 8, unsigned int nBlockSamples = 512);
		static bool DestroyAudio();
		static void SetUserSynthFunction(std::function<float(int, float, float)> func);
		static void SetUserFilterFunction(std::function<float(int, float, float)> func);

	public:
		static int LoadAudioSample(std::string sWavFile, olc::ResourcePack *pack = nullptr);
		static void PlaySample(int id, bool bLoop = false);
		static void StopSample(int id);
		static void StopAll();
		static float GetMixerOutput(int nChannel, float fGlobalTime, float fTimeStep);


	private:
#ifdef USE_WINDOWS // Windows specific sound management
		static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
		static unsigned int m_nSampleRate;
		static unsigned int m_nChannels;
		static unsigned int m_nBlockCount;
		static unsigned int m_nBlockSamples;
		static unsigned int m_nBlockCurrent;
		static short* m_pBlockMemory;
		static WAVEHDR *m_pWaveHeaders;
		static HWAVEOUT m_hwDevice;
		static std::atomic<unsigned int> m_nBlockFree;
		static std::condition_variable m_cvBlockNotZero;
		static std::mutex m_muxBlockNotZero;
#endif

#ifdef USE_ALSA
		static snd_pcm_t *m_pPCM;
		static unsigned int m_nSampleRate;
		static unsigned int m_nChannels;
		static unsigned int m_nBlockSamples;
		static short* m_pBlockMemory;
#endif

#ifdef USE_OPENAL
		static std::queue<ALuint> m_qAvailableBuffers;
		static ALuint *m_pBuffers;
		static ALuint m_nSource;
		static ALCdevice *m_pDevice;
		static ALCcontext *m_pContext;
		static unsigned int m_nSampleRate;
		static unsigned int m_nChannels;
		static unsigned int m_nBlockCount;
		static unsigned int m_nBlockSamples;
		static short* m_pBlockMemory;
#endif

		static void AudioThread();
		static std::thread m_AudioThread;
		static std::atomic<bool> m_bAudioThreadActive;
		static std::atomic<float> m_fGlobalTime;
		static std::function<float(int, float, float)> funcUserSynth;
		static std::function<float(int, float, float)> funcUserFilter;
	};
}

#endif // OLC_PGEX_SOUND