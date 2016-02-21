all: 
	make -C JRift
	make -C JRiftLibrary

clean:
	make -C JRiftLibrary clean
	make -C JRift clean
