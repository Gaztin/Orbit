package com.gaztin.orbit.samples.instancing;

import android.app.NativeActivity;

public class MainActivity extends NativeActivity
{
	static
	{
		System.loadLibrary( "05-Instancing" );
	}
}
