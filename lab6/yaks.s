YKEnterMutex:
	cli
	ret
YKExitMutex:
	sti
	ret
YKsaveContext:
	pushf
	push	bx
	add	sp, 2
	mov	bx, sp
	and	word[bx], 0xFDFF
	sub	sp, 2
	pop	bx
	push	cs
	push	word[bp+2]
	push	word[bp]
	push	si
	push	di
	push	ds
	push	es
	push	dx
	push	cx
	push	bx
	push	ax
	mov	bx, [YKsave]
	mov	[bx], sp
	jmp	YKrestorecontext
YKDispatcher:
	push	bp
	mov	bp, sp
	push	ax
	mov	ax, [bp+4]
	cmp	ax, 1
	pop	ax
	je	YKsaveContext
YKrestorecontext:
	mov	sp, [YKrestore]
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
YKsaveSP:
	push 	bp
	mov 	bp, sp
	add	sp, 8
	mov 	bx, [YKsave]
	mov 	[bx], sp
	sub	sp, 8
	mov 	sp, bp
	pop	bp
	ret
