// author: marines marinesovitch alias Darek Slusarczyk 2012-2013
package com.amtest.frontend;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;

import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceFragmentCompat;

// ----------------------------------------------------------------------------

public class AMPrefs extends AppCompatActivity
{
	public static boolean getShowParams( Context context )
	{
		SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences( context );
		return sharedPreferences.getBoolean( OptionShowParams, OptionShowParamsDefault );
	}

	public static boolean getExtraGesturesEnabled( Context context )
	{
		SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences( context );
		return sharedPreferences.getBoolean( OptionEnableExtraGestures, OptionEnableExtraGesturesDefault );
	}

	// ----------------------------------------------------------------------------

	@Override
	protected void onCreate( Bundle savedInstanceState )
	{
		super.onCreate( savedInstanceState );
		setContentView(R.layout.preferences);
		getSupportFragmentManager().beginTransaction().replace(R.id.preferences, new AMPrefsFragment()).commit();
	}

	public static class AMPrefsFragment extends PreferenceFragmentCompat {

		@Override
		public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
			setPreferencesFromResource(R.xml.settings, rootKey);
		}
	}

	// ----------------------------------------------------------------------------

	private static final String OptionShowParams = "show_params";
	private static final boolean OptionShowParamsDefault = false;

	private static final String OptionEnableExtraGestures = "enable_extra_gestures";
	private static final boolean OptionEnableExtraGesturesDefault = true;

}
