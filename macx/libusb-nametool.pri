LIBUSB0_DIR = $$system("pkg-config --variable libdir libusb")
LIBUSB1_DIR = $$system("pkg-config --variable libdir libusb-1.0")

LIBUSB0_FILE = libusb-0.1.4.dylib
LIBUSB1_FILE = libusb-1.0.dylib

LIBUSB0_FILEPATH = $$LIBUSB0_DIR/$$LIBUSB0_FILE
LIBUSB1_FILEPATH = $$LIBUSB0_DIR/$$LIBUSB1_FILE

LIBUSB0_INSTALL_NAME_TOOL = install_name_tool -change $$LIBUSB0_FILEPATH \
				@executable_path/../Frameworks/$$LIBUSB0_FILE
LIBUSB1_INSTALL_NAME_TOOL = install_name_tool -change $$LIBUSB1_FILEPATH \
				@executable_path/../Frameworks/$$LIBUSB1_FILE

contains(PKGCONFIG, libusb) {
	isEmpty(nametool.commands) {
	} else {
		nametool.commands += "&&"
	}

	nametool.commands += $$LIBUSB0_INSTALL_NAME_TOOL $$OUTFILE && \
			     $$LIBUSB1_INSTALL_NAME_TOOL $$OUTFILE
}

LIBUSB.path     = $$INSTALLROOT/$$LIBSDIR
LIBUSB.files    += $$LIBUSB0_FILEPATH
LIBUSB.files    += $$LIBUSB1_FILEPATH
