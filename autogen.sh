#! /bin/sh
echo "Generating configuration files for Gaim-Netsoul, please wait...."
echo;

echo "Running libtoolize, please ignore non-fatal messages...."
echo n | libtoolize --copy --force || exit;
echo;

aclocal || exit;
autoheader || exit;
automake --add-missing --copy
autoconf || exit;
automake || exit;

NS_DEFAULT_PATH=/usr
if [ -e ./configure ];then
    echo "Please enter the installation path of gaim-netsoul : default /usr/lib/gaim"
    read $NS_PATH
    if [ "$NS_PATH" == "" ];then
	NS_PATH=$NS_DEFAULT_PATH
    fi
    echo "Gaim-Netsoul will be installed in : $NS_PATH"
    ./configure --prefix=$NS_PATH
else
    echo "Error in configure file generation."
fi