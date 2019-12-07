package com.gaztin.orbit.samples.cube;

import android.app.NativeActivity;

public class MainActivity extends NativeActivity
{
	static
	{
		System.loadLibrary( "02-Cube" );
	}
}
