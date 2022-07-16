// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
package com.amtest.frontend;

import android.util.Log;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.*;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

// ----------------------------------------------------------------------------

public class AMPrefs extends PreferenceActivity
{
	public static boolean getShowParams( Context context )
	{
		SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences( context );
		boolean result = sharedPreferences.getBoolean( OptionShowParams, OptionShowParamsDefault );
		return result;
	}

	public static boolean getExtraGesturesEnabled( Context context )
	{
		SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences( context );
		boolean result = sharedPreferences.getBoolean( OptionEnableExtraGestures, OptionEnableExtraGesturesDefault );
		return result;
	}

	// ----------------------------------------------------------------------------

	@Override
	protected void onCreate( Bundle savedInstanceState )
	{
		super.onCreate( savedInstanceState );
		addPreferencesFromResource( R.xml.settings );
	}

	// ----------------------------------------------------------------------------

	private static final String OptionShowParams = "show_params";
	private static final boolean OptionShowParamsDefault = false;

	private static final String OptionEnableExtraGestures = "enable_extra_gestures";
	private static final boolean OptionEnableExtraGesturesDefault = true;

}
