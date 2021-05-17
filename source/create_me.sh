#! /bin/bash
# You want to source the setup script before you run this
if [[ -z "$OFFLINE_MAIN" ]]
then
 echo "please source the setup script"
 exit 1
fi
# create all files (including autogen.sh, configure.ac and Makefile.am)
CreateG4Subsystem.pl myZDC --all
local_installdir=`pwd`/install

echo
echo "now create a build subdir, go to it and run autogen.sh and make"
echo "the installation directory (--prefix) will be created if it does not exist"
echo
echo "  mkdir build"
echo "  cd build"
echo "  ../autogen.sh --prefix=$local_installdir"
echo "  make install"
echo
echo "before testing, do not forget to source the setup_local.csh"
echo
echo "  source $OPT_SPHENIX/bin/setup_local.csh $local_installdir"
echo
unset local_installdir
