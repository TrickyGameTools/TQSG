#include "TQSG.hpp"
// Lic:
// TQSG.cpp
// TQSG Code
// version: 20.08.22
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

// TODO: Bundle support over JCR

// C++
#include <string>

// Myself
#include <TQSG.hpp>

// Tricky's Units
#include <QuickStream.hpp>
#include <QuickString.hpp>

// JCR6
#include <jcr6_core.hpp>


using namespace std;

namespace TrickyUnits {

	static string LastError = "";
	static SDL_Renderer* gRenderer;

	double scalex = 1, scaley = 1;

	string TQSG_GetError() { return LastError; }

	void TQSG_Image::LoadRWops(SDL_RWops* data, int frame, int autofree) {
		LastError = "";
		if (frame >= Textures.size()) { 
			char FE[400];
			sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
			LastError = FE; 
			return;
		}
		auto buf = IMG_LoadTexture_RW(gRenderer, data, autofree);
		if (buf == NULL) { LastError = "Getting texture from SLD_RWops failed!"; return; }
		if (frame < 0) {
			Textures.push_back(buf);
		}
		else {
			Textures[frame] = buf;
		}
	}

	TQSG_Image::TQSG_Image(SDL_RWops* data, int autofree) {
		LastError = "";
		LoadRWops(data, -1, autofree);
	}

	TQSG_Image::TQSG_Image(std::string file) {
		LastError = "";
		Textures.clear(); // Should not be needed, but to be 100% sure!
		if (!FileExists(file)) {
			LastError = "File \"" + file + "\" not found";
			return;
		}
		if (jcr6::Recognize(file) != "") {
			LastError = "No support for bundled jcr yet!\nRome wasn't built in one day either, you know!";
		}
		SDL_Surface* loadedSurface = IMG_Load(file.c_str());
		if (loadedSurface == NULL) {
			char FE[300];
			sprintf_s(FE, 295, "Unable to load image %s!\nSDL_image Error: %s", file.c_str(), IMG_GetError());
			LastError = FE;
			return;
		}
		//Create texture from surface pixels
		auto newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL) {
			char FE[300];
			sprintf_s(FE, 295, "Unable to create texture from %s!\nSDL Error: %s", file.c_str(), SDL_GetError());
			LastError = FE;
			return;
		}
		Textures.push_back(newTexture);
	}

	static SDL_Texture* Tex_From_JCR(jcr6::JT_Dir& JD, std::string entry) {
		SDL_RWops* RWBuf = NULL;
		jcr6::JT_Entry E = JD.Entry(entry);
		jcr6::JT_EntryReader buf;
		JD.B(entry, buf);
		RWBuf = SDL_RWFromMem(buf.pointme(), buf.getsize());
		SDL_Texture* ret = IMG_LoadTexture_RW(gRenderer, RWBuf, 1);
		return ret;
	}

	void TQSG_Image::TrueLoadJCR(jcr6::JT_Dir& JCR, std::string entry) {
		LastError = "";
		Textures.clear();
		if (JCR.EntryExists(entry)) {
			// Load Entry
			Textures.push_back(Tex_From_JCR(JCR, entry));
		}
		else if (JCR.DirectoryExists(entry)) {
			// Bundle
			auto ue = TReplace(Upper(entry), '\\', '/');
			if (!suffixed(ue, "/")) ue += "/";
			for (auto LT : JCR.Entries()) {
				if (prefixed(LT.first, ue)) Textures.push_back(Tex_From_JCR(JCR, LT.first));
			}
		}
		else {
			char FE[300];
			sprintf_s(FE, 295, "I could not find any JCR data for image \"%s\"!", entry.c_str());
			LastError = FE;
		}
	}

	TQSG_Image::TQSG_Image(jcr6::JT_Dir& JCR, std::string entry) {
		TrueLoadJCR(JCR, entry);
	}

	TQSG_Image::TQSG_Image(std::string mainfile, std::string entry) {
		auto J = jcr6::Dir(mainfile);
		TrueLoadJCR(J, entry);
	}

	void TQSG_Image::Draw(int x, int y, int frame) {
		LastError = "";
		if (frame < 0 || frame >= Textures.size()) {
			char FE[400];
			sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
			LastError = FE;
			return;
		}
		SDL_Rect Target;
		Target.x = x - ceil(hotx*scalex);
		Target.y = y - ceil(hoty*scaley);
		Target.w = ceil(Width() * scalex);
		Target.h = ceil(Height() * scaley);
		SDL_RenderCopy(gRenderer, Textures[frame], NULL, &Target);
	}

	int TQSG_Image::Width()	{
		LastError = "";
		if (Textures.size() == 0) { LastError = "No textures!"; return 0; }
		int w, h;
		SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
		return w;
	}

	int TQSG_Image::Height() {
		LastError = "";
		if (Textures.size() == 0) { LastError = "No textures!"; return 0; }
		int w, h;
		SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
		return h;
	}

	void TQSG_Image::KillAll() {
		for (int i = 0; i <= Textures.size(); ++i) SDL_DestroyTexture(Textures[i]);
	}


	TQSG_Image::~TQSG_Image() {
		KillAll();
	}

	void SetScale(double x, double y) {
		scalex = x;
		scaley = y;
	}

	void GetScale(double& x, double& y) {
		x = scalex;
		y = scaley;
	}

	void TQSG_Init() {
		// TODO!
	}

	void TQSG_Close() {
		// TODO!
	}
}