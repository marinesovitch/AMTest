// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
package com.amtest.frontend;

import android.util.Log;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.*;
import android.graphics.*;
import android.os.*;
import android.view.*;
import java.io.*;

// ----------------------------------------------------------------------------

public class AMTest extends Activity
{
	@Override
	public void onCreate( Bundle savedInstanceState )
	{
		super.onCreate( savedInstanceState );

		d_beInstanceHandle = allocBackEndInstance();
		d_document = new KDocument( d_beInstanceHandle );
		d_controller = new KController( d_beInstanceHandle );
		d_view = new KView( this, d_document, d_controller );
		setContentView( d_view );
	}

	@Override
	protected void onDestroy()
	{
		destroyBackendInstance( d_beInstanceHandle );
		super.onDestroy();
	}

	@Override
	public void onConfigurationChanged( Configuration newConfig )
	{
		super.onConfigurationChanged( newConfig );
		d_view.updateSize();
	}

	@Override
	public boolean onCreateOptionsMenu( Menu menu )
	{
		super.onCreateOptionsMenu( menu );
		MenuInflater inflater = getMenuInflater();
		inflater.inflate( R.menu.menu, menu );
		return true;
	}

	@Override
	public boolean onOptionsItemSelected( MenuItem item )
	{
		boolean result = true;
		switch ( item.getItemId() )
		{
			case R.id.settings:
				showPreferences();
				break;

			case R.id.reset_view:
				resetView();
				break;

			default:
				result = false;

		}
		return result;
	}

	static
	{
		System.loadLibrary( "amtestbackend" );
	}

	// ----------------------------------------------------------------------------

	private int allocBackEndInstance()
	{
		int result = 0;

		try
		{
			byte[] mapBytesArray = readMap( MapFileName );
			if ( mapBytesArray != null )
				result = createBackendInstance( mapBytesArray, mapBytesArray.length );
		}
		catch ( IOException e )
		{
			String errorMessage = e.getMessage();
			Log.e( TAG, errorMessage );
		}

		return result;
	}

	private byte[] readMap( String mapFileName ) throws IOException
	{
		Resources resources = getResources();
		AssetManager assetManager = resources.getAssets();
		InputStream is = assetManager.open( mapFileName );
		ByteArrayOutputStream mapBytesStream = new ByteArrayOutputStream();

		boolean isEof = false;
		final int BufferSize = 16 * 1024;
		byte[] buffer = new byte[ BufferSize ];

		while ( !isEof )
		{
			int readCount = is.read( buffer, 0, buffer.length );
			if ( readCount != -1 )
				mapBytesStream.write( buffer, 0, readCount );
			else
				isEof = true;
		}

		byte[] result = mapBytesStream.toByteArray();
		return result;
	}

	private void showPreferences()
	{
		startActivity( new Intent( this, AMPrefs.class ) );
	}

	private void resetView()
	{
		if ( d_controller.resetView() )
			d_view.invalidate();
	}

	// ----------------------------------------------------------------------------

	private native int createBackendInstance( byte[] mapBytesArray, int mapBytesArraySize );
	private native void destroyBackendInstance( long beInstanceHandle );

	private long d_beInstanceHandle;

	private KView d_view;
	private KDocument d_document;
	private KController d_controller;

	private static final String TAG = "AMtest";
	private static final String MapFileName = "mapa.dat";

}

// ----------------------------------------------------------------------------

class KDocument
{
	KDocument( long beInstanceHandle )
	{
		d_beInstanceHandle = beInstanceHandle;
	}

	// ----------------------------------------------------------------------------

	String getState()
	{
		return getState( d_beInstanceHandle );
	}

	void setState( String stateString )
	{
		setState( d_beInstanceHandle, stateString );
	}

	short getBkColor()
	{
		return getBkColor( d_beInstanceHandle );
	}

	// ----------------------------------------------------------------------------

	private native String getState( long beInstanceHandle );
	private native void setState( long beInstanceHandle, String stateString );
	private native short getBkColor( long beInstanceHandle );

	private long d_beInstanceHandle;

}

// ----------------------------------------------------------------------------

class KController
{
	KController( long beInstanceHandle )
	{
		d_beInstanceHandle = beInstanceHandle;
	}

	boolean setDeviceSize( int width, int height )
	{
		return setDeviceSize( d_beInstanceHandle, width, height );
	}

	boolean moveTo( int focusPosX, int focusPosY )
	{
		return moveTo( d_beInstanceHandle, focusPosX, focusPosY );
	}

	boolean moveInDirection( EDirection direction )
	{
		int rawDirection = direction.ordinal();
		return moveInDirection( d_beInstanceHandle, rawDirection );
	}

	boolean moveDelta( int deltaX, int deltaY )
	{
		return moveDelta( d_beInstanceHandle, deltaX, deltaY );
	}

	boolean zoomIn( int delta, boolean zoomInPlace, int focusPosX, int focusPosY )
	{
		return zoomIn( d_beInstanceHandle, delta, zoomInPlace, focusPosX, focusPosY );
	}

	boolean zoomOut( int delta, boolean zoomInPlace, int focusPosX, int focusPosY )
	{
		return zoomOut( d_beInstanceHandle, delta, zoomInPlace, focusPosX, focusPosY );
	}

	boolean resetView()
	{
		return resetView( d_beInstanceHandle );
	}

	boolean generateContents( Bitmap bitmap )
	{
		return generateContents( d_beInstanceHandle, bitmap );
	}

	String getParamsDescription()
	{
		return getParamsDescription( d_beInstanceHandle );
	}

	// ----------------------------------------------------------------------------

	enum EDirection
	{
		UnknownDirection,
		North,
		East,
		South,
		West
	}

	// ----------------------------------------------------------------------------

	private native boolean setDeviceSize( long beInstanceHandle, int width, int height );
	private native boolean moveTo( long beInstanceHandle, int focusPosX, int focusPosY );
	private native boolean moveInDirection( long beInstanceHandle, int direction );
	private native boolean moveDelta( long beInstanceHandle, int deltaX, int deltaY );
	private native boolean zoomIn(
		long beInstanceHandle,
		int delta,
		boolean zoomInPlace,
		int focusPosX,
		int focusPosY );
	private native boolean zoomOut(
		long beInstanceHandle,
		int delta,
		boolean zoomInPlace,
		int focusPosX,
		int focusPosY );
	private native boolean resetView( long beInstanceHandle );
	private native boolean generateContents( long beInstanceHandle, Bitmap bitmap );
	private native String getParamsDescription( long beInstanceHandle );

	// ----------------------------------------------------------------------------

	private int d_beInstanceHandle;
}

// ----------------------------------------------------------------------------

class KView extends View
{
	KView(
		Context context,
		KDocument document,
		KController controller )
	{
		super( context );
		setId( ViewId );
		d_document = document;
		d_controller = controller;

		java.util.ArrayList< View > touchables = new java.util.ArrayList< View >();
		touchables.add( this );
		addTouchables( touchables );

		setFocusable( true );
		setFocusableInTouchMode( true );


		d_simpleGestureListener = new KSimpleGestureListener( getContext(), this, d_controller );
		d_twoFingersTapGestureDetector = new KTwoFingersTapGestureDetector( d_simpleGestureListener );
		d_gestureDetector = new GestureDetector( getContext(), d_simpleGestureListener );

		d_scaleGestureListener = new KScaleGestureListener( this, d_controller );
		d_scaleGestureDetector = new ScaleGestureDetector( getContext(), d_scaleGestureListener );
	}

	void updateSize()
	{
		int width = getWidth();
		int height = getHeight();
		updateSize( width, height );
	}

	// ----------------------------------------------------------------------------

	@Override
	protected void onSizeChanged( int width, int height, int oldWidth, int oldHeight )
	{
		super.onSizeChanged( width, height, oldWidth, oldHeight );
		updateSize( width, height );
	}

	@Override
	public boolean onKeyDown( int keyCode, KeyEvent event)
	{
		boolean consumed = false;
		KController.EDirection direction = KController.EDirection.UnknownDirection;
		switch ( keyCode )
		{
			case KeyEvent.KEYCODE_DPAD_UP:
				direction = KController.EDirection.North;
				break;

			case KeyEvent.KEYCODE_DPAD_RIGHT:
				direction = KController.EDirection.East;
				break;

			case KeyEvent.KEYCODE_DPAD_DOWN:
				direction = KController.EDirection.South;
				break;

			case KeyEvent.KEYCODE_DPAD_LEFT:
				direction = KController.EDirection.West;
				break;

			default:
				consumed = super.onKeyDown( keyCode, event);
		}

		if ( direction != KController.EDirection.UnknownDirection )
		{
			consumed = true;
			if ( d_controller.moveInDirection( direction ) )
				invalidate();
		}
		return consumed;
	}

	@Override
	public boolean onTouchEvent( MotionEvent event )
	{
		boolean result = d_twoFingersTapGestureDetector.onTouchEvent( event );
		result |= d_gestureDetector.onTouchEvent( event );
		if ( AMPrefs.getExtraGesturesEnabled( getContext() ) )
			result |= d_scaleGestureDetector.onTouchEvent( event );
		return result;
	}

	@Override
	protected void onDraw( Canvas canvas )
	{
		assert d_bitmap != null;
		if ( !d_controller.generateContents( d_bitmap ) )
			fillBackground();
		canvas.drawBitmap( d_bitmap, 0, 0, null );
		if ( AMPrefs.getShowParams( getContext() ) )
			drawParamsDescription( canvas );
	}

	@Override
	protected Parcelable onSaveInstanceState()
	{
		Parcelable state = super.onSaveInstanceState();
		Bundle bundle = new Bundle();
		String stateStr = d_document.getState();
		bundle.putString( StateStringKey, stateStr );
		bundle.putParcelable( ViewStateKey, state );
		return bundle;
	}

	@Override
	protected void onRestoreInstanceState( Parcelable state )
	{
		Bundle bundle = (Bundle) state;
		String stateStr = bundle.getString( StateStringKey );
		d_document.setState( stateStr );
		super.onRestoreInstanceState( bundle.getParcelable( ViewStateKey ) );
	}

	// ----------------------------------------------------------------------------

	private void updateSize( int width, int height )
	{
		if ( d_controller.setDeviceSize( width, height ) )
			d_bitmap = Bitmap.createBitmap( width, height, Bitmap.Config.RGB_565 );
		// invalidate even if device size is the same, important for hypothetical
		// rectangle-shape devices after change of orientation
		invalidate();
	}

	private void fillBackground()
	{
		int color = d_document.getBkColor();
		d_bitmap.eraseColor( color );
	}

	private void drawParamsDescription( Canvas canvas )
	{
		Paint paint = new Paint();
		paint.setTextSize( paint.getTextSize() * 1.2f );
		int DescriptionScreenOffset = 25;
		String description = d_controller.getParamsDescription();
		canvas.drawText( description, DescriptionScreenOffset, DescriptionScreenOffset, paint );
	}

	// ----------------------------------------------------------------------------

	private KDocument d_document;
	private KController d_controller;
	private Bitmap d_bitmap;

	private KSimpleGestureListener d_simpleGestureListener;
	private KTwoFingersTapGestureDetector d_twoFingersTapGestureDetector;
	private GestureDetector d_gestureDetector;

	private KScaleGestureListener d_scaleGestureListener;
	private ScaleGestureDetector d_scaleGestureDetector;


	static final int ViewId = 1;
	static final String StateStringKey = "StateString";
	static final String ViewStateKey = "ViewState";

}

// ----------------------------------------------------------------------------

class KTwoFingersTapGestureDetector
{
	interface IOnTwoFingersTapListener
	{
		boolean onTwoFingersTap( MotionEvent event );
	}

	// ----------------------------------------------------------------------------

	KTwoFingersTapGestureDetector( IOnTwoFingersTapListener listener )
	{
		d_listener = listener;
	}

	// ----------------------------------------------------------------------------

	boolean onTouchEvent( MotionEvent event )
	{
		boolean result = false;

		if ( checkTimeDiff( event ) )
			result = processTouchEvent( event );
		else
			resetState();

		return result;
	}

	// ----------------------------------------------------------------------------

	private boolean checkTimeDiff( MotionEvent event )
	{
		boolean result = true;

		if ( d_startTimestamp == 0 )
		{
			d_startTimestamp = event.getEventTime();
		}
		else
		{
			long endTimestamp = event.getEventTime();
			long gestureDuration = endTimestamp - d_startTimestamp;
			if ( GestureDurationTolerance < gestureDuration )
				result = false;
		}

		return result;
	}

	private boolean processTouchEvent( MotionEvent event )
	{
		boolean result = false;

		int action = event.getActionMasked();
		switch ( action )
		{
			case MotionEvent.ACTION_DOWN:
				result = onTouchDown(
					event,
					0,
					EState.WaitForFirstTouchDown,
					EState.WaitForSecondTouchDown );
				break;

			case MotionEvent.ACTION_POINTER_DOWN:
				result = onTouchDown(
					event,
					1,
					EState.WaitForSecondTouchDown,
					EState.WaitForFirstTouchUp );
				break;

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
				result = onTouchUp( event );
				break;

			case MotionEvent.ACTION_MOVE:
				if ( isMovementDetected( event ) )
					resetState();
				break;
		}

		return result;
	}

	private boolean onTouchDown(
		MotionEvent event,
		int pointerIndex,
		EState expectedCurrentState,
		EState newState )
	{
		boolean result = false;
		if ( d_state == expectedCurrentState )
		{
			result = true;
			d_state = newState;
			storePointerPos( event, pointerIndex );
		}
		else
		{
			resetState();
		}
		return result;
	}

	private boolean onTouchUp( MotionEvent event)
	{
		boolean result = true;
		switch ( d_state )
		{
			case WaitForFirstTouchUp:
				d_state = EState.WaitForSecondTouchUp;
				break;

			case WaitForSecondTouchUp:
				gestureDetected( event );
				break;

			default:
				resetState();
				result = false;
		}
		return result;
	}

	private boolean isMovementDetected( MotionEvent event )
	{
		boolean result = isPointerMoved( event, 0 ) || isPointerMoved( event, 1 );
		return result;
	}

	private boolean gestureDetected( MotionEvent event )
	{
		resetState();
		boolean result = d_listener.onTwoFingersTap( event );
		return result;
	}

	private void resetState()
	{
		d_state = EState.WaitForFirstTouchDown;
		d_startTimestamp = 0;
		d_pointers[ 0 ] = null;
		d_pointers[ 1 ] = null;
	}

	// ----------------------------------------------------------------------------

	private boolean isPointerMoved( MotionEvent event, int pointerIndex )
	{
		boolean result = false;
		if ( pointerIndex < event.getPointerCount() )
		{
			PointF pointerInitPos = d_pointers[ pointerIndex ];
			if ( pointerInitPos != null )
			{
				float x = event.getX( pointerIndex );
				float y = event.getY( pointerIndex );

				result = ( MovementTolerance < Math.abs( pointerInitPos.x - x ) )
					|| ( MovementTolerance < Math.abs( pointerInitPos.y - y ) );
			}
		}
		return result;
	}

	private void storePointerPos(
		MotionEvent event,
		int pointerIndex )
	{
		if ( pointerIndex < event.getPointerCount() )
		{
			PointF pointer = getPointer( pointerIndex );
			pointer.x = event.getX( pointerIndex );
			pointer.y = event.getY( pointerIndex );
		}
	}

	private PointF getPointer( int pointerIndex )
	{
		assert ( pointerIndex == 0 ) || ( pointerIndex == 1 );
		PointF pointer = d_pointers[ pointerIndex ];
		if ( pointer == null )
		{
			pointer = new PointF();
			d_pointers[ pointerIndex ] = pointer;
		}
		return pointer;
	}

	// ----------------------------------------------------------------------------

	private enum EState
	{
		WaitForFirstTouchDown,
		WaitForSecondTouchDown,
		WaitForFirstTouchUp,
		WaitForSecondTouchUp
	}

	private EState d_state = EState.WaitForFirstTouchDown;

	private long d_startTimestamp;

	private PointF[] d_pointers = new PointF[ 2 ];

	private IOnTwoFingersTapListener d_listener;

	private static final float MovementTolerance = 20f; // points
	private static final long GestureDurationTolerance = 250; // milliseconds

}

// ----------------------------------------------------------------------------

class KSimpleGestureListener extends GestureDetector.SimpleOnGestureListener
	implements KTwoFingersTapGestureDetector.IOnTwoFingersTapListener
{
	KSimpleGestureListener(
		Context context,
		View view,
		KController controller )
	{
		d_context = context;
		d_view = view;
		d_controller = controller;
	}

	// ----------------------------------------------------------------------------
	// GestureDetector.SimpleOnGestureListener

	@Override
	public boolean onSingleTapConfirmed( MotionEvent event )
	{
		int x = (int) event.getX();
		int y = (int) event.getY();
		if ( d_controller.moveTo( x, y ) )
			d_view.invalidate();
		return true;
	}

	@Override
	public boolean onDoubleTap( MotionEvent event )
	{
		int x = (int) event.getX();
		int y = (int) event.getY();
		if ( d_controller.zoomIn( 1, false, x, y ) )
			d_view.invalidate();
		return true;
	}

	@Override
	public boolean onScroll(
		MotionEvent event1,
		MotionEvent event2,
		float distanceX,
		float distanceY )
	{
		boolean consumed = AMPrefs.getExtraGesturesEnabled( d_context );
		if ( consumed )
		{
			int deltaX = (int) distanceX;
			int deltaY = (int) distanceY;
			if ( d_controller.moveDelta( deltaX, deltaY ) )
				d_view.invalidate();
		}
		return consumed;
	}

	// ----------------------------------------------------------------------------
	// KTwoFingersTapGestureDetector.IOnTwoFingersTapListener

	@Override
	public boolean onTwoFingersTap( MotionEvent event )
	{
		int x = (int) event.getX();
		int y = (int) event.getY();
		if ( d_controller.zoomOut( 1, false, x, y ) )
			d_view.invalidate();
		return true;
	}

	// ----------------------------------------------------------------------------

	private Context d_context;
	private View d_view;
	private KController d_controller;

}

// ----------------------------------------------------------------------------

class KScaleGestureListener implements ScaleGestureDetector.OnScaleGestureListener
{
	KScaleGestureListener(
		View view,
		KController controller )
	{
		d_view = view;
		d_controller = controller;
	}

	// ----------------------------------------------------------------------------
	// ScaleGestureDetector.OnScaleGestureListener

	@Override
	public boolean onScaleBegin( ScaleGestureDetector detector )
	{
		d_zoomSpanThreshold = calcZoomSpanThreshold();
		d_startSpan = (int) detector.getCurrentSpan();
		d_prevZoomFactorDeviation = 0;
		return true;
	}

	@Override
	public boolean onScale( ScaleGestureDetector detector )
	{
		boolean consumed = performZoom( detector );
		return consumed;
	}

	@Override
	public void onScaleEnd( ScaleGestureDetector detector )
	{
		performZoom( detector );
	}

	// ----------------------------------------------------------------------------

	boolean performZoom( ScaleGestureDetector detector )
	{
		int currentSpan = (int) detector.getCurrentSpan();
		int diffSpan = currentSpan - d_startSpan;
		int currentZoomFactorDeviation = diffSpan / d_zoomSpanThreshold;

		int zoomDelta = currentZoomFactorDeviation - d_prevZoomFactorDeviation;
		boolean zoomChanged = ( zoomDelta != 0 );
		if ( zoomChanged )
		{
			boolean refresh = false;
			d_prevZoomFactorDeviation = currentZoomFactorDeviation;
			int focusX = (int) detector.getFocusX();
			int focusY = (int) detector.getFocusY();
			if ( zoomDelta < 0 )
			{
				refresh = d_controller.zoomOut( -zoomDelta, true, focusX, focusY );
			}
			else if ( 0 < zoomDelta )
			{
				refresh = d_controller.zoomIn( zoomDelta, true, focusX, focusY );
			}
			if ( refresh )
				d_view.invalidate();
		}

		return zoomChanged;
	}

	private int calcZoomSpanThreshold()
	{
		int scale = Math.max( d_view.getWidth(), d_view.getHeight() );
		int prefZoomSpanThreshold = scale / 10;
		int result = Math.max( MinZoomSpanThreshold, prefZoomSpanThreshold );
		return result;
	}

	// ----------------------------------------------------------------------------

	private View d_view;
	private KController d_controller;

	private int d_zoomSpanThreshold;
	private int d_startSpan;
	private int d_prevZoomFactorDeviation;

	static private final int MinZoomSpanThreshold = 50;

}
