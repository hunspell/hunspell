#include "aff_manager.hxx"

#include <iostream>

int main()
{
	hunspell::aff_data aff;
	aff.encoding = "UTF-8";
	
	std::cout << aff.encoding;
	
	return 0;
}
