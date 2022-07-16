// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_BE_INSTANCE_H
#define INC_BE_INSTANCE_H

namespace be
{

struct IMapStream;
struct IDocument;
struct IController;

struct SInstance
{
	SInstance();
	~SInstance();

	bool init( IMapStream* mapStream );

	IDocument* d_document;
	IController* d_controller;
};

} // namespace be

#endif
