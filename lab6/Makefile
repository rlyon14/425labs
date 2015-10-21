
lab6b.bin:	lab6final.s
		nasm lab6final.s -o lab6.bin -l lab6.lst

lab6final.s:	clib.s isr.s yaks.s yakc.s lab6app.s
		cat clib.s isr.s yaks.s yakc.s lab6app.s > lab6final.s

yakc.s:		yakc.c yakk.h yaku.h clib.h
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

lab6app.s:	lab6app.c yakk.h yaku.h clib.h
		cpp lab6app.c lab6app.i
		c86 -g lab6app.i lab6app.s			

clean:
		rm lab6.bin lab6.lst lab6final.s yakc.s lab6app.s yakc.i lab6app.i

.PHONY: export
export:
	export PATH=$$PATH:/ee2/ee425/bin

