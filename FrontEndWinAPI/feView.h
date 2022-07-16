// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
#ifndef INC_FE_VIEW_H
#define INC_FE_VIEW_H

namespace be
{
	struct IController;
	struct IDocument;
}

namespace fe
{

struct IView
{
	public:
		virtual ~IView();

	public:
		virtual bool init( const int cmdShow ) = 0;
		virtual int run() = 0;

};

IView* createView( 
	HINSTANCE hInstance,
	be::IDocument* beDocument,
	be::IController* beController );

} // namespace fe

#endif
