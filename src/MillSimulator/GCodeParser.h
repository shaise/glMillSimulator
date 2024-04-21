#ifndef __csgcodeparser_h__
#define __csgcodeparser_h__
#include "MillMotion.h"
#include <vector>

namespace MillSim
{
	struct GCToken
	{
		char letter;
		float fval;
		int ival;
	};

	class GCodeParser
	{
	public:
		GCodeParser() {}
		virtual ~GCodeParser();
		bool Parse(const char* filename);

	public:
		std::vector<MillMotion> Operations;
		MillMotion lastState = { eNop };

	protected:
		char* GetNextToken(char* ptr, GCToken* token);
		bool IsValidTok(char tok);
		char* ParseFloat(char* ptr, float* retFloat);
		bool ParseLine(char* ptr);
		bool AddLine(char* ptr);
		int lastTool = -1;
	};
}
#endif
