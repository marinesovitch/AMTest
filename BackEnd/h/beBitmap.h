// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_BITMAP_H
#define INC_BE_BITMAP_H

#include "beTypes.h"

namespace be
{

struct IBitmap
{
	public:
		virtual ~IBitmap();

	public:
		virtual bool lock( color_t** buffer ) = 0;
		virtual void unlock() = 0;

};

} // namespace be

#endif
