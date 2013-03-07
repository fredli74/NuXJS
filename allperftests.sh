if [ -z "$1" ]; then
	EXE="build/Release/NuXJScript"
else
	EXE=$1
fi

for f in perftest*.js; do
	echo
	echo $f
	time $EXE $f 2>/dev/null
	echo
done
