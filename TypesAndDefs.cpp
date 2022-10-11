#include "TypesAndDefs.h"

Resolution::operator ScreenArea() {
	
	return ScreenArea{ 0, 0, width, height };

}