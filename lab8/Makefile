
lab8b.bin:	lab8final.s
		nasm lab8final.s -o lab8.bin -l lab8.lst

lab8final.s:	clib.s isr.s yaks.s yakc.s lab8app.s
		cat clib.s simptris.s isr.s yaks.s yakc.s lab8app.s > lab8final.s

yakc.s:		yakc.c yakk.h yaku.h clib.h
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

lab8app.s:	lab8app.c yakk.h yaku.h clib.h simptris.h simdefs.h
		cpp lab8app.c lab8app.i
		c86 -g lab8app.i lab8app.s			

clean:
		rm lab8.bin lab8.lst lab8final.s yakc.s lab8app.s yakc.i lab8app.i

.PHONY: export
export:
	export PATH=$$PATH:/ee2/ee425/bin

