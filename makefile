c:
	./waf configure && ./waf build
l: c
	deploypebble.sh load ~/Pebble/phosphor/build/phosphor.pbw
d: c
	deploypebble.sh reinstall  ~/Pebble/phosphor/build/phosphor.pbw 
