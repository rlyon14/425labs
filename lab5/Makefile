
lab5b.bin:	lab5final.s
		nasm lab5final.s -o lab5.bin -l lab5.lst

lab5final.s:	clib.s isr.s yaks.s yakc.s lab5app.s
		cat clib.s isr.s yaks.s yakc.s lab5app.s > lab5final.s

yakc.s:		yakc.c yakk.h yaku.h clib.h
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

lab5app.s:	lab5app.c yakk.h yaku.h clib.h
		cpp lab5app.c lab5app.i
		c86 -g lab5app.i lab5app.s			

clean:
		rm lab5.bin lab5.lst lab5final.s yakc.s lab5app.s yakc.i lab5app.i

.PHONY: export
export:
	export PATH=$$PATH:/ee2/ee425/bin

