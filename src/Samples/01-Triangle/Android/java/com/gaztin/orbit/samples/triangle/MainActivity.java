package com.gaztin.orbit.samples.triangle;

import android.app.NativeActivity;

public class MainActivity extends NativeActivity
{
	static
	{
		System.loadLibrary( "01-Triangle" );
	}
}
