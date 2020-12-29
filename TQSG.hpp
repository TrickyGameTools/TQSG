// Lic:
// TQSG.hpp
// TQSG Header
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

#define TQSG_ACol(R,G,B,A) { TrickyUnits::TQSG_Color(R,G,B); TrickyUnits::TQSG_SetAlpha(A); }

// SDL
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_image.h>
// #include <SDL_ttf.h>

// JCR6
#include <jcr6_core.hpp>

// C++
#include <vector>
#include <string>
#include <memory>

namespace TrickyUnits {

	typedef void (*TQSG_PanicType)(std::string errormessage);

	extern TQSG_PanicType TQSG_Panic;

	enum class TQSG_Blend {
		NONE = SDL_BLENDMODE_NONE,
		ALPHA = SDL_BLENDMODE_BLEND,
		ADDITIVE = SDL_BLENDMODE_ADD,
		COLOR = SDL_BLENDMODE_MOD
	};


	class TQSG_Image {
	private:
		bool Faulty = false; // When this contains 'true' all operations can be refused!
		std::string FaultyReason = "";
		std::vector<SDL_Texture*> Textures;
		int hotx = 0, hoty = 0;
		bool altframing = false;
		std::vector<SDL_Rect> AltFrames;
		void LoadRWops(SDL_RWops* data, int frame = -1, int autofree = 1);
		void TrueLoadJCR(jcr6::JT_Dir& JCR, std::string entry);
	public:
		/// <summary>
		/// When set to 'true', the destructor will automatically dispose the texture data inside.
		/// </summary>
		bool AutoClean = false;

		int Frames();

		void Create(SDL_RWops* data, int autofree = 1);
		void Create(SDL_Texture* data);
		void Create(std::string file);
		void Create(jcr6::JT_Dir& JCR, std::string entry);
		void Create(std::string mainfile, std::string entry);

		void AltFrame(int w, int h, int num);

		/// <summary>
		/// Draws an image with the basic settings. Please note, rotation and flipping settings are ignored.
		/// </summary>
		void Draw(int x, int y, int frame = 0);

		void XDraw(int x, int y, int frame = 0);

		/// <summary>
		/// Draws and stretches an image to fit in the set square. Please note, hot spots, format ratios and all that kind of stuff is ignored. It just draws in the square set by you, and nothing more.
		/// </summary>
		/// <param name="x"></param>
		/// <param name="y"></param>
		/// <param name="w"></param>
		/// <param name="h"></param>
		/// <param name="frame"></param>
		void StretchDraw(int x, int y, int w, int h, int frame = 0);

		/// <summary>
		/// Tiles the image over the set area. Please note scaling, rotation and flipping settings are all ignored. Color is not ignored.
		/// </summary>
		/// <param name="x"></param>
		/// <param name="y"></param>
		/// <param name="w"></param>
		/// <param name="h"></param>
		/// <param name="frame"></param>
		/// <param name="ix"></param>
		/// <param name="iy"></param>
		void Tile(int x, int y, int w, int h, int frame = 0, int ix = 0, int iy = 0);
		void Blit(int x, int y, int isx, int isy, int iex, int iey, int frame = 0);


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
		void HotGet(int& x, int& y);
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
		void Draw(const char* txt, int x, int y, unsigned char ha = 0, unsigned char va = 0);
		void Draw(std::string txt, int x, int y, unsigned char ha = 0, unsigned char va = 0);
		void LoadFont(SDL_RWops* buf, int size = 24, int autofree = 1);
		void LoadFont(std::string file, int fontsize = 24);
		void LoadFont(jcr6::JT_Dir& JCRRes, std::string Entry, int fontsize = 24);
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
		int TextHeight(const char* txt);

		void DrawLetter(Uint8 base, Uint8 ch, int x, int y);

		void Draw(const char* txt, int x, int y, unsigned char ha = 0, unsigned char va = 0, int autonext = 0);
		void Draw(std::string txt, int x, int y, unsigned char ha = 0, unsigned char va = 0, int autonext = 0);
		int LastAutoNextCount = 1;
		//void LoadFont(SDL_RWops* buf, int size = 24, int autofree = 1);
		void LoadFont(jcr6::JT_Dir& JCRRes, std::string Bundle, bool all = false);
		void LoadFont(std::string JCRFile, std::string Bundle, bool all = false);
		void LoadFont(std::string file, bool all = false);
		~TQSG_ImageFont();
		TQSG_ImageFont();
	};

	std::string TQSG_GetError();
	void SetScale(double x = 1, double y = 1);
	void GetScale(double& x, double& y);
	void TQSG_Flip(int minticks = -1);

	void TQSG_Color(Uint8 r, Uint8 g, Uint8 b);
	void TQSG_ColorHSV(double Hue, double Saturation, double Value);
	void TQSG_GetColor(Uint8* r, Uint8* g, Uint8* b);
	void TQSG_SetAlpha(Uint8 a);
	Uint8 TQSG_GetAlpha();

	void TQSG_Rect(int x, int y, int w, int h, bool open = false);
	void TQSG_Circle(int x, int y, int radius);
	void TQSG_Line(int x1, int y1, int x2, int y2);
	void TQSG_ClsColor(int r, int g, int b);
	void TQSG_Cls();
	void TQSG_SetBlend(SDL_BlendMode BM);
	void TQSG_SetBlend(TQSG_Blend BM);
	TQSG_Blend TQSG_GetBlend();

	void TQSG_Plot(int x, int y);

	int TQSG_ScreenWidth();
	int TQSG_ScreenHeight();
	void TQSG_ScreenSize(int* x, int* y);
	// Note this is yet only in config there is no actual effect yet, but there wull be in the future
	void TQSG_ViewPort(int x, int y, int w, int h);
	void TQSG_ViewPort();
	void TQSG_GetViewPort(int* x, int* y, int* w, int* h);
	void TQSG_Rotate(double degrees);
	void TQSG_RotateRAD(double radians);
	void TQSG_RotateGRAD(double gradians);
	double TQSG_Rotate();
	double TQSG_RotateRAD();
	double TQSG_RotateGRAD();

	int TQSG_DesktopWidth();
	int TQSG_DesktopHeight();
	void TQSG_DesktopSize(int& w, int& h);

	void TQSG_SetOrigin(int x, int y);
	void TQSG_GetOrigin(int& x, int& y);

	void TQSG_SetMouse(int x, int y);

	bool TQSG_Init(std::string WindowTitle, int WinWidth = 800, int WinHeight = 600, bool fullscreen = false);
	void TQSG_Close();

	


	// Don't EVER use this directly, but always use with a shared pointer in stead. Safer that way!
	class TQSG_PureAutoImage {
	private:
		TQSG_Image _img;
	public:
		TQSG_Image* Img(); // Only there to quickly try things out, but beware for deprecation over time, as I don't like this pointer to be here!
		~TQSG_PureAutoImage();
		void Draw(int x, int y, int frame = 0);
		void Tile(int x, int y, int w, int h, int frame = 0);
		void Tile(int x, int y, int w, int h, int ix, int iy, int frame=0);
		void Stretch(int x, int y, int w, int h, int frame = 0);
		void Blit(int x, int y, int isx, int isy, int iex, int iey, int frame = 0);
		void HotBottomRight();
		int W();
		int H();
		void Hot(int x, int y);
		void HotCenter();
		void HotGet(int& x, int& y);		
	};

	typedef std::shared_ptr<TQSG_PureAutoImage> TQSG_AutoImage;
	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(std::string file);
	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(std::string jcrfile,std::string file);
	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(jcr6::JT_Dir jcrdir, std::string file);
	std::shared_ptr<TQSG_PureAutoImage> TQSG_LoadAutoImage(int size, const char* buf);
	std::shared_ptr<TQSG_PureAutoImage> TQSG_GrabScreen();

	// Don't EVER use this directly but always use a shared pointer in stead. Safer that way.
	class TQSG_PureAutoImageFont {
	private:
		TQSG_ImageFont _fnt;
	public:
		TQSG_ImageFont* Font(); // Only meant to try things out, but beware of deprecation over time, and I don't like this pointer ot be here!
		void Draw(std::string txt, int x, int y, unsigned char ha = 0, unsigned char va = 0);
		int W(std::string txt);
		int H(std::string txt);
		//TQSG_PureAutoImageFont(jcr6::JT_Dir* D, const char* File);
		~TQSG_PureAutoImageFont();
	};
	typedef std::shared_ptr<TQSG_PureAutoImageFont> TQSG_AutoImageFont;

	TQSG_AutoImageFont TQSG_LoadAutoImageFont(std::string jcrfile, std::string Dir);
	TQSG_AutoImageFont TQSG_LoadAutoImageFont(jcr6::JT_Dir* jcrdir, std::string File);

	
}