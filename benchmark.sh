if [ -z "$1" ]; then
	EXE="build/Release/NuXJScript -s -t"
else
	EXE=$1
fi

for f in benchmarks/*.js; do
	echo
	echo $f
	time $EXE $f
	echo
done
