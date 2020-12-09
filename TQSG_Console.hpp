#pragma once

// C++
#include <memory>

// TQSG units (TQSA is audio an not needed here).
#include "TQSE.hpp"
#include "TQSG.hpp"


#define New_ConsoleLine std::make_shared<TrickyUnits::ConsoleLine_Real>()
//#define ConsoleLine std::shared_ptr<ConsoleLine_Real> // If typedef refuses to work, then the DIRTY method, I guess?

namespace TrickyUnits {


	class ConsoleLine_Real;

	class ConsoleLine_Real {
	public:
		int Y{ 0 };
		int X{ 0 };
		std::string Txt{ "" };
		int R{ 0 }, G{ 0 }, B{ 0 };
		static int MaxOverHeight;
		std::shared_ptr<ConsoleLine_Real> Linked{ nullptr };
	};	
	typedef std::shared_ptr<ConsoleLine_Real> ConsoleLine;

	extern bool ConsoleBackStretch;


	extern TQSG_AutoImage ConsoleBackGroundPicture;
	extern std::string ConsoleResultLine;

	// For the time being only there for compatibility with translations form BlitzMax.
	void ConsoleWriteLogFile(std::string LogFile, std::string BGCOLOR = "#000000");

	// For the time being only there for compatibility with translations form BlitzMax.
	void ConsoleCloseLogFile();

	// Set the console font.
	// You won't see anything at all when no font is loaded (or have a crash).
	void SetConsoleFont(jcr6::JT_Dir* JCR, std::string FontBundle);
	void SetConsoleFont(std::string FontBundle);

	void ConsoleShow();

	// Prints a line onto the console. Returns the line data pointer in case you need it.
	ConsoleLine ConsolePrint(std::string Txt, int  R = 255, int G = 255, int B = 255);


	// Alias for ConsolePrint
	ConsoleLine ConsoleWrite(std::string Txt, int  R = 255, int G = 255, int B = 255);

	// Prints two kinds of kext on the same line
	void ConsoleDoublePrint(std::string Txt1, std::string Txt2, int R = 255, int G = 255, int B = 255, int R2 = 255, int G2 = 255, int B2 = 255);
	//void ConsoleDoublePrint(std::string Txt1, std::string Txt2, int offs, int R = 255, int G = 255, int B = 255, int R2 = 255, int G2 = 255, int B2 = 255);

	void ConsoleDoublePrint2(std::string Txt1, std::string Txt2, int offs, int R = 255, int G = 255, int B = 255, int R2 = 255, int G2 = 255, int B2 = 255);

	void ConsoleModify(ConsoleLine Line, std::string Txt, int R = -1, int G = -1, int B = -1);

	// This is a basic control for the console, but feel free to write another if this is not what you need! NOTE! You must call TQSE_Poll() prior to calling this function, or it will NOT work!
	std::string ConsoleBaseControl();

}