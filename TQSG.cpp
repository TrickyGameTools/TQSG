// Lic:
// TQSG.cpp
// TQSG Code
// version: 20.12.29
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
#include <iostream>
#include <string>
#include <algorithm>
#include <memory>

// Myself
#include "TQSG.hpp"

// Tricky's Units
#include <QuickStream.hpp>
#include <QuickString.hpp>
#include <TrickySTOI.hpp>

// JCR6
#include <jcr6_core.hpp>

#undef TQSG_Debug

using namespace std;

namespace TrickyUnits {

	// This code is obtained from Stack Overflow. It's only meant for internal use
	typedef struct {
		double r;       // a fraction between 0 and 1
		double g;       // a fraction between 0 and 1
		double b;       // a fraction between 0 and 1
	} rgb;

	typedef struct {
		double h;       // angle in degrees
		double s;       // a fraction between 0 and 1
		double v;       // a fraction between 0 and 1
	} hsv;

	static hsv   rgb2hsv(rgb in);
	static rgb   hsv2rgb(hsv in);

	hsv rgb2hsv(rgb in) {
		hsv         out;
		double      min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min < in.b ? min : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max > in.b ? max : in.b;

		out.v = max;                                // v
		delta = max - min;
		if (delta < 0.00001) {
			out.s = 0;
			out.h = 0; // undefined, maybe nan?
			return out;
		}
		if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
			out.s = (delta / max);                  // s
		} else {
			// if max is 0, then r = g = b = 0              
			// s = 0, h is undefined
			out.s = 0.0;
			out.h = NAN;                            // its now undefined
			return out;
		}
		if (in.r >= max)                           // > is bogus, just keeps compilor happy
			out.h = (in.g - in.b) / delta;        // between yellow & magenta
		else
			if (in.g >= max)
				out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
			else
				out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

		out.h *= 60.0;                              // degrees

		if (out.h < 0.0)
			out.h += 360.0;

		return out;
	}


	rgb hsv2rgb(hsv in) {
		double      hh, p, q, t, ff;
		long        i;
		rgb         out;

		if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
			out.r = in.v;
			out.g = in.v;
			out.b = in.v;
			return out;
		}
		hh = in.h;
		if (hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = in.v * (1.0 - in.s);
		q = in.v * (1.0 - (in.s * ff));
		t = in.v * (1.0 - (in.s * (1.0 - ff)));

		switch (i) {
		case 0:
			out.r = in.v;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = in.v;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = in.v;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = in.v;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = in.v;
			break;
		case 5:
		default:
			out.r = in.v;
			out.g = p;
			out.b = q;
			break;
		}
		return out;
	}


	static void Ouwehoeren(std::string whatever) {
#ifdef TQSG_Debug
		printf("\x1b[32mTQSG DEBUG>\x1b[0m %s\n", whatever.c_str());
#endif
	}

	string TError{ "No Error" };
	TQSG_PanicType TQSG_Panic = NULL;

	static string LastError = "";

	//The window we'll be rendering to
	static SDL_Window* gWindow = NULL;

	//The surface contained by the window
	static SDL_Surface* gScreenSurface = NULL;
	static SDL_Renderer* gRenderer = NULL;

	

	static double scalex = 1, scaley = 1;
	static Uint8 tcr = 255, tcg = 255, tcb = 255, tcalpha=255;
	static double rotatedegrees = 0;
	static SDL_RendererFlip imgflip = SDL_FLIP_NONE;
	static SDL_BlendMode BlendMode = SDL_BLENDMODE_BLEND;

	static int TQSG_OriginX{ 0 };
	static int TQSG_OriginY{ 0 };

	static void _Panic(std::string errormessage) {
		LastError = errormessage;
		if (TQSG_Panic) TQSG_Panic(errormessage);
	}

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

	int TQSG_Image::Frames() {
		if (altframing)
			return AltFrames.size();
		else
			return Textures.size();
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
			if (JCR.EntryExists(StripExt(entry) + ".frames")) {
				auto frameinst = JCR.String(StripExt(entry) + ".frames");
				auto params = Split(frameinst, ',');
				AltFrame(stoi(params[0]), stoi(params[1]), stoi(params[3]));
				if (stoi(params[2]) != 0) printf("\x7Warning! Parameter 3 contained valued %d. The TQSG engine ignores this value!", stoi(params[2]));
			}
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
			//LastError = FE;
			_Panic(FE);
		}
	}

	void TQSG_Image::Create(jcr6::JT_Dir& JCR, std::string entry) {
		TrueLoadJCR(JCR, entry);
	}

	void TQSG_Image::Create(std::string mainfile, std::string entry) {
		auto J = jcr6::Dir(mainfile);
		TrueLoadJCR(J, entry);
	}

	void TQSG_Image::AltFrame(int w, int h, int num) {
		LastError = "";
		if (altframing) {
			//LastError = "Image already altframed";
			_Panic("Image already altframed");
			return;
		}
		if (Textures.size() != 1) {
			//LastError = "Altframing only possible when there is 1 single texture in an image. No more, no less!";
			_Panic("Altframing only possible when there is 1 single texture in an image. No more, no less!");
			return;
		}
		int ow = Width();
		int oh = Height();
		int collumns = floor(ow / w);
		int rows = floor(oh / h);
		if (collumns * rows < num) {
			char FE[255];
			sprintf_s(FE, 250, "Only %d frames possible, but %d were requested!", collumns * rows, num);
			LastError = FE;
		}
		AltFrames.clear();
		for (int frame = 0; frame < num; ++frame) {
			SDL_Rect framerect;
			framerect.x = (frame % collumns) * w;
			framerect.y = floor(frame / collumns) * h;
			framerect.w = w;
			framerect.h = h;
			AltFrames.push_back(framerect);
		}
		altframing = true;
	}

	void TQSG_Image::Draw(int x, int y, int frame) {
		LastError = "";
		if (altframing) {
			if (frame < 0 || frame >= AltFrames.size()) {
				char FE[400];
				sprintf_s(FE, 395, "DRAW:Texture frame assignment out of bouds! (%d/%d/A)", frame, (int)AltFrames.size());
				_Panic(FE);
				return;
			}
		} else if (frame < 0 || frame >= Textures.size()) {
			char FE[400];
			sprintf_s(FE, 395, "DRAW:Texture frame assignment out of bouds! (%d/%d/R)", frame, (int)Textures.size());
			//LastError = FE;
			_Panic(FE);
			return;
		}
		//printf("DEBUG! B.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		//printf("DEBUG! A.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		SDL_Rect Target;
		Target.x = (x - (int)ceil(hotx*scalex))+TQSG_OriginX;
		Target.y = (y - (int)ceil(hoty*scaley))+TQSG_OriginY;
		Target.w = (int)ceil(Width() * scalex);
		Target.h = (int)ceil(Height() * scaley);
		if (altframing) {
			SDL_SetTextureBlendMode(Textures[0], BlendMode);
			SDL_SetTextureAlphaMod(Textures[0], tcalpha);
			SDL_SetTextureColorMod(Textures[0], tcr, tcg, tcb);
			SDL_RenderCopy(gRenderer, Textures[0], &AltFrames[frame], &Target);
		} else {
			SDL_SetTextureBlendMode(Textures[frame], BlendMode);
			SDL_SetTextureAlphaMod(Textures[frame], tcalpha);
			SDL_SetTextureColorMod(Textures[frame], tcr, tcg, tcb);
			SDL_RenderCopy(gRenderer, Textures[frame], NULL, &Target);
		}
	}

	void TQSG_Image::XDraw(int x, int y, int frame) {
		LastError = "";
		if (frame < 0 || frame >= Textures.size()) {
			char FE[400];
			sprintf_s(FE, 395, "XDRAW:Texture frame assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
			//LastError = FE;
			_Panic(FE);
			return;
		}
		//printf("DEBUG! B.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		//printf("DEBUG! A.Color(%3d, %3d, %3d) \n", tcr, tcg, tcb);
		SDL_Rect Target;
		Target.x = (x -(int)ceil(hotx * scalex))+TQSG_OriginX;
		Target.y = (y -(int)ceil(hoty * scaley))+TQSG_OriginY;
		Target.w = (int)ceil(Width() * scalex);
		Target.h = (int)ceil(Height() * scaley);
		SDL_Point cpoint;
		cpoint.x = hotx;
		cpoint.y = hoty;
		//SDL_RenderCopy(gRenderer, Textures[frame], NULL, &Target);
		if (altframing) {
			SDL_SetTextureBlendMode(Textures[0], BlendMode);
			SDL_SetTextureAlphaMod(Textures[0], tcalpha);
			SDL_SetTextureColorMod(Textures[0], tcr, tcg, tcb);
			SDL_RenderCopyEx(gRenderer, Textures[0], &AltFrames[frame], &Target, rotatedegrees, &cpoint, imgflip);
		} else {
			SDL_SetTextureBlendMode(Textures[frame], BlendMode);
			SDL_SetTextureAlphaMod(Textures[frame], tcalpha);
			SDL_SetTextureColorMod(Textures[frame], tcr, tcg, tcb);
			SDL_RenderCopyEx(gRenderer, Textures[frame], NULL, &Target, rotatedegrees, &cpoint, imgflip);
		}

	}



	void TQSG_Image::StretchDraw(int x, int y, int w, int h, int frame) {
		LastError = "";
		if (frame < 0 || frame >= Textures.size()) {
			char FE[400];
			sprintf_s(FE, 395, "Texture assignment out of bouds! (%d/%d)", frame, (int)Textures.size());
			LastError = FE;
			return;
		}
		SDL_Rect Target;
		Target.x = x+TQSG_OriginX;
		Target.y = y+TQSG_OriginY;
		Target.w = w;
		Target.h = h;
		if (altframing) {
			SDL_SetTextureBlendMode(Textures[0], BlendMode);
			SDL_SetTextureAlphaMod(Textures[0], tcalpha);
			SDL_SetTextureColorMod(Textures[0], tcr, tcg, tcb);
			SDL_RenderCopy(gRenderer, Textures[frame], &AltFrames[frame], &Target);
		} else {
			SDL_SetTextureBlendMode(Textures[frame], BlendMode);
			SDL_SetTextureAlphaMod(Textures[frame], tcalpha);
			SDL_SetTextureColorMod(Textures[frame], tcr, tcg, tcb);
			SDL_RenderCopy(gRenderer, Textures[frame], NULL, &Target);
		}
	}

	

	void TQSG_Image::Tile(int ax, int ay, int w, int h, int frame,int ix, int iy) {
		auto x = ax + TQSG_OriginX;
		auto y = ay + TQSG_OriginY;
		LastError = "";
		if (altframing) {
			LastError = "Altframing NOT supported for tiling (and is not likely to be supported in the future, either!)";
			return;
		}
		// todo: Fix issues with negative ix
		if (ix < 0) {
			//cout << "neg x:" << ix << " to ";
			ix = (x-(Width() + ix))%Width();
			//cout << ix << "\n";
		}
		if (iy < 0) {
			//cout << "neg x:" << ix << " to ";
			iy = (y-(Height() + iy))%Height();
			//cout << ix << "\n";
		}
		int ox, oy, ow, oh;
		TQSG_GetViewPort(&ox, &oy, &ow, &oh);
		int tsx, tsy, tex, tey, tw, th;
		int imgh = Height();
		int imgw = Width();
		/*
		tsx = max(ox, x);
		tsy = max(oy, y);
		tex = min(ow + ox, x + w); tw = tex - tsx;
		tey = min(oh + oy, y + h); th = tey - tsy;
		*/
		tsx = x;
		tsy = y;
		tex = w + x;
		tey = h + y;
		tw = w;
		th = h;
		if (tw <= 0 || th <= 0) return; // Nothing to do but getting bugged!
		//cout << "TILE: Rect("<<x<<","<<y<<") "<<w<<"x"<<h<<" "<<"\n";
		//cout << "\tViewPort(" << tsx << "," << tsy << "," << tw << "[" << tex << "]" << "," << th << "[" << tey << "])\n";
		SDL_Rect Target, Source;
		//TQSG_ViewPort(tsx, tsy, tw, th);
		//TQSG_Rect(tsx, tsy, tw, th);
		//cout << "for (int dy = tsy("<<tsy<<") - iy("<<iy<<")(" << (tsy - iy) << "); dy < tey(" << tey << "); dy += imgh(" << imgh << ")) \n";
		SDL_SetTextureColorMod(Textures[frame],tcr, tcg, tcb);
		SDL_SetTextureBlendMode(Textures[frame], BlendMode);
		SDL_SetTextureAlphaMod(Textures[frame], tcalpha);
		for (int dy = tsy - iy; dy < tey; dy += imgh) {
			//cout << "(" << x << "," << y << ")\tdy:" << dy << "; tsy:" << tsy << " imgh:" << imgh << " th:" << th << "\n";
			 for (int dx = tsx - ix; dx < tex; dx += imgw) {
				//cout << "\t\tDrawTile(" << dx << "," << dy << "," << imgw << "," << imgh << ")\n";
				Target.x = dx;
				Target.y = dy;
				Target.w = imgw;
				Target.h = imgh;
				Source.x = 0;
				Source.y = 0;
				Source.w = imgw;
				Source.h = imgh;
				//cout << "tgt (" << Target.x << "," << Target.y << ") " << Target.w << "x" << Target.h<<"\n";
				//cout << "src (" << Source.x << "," << Source.y << ") " << Source.w << "x" << Source.h<<"; Frame:"<<frame<<"\n\n";
				//cout << "("<<x<<","<<y<<")\tdx:" << dx << "; tsx:" << tsx << " imgw:" << imgw << " tw:" << tw<<"\n";
				if (dx >= tsx && (dx + imgw) > tex) {
					Source.w = imgw - ((dx + imgw) - tex);
					Target.w = Source.w; //(dx + imgw) - tex;
					//cout << "aw " << Source.w << "\n";
				} else if (dx <= tsx) {
					Source.x = tsx - dx;
					Source.w = imgw - Source.x;
					Target.x = tsx;
					Target.w = Source.w;
				}
				if (dy <= tsy && dy + imgh > tey) {
					Source.y = tsy - dy;
					Source.h = th;
					Target.y = tsy;
					Target.h = th;
				} else if (dy >= tsy && (dy + imgh) > tey) {
					Source.h = imgh - ((dy + imgh) - tey);
					Target.h = Source.h;//(dy + imgh) - tey;
					//cout << "ah " << Source.h << "\t" << dy << "\tImgHeight:>" << imgh << "; img-maxy::>" << (dy + imgh) << "; rect-maxy::>" << tey << "=="<<(h+y)<<"\n";
				} else if (dy <= tsy) {
					Source.y = tsy - dy;
					Source.h = imgh - Source.y;
					Target.y = tsy;
					Target.h = Source.h;
				}


				// TQSG_Rect(dx, dy, imgw, imgh);//debug
				if (frame < 0 || frame >= Textures.size()) {
					_Panic("<IMAGE>.Tile(" + to_string(x) + "," + to_string(y) + "," + to_string(w) + "," + to_string(h) + "," + to_string(frame) + "): Out of frame boundaries (framecount: " + to_string(Textures.size()) + ")"); return;
				}				
				SDL_RenderCopy(gRenderer, Textures[frame], &Source, &Target);
			}
		}
		//TQSG_ViewPort(ox, oy, ow, oh);
		//TQSG_Color(180, 0, 255);
		//TQSG_Rect(tsx, tsy, tw, th,true);
		
	}

	void TQSG_Image::Blit(int ax, int ay, int isx, int isy, int iex, int iey, int frame) {
		auto
			x = ax + TQSG_OriginX,
			y = ay + TQSG_OriginY,
			imgh = Height(),
			imgw = Width();
		SDL_Rect 
			Target, 
			Source;
		if (altframing) {
			LastError = "Altframing NOT supported for Blitting (and is not likely to be supported in the future, either!)";
			return;
		}
		Source.x = max(isx, 0);
		Source.y = max(isy, 0);
		Source.w = min(iex, imgw - isx);
		Source.h = min(iey, imgh - isy);
		if (Source.w < 1 || Source.h < 1) { LastError = "Blit format error"; return; }
		Target.x = x;
		Target.y = y;
		Target.w = Source.w * scalex;
		Target.h = Source.h * scaley;
		SDL_SetTextureColorMod(Textures[frame], tcr, tcg, tcb);
		SDL_SetTextureBlendMode(Textures[frame], BlendMode);
		SDL_SetTextureAlphaMod(Textures[frame], tcalpha);
		SDL_RenderCopy(gRenderer, Textures[frame], &Source, &Target);
	}


	int TQSG_Image::Width()	{
		LastError = "";
		if (Textures.size() == 0) { LastError = "No textures!"; return 0; }
		if (altframing) {
			if (AltFrames.size() == 0) {
				LastError = "No alt frame rectangle config!";
				return 0;
			}
			return AltFrames[0].w;
		}
		int w, h;
		SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
		return w;
	}

	int TQSG_Image::Height() {
		LastError = "";
		if (Textures.size() == 0) { LastError = "No textures!"; return 0; }
		if (altframing) {
			if (AltFrames.size() == 0) {
				LastError = "No alt frame rectangle config!";
				return 0;
			}
			return AltFrames[0].w;
		}
		int w, h;
		SDL_QueryTexture(Textures[0], NULL, NULL, &w, &h);
		return h;
	}

	void TQSG_Image::KillAll() {
		for (int i = 0; i < Textures.size(); ++i) SDL_DestroyTexture(Textures[i]);
	}

	void TQSG_Image::Hot(int x, int y) {
		hotx = x;
		hoty = y;
	}

	void TQSG_Image::HotCenter() {
		hotx = floor(Width() / 2);
		hoty = floor(Height() / 2);
	}

	void TQSG_Image::HotBottomCenter() {
		hotx = floor(Width() / 2);
		hoty = Height();
	}

	void TQSG_Image::HotGet(int& x, int& y) {
		x = hotx;
		y = hoty;
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

	void TQSG_SetBlend(SDL_BlendMode BM) { BlendMode = BM; }
	void TQSG_SetBlend(TQSG_Blend BM) { BlendMode = (SDL_BlendMode)BM; }
	TQSG_Blend TQSG_GetBlend() { return (TQSG_Blend)BlendMode; }


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

	int TQSG_DesktopWidth() {
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(0, &mode);
		return mode.w;
	}

	int TQSG_DesktopHeight() {
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(0, &mode);
		return mode.h;
	}

	void TQSG_DesktopSize(int &w,int &h){
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(0, &mode);
		w = mode.w;
		h = mode.h;
	}

	void TQSG_SetOrigin(int x, int y) {
		TQSG_OriginX = x;
		TQSG_OriginY = y;
	}

	void TQSG_GetOrigin(int& x, int& y) {
		x = TQSG_OriginX;
		y = TQSG_OriginY;
	}

	void TQSG_SetMouse(int x, int y) {
		SDL_WarpMouseInWindow(gWindow, x, y);
	}


	void TQSG_ScreenSize(int* w, int* h) {
		SDL_GetRendererOutputSize(gRenderer, w, h);
	}

	void TQSG_ViewPort(int x, int y,int w,int h) {
		SDL_Rect R;
		R.x = x;
		R.y = y;
		R.w = w;
		R.h = h;
		if (SDL_RenderSetViewport(gRenderer, &R) != 0) {
			cout << "\x7\x1b[31mERROR!\x1b[0m\t TQSQ_ViewPort(" << x << "," << y << "," << w << "," << h << "): " << SDL_GetError() << endl;
		}
		//SDL_RenderSetClipRect(gRenderer, &R);
	}

	void TQSG_ViewPort() {
		int w, h;
		//SDL_GetRendererOutputSize(gRenderer, &w, &h);
		if (SDL_RenderSetViewport(gRenderer, NULL) != 0) {
			cout << "\x7\x1b[31mERROR!\x1b[0m\t TQSQ_ViewPort(): " << SDL_GetError() << endl;
		}
		//TQSG_ViewPort(0, 0, w, h);
	}

	void TQSG_GetViewPort(int* x, int* y, int* w, int* h) {
		SDL_Rect R;
		SDL_RenderGetViewport(gRenderer, &R);
		//SDL_RenderGetClipRect(gRenderer, &R);
		*x = R.x;
		*y = R.y;
		*w = R.w;
		*h = R.h;
	}

	void TQSG_Rotate(double degrees) { rotatedegrees = degrees; }
	void TQSG_RotateRAD(double radians) { rotatedegrees = radians * (180 / M_PI); }
	void TQSG_RotateGRAD(double gradians) { rotatedegrees = gradians * 180 / 200; }
	double TQSG_Rotate() { return rotatedegrees; }
	double TQSG_RotateRAD() { return rotatedegrees * M_PI / 180; }
	double TQSG_RotateGRAD() { return rotatedegrees * (200 / 180); }


	void TQSG_Flip(int minticks) {
		//SDL_UpdateWindowSurface(gWindow);
		static auto oud{ SDL_GetTicks() };		
		static auto mt{ 0 };
		if (minticks >= 0) mt = minticks;
		while (minticks && (SDL_GetTicks() - oud < mt)) SDL_Delay(1);
		oud = SDL_GetTicks();		
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

	void TQSG_ColorHSV(double Hue, double Saturation, double Value) {
		hsv _hsv{ Hue,Saturation,Value };
		auto _rgb = hsv2rgb(_hsv);
		Uint8 R = floor(_rgb.r * 255.0);
		Uint8 G = floor(_rgb.g * 255.0);
		Uint8 B = floor(_rgb.b * 255.0);
		TQSG_Color(R, G, B);
	}

	void TQSG_SetAlpha(Uint8 a) {
		tcalpha = a;
	}

	Uint8 TQSG_GetAlpha() {
		return tcalpha;
	}

	void TQSG_Rect(int x, int y, int w, int h,bool open) {
		//SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 0xFF, 0x00, 0xFF));
		//printf("WARNING! Rect not fully implemented yet"); 
		SDL_Rect r;
		r.x = x;
		r.y = y;
		r.w = w;
		r.h = h;		
		SDL_SetRenderDrawBlendMode(gRenderer,BlendMode);
		SDL_SetRenderDrawColor(gRenderer, tcr, tcg, tcb, tcalpha);
		if (open)
			SDL_RenderDrawRect(gRenderer, &r);
		else
			SDL_RenderFillRect(gRenderer, &r);
	}

	void TQSG_Circle(int x, int y, int radius) {
		SDL_SetRenderDrawColor(gRenderer, tcr, tcg, tcb, tcalpha);
		//cout << "Circle: " << radius << "\n"; // debug
		float lastx = x, lasty = (radius)+y;
		for (double i = 0; i < 2 * 3.14; i += 0.025) {
			//SDL_RenderDrawPoint(gRenderer, floor(sin(i) * radius) + x, floor(cos(i) * radius) + y);			
			//SDL_RenderDrawPoint(gRenderer, (sin(i) * radius) + x, (cos(i) * radius) + y);
			float cx = (sin(i) * radius) + x, cy = (cos(i) * radius) + y;
			SDL_RenderDrawLine(gRenderer, lastx, lasty, cx, cy);
			lastx = cx; lasty = cy;
		}
	}

	void TQSG_Line(int x1, int y1, int x2, int y2) {
		SDL_SetRenderDrawColor(gRenderer, tcr, tcg, tcb, tcalpha);
		SDL_RenderDrawLine(gRenderer, x1, y1, x2, y2);		
	}

	bool TQSG_Init(string WindowTitle,int WinWidth,int WinHeight,bool fullscreen) {
		printf("Starting graphics screen: %dx%d; fullscreen=%d\n", WinWidth, WinHeight, fullscreen);
		// TODO!
				//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
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
						printf("\a\x1b[33mWARNING!\x1b[0m\t Going into full screen was unsuccesful\n\n");
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
						LastError="SDL_Image: " + string(IMG_GetError());
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
		//TTF_Quit();
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
			sprintf_s(FE, "Tried to load sub for [%d,%d] while no JCR6 resource has been linked\n", base, ch);
			LastError = FE;
			printf("\x1b[31mERROR! \x1b[32mTQSG_IMGFONT\x1b[0m\t%s", FE);
			printf("\tFnt: %s\n", Prefix.c_str());
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

	void TQSG_ImageFont::Draw(const char* txt, int x, int y, unsigned char ha, unsigned char va,int autonext) {
		if (!txt[0]) return;
		if (autonext) {
			string w = txt;
			vector<string> Lijst;
			string t = "";
			for (int i = 0; txt[i]; i++) {
				string tw = t + txt[i];
				if (x+TextWidth(tw.c_str())>autonext){
					Lijst.push_back(t);
					t = "" + txt[i];
				} else {
					t += txt[i];
				}
			}
			Lijst.push_back(t);
			int ty = y;
			for (auto txt : Lijst) {
				Draw(txt.c_str(), x, ty, ha, va, 0);
				ty += TextHeight(txt.c_str());
			}
			LastAutoNextCount = Lijst.size();
			return;
		}
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

	void TQSG_ImageFont::Draw(std::string txt, int x, int y, unsigned char ha, unsigned char va,int autonext) {
		Draw(txt.c_str(), x, y, ha, va,autonext);
	}

	void TQSG_ImageFont::LoadFont(jcr6::JT_Dir& JCRRes, std::string Bundle, bool all) {
		cout << "Linking Font to JCR6 for (" << Bundle << ")\n";
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
		printf("Image font destruction call (%x) \n",(int)this);
		Kill();
	}

	TQSG_ImageFont::TQSG_ImageFont() {
	}



	TQSG_Image* TQSG_PureAutoImage::Img() {
		return &_img;
	}

	TQSG_PureAutoImage::~TQSG_PureAutoImage() {
		_img.KillAll();
	}

	void TQSG_PureAutoImage::HotCenter() {
		_img.HotCenter();
	}

	void TQSG_PureAutoImage::Draw(int x, int y, int frame) { _img.Draw(x, y, frame); }
	void TQSG_PureAutoImage::Tile(int x, int y, int w, int h, int frame) { _img.Tile(x, y, w, h, frame); }
	void TQSG_PureAutoImage::Tile(int x, int y, int w, int h, int ix, int iy, int frame) { _img.Tile(x, y, w, h, frame,ix,iy); }
	void TQSG_PureAutoImage::Stretch(int x, int y, int w, int h, int frame) { _img.StretchDraw(x, y, w, h, frame); }
	void TQSG_PureAutoImage::Blit(int x, int y, int isx, int isy, int iex, int iey, int frame) { _img.Blit(x, y, isx, isy, iex, iey, frame); }
	void TQSG_PureAutoImage::HotBottomRight() { _img.Hot(_img.Width(), _img.Height()); }

	int TQSG_PureAutoImage::W() {
		return _img.Width();
	}

	int TQSG_PureAutoImage::H() {
		return _img.Height();
	}

	void TQSG_PureAutoImage::Hot(int x, int y) { _img.Hot(x, y); }

	void TQSG_PureAutoImage::HotGet(int& x, int& y) { _img.HotGet(x, y); }

	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(std::string file) {
		auto ret{ std::make_shared<TQSG_PureAutoImage>() };
		ret->Img()->Create(file);
		return ret;
	}

	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(std::string jcrfile, std::string file) {
		auto ret{ std::make_shared<TQSG_PureAutoImage>() };
		ret->Img()->Create(jcrfile,file);		
		return ret;
	}

	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(jcr6::JT_Dir& jcrdir, std::string file) {
		auto ret{ std::make_shared<TQSG_PureAutoImage>() };
		auto hot = StripExt(file)+".hot";
		ret->Img()->Create(jcrdir, file);
		if (jcrdir.EntryExists(hot)) {
			auto hd = Split(jcrdir.String(hot), ',');
			if (hd.size() == 2) cout << "\7\x1b[31mERROR! ERROR!\x1b[0m\tInvalid hot file: " << hot << endl;
			else {
				auto
					x = ToInt(hd[0]),
					y = ToInt(hd[1]);
				ret->Img()->Hot(x, y);
			}
		}
		return ret;
	}

	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(int size, const char* buf) {
		auto ret{ std::make_shared<TQSG_PureAutoImage>() };
		SDL_RWops* RWBuf{ NULL };
		RWBuf = SDL_RWFromMem((void*)buf, size);
		ret->Img()->Create(RWBuf);
		return ret;
	}

	std::shared_ptr<TQSG_PureAutoImage> TQSG_GrabScreen() {
		auto Tex{ SDL_CreateTextureFromSurface(gRenderer, SDL_GetWindowSurface(gWindow)) };
		auto Ret{ std::make_shared<TQSG_PureAutoImage>() };
		Ret->Img()->Create(Tex);
		return Ret;
	}

	TQSG_AutoImageFont TQSG_LoadAutoImageFont(std::string jcrfile, std::string Dir) {
		auto J = jcr6::Dir(jcrfile);
		return TQSG_LoadAutoImageFont(&J, Dir);
	}

	TQSG_AutoImageFont TQSG_LoadAutoImageFont(jcr6::JT_Dir* jcrdir, std::string File) {
		cout << "Loading Auto Image Font\n"; // debug
		auto ret{ std::make_shared<TQSG_PureAutoImageFont>() };
		ret->Font()->LoadFont(*jcrdir,File);
		return ret;
	}

	TQSG_ImageFont* TQSG_PureAutoImageFont::Font() { return &_fnt; }

	void TQSG_PureAutoImageFont::Draw(std::string txt, int x, int y, unsigned char ha, unsigned char va) { _fnt.Draw(txt, x, y, ha, va); }

	int TQSG_PureAutoImageFont::W(std::string txt) {
		return _fnt.TextWidth(txt.c_str());
	}

	int TQSG_PureAutoImageFont::H(std::string txt) {
		if (!_fnt.TextHeight("TESTING")) _fnt.TextWidth("TESTING"); // Cheating to void a bug I've not yet been around to fix
		return _fnt.TextHeight(txt.c_str());
	}

	TQSG_PureAutoImageFont::~TQSG_PureAutoImageFont() {
		_fnt.Kill();
	}


}