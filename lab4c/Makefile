
lab4b.bin:	lab4final.s
		nasm lab4final.s -o lab4.bin -l lab4.lst

lab4final.s:	clib.s isr.s yaks.s yakc.s lab4app.s
		cat clib.s isr.s yaks.s yakc.s lab4app.s > lab4final.s

yakc.s:		yakc.c yakk.h yaku.h clib.h
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

lab4app.s:	lab4app.c yakk.h yaku.h clib.h
		cpp lab4app.c lab4app.i
		c86 -g lab4app.i lab4app.s			

clean:
		rm lab4.bin lab4.lst lab4final.s yakc.s lab4app.s yakc.i lab4app.i

.PHONY: export
export:
	export PATH=$$PATH:/ee2/ee425/bin

