// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#include "stdafx.h"
#include "feView.h"
#include "BackEnd/h/beDocument.h"
#include "BackEnd/h/beController.h"
#include "BackEnd/h/beBitmap.h"
#include "BackEnd/h/beTypes.h"
#include "resource.h"

namespace fe
{

namespace
{

const char* WindowClassName = "FrontEndWinAPIWindowClassName";

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

template< typename THandle >
class auto_gdi_object
{
	public:
		auto_gdi_object( HDC hdc, THandle gdiObject )
			: d_hdc( hdc )
		{
			d_prevGdiObject = ::SelectObject( d_hdc, gdiObject );
		}

		~auto_gdi_object()
		{
			::SelectObject( d_hdc, d_prevGdiObject );
		}

	private:
		HDC d_hdc;
		HGDIOBJ d_prevGdiObject;

};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KPen
{
	public:
		KPen( const int thickness );
		~KPen();

	public:
		operator HPEN();

	private:
		HPEN d_handle;

};

// ----------------------------------------------------------------------------

KPen::KPen( const int thickness )
	: d_handle( ::CreatePen( PS_SOLID, thickness, RGB( 0, 128, 0 ) ) )
{
}

KPen::~KPen()
{
	::DeleteObject( d_handle );
}

KPen::operator HPEN()
{
	return d_handle;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KBitmap :public be::IBitmap
{
	public:
		KBitmap( const be::SSize& deviceSize );
		virtual ~KBitmap();

	public:
		virtual bool lock( be::color_t** buffer );
		virtual void unlock();

		be::coord_t getWidth() const;
		be::coord_t getHeight() const;
		COLORREF getPixel( const int x, const int y ) const;

	private:
		const be::coord_t d_width;
		const be::coord_t d_height;
		be::color_t* d_pixels;

};

// ----------------------------------------------------------------------------

KBitmap::KBitmap( const be::SSize& deviceSize )
	: d_width( deviceSize.width )
	, d_height( deviceSize.height )
	, d_pixels( 0 )
{
}

KBitmap::~KBitmap()
{
	delete[] d_pixels;
}

bool KBitmap::lock( be::color_t** buffer )
{
	assert( d_pixels == 0 );
	d_pixels = new be::color_t[ d_width * d_height ];
	*buffer = d_pixels;
	const bool result = d_pixels != 0;
	return result;
}

void KBitmap::unlock()
{
}

be::coord_t KBitmap::getWidth() const
{
	return d_width;
}

be::coord_t KBitmap::getHeight() const
{
	return d_height;
}

COLORREF KBitmap::getPixel( const int x, const int y ) const
{
	const int pixelIndex = ( y * d_width ) + x;
	const be::color_t color = d_pixels[ pixelIndex ];
	const COLORREF result = color;
	return result;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KPainter
{
	public:
		KPainter( HWND hWnd, const be::color_t bkColor );
		~KPainter();

	public:
		void run( const KBitmap& bitmap );
		void fillBackground( const RECT& rc, HDC hdc = 0 );
		void drawParamsDescription( const std::string& description );

	private:
		HWND d_hwnd;
		PAINTSTRUCT d_ps;
		HDC d_hdc;
		const be::color_t d_bkColor;

};

// ----------------------------------------------------------------------------

KPainter::KPainter( HWND hWnd, const be::color_t bkColor )
	: d_hwnd( hWnd )
	, d_hdc( 0 )
	, d_bkColor( bkColor )
{
	d_hdc = ::BeginPaint( d_hwnd, &d_ps );
}

KPainter::~KPainter()
{
	if ( d_hdc )
		::EndPaint( d_hwnd, &d_ps );
}

void KPainter::run( const KBitmap& bitmap )
{
	const be::coord_t width = bitmap.getWidth();
	const be::coord_t height = bitmap.getHeight();

	HDC hdcMem = ::CreateCompatibleDC(d_hdc);
	HBITMAP hBmp = ::CreateCompatibleBitmap( d_hdc, width, height );
	::SelectObject(hdcMem, hBmp);

	const RECT rc = { 0, 0, width, height };
	fillBackground( rc, hdcMem );

	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			const COLORREF color = bitmap.getPixel( x, y );
			if ( color != d_bkColor )
			{
				::SetPixel( hdcMem, x, y, color );
			}
		}
	}

	::BitBlt(d_hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
	::DeleteDC(hdcMem);
	::DeleteObject( hBmp );
}

void KPainter::fillBackground( const RECT& rc, HDC hdc )
{
	if ( hdc == 0 )
		hdc = d_hdc;
	HBRUSH hbrush = ::CreateSolidBrush( d_bkColor );
	::FillRect( hdc, &rc, hbrush );
	::DeleteObject( hbrush );
}

void KPainter::drawParamsDescription( const std::string& description )
{
	const int DescriptionScreenOffset = 10;
	::TextOut(
		d_hdc,
		DescriptionScreenOffset,
		DescriptionScreenOffset,
		description.c_str(),
		static_cast<int>(description.length()) );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

class KView : public IView
{
	public:
		KView(
			HINSTANCE hInstance,
			be::IDocument* beDocument,
			be::IController* beController );
		virtual ~KView();

	public:
		virtual bool init( const int cmdShow );
		virtual int run();

	private:
		void registerClass();

	private:
		static LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

		void onSize( WPARAM wParam, LPARAM lParam );
		void onPaint();

		void onLButtonDown( WPARAM wParam, LPARAM lParam );
		void onRButtonDown( WPARAM wParam, LPARAM lParam );
		bool onKeyPressed( WPARAM wParam, LPARAM lParam );

		bool onCommand( WPARAM wParam, LPARAM lParam );

	private:
		void move( const be::EDirection direction );
		void move( const be::SPoint& focusScreenPoint );
		void zoomIn( const be::SPoint& focusScreenPoint );
		void zoomOut( const be::SPoint& focusScreenPoint );
		void resetView();

		void refresh();

		void showPopupMenu( const int x, const int y );

		void toggleShowParams();

	private:
		HINSTANCE d_hInstance;
		HWND d_hwnd;
		be::IDocument* d_beDocument;
		be::IController* d_beController;
		bool d_showParams;

};

// ----------------------------------------------------------------------------

KView::KView(
	HINSTANCE hInstance,
	be::IDocument* beDocument,
	be::IController* beController )
	: d_hInstance( hInstance )
	, d_beDocument( beDocument )
	, d_beController( beController )
	, d_showParams( true )
{
	registerClass();
}

KView::~KView()
{
}

int KView::run()
{
	HACCEL hAccelTable = ::LoadAccelerators( d_hInstance, MAKEINTRESOURCE( IDC_FRONTENDWINAPI ) );

	MSG msg;
	while ( 0 < ::GetMessage( &msg, 0, 0, 0 ) )
	{
		if ( ! ::TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
		{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
	}

	const int result = static_cast< int >( msg.wParam );
	return result;
}

bool KView::init( const int cmdShow )
{
	d_hwnd = ::CreateWindow(
		WindowClassName,
		"AMtest",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		d_hInstance,
		this );

	if ( d_hwnd)
	{
		::ShowWindow( d_hwnd, cmdShow );
		::UpdateWindow( d_hwnd );
	}

	const bool result = ( d_hwnd != 0 );
	return result;
}

// ----------------------------------------------------------------------------

void KView::registerClass()
{
	static bool s_registered = false;
	if ( !s_registered )
	{
		s_registered = true;

		WNDCLASSEX wcex;

		wcex.cbSize = sizeof( WNDCLASSEX );

		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcex.lpfnWndProc = &KView::WndProc;
		wcex.cbClsExtra	= 0;
		wcex.cbWndExtra	= 0;
		wcex.hInstance = d_hInstance;
		wcex.hIcon = ::LoadIcon( d_hInstance, MAKEINTRESOURCE(IDI_FRONTENDWINAPI) );
		wcex.hCursor = ::LoadCursor( 0, IDC_ARROW );
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_FRONTENDWINAPI);
		wcex.lpszClassName = WindowClassName;
		wcex.hIconSm = ::LoadIcon( wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL) );

		::RegisterClassEx( &wcex );
	}
}

// ----------------------------------------------------------------------------

LRESULT CALLBACK KView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT result = 0;
	static KView* s_view = 0;

	bool callDefProc = false;
	switch (message)
	{
		case WM_NCCREATE:
			assert( s_view == 0 );
			s_view = reinterpret_cast< KView* >( reinterpret_cast< LPCREATESTRUCT >( lParam )->lpCreateParams );
			result = ::DefWindowProc( hWnd, message, wParam, lParam );
			break;

		case WM_SIZE:
			s_view->onSize( wParam, lParam );
			break;

		case WM_PAINT:
			s_view->onPaint();
			break;

		case WM_ERASEBKGND:
			result = true;
			break;

		case WM_LBUTTONDOWN:
			s_view->onLButtonDown( wParam, lParam );
			break;

		case WM_RBUTTONDOWN:
			s_view->onRButtonDown( wParam, lParam );
			break;

		case WM_KEYUP:
			if ( !s_view->onKeyPressed( wParam, lParam ) )
				callDefProc = true;
			break;

		case WM_COMMAND:
			if ( !s_view->onCommand( wParam, lParam ) )
				callDefProc = true;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			callDefProc = true;
	}

	if ( callDefProc )
		result = DefWindowProc( hWnd, message, wParam, lParam );

	return result;
}

void KView::onSize( WPARAM /*wParam*/, LPARAM lParam )
{
	const be::coord_t width = LOWORD( lParam );
	const be::coord_t height = HIWORD( lParam );
	const be::SSize deviceSize( width, height );
	d_beController->setDeviceSize( deviceSize );
	refresh();
}

void KView::onPaint()
{
	RECT rc = { 0, 0, 0, 0 };
	::GetClientRect( d_hwnd, &rc );
	const be::SSize deviceSize( rc.right - rc.left, rc.bottom - rc.top );
	const be::color_t bkColor = d_beDocument->getBkColor();
	KPainter painter( d_hwnd, bkColor );
	KBitmap bitmap( deviceSize );
	if ( d_beController->generateContents( &bitmap ) )
	{
		painter.run( bitmap );
	}
	else
	{
		painter.fillBackground( rc );
	}

	if ( d_showParams )
	{
		const std::string& description = d_beController->getParamsDescription();
		painter.drawParamsDescription( description );
	}
}

void KView::onLButtonDown( WPARAM wParam, LPARAM lParam )
{
	const be::coord_t x = LOWORD( lParam );
	const be::coord_t y = HIWORD( lParam );
	const be::SPoint focusScreenPoint( x, y );
	if ( ( wParam & MK_CONTROL ) == MK_CONTROL )
	{
		zoomIn( focusScreenPoint );
	}
	else
	{
		move( focusScreenPoint );
	}
}

void KView::onRButtonDown( WPARAM wParam, LPARAM lParam )
{
	const int x = LOWORD( lParam );
	const int y = HIWORD( lParam );
	if ( ( wParam & MK_CONTROL ) == MK_CONTROL )
	{
		const be::SPoint focusScreenPoint( x, y );
		zoomOut( focusScreenPoint );
	}
	else
	{
		POINT pt = { x, y };
		::ClientToScreen( d_hwnd, &pt );
		showPopupMenu( pt.x, pt.y );
	}
}

bool KView::onKeyPressed( WPARAM wParam, LPARAM /*lParam*/ )
{
	be::EDirection direction = be::UnknownDirection;
	switch( wParam )
	{
		case VK_UP:
			direction = be::North;
			break;

		case VK_DOWN:
			direction = be::South;
			break;

		case VK_LEFT:
			direction = be::West;
			break;

		case VK_RIGHT:
			direction = be::East;
			break;
	}

	const bool handled = ( direction != be::UnknownDirection );
	if ( handled )
		move( direction );
	return handled;
}

bool KView::onCommand( WPARAM wParam, LPARAM lParam )
{
	bool result = true;
	const int wmId = LOWORD(wParam);
	switch ( wmId )
	{
		case IDM_SHOW_PARAMS:
			toggleShowParams();
			break;

		case IDM_RESET_VIEW:
			resetView();
			break;

		default:
			result = false;
	}
	return result;
}

// ----------------------------------------------------------------------------

void KView::move( const be::EDirection direction )
{
	const be::SMoveData moveData( direction );
	if ( d_beController->move( moveData ) )
		refresh();
}

void KView::move( const be::SPoint& focusScreenPoint )
{
	const be::SMoveData moveData( focusScreenPoint );
	if ( d_beController->move( moveData ) )
		refresh();
}

void KView::zoomIn( const be::SPoint& focusScreenPoint )
{
	const be::SZoomData zoomData( be::SZoomData::ZoomIn, 1, false, focusScreenPoint );
	if ( d_beController->zoom( zoomData ) )
		refresh();
}

void KView::zoomOut( const be::SPoint& focusScreenPoint )
{
	const be::SZoomData zoomData( be::SZoomData::ZoomOut, 1, false, focusScreenPoint );
	if ( d_beController->zoom( zoomData ) )
		refresh();
}

void KView::resetView()
{
	d_beController->resetView();
	refresh();
}

void KView::refresh()
{
	::InvalidateRect( d_hwnd, 0, false );
}

void KView::showPopupMenu( const int x, const int y )
{
	HMENU popupMenu = ::CreatePopupMenu();
	::InsertMenu( popupMenu, 0, MF_BYPOSITION | MF_STRING, IDM_SHOW_PARAMS, "Show params" );
	::InsertMenu( popupMenu, 0, MF_BYPOSITION | MF_STRING, IDM_RESET_VIEW, "Reset view");
	::TrackPopupMenu( popupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, d_hwnd, 0 );
}

void KView::toggleShowParams()
{
	d_showParams = !d_showParams;
	refresh();
}

} // anonymous namespace

// ----------------------------------------------------------------------------

IView::~IView()
{
}

IView* createView(
	HINSTANCE hInstance,
	be::IDocument* beDocument,
	be::IController* beController )
{
	IView* view = new KView( hInstance, beDocument, beController );
	return view;
}

} // namespace fe
