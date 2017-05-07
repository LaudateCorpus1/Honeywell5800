This is a work in progress.

It currently receives and decodes. It does not yet send the data, but it 
will execute the command line specified in config.h.

You will have to install the rtlsdr library and an appropriate compiler to build the Raspberry PI software.

Note that data/debug output is controlled by compile flags. See build.sh.

On Rasbian:

	sudo apt-get install rtl-sdr librtlsdr-dev
	sh build.sh
