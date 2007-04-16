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

if [ ! `gaim --version` == "Gaim 2.0.0beta6" ]; then
    echo "Please install Gaim 2.0.0beta6"
    exit 1
fi


if [ -e ./configure ];then
    echo "Configuration of the target path"
    if [ -e /usr/lib/gaim/gaimrc.so ]; then
	NS_PATH=$NS_DEFAULT_PATH
    else
	if [ -e /usr/local/lib/gaim/gaimrc.so ]; then
	    NS_PATH=$NS_DEFAULT_PATH/local/
	else
	    echo "Gaim lib folder not found ( /usr/lib and /usr/local/lib tested) "
	    echo "Please enter the path where gaim's lib are"
	    read $NS_PATH
	fi
    fi
    echo "Gaim-Netsoul will be installed in : $NS_PATH"
    ./configure --prefix=$NS_PATH
else
    echo "Error in configure file generation."
fi