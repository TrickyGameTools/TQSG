// Lic:
// TQSE.hpp
// Tricky's Quick SDL Events
// version: 20.08.25
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

#include <SDL.h>

namespace TrickyUnits {

	/// <summary>Sets the startup data for TQSE. Please note unloading is not required as no pointers are used or memory is being allocated. This routine is only to make sure that all start values are correct!</summary>
	void TQSE_Init();

	/// <summary>Polls SDL events and updates all keypresses, mouse stuff and so on.... Please note, only when this function is called, this stuff gets updated!</summary>
	void TQSE_Poll();


	/// <summary>True if a quit request was done on the last TQSE_Poll()</summary>
	bool TQSE_Quit();

	bool TQSE_KeyHit(SDL_KeyCode c);
	bool TQSE_KeyDown(SDL_KeyCode c);

}