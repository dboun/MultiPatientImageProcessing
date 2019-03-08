#!/bin/bash
#---------------------------------------------------------------------------------------------------
# Set these variables accordingly (Don't use a '/' at the end of the path)
# Make sure qmake exists (try "qmake -v")
# Use this after building MPIP (with "make")
# RUN THIS WITH sudo !
#
# In order the variables to set are:
# - Downloads dir (i.e. /home/username/Downloads)
# - Build directory of MPIP (i.e. /home/username/workspace/MPIP/build)
# - Where to create the standalone (choose path outside of filesystem's root)
# - Qt Directory (i.e. /home/username/Qt/5.11.1/gcc_64)
# - MITK superbuild directory (i.e. /opt/MITK/MITK-superbuild)
#---------------------------------------------------------------------------------------------------
export YOUR_DOWNLOADS_DIR=
export MPIP_BUILD_DIRECTORY=
export WHERE_TO_CREATE_STANDALONE=
export QT_DIRECTORY=
export MITK_SUPERBUILD_DIRECTORY=

#-----------------------------------
#Don't change anything below (or do)
#-----------------------------------

# Check if running as root
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

# Create the standalone directories
WHERE_TO_CREATE_STANDALONE_RESOURCES=$WHERE_TO_CREATE_STANDALONE/MPIP/linux
mkdir -p $WHERE_TO_CREATE_STANDALONE
mkdir -p $WHERE_TO_CREATE_STANDALONE_RESOURCES
mkdir    $WHERE_TO_CREATE_STANDALONE_RESOURCES/lib

echo "Copying everything to standalone destination"
cp -a $MPIP_BUILD_DIRECTORY/. $WHERE_TO_CREATE_STANDALONE_RESOURCES

# This is apparently kind of needed to be installed
echo "Installing libgtk2.0-dev dependency"
sudo apt-get update
sudo apt-get -y install libgtk2.0-dev

# Linux deploy needs some files that qt doesn't have by default
echo "Building qtstyleplugins"
cd $YOUR_DOWNLOADS_DIR
git clone http://code.qt.io/qt/qtstyleplugins.git
cd qtstyleplugins
qmake
make -j$(nproc)

# Copy things to qt plugins
echo "Copying qtstyleplugins libs to Qt"
cp plugins/styles/libbb10styleplugin.so $QT_DIRECTORY/plugins/styles
cp plugins/styles/libqcleanlooksstyle.so $QT_DIRECTORY/plugins/styles
cp plugins/styles/libqgtk2style.so $QT_DIRECTORY/plugins/styles
cp plugins/styles/libqmotifstyle.so $QT_DIRECTORY/plugins/styles
cp plugins/styles/libqplastiquestyle.so $QT_DIRECTORY/plugins/styles

cp plugins/styles/libbb10styleplugin.so.debug $QT_DIRECTORY/plugins/styles/styles
cp plugins/styles/libqcleanlooksstyle.so.debug $QT_DIRECTORY/plugins/styles/styles
cp plugins/styles/libqgtk2style.so.debug $QT_DIRECTORY/plugins/styles/styles
cp plugins/styles/libqmotifstyle.so.debug $QT_DIRECTORY/plugins/styles/styles
cp plugins/styles/libqplastiquestyle.so.debug $QT_DIRECTORY/plugins/styles/styles

cp plugins/platformthemes/libqgtk2.so $QT_DIRECTORY/plugins/platformthemes
cp plugins/platformthemes/libqgtk2.so.debug $QT_DIRECTORY/plugins/platformthemes

# Remove downloaded plugin repo
echo "Removing qtstyleplugins"
cd $YOUR_DOWNLOADS_DIR
rm -rf qtstyleplugins

# linuxdeployqt
echo "Downloading and running linuxdeployqt"
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
export LD_LIBRARY_PATH=QT_DIRECTORY/lib
./linuxdeployqt-continuous-x86_64.AppImage $WHERE_TO_CREATE_STANDALONE_RESOURCES/MPIP -unsupported-allow-new-glibc
rm $WHERE_TO_CREATE_STANDALONE_RESOURCES/AppRun
rm linuxdeployqt-continuous-x86_64.AppImage

# Copy some opengl things
echo "Copying libGL"
cp /usr/lib/x86_64-linux-gnu/libGL.so $WHERE_TO_CREATE_STANDALONE_RESOURCES
cp /usr/lib/x86_64-linux-gnu/libGL.so.1 $WHERE_TO_CREATE_STANDALONE_RESOURCES
cp /usr/lib/x86_64-linux-gnu/libGL.so.1.0.0 $WHERE_TO_CREATE_STANDALONE_RESOURCES
cp /usr/lib/x86_64-linux-gnu/libGL.so $WHERE_TO_CREATE_STANDALONE_RESOURCES/lib
cp /usr/lib/x86_64-linux-gnu/libGL.so.1 $WHERE_TO_CREATE_STANDALONE_RESOURCES/lib
cp /usr/lib/x86_64-linux-gnu/libGL.so.1.0.0 $WHERE_TO_CREATE_STANDALONE_RESOURCES/lib

# Copy literally all MITK libs
echo "Copying MITK libraries"
cp -a $MITK_SUPERBUILD_DIRECTORY/ep/lib/. $WHERE_TO_CREATE_STANDALONE_RESOURCES/lib
cp -a $MITK_SUPERBUILD_DIRECTORY/MITK-build/lib/. $WHERE_TO_CREATE_STANDALONE_RESOURCES/lib

# Create run script and README
echo "Creating run script"
echo "chmod u+x linux/MPIP && export MESA_GL_VERSION_OVERRIDE=3.3 && export LD_LIBRARY_PATH=linux/lib:linux && linux/MPIP" >> $WHERE_TO_CREATE_STANDALONE_RESOURCES/../RunMPIP.sh
echo "To run MPIP:" >> $WHERE_TO_CREATE_STANDALONE_RESOURCES/../HowToRun.txt
echo "chmod u+x RunMPIP.sh" > $WHERE_TO_CREATE_STANDALONE_RESOURCES/../HowToRun.txt
echo "./RunMPIP.sh" > $WHERE_TO_CREATE_STANDALONE_RESOURCES/../HowToRun.txt

# Make the files owned by user (and not root)
sudo chown -R $SUDO_USER:$SUDO_USER $WHERE_TO_CREATE_STANDALONE

# Finishing up
echo "The standalone is now in your build directory (all the MPIP directory)"
echo "To run see instructions in MPIP/HowToRun.txt"
echo "To distribute zip the MPIP directory."
