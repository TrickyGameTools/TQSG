// Lic:
// TQSG.cpp
// TQSG Code
// version: 20.08.27
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
#include "TQSG.hpp"

// TODO: Bundle support over JCR

// C++
#include <string>
#include <algorithm>

// Myself
#include <TQSG.hpp>

// Tricky's Units
#include <QuickStream.hpp>
#include <QuickString.hpp>

// JCR6
#include <jcr6_core.hpp>

#undef TQSG_Debug

using namespace std;

namespace TrickyUnits {

	static void Ouwehoeren(std::string whatever) {
#ifdef TQSG_Debug
		printf("\x1b[32mTQSG DEBUG>\x1b[0m %s\n", whatever.c_str());
#endif
	}

	static string LastError = "";

	//The window we'll be rendering to
	static SDL_Window* gWindow = NULL;

	//The surface contained by the window
	static SDL_Surface* gScreenSurface = NULL;
	static SDL_Renderer* gRenderer = NULL;

	

	static double scalex = 1, scaley = 1;
	static Uint8 tcr = 255, tcg = 255, tcb = 255, tcalpha=255;

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

	void TQSG_Image::Create(SDL_RWops* data, int autofree) {
		Textures.clear();
		LastError = "";
		LoadRWops(data, -1, autofree);
	}

	void TQSG_Image::Create(SDL_Texture* data) {
		Textures.clear();
		LastError = "";
		Textures.push_back(data);
	}

	void TQSG_Image::Create(std::string file) {		
		LastError = "";
		Textures.clear();
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

	void TQSG_Image::Create(jcr6::JT_Dir& JCR, std::string entry) {
		TrueLoadJCR(JCR, entry);
	}

	void TQSG_Image::Create(std::string mainfile, std::string entry) {
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
		//printf("DEBUG! B.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		SDL_SetTextureColorMod(Textures[frame],tcr, tcg, tcb);
		//printf("DEBUG! A.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		SDL_Rect Target;
		Target.x = x - (int)ceil(hotx*scalex);
		Target.y = y - (int)ceil(hoty*scaley);
		Target.w = (int)ceil(Width() * scalex);
		Target.h = (int)ceil(Height() * scaley);
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
		for (int i = 0; i < Textures.size(); ++i) SDL_DestroyTexture(Textures[i]);
	}


	TQSG_Image::~TQSG_Image() {
		if (AutoClean) KillAll();
	}

	void SetScale(double x, double y) {
		scalex = x;
		scaley = y;
	}

	void GetScale(double& x, double& y) {
		x = scalex;
		y = scaley;
	}

	static int CLSR = 0, CLSG = 0, CLSB = 0;

	void TQSG_ClsColor(int r, int g, int b) {
		CLSR = r;
		CLSG = g;
		CLSB = b;
	}

	void TQSG_Cls() {
		//SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, CLSR, CLSG, CLSB));
		Uint8  r, g, b,a;        
		SDL_GetRenderDrawColor(gRenderer, &r, &g, &b, &a);
		SDL_SetRenderDrawColor(gRenderer, CLSR, CLSG, CLSB, 255);
		SDL_RenderClear(gRenderer);
		SDL_SetRenderDrawColor(gRenderer, r,g,b,a);		
	}

	void TQSG_Plot(int x, int y) {
		SDL_SetRenderDrawColor(gRenderer, tcr, tcg, tcb,tcalpha);
		SDL_RenderDrawPoint(gRenderer,x, y);
	}

	int TQSG_ScreenWidth() {
		int w, h;
		SDL_GetRendererOutputSize(gRenderer, &w, &h);
		return w;
	}
	int TQSG_ScreenHeight() {
		int w, h;
		SDL_GetRendererOutputSize(gRenderer, &w, &h);
		return h;
	}

	void TQSG_ScreenSize(int* w, int* h) {
		SDL_GetRendererOutputSize(gRenderer, w, h);
	}


	void TQSG_Flip() {
		//SDL_UpdateWindowSurface(gWindow);
		SDL_RenderPresent(gRenderer);
	}

	void TQSG_Color(Uint8 r, Uint8 g, Uint8 b)	{
		tcr = r;
		tcg = g;
		tcb = b;
	}

	void TQSG_GetColor(Uint8* r, Uint8* g, Uint8* b) {
		*r = tcr;
		*g = tcg;
		*b = tcb;
	}

	void TQSG_SetAlpha(Uint8 a) {
		tcalpha = a;
	}

	Uint8 TQSG_GetAlpha() {
		return tcalpha;
	}

	void TQSG_Rect(int x, int y, int w, int h) {
		//SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 0xFF, 0x00, 0xFF));
		//printf("WARNING! Rect not fully implemented yet"); 
		SDL_Rect r;
		r.x = x;
		r.y = y;
		r.w = w;
		r.h = h;
		SDL_SetRenderDrawColor(gRenderer, tcr, tcg, tcb, tcalpha);
		SDL_RenderDrawRect(gRenderer, &r);
	}

	void TQSG_Circle(int x, int y, int radius) {
		SDL_SetRenderDrawColor(gRenderer, tcr, tcg, tcb, tcalpha);
		for (double i = 0; i < 2 * 3.14; i += .1) {
			SDL_RenderDrawPoint(gRenderer, floor(sin(i) * radius) + x, floor(cos(i) * radius) + y);
		}
	}

	bool TQSG_Init(string WindowTitle,int WinWidth,int WinHeight,bool fullscreen) {
		printf("Starting graphics screen: %dx%d; fullscreen=%d\n", WinWidth, WinHeight, fullscreen);
		// TODO!
				//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
			return false; //success = false;
		}
		else
		{
			//Create window
			gWindow = SDL_CreateWindow(WindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WinWidth, WinHeight, SDL_WINDOW_SHOWN);
			if (gWindow == NULL)
			{
				printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
				return false;//success = false;
			}
			else
			{
				if (fullscreen) {
					int fcsuc = SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
					if (fcsuc != 0) {
						printf("\a\x1b[33mWARNING!\x1b[0m\t Going into full screen was unsuccesful");
					}
				}
				gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
				if (gRenderer == NULL)
				{
					printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
					SDL_DestroyWindow(gWindow);
					return false;
					// success = false;
				}
				else
				{
					//Initialize renderer color
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

					//Initialize PNG loading
					int imgFlags = IMG_INIT_PNG;
					if (!(IMG_Init(imgFlags) & imgFlags))
					{
						printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
						SDL_DestroyWindow(gWindow);
						SDL_DestroyRenderer(gRenderer);
						return false;

						//success = false;
					}
				}

				//Get window surface
				gScreenSurface = SDL_GetWindowSurface(gWindow);

				// Load Death
				//printf("Loading Death Picture\n");
				//Tex_Death = Tex_From_JCR(ARF, "Pics/Death.png");
#ifdef TQSG_AllowTTF
				Ouwehoeren("Init TTF");
				if (TTF_Init() == -1) {
					printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
					return false;
				}
#endif

				return true;
			}
		}

	}

	void TQSG_Close() {
		TTF_Quit();
		IMG_Quit();
		SDL_DestroyRenderer(gRenderer);
		SDL_DestroyWindow(gWindow);

	}

#ifdef TQSG_AllowTTF
	void TQSG_TTF::Kill() {		
		if (FontItself == NULL) return;
		TTF_CloseFont(FontItself);
		FontItself = NULL;
	}

	TQSG_Image TQSG_TTF::Text(const char* txt) {
		TQSG_Image ret;
		LastError = "";
		SDL_Surface* textSurface = TTF_RenderText_Solid(FontItself, txt, { 0xff,0xff,0xff });
		if (textSurface == NULL) {
			char FE[300];
			sprintf_s(FE,295,"Unable to render text surface!\nSDL_ttf Error: %s", TTF_GetError());
			LastError = FE;
			return ret;
		}
		SDL_Texture* mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL) {
			char FE[300];
			sprintf_s(FE,295,"Unable to create texture from rendered text!\nSDL Error: %s", SDL_GetError());
			LastError = FE;
			return ret;
		}
		SDL_FreeSurface(textSurface);
		ret.Create(mTexture);
		// No, we don't need to rlease mTexture... In fact we may absolutely NOT do that at this point!
		return ret;
	}



	TQSG_Image TQSG_TTF::Text(std::string txt) {
		return Text(txt.c_str());
	}

	void TQSG_TTF::Draw(const char* txt, int x, int y,unsigned char ha, unsigned char va) {
		int mx = 0, my = 0;
		LastError = "";
		if (FontItself == NULL) { 
			LastError = "Trying to blit text without a font loaded!";  
//#ifdef TQSG_Debug
			printf("ERROR! %s\n", LastError.c_str());
//#endif
			return; 
		}
		TQSG_Image I;
		// return; // debug
		try {
			// Create I
			SDL_Color fg; fg.r = 255; fg.g = 255; fg.b = 255; fg.a = 255;
			SDL_Surface* textSurface = TTF_RenderText_Solid(FontItself, txt, fg);
			SDL_Texture* mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
			SDL_FreeSurface(textSurface);
			I.Create(mTexture);

			switch (ha) {
			case 0: break;
			case 1: mx = I.Width(); break;
			case 2: my = floor(I.Width() / 2); break;
			default:
				LastError = "Unknown horizontal alignment";
				return;
			}
			switch (va) {
			case 0: break;
			case 1: my = I.Height(); break;
			case 2: my = floor(I.Height() / 2); break;
			default:
				LastError = "Unknown vertical alignment";
				return;
			}
			I.Draw(x - mx, y - my);
		} catch(const std::exception& e){
			printf("ERROR: %s\n",e.what());
			LastError = "Drawing text failed: ";
		} 
		I.KillAll();
		
	}

	void TQSG_TTF::LoadFont(SDL_RWops* buf, int size, int autofree) {
		Ouwehoeren("Loading font from RWops buffer!");
		if (FontItself != NULL) {
			TTF_CloseFont(FontItself); Ouwehoeren("Closing old font on this buffer");
		}
		LastError = "";
		FontItself = TTF_OpenFontRW(buf, autofree, size);
		if (FontItself==NULL) {
			char FE[300];
			sprintf_s(FE,296,"Error loading font buffer\n%s!", SDL_GetError());
			LastError = FE;					
		}
	}

	void TQSG_TTF::LoadFont(std::string file, int fontsize) {
		if (FontItself != NULL) TTF_CloseFont(FontItself);
		FontItself = TTF_OpenFont(file.c_str(), fontsize);
		char FE[300];
		sprintf_s(FE, 296, "Error loading font file\"%s\" (size %d)\n%s!", file.c_str(),fontsize,SDL_GetError());
		LastError = FE;
	}

	void TQSG_TTF::LoadFont(jcr6::JT_Dir& JCRRes, std::string Entry, int fontsize) {
		Ouwehoeren("Retrieving \"" + Entry + "\" from JCR6 resource in order to get a font out of it!");
		jcr6::JT_EntryReader buf;
		JCRRes.B(Entry, buf); if (JCR_Error != "Ok") { LastError = "JCR6 Error: " + JCR_Error; return; }
		auto RWops = SDL_RWFromMem(buf.pointme(), buf.getsize());
		if (RWops == NULL) { LastError = "Coulnd't retrieve \"" + Entry + "\" from JCR6 resource!"; return; }
		LoadFont(RWops, fontsize, 1);
	}

	void TQSG_TTF::LoadFont(std::string JCRFile, std::string Entry, int fontsize) {
		auto JCR = jcr6::Dir(JCRFile); if (JCR_Error != "Ok") { LastError = "JCR6 Error: " + JCR_Error; return; }
		LoadFont(JCR, Entry, fontsize);
	}

	void TQSG_TTF::Draw(std::string txt, int x, int y, unsigned char ha, unsigned char va) {
		Draw(txt.c_str(), x, y,ha,va);
	}

	bool TQSG_TTF::HasFont() {
		return FontItself != NULL;
	}

	TQSG_TTF::~TQSG_TTF() { 
		Ouwehoeren("TFF Destructor called");
		Kill(); 
	}
#endif
	void TQSG_ImageFont::SubLoad(Uint8 base, Uint8 ch) {
		if (TriedToLoad.count(base)==0){}
		else if (TriedToLoad[base].count(ch) == 0) {}
		else if (TriedToLoad[base][ch]) return; // If loaded before, don't load it again
		if (!JCR_Linked) {
			char FE[300];
			sprintf_s(FE, 298, "Tried to load sub for [%d,%d] while no JCR6 resource has been linked", base, ch);
			LastError = FE;
			printf("\x1b[31mERROR!\x1b\t%s", FE);
			return;
		}
		Ouwehoeren("Loading new letter (" + to_string(base) + "," + to_string(ch) + ")");
		TriedToLoad[base][ch] = true;
		LastError = "";
		std::string entry = Prefix;
		if (base == 0) {
			entry += to_string(ch) + ".png";
		} else {
			entry += to_string(base) + "." + to_string(ch) + ".png";
		}
		if (JCR.EntryExists(entry)) {
			// Load Entry
			Letter[base][ch] = Tex_From_JCR(JCR, entry);
			if (Letter[base][ch] == NULL) {
				Letter[base].erase(ch); // Enforces ignoring
				printf("ERROR! Loading letter (%d,%d) failed: %s", base, ch, JCR_Error.c_str());
				LastError = JCR_Error;
			} else {
				int w, h;
				SDL_QueryTexture(Letter[base][ch], NULL, NULL, &w, &h);
				width = std::max(width,  w);
				height = std::max(height, h);
			}
		} else {
			printf_s("WARNING! Desired character not found \"%s\"!\tWill be ignored from now on!\n", entry.c_str());			
		}


	}

	void TQSG_ImageFont::Kill() {
		for (auto& base : Letter) {
			for (auto& ch : base.second) {
				Ouwehoeren("ImageFont Kill: (" + to_string(base.first) + "," + to_string(ch.first) + ")");
				SDL_DestroyTexture(ch.second);
			}
		}
		Fixed = false;
		JCR_Linked = false;
		Letter.clear();
		TriedToLoad.clear();
	}

// #pragma message ("TextWidth NOT yet done, so always returning 0 for now (unless fixed)")
	int TQSG_ImageFont::TextWidth(const char* txt) {
		int pos = 0;
		int maxw = 0;
		int tempw = 0;
		while (txt[pos]) {
			if (txt[pos] == '\n')
				tempw = 0;
			else if (txt[pos]=='|') {
				if (Fixed) {
					pos += 2;
					tempw += width;
				} else {
					Uint8 base = txt[pos + 1];
					Uint8 ch = txt[pos + 2];
					if (Letter.count(base) == 0 || Letter[base].count(ch) == 0) SubLoad(base, ch);
					int w, h;
					SDL_QueryTexture(Letter[base][ch], NULL, NULL, &w, &h);
					tempw += w;
					pos += 2;
				}
			} else if (Fixed)
				tempw += width;
			else {
				auto ch = txt[pos];
				if (Letter.count(0) == 0 || Letter[0].count(ch) == 0) SubLoad(0, ch);
				int w, h;
				SDL_QueryTexture(Letter[0][ch], NULL, NULL, &w, &h);
				tempw += w;
			}
			if (tempw > maxw) maxw = tempw;
			pos++;
		}
		//if (Fixed) return (int)strlen(txt) * width;
		return maxw;
	}

	int TQSG_ImageFont::TextHeight(const char* txt) {
		int pos=0;
		int lines=1;
		if (txt[0] == 0) return 0; // No need to check the rest!
		while (txt[pos]) {
			if (txt[pos] == '|')
				pos += 2; // No need to check more out! 
			else if (txt[pos] == '\n')
				lines++;
			pos++;
		}
		//printf("<TextHeight height='%d' lines='%d' return='%d' >\n%s\n</TextHeight>\n",height,lines,height*lines,txt); // debug
		return height*lines;
		// This is not entirely correct since line breaks can exist... Oh well.
	}

	void TQSG_ImageFont::DrawLetter(Uint8 base, Uint8 ch, int x, int y) {
		if (Letter.count(base) == 0 || Letter[base].count(ch) == 0) SubLoad(base, ch);
		if (Letter.count(base) == 0 || Letter[base].count(ch) == 0) return;
		if (Letter[base][ch] == NULL) return;
		SDL_SetTextureColorMod(Letter[base][ch], tcr, tcg, tcb);
		//printf("DEBUG! A.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		int w, h;
		SDL_QueryTexture(Letter[base][ch], NULL, NULL, &w, &h);
		SDL_Rect Target;
		Target.x = x ;
		Target.y = y ;
		Target.w = w;
		Target.h = h;
		SDL_RenderCopy(gRenderer, Letter[base][ch], NULL, &Target);
	}

	void TQSG_ImageFont::Draw(const char* txt, int x, int y, unsigned char ha, unsigned char va) {
		int mx = 0, my = 0;
		switch (ha) {
		case 0: break;
		case 1: mx = TextWidth(txt); break;
		case 2: mx = (int)ceil(TextWidth(txt) / 2); break;
		default: LastError = "Unknown horizontal alignment"; return;
		}
		switch (va) {
		case 0: break;
		case 1: my = TextHeight(txt); break;
		case 2: my = (int)ceil(TextHeight(txt) / 2); break;
		default: LastError = "Unknown vertical alignment"; return;
		}
		int wx = x - mx;
		int wy = y - my;
		int pos = 0;
		while (txt[pos]) {
			switch (txt[pos]) {
			case 0: return; // Safety precaution
			case '\n': wx = x - mx; wy += height; break;
			case '\r': break; // Ignore Ignore Ignore!
			case '|': {
				pos += 1; if (!txt[pos]) {
					LastError = "Unexpected end of DrawText string";
					return;
				}
				Uint8 base = (Uint8)txt[pos];
				pos += 1; if (!txt[pos]) {
					LastError = "Unexpected end of DrawText string";
					return;
				}
				Uint8 ch = (Uint8)txt[pos];
				DrawLetter(base, ch, wx, wy);
				if (Fixed)
					wx += width;
				else {
					int w, h;
					SDL_QueryTexture(Letter[base][ch], NULL, NULL, &w, &h);
					wx += w;
				}
				break;
			}
			default: {
				Uint8 ch = (Uint8)txt[pos];
				DrawLetter(0, ch, wx, wy);
				if (Fixed)
					wx += width;
				else {
					int w, h;
					SDL_QueryTexture(Letter[0][ch], NULL, NULL, &w, &h);
					wx += w;
				}
				break;
			}
			}
			++pos;
		}
	}

	void TQSG_ImageFont::Draw(std::string txt, int x, int y, unsigned char ha, unsigned char va) {
		Draw(txt.c_str(), x, y, ha, va);
	}

	void TQSG_ImageFont::LoadFont(jcr6::JT_Dir& JCRRes, std::string Bundle, bool all) {
		JCR = JCRRes;
		Prefix = Bundle;
		JCR_Linked = true;
		if (Prefix != "") if (!suffixed(Prefix, "/")) Prefix += "/";
		if (all) {
			for (int base = 0; base < 256; base++) for (int ch = 0; ch < 256; ch++) SubLoad(base, ch);
		}
	}
	void TQSG_ImageFont::LoadFont(std::string JCRFile, std::string Bundle, bool all) {
		jcr6::JT_Dir J;
		J = jcr6::Dir(JCRFile);
		string E = JCR_Error;
		if (E != "Ok") { LastError = "LoadFont: JCR Error on loading " + JCRFile + "::" + Bundle + "\n" + E; return; }
		LoadFont(J, Bundle,all);
	}

	void TQSG_ImageFont::LoadFont(std::string file, bool all) {
		LoadFont(file, "", all);
	}

	TQSG_ImageFont::~TQSG_ImageFont() {
		printf("Image font destruction call\n");
		Kill();
	}

	TQSG_ImageFont::TQSG_ImageFont() {
	}


}