// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_CONTROLLER_H
#define INC_BE_CONTROLLER_H

#include "beTypes.h"

namespace be
{

struct IBitmap;

struct IController
{
	public:
		virtual ~IController();

	public:
		virtual bool setDeviceSize( const SSize& deviceSize ) = 0;
		virtual bool move( const SMoveData& moveData ) = 0;
		virtual bool zoom( const SZoomData& zoomData ) = 0;
		virtual bool resetView() = 0;

		virtual bool generateContents( IBitmap* bitmap ) = 0;

		virtual std::string getParamsDescription() const = 0;
};

} // namespace be

#endif
