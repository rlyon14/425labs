
lab7b.bin:	lab7final.s
		nasm lab7final.s -o lab7.bin -l lab7.lst

lab7final.s:	clib.s isr.s yaks.s yakc.s lab7app.s
		cat clib.s isr.s yaks.s yakc.s lab7app.s > lab7final.s

yakc.s:		yakc.c yakk.h yaku.h clib.h
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

lab7app.s:	lab7app.c yakk.h yaku.h clib.h
		cpp lab7app.c lab7app.i
		c86 -g lab7app.i lab7app.s			

clean:
		rm lab7.bin lab7.lst lab7final.s yakc.s lab7app.s yakc.i lab7app.i

.PHONY: export
export:
	export PATH=$$PATH:/ee2/ee425/bin

