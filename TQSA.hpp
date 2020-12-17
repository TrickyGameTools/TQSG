// Lic:
// TQSA.hpp
// Tricky's Quick SDL Audio
// version: 20.09.01
// Copyright (C) 2020 Jeroen P. Broks
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// EndLic
#pragma once

// C++
#include <memory>
#include<string>

// SDL
#include <sdl.h>
#include <SDL_mixer.h>

// JCR6
#include <jcr6_core.hpp>

namespace TrickyUnits {

	class TQSA_Audio {
	private:
		Mix_Chunk* ActualChunk = NULL;
	public:
		/// <summary>
		/// Disposes the sound chunk attacked from the memory (would happen automatically if the destructor is called)
		/// </summary>
		void Kill();

		void Load(const char* File);
		void Load(jcr6::JT_Dir JCRResource, std::string JCREntry);
		void Load(std::string JCRMain, std::string JCREntry);

		int Play(int loops = 0);
		void ChPlay(int channel, int loops = 0);

		bool HasChunk();
		bool AutoKill{ true }; // Use with CARE!
		~TQSA_Audio();
	};

	/* for now not interested!
	class TQSA_Music {

	};
	*/

	class TQSA_AutoAudioReal {
	private:
		TQSA_Audio ActualAudio;
	public:
		TQSA_Audio* Audio(); // Pointer needed for some stuff, but I reocmmend against calling this directly.
		bool AlwaysLoop{ false };
		int Play(int loops = 0);
		void ChPlay(int channel, int loops = 0);
		bool HasChunk();
		TQSA_AutoAudioReal();
		~TQSA_AutoAudioReal();
	};
	typedef std::shared_ptr<TQSA_AutoAudioReal> TQSA_AutoAudio;

	TQSA_AutoAudio LoadAudio(const char* File);
	TQSA_AutoAudio LoadAudio(std::string MainFile, std::string Entry);
	TQSA_AutoAudio LoadAudio(jcr6::JT_Dir& MainFile, std::string Entry);

	bool TQSA_Init(int demandflags = 0);
	void TQSA_Close();
}