// Lic:
// TQSG.hpp
// TQSG Header
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

/*
  
  TSQG has been designed to make some things on the graphics department easier on me.
  It has been completely designed to fill my own needs. TQSG relies strongly on Tricky's Units 
  (although in a separate folder, officially still part of them) and JCR6. It goes without saying
  that if you plan to use TQSG yourself you do need both of those. 

  You don't have to ponder about the extra drivers in JCR6. Only the main project has to take care
  of that and JCR6 will do the rest.

*/
#pragma once

// SDL
#include <SDL.h>
#include <SDL_image.h>

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
		TQSG_Image(SDL_RWops* data, int autofree = 1);
		TQSG_Image(std::string file);
		TQSG_Image(jcr6::JT_Dir& JCR, std::string entry);
		TQSG_Image(std::string mainfile, std::string entry);
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
		~TQSG_Image();
	};

	std::string TQSG_GetError();
	void SetScale(double x=1, double y=1);
	void GetScale(double& x, double& y);

	void TQSG_Init();
	void TQSG_Close();
}