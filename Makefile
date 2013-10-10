all: p2l97.exe pmp2l97.exe

p2l97.exe: version.h lm78.h p2l97.h p2l97.c p2l97.def
	gcc -Zomf -Zsys -O2 -s p2l97.c p2l97.def

pmp2l97.exe: version.h lm78.h p2l97.h pmp2l97.h pmp2l97.c pmp2l97.res pmp2l97.def
	gcc -Zomf -Zsys -O2 -s pmp2l97.c pmp2l97.res pmp2l97.def

pmp2l97.res: pmp2l97.rc pmp2l97.h pmp2l97.ico
	rc -r -i f:\emx\include pmp2l97.rc
