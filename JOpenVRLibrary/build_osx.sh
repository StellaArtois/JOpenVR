SCRIPTDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
echo $SCRIPTDIR
CURDIR=.
echo $CURDIR

rm -rf "$SCRIPTDIR/natives/osx/64/libJOpenVRLibrary64.jnilib"
rm -rf "$SCRIPTDIR/natives/osx/64/libJOpenVRLibrary64.dylib"
rm -rf "$SCRIPTDIR/natives/osx/64/libopenvr_api.dylib"
rm -rf "$SCRIPTDIR/osx64"
mkdir "$SCRIPTDIR/osx64"
cd "$SCRIPTDIR/osx64"
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 "$SCRIPTDIR" -Wno-dev
make
cp ./libJOpenVRLibrary64.dylib "$SCRIPTDIR/natives/osx/64/libJOpenVRLibrary64.dylib"
cd "$CURDIR"

rm -rf "$SCRIPTDIR/natives/osx/32/libJOpenVRLibrary.jnilib"
rm -rf "$SCRIPTDIR/natives/osx/32/libJOpenVRLibrary.dylib"
rm -rf "$SCRIPTDIR/natives/osx/32/libopenvr_api.dylib"
rm -rf "$SCRIPTDIR/osx32"
mkdir "$SCRIPTDIR/osx32"
cd "$SCRIPTDIR/osx32"
cmake -DCMAKE_OSX_ARCHITECTURES=i386 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 "$SCRIPTDIR" -Wno-dev
make
cp ./libJOpenVRLibrary.dylib "$SCRIPTDIR/natives/osx/32/libJOpenVRLibrary.dylib"
cd "$CURDIR"



