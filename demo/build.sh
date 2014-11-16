################ Please update below enviroment varable #####################################
export DVMLIB_DIR=/home/cmccclient/dthing/dvmlib
export VMTOOL=/home/cmccclient/dthing/tools
export WTK_ROOT=/home/cmccclient/dthing/tools/WTK2.5.2

export PATH=$PATH:/home/cmccclient/dthing/sdk/build-tools/android-4.3
export PATH=$PATH:$WTK_ROOT/bin
export MIDPAPI=$DVMLIB_DIR/df.jar
export DVMLIB=$DVMLIB_DIR/df.jar

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
preverify -classpath $MIDPAPI:$DVMLIB tmpclasses -d . tmpclasses
jar cvfm $DEMO_NAME.jar classes/MANIFEST.MF *.class
dx --dex --output=classes.dex $DEMO_NAME.jar
jar cvfm $DEMO_NAME.dex.jar classes/MANIFEST.MF
aapt add $DEMO_NAME.dex.jar classes.dex

cp -rf *.jar ../

cd ..
rm -rf $RELEASE_DIR

cd ..
