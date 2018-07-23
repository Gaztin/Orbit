package com.gaztin.orbit;

import android.support.wearable.activity.WearableActivity;

public class MainActivity extends WearableActivity
{
	static
	{
		System.loadLibrary("orb01");
	}
}
