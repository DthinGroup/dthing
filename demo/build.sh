################ Please update below enviroment varable #####################################
export ROOTDIR=/storage/auto
export DVMLIB_DIR=$ROOTDIR/dvmlib
export VMTOOL=$ROOTDIR/tools
export WTK_ROOT=$ROOTDIR/tools/WTK2.5.2

export PATH=$PATH:$ROOTDIR/sdk/build-tools/android-4.3
export PATH=$PATH:$WTK_ROOT/bin
export MIDPAPI=$DVMLIB_DIR/df.jar:$DVMLIB_DIR/df_tp.jar
export DVMLIB=$DVMLIB_DIR/df.jar:$DVMLIB_DIR/df_tp.jar

export DEMO_NAME=$1
export RELEASE_DIR=release

cd $DEMO_NAME
mkdir -p $RELEASE_DIR

cd $RELEASE_DIR

mkdir -p src
mkdir -p tmpclasses
mkdir -p classes
mkdir -p bin

cp -rf ../*.java src/
cp -rf ../MANIFEST.MF classes/

javac -d tmpclasses -bootclasspath $MIDPAPI -classpath $DVMLIB src/*.java
#preverify -classpath $MIDPAPI:$DVMLIB tmpclasses -d . tmpclasses
cp tmpclasses/*.class .
jar cvfm $DEMO_NAME.jar classes/MANIFEST.MF *.class
dx --dex --output=classes.dex $DEMO_NAME.jar
jar cvfm $DEMO_NAME.dex.jar classes/MANIFEST.MF
aapt add $DEMO_NAME.dex.jar classes.dex

cp -rf *.jar ../

cd ..
rm -rf $RELEASE_DIR

cd ..
