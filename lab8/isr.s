isr_reset:
	mov	ax, 0x0
	push	ax
	call	exit
	iret
isr_keypress:
	push	bp
	push	si
	push	di
	push	ds
	push	es
	push	dx
	push	cx
	push	bx
	push	ax
	call	YKEnterISR
	sti
	call	YKkeypress
	cli
	mov	al, 0x20
	out	0x20, al
	call	YKExitISR	
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp	
	iret
isr_tick:
	push	bp
	push	si	
	push	di
	push	ds
	push	es
	push	dx
	push	cx
	push	bx
	push	ax
	call	YKEnterISR
	call	YKTickHandler
	cli
	mov	al, 0x20
	out	0x20, al
	call	YKExitISR	
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp	
	iret
isr_simptris_lineclear:
	inc	word[score]
	push	ax
	mov	al, 0x20
	out	0x20, al
	pop	ax
	iret	
isr_simptris_touchdown:
	push	ax
	mov	al, 0x20
	out	0x20, al
	pop	ax
	iret	
isr_simptris_recievedCmd:
	push	bp
	push	si
	push	di
	push	ds
	push	es
	push	dx
	push	cx
	push	bx
	push	ax
	call	YKEnterISR
	sti
	call	SMrecievedCmdHdlr
	cli
	mov	al, 0x20
	out	0x20, al
	call	YKExitISR	
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp	
	iret
isr_simptris_newpeice:
	push	bp
	push	si	
	push	di
	push	ds
	push	es
	push	dx
	push	cx
	push	bx
	push	ax
	call	YKEnterISR
	;sti don't nest newpeice interrupts
	call	SMnewpieceHdlr
	cli
	mov	al, 0x20
	out	0x20, al
	call	YKExitISR	
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp	
	iret	
isr_simptris_gameover:
	push	bp
	push	si
	push	di
	push	ds
	push	es
	push	dx
	push	cx
	push	bx
	push	ax
	call	YKEnterISR
	sti
	call	SMgameOverHdlr
	cli
	mov	al, 0x20
	out	0x20, al
	call	YKExitISR	
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp	
	iret	

