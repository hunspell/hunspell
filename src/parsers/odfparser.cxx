#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "../hunspell/csutil.hxx"
#include "odfparser.hxx"


#ifndef W32
using namespace std;
#endif

ODFParser::ODFParser(const char * wordchars)
{
	init(wordchars);
}

ODFParser::ODFParser(unsigned short * wordchars, int len)
{
	init(wordchars, len);
}

ODFParser::~ODFParser() 
{
}
