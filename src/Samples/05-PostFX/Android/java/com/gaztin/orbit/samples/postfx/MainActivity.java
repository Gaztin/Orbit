package com.gaztin.orbit.samples.postfx;

import android.app.NativeActivity;

public class MainActivity extends NativeActivity
{
	static
	{
		System.loadLibrary( "05-PostFX" );
	}
}
