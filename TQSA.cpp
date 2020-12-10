// Lic:
// TQSA.cpp
// Tricky's Quick SDL Audio
// version: 20.09.05
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
#include <iostream>
#include "TQSA.hpp"

TQSA_AutoAudio LoadAudio(const char* File) {
	auto ret{ std::make_shared<TQSA_AutoAudioReal>() };
	ret->Audio()->Load(File);
	return ret;
}

TQSA_AutoAudio LoadAudio(std::string MainFile, std::string Entry) {
	auto ret{ std::make_shared<TQSA_AutoAudioReal>() };
	ret->Audio()->Load(MainFile,Entry);
	return ret;
}

TQSA_AutoAudio LoadAudio(jcr6::JT_Dir& MainFile, std::string Entry) {
	auto ret{ std::make_shared<TQSA_AutoAudioReal>() };
	ret->Audio()->Load(MainFile, Entry);
	return ret;
}

bool TQSA_Init(int demandflags) {
	bool success = true;
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cout << "\aMix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048) failed\n";
		return false;
	}
	int flags=  Mix_Init(MIX_INIT_OGG | MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MID);
	success = success && (flags != 0);
	if (demandflags) success = success && ((flags & demandflags)==demandflags);
	if (!success) std::cout << "Initizing the mixer has failed\n";
	return success;
}

void TQSA_Close() {
	Mix_CloseAudio();
}

void TQSA_Audio::Kill() {
	if (ActualChunk) {
		Mix_FreeChunk(ActualChunk);
		ActualChunk = NULL;
	}
}

void TQSA_Audio::Load(const char* File) {
	Kill();
	ActualChunk = Mix_LoadWAV(File);
	if (!ActualChunk) std::cout << "Loading from file \"" << File << "\" failed!\n";
}

void TQSA_Audio::Load(jcr6::JT_Dir JCRResource, std::string JCREntry) {
	Kill();
	if (!JCRResource.EntryExists(JCREntry)) { std::cout << "Loading JCR Entry \"" << JCREntry << "\" not possible as the entry does not exist!\n"; return; }
	SDL_RWops* RWBuf = NULL;
	jcr6::JT_Entry E = JCRResource.Entry(JCREntry);
	jcr6::JT_EntryReader buf;
	JCRResource.B(JCREntry, buf);
	RWBuf = SDL_RWFromMem(buf.pointme(), buf.getsize());
	ActualChunk = Mix_LoadWAV_RW(RWBuf, 1);
	if (!JCRResource.EntryExists(JCREntry)) { std::cout << "Loading JCR Entry \"" << JCREntry << "\" failed!\n"; return; }
}

void TQSA_Audio::Load(std::string JCRMain, std::string JCREntry) {
	auto JD = jcr6::Dir(JCRMain);
	Load(JD, JCREntry);
	if (!ActualChunk) std::cout << "= Failure from main: " << JCRMain << "\n";
}

int TQSA_Audio::Play(int loops) {
	return Mix_PlayChannel(-1,ActualChunk,loops);
}

void TQSA_Audio::ChPlay(int channel, int loops) {
	Mix_PlayChannel(channel, ActualChunk, loops);
}

bool TQSA_Audio::HasChunk() {
	return ActualChunk!=NULL;
}

TQSA_Audio::~TQSA_Audio() {
	if (AutoKill) Kill();
}

TQSA_Audio* TQSA_AutoAudioReal::Audio() {
	return &ActualAudio;
}

int TQSA_AutoAudioReal::Play(int loops) {
	return ActualAudio.Play(loops||AlwaysLoop);	
}

void TQSA_AutoAudioReal::ChPlay(int channel, int loops) {
	return ActualAudio.ChPlay(channel, loops || AlwaysLoop);
}

bool TQSA_AutoAudioReal::HasChunk() {
	return ActualAudio.HasChunk();
}

TQSA_AutoAudioReal::TQSA_AutoAudioReal() { ActualAudio.AutoKill = false; }

TQSA_AutoAudioReal::~TQSA_AutoAudioReal() {	ActualAudio.Kill();}
