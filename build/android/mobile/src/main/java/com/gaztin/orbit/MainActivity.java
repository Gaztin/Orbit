package com.gaztin.orbit;

import android.app.NativeActivity;

public class MainActivity extends NativeActivity
{
	static
	{
		System.loadLibrary("orb01");
	}
}
