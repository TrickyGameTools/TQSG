// Lic:
// TQSG.hpp
// TQSG Header
// version: 20.08.28
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

/*
  
  TSQG has been designed to make some things on the graphics department easier on me.
  It has been completely designed to fill my own needs. TQSG relies strongly on Tricky's Units 
  (although in a separate folder, officially still part of them) and JCR6. It goes without saying
  that if you plan to use TQSG yourself you do need both of those. 

  You don't have to ponder about the extra drivers in JCR6. Only the main project has to take care
  of that and JCR6 will do the rest.

*/
#pragma once

#undef TQSG_AllowTTF

// SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// JCR6
#include <jcr6_core.hpp>

// C++
#include <vector>
#include <string>

namespace TrickyUnits {

	class TQSG_Image {
	private:
		bool Faulty = false; // When this contains 'true' all operations can be refused!
		std::string FaultyReason = ""; 
		std::vector<SDL_Texture*> Textures;
		int hotx=0, hoty=0;
		void LoadRWops(SDL_RWops* data, int frame=-1, int autofree = 1);
		void TrueLoadJCR(jcr6::JT_Dir& JCR, std::string entry);
	public:
		/// <summary>
		/// When set to 'true', the destructor will automatically dispose the texture data inside.
		/// </summary>
		bool AutoClean = false;
		
		void Create(SDL_RWops* data, int autofree = 1);
		void Create(SDL_Texture* data);
		void Create(std::string file);
		void Create(jcr6::JT_Dir& JCR, std::string entry);
		void Create(std::string mainfile, std::string entry);
		/// <summary>
		/// Draws an image with the basic settings
		/// </summary>
		void Draw(int x, int y, int frame = 0);
		/// <returns>Width of image</returns>
		int Width();
		/// <returns>Height of image</returns>
		int Height();
		/// <summary>
		/// Use with CARE! This will destroy all textures stored in this object! Please note, the destructor calls this feature automatically.
		/// </summary>
		void KillAll();
		void Hot(int x, int y);
		void HotCenter();
		void HotBottomCenter();
		~TQSG_Image();
	};

#ifdef TQSG_AllowTTF
	class TQSG_TTF {
	private:
		TTF_Font* FontItself = NULL;
	public:
		bool HasFont();
		void Kill();
		TQSG_Image Text(const char* txt);
		TQSG_Image Text(std::string txt);
		void Draw(const char* txt, int x, int y, unsigned char ha=0, unsigned char va=0);
		void Draw(std::string txt, int x, int y, unsigned char ha=0, unsigned char va=0);
		void LoadFont(SDL_RWops* buf, int size=24,int autofree = 1);
		void LoadFont(std::string file, int fontsize=24);
		void LoadFont(jcr6::JT_Dir& JCRRes,std::string Entry, int fontsize = 24);
		void LoadFont(std::string JCRFile, std::string Entry, int fontsize = 24);
		~TQSG_TTF();
	};
#endif

	class TQSG_ImageFont {
	private:
		std::map<int, std::map<int, SDL_Texture*>> Letter;
		std::map<int, std::map<int, bool>> TriedToLoad;
		jcr6::JT_Dir JCR;
		bool JCR_Linked = false;
		std::string Prefix = "";
		void SubLoad(Uint8 base, Uint8 ch);
		int width = 0;
		int height = 0;
	public:
		/// <summary>
		/// When set to "true" the system will always handle the font as a "true type" (if it's able to).
		/// </summary>
		bool Fixed = false;
		/// <summary>
		/// Kills the loaded font.. Completely, so you are ready to load another (when trying to load another, this happens automatically)
		/// </summary>
		void Kill();
		//TQSG_Image Text(const char* txt);
		//TQSG_Image Text(std::string txt);
		int TextWidth(const char* txt);
		int TextHeight(const char*txt);

		void DrawLetter(Uint8 base, Uint8 ch,int x, int y);

		void Draw(const char* txt, int x, int y, unsigned char ha = 0, unsigned char va = 0);
		void Draw(std::string txt, int x, int y, unsigned char ha = 0, unsigned char va = 0);
		//void LoadFont(SDL_RWops* buf, int size = 24, int autofree = 1);
		void LoadFont(jcr6::JT_Dir& JCRRes, std::string Bundle, bool all=false);
		void LoadFont(std::string JCRFile, std::string Bundle, bool all=false);
		void LoadFont(std::string file, bool all=false);
		~TQSG_ImageFont();
		TQSG_ImageFont();
	};

	std::string TQSG_GetError();
	void SetScale(double x=1, double y=1);
	void GetScale(double& x, double& y);
	void TQSG_Flip();

	void TQSG_Color(Uint8 r, Uint8 g, Uint8 b);
	void TQSG_GetColor(Uint8* r, Uint8* g, Uint8* b);
	void TQSG_SetAlpha(Uint8 a);
	Uint8 TQSG_GetAlpha();

	void TQSG_Rect(int x, int y, int w, int h,bool open=false);
	void TQSG_Circle(int x, int y, int radius);
	void TQSG_Line(int x1, int y1, int x2, int y2);
	void TQSG_ClsColor(int r, int g, int b);
	void TQSG_Cls();

	void TQSG_Plot(int x, int y);

	int TQSG_ScreenWidth();
	int TQSG_ScreenHeight();
	void TQSG_ScreenSize(int* x, int* y);


	bool TQSG_Init(std::string WindowTitle, int WinWidth = 800, int WinHeight = 600,bool fullscreen=false);
	void TQSG_Close();
	
}