#include "interface.h"

void clearLine()
{
	cout << "\r";
	for (int i = 0; i < 120; i++)
		cout << ' ';
}