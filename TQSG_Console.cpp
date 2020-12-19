// C++
#include <vector>
#include <iostream>

// Tricky's Units
#include <QuickString.hpp>

// Self
#include "TQSG_Console.hpp"
#include <algorithm>


namespace TrickyUnits {
	int ConsoleLine_Real::MaxOverHeight{ 3000 };
	static std::vector<std::shared_ptr<ConsoleLine_Real>> ConsoleLines;
	std::string ConsoleCommand{ "" };
	TQSG_ImageFont ConsoleFont{};
	static int Scroll{ 0 };

	bool ConsoleBackStretch{ false };

	std::string HTMCol(int R, int G, int B) {
		return "#" + right(Hex(R), 2) + right(Hex(G), 2) + right(Hex(B), 2);
	}

	TQSG_AutoImage ConsoleBackGroundPicture{ nullptr };
	std::string ConsoleResultLine{ "" };

	void ConsoleWriteLogFile(std::string LogFile, std::string BGCOLOR) {
		// This has only been added to prevent shit in the GALE translation, but (at least not for the shor term) I do not plan to use it.
		// Below is the original BlitzMax code for reference.
		/*
		ConsoleLogBT = WriteFile(LogFile$)
		WriteLine ConsoleLogBT,"<?xml version=~q1.0~q encoding=~qUTF-8~q?>"
		WriteLine COnsoleLogBT,"<!DOCTYPE HTML PUBLIC ~q-//W3C//DTD HTML 4.01 Transitional//EN~q  ~qhttp://www.w3.org/TR/html4/loose.dtd~q><html Lang=~qEnglish~q dir=~qltr~q>"
		WriteLine ConsoleLogBT,"<head><title>Log from "+AppTitle+" ("+AppFile+")</title></head><body>"
		WriteLine COnsoleLogBT,"<center><table width='95%'>"
		WriteLine ConsoleLogBT,"<tr valign=top><td align=right>Log from:</td><td>"+AppTitle+"</td></tr>"
		WriteLine ConsoleLogBT,"<tr valign=top><td align=Right>Application File:</td><td>"+AppFile+"</td></tr>"
		WriteLine ConsoleLogBT,"<tr valign=top><td align=right>Log Started:</td><td>"+CurrentDate()+"; "+CurrentTime()+"</td></tr>";
		WriteLine ConsoleLogBT,"</table><p>";
		WriteLine ConsoleLogBT,"<table style='font-family: Courier; Background-Color:"+BGCOLOR+"' width='95%'>"
		*/
	}
	
	void ConsoleCloseLogFile() {
		// This has only been added to prevent shit in the GALE translation, but (at least not for the shor term) I do not plan to use it.
		// Below is the original BlitzMax code for reference.
		/*
		If ConsoleLogBT
			WriteLine consolelogbt,"</table></center>"
			WriteLine consolelogbt,"<p><a href=~qhttp://validator.w3.org/check?uri=referer~q><img src=~qhttp://www.w3.org/Icons/valid-html401~q alt=~qValid HTML 4.01 Transitional~q height=~q31~q width=~q88~q></a></p><p>(As this document was automatically generated validation errors may exist, though I deem it unlikely)</p>"
			WriteLine consolelogbt,"</body></html>"
			CloseFile consolelogbt
			ConsoleLogBT = Null
		EndIf
		*/
	}

	void SetConsoleFont(jcr6::JT_Dir* JCR, std::string FontBundle) {
		std::cout << "Setting TQSG_ConsoleFont to bundle: " << FontBundle << "!\n";
		ConsoleFont.LoadFont(*JCR, FontBundle);
		ConsoleFont.Draw("The quick brown fox, jumps over the lazy dog! THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!", TQSG_ScreenWidth(), TQSG_ScreenHeight()); // Drawn OUTSIDE the screen boundaries, but this makes sure the height calculation is right, as that is a bit of a problem, still... :-/
		std::cout << "Font Width Test: " << ConsoleFont.TextWidth("The quick brown fox, jumps over the lazy dog! THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!") <<
			"; Font Height test: " << ConsoleFont.TextHeight("ABC") << std::endl;
	}

	void SetConsoleFont(std::string FontBundle) {
		ConsoleFont.LoadFont(FontBundle,true);
	}

	void ConsoleShow() {
		auto ConsoleWidth{ TQSG_ScreenWidth() };
		auto ConsoleHeight{ TQSG_ScreenHeight() };
		TQSG_ClsColor(0, 0, 0);
		TQSG_Cls();
		TQSG_Color(255, 255, 255);					
		if (ConsoleBackGroundPicture) { 
			if (ConsoleBackStretch) {
				ConsoleBackGroundPicture->Stretch(0, 0, ConsoleWidth, ConsoleHeight);
			} else {
				ConsoleBackGroundPicture->Tile(0, 0, ConsoleWidth, ConsoleHeight);
			}
		}
		if (ConsoleLines.size()) {
			auto firstline = ConsoleLines[0];
			if (firstline->Y > ConsoleLine_Real::MaxOverHeight) ConsoleLines.erase(ConsoleLines.begin());
		}
		for (auto CL : ConsoleLines){
			auto X{ CL->X };
			auto Y{ (ConsoleHeight - CL->Y) + Scroll };
		TQSG_Color(0, 0, 0);
		ConsoleFont.Draw(CL->Txt, X + 2, Y + 2, 0, 1); //DrawText Cl.Txt, CL.X + 2, CL.y + 2
		TQSG_Color(CL->R, CL->G, CL->B);
		ConsoleFont.Draw(CL->Txt, X, Y , 0, 1);//	DrawText CL.Txt, CL.X, Cl.Y
			}
		TQSG_Color(255, 255, 255);
			//DrawText ">" + ConsoleCommand + "_", 0, GraphicsHeight() - 15
		std::string cmd{ ">" + ConsoleCommand + "_" };
		ConsoleFont.Draw(cmd, 0, ConsoleHeight+Scroll, 0, 1);
	}

	ConsoleLine ConsolePrint(std::string Txt, int R, int G, int B) {
		auto CL = New_ConsoleLine;
		//Local CL2 : ConsoleLine
		auto MaxY = 0;
		//'Enter the text
		//Print "Console: " + Txt
		CL->Txt = Txt;
		CL->R = R;
		CL->G = G;
		CL->B = B;
		// Old stuff made a new "screen position" but the new stuff just moves the old a bit up.
			//'Find the right screen position
			//For Cl2 = EachIn ConsoleLines
			//If CL2.Y > MaxY MaxY = CL2.Y
			//Next
			//MaxY : +15
			//CL.Y = MaxY
		// <New Stuff>
		for (auto CL2 : ConsoleLines) { 
			if (CL2->Linked)
				CL2->Y = CL2->Linked->Y;
			else
				CL2->Y += ConsoleFont.TextHeight(CL2->Txt.c_str());
		}
		CL->Y = ConsoleFont.TextHeight(std::string(">"+ConsoleCommand).c_str());		
		// </New Stuff>

			//'Add this text to the screen
		ConsoleLines.push_back(CL); //ListAddLast ConsoleLines, CL

		// Lines below no longer needed
			//'Move text up when as long is there's text below the bottom of the screen
			//While MaxY > GraphicsHeight() - 30
			//For CL2 = EachIn ConsoleLines
			//CL2.Y:-15
			//MaxY = CL2.Y
			//Next
			//Wend
			//'Remove all text that has gone past the top of the screen
			//For CL2 = EachIn Consolelines
			//If CL2.Y < 0 ListRemove ConsoleLines, CL2
			//Next

		// Console Log on disk for now out of use
			//If ConsoleLogBT
			//WriteLine ConsoleLogBT, "<tr valign=top><td colspan=2 style='font-family: Courier; color:" + HTMCol(R, G, B) + "'>" + HTMTxt(Txt) + "</td></tr>"
			//EndIf

		// All done!
		return CL;	
	}

	void ConsoleDoublePrint2(std::string Txt1, std::string Txt2, int offs, int R, int G, int B, int R2, int G2, int B2) {
		auto CL = New_ConsoleLine;
		auto CLA = New_ConsoleLine;
		auto MaxY = 0;
		CL->Txt = Txt1;
		CL->R = R;
		CL->G = G;
		CL->B = B;
		CLA->Txt=Txt2;
		CLA->R = R2;
		CLA->G = G2;
		CLA->B = B2;
		if (offs)
			CLA->X = offs;
		else
			CLA->X = ConsoleFont.TextWidth(CL->Txt.c_str());
		for (auto CL2 : ConsoleLines) {
			if (CL2->Linked)
				CL2->Y = CL2->Linked->Y;
			else
				CL2->Y += ConsoleFont.TextHeight(CL2->Txt.c_str());
		}
		CL->Y = ConsoleFont.TextHeight(std::string(">"+ConsoleCommand+"_").c_str());
		CLA->Y = CL->Y;
		CLA->Linked = CL;
		ConsoleLines.push_back(CL); 
		ConsoleLines.push_back(CLA);
	}
	ConsoleLine ConsoleWrite(std::string Txt, int R, int G, int B) { return ConsolePrint(Txt, R, G, B); }
	void ConsoleDoublePrint(std::string Txt1, std::string Txt2,  int R, int G, int B, int R2, int G2, int B2){ ConsoleDoublePrint2(Txt1, Txt2, 0, R, G, B, R2, G2, B2); }
	//void ConsoleDoublePrint2(std::string Txt1, std::string Txt2, int offs, int R, int G, int B, int R2, int G2, int B2) { ConsoleDoublePrint(Txt1, Txt2, offs, R, G, B, R2, G2, B2); }

	void ConsoleModify(ConsoleLine Line, std::string Txt, int R, int G, int B) {
		Line->Txt = Txt;
		if (R >= 0) Line->R = R;
		if (G >= 0) Line->G = G;
		if (B >= 0) Line->B = B;
	}

	std::string ConsoleBaseControl() {
		using namespace std;
		auto MaxScroll{ 0 };
		auto gch{ TQSE_GetChar() };
		string ret{ "" };
		if (ConsoleLines.size()) MaxScroll = ConsoleLines[0]->Y;
		if (TQSE_KeyDown(SDLK_UP)) Scroll = min(Scroll + 4, MaxScroll);
		if (TQSE_KeyDown(SDLK_DOWN)) Scroll = max(Scroll - 4, 0);
		if (gch >= 32 && gch <= 126) {
			string s = ">" + ConsoleCommand + "aa_";
			if (ConsoleFont.TextWidth(s.c_str()) < TQSG_ScreenWidth()) ConsoleCommand += gch;
		}
		if (TQSE_KeyHit(SDLK_BACKSPACE) && ConsoleCommand.size()) ConsoleCommand = left(ConsoleCommand, ConsoleCommand.size() - 1);
		if (TQSE_KeyHit(SDLK_KP_ENTER) || TQSE_KeyHit(SDLK_RETURN) || TQSE_KeyHit(SDLK_RETURN2)) {
			ret = ConsoleCommand;
			ConsolePrint(">" + ret);
			ConsoleCommand = "";
		}
		return ret;
	}

	

}