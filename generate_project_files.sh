#!/bin/bash
if [ ! -f "./premake5" ]; then
	echo "Premake executable not found. Run 'init' to fetch one."
	read -p "Press enter to continue.."
	exit
fi

# If passing custom arguments, use those instead
if [ $# -gt 0 ]; then
	args=$@
else
	# Determine default action from system
	os=$(uname -s)
	if [ "$os" == "Linux" ]; then
		args="codelite"
	elif [ "$os" == "Darwin" ]; then
		args="xcode4"
	else
		args="vs2017"
	fi
fi

./premake5 $args
