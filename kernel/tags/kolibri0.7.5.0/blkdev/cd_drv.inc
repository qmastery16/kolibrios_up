;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 758 $


;**********************************************************
;  ���������������� ������ � ����������� �D (ATAPI)
;**********************************************************
; ����� ����� ��������� ������ ������� �������� �����������
; ���������, ��������� � ���������� Mario79

; ������������ ���������� ���������� �������� ������
MaxRetr equ 10
; ���������� ����� �������� ���������� � ������ �������
; (� �����)
BSYWaitTime equ 1000  ;2
NoTickWaitTime equ 0xfffff

;*************************************************
;*      ������ ������ ������� �������-�����      *
;* ����������� ������ ������������, ����������   *
;* ��������� � ����������� ����������            *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������;           *
;* CDSectorAddress - ����� ������������ �������. *
;* ������ ����������� � ������ CDDataBuf.        *
;*************************************************
ReadCD:
	pusha
; ������ ������ �������
	mov	  [CDBlockSize],2048 ;2352
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������������ �������� ������� ��� ����������
; ������� ������
	; ������ ��� ������� Read CD
	mov	  [PacketCommand],byte 0x28  ;0xBE
	; ������ ����� �������
	mov   AX,word [CDSectorAddress+2]
	xchg  AL,AH
	mov   word [PacketCommand+2],AX
	mov   AX,word [CDSectorAddress]
	xchg  AL,AH
	mov   word [PacketCommand+4],AX
;        mov   eax,[CDSectorAddress]
;        mov   [PacketCommand+2],eax
	; ������ ���������� ����������� ��������
	mov	  [PacketCommand+8],byte 1
	; ������ ���������� ������ � ������ ������
;        mov     [PacketCommand+9],byte 0xF8
; ������ �������
	call  SendPacketDatCommand
	popa
	ret

;********************************************
;*        ������ ������� � ���������        *
;* ������������ ���������� ������ ��� ����� *
;********************************************
ReadCDWRetr:
;-----------------------------------------------------------
; input  : eax = block to read
;          ebx = destination
;-----------------------------------------------------------
    pushad
    mov   eax,[CDSectorAddress]
    mov   ebx,[CDDataBuf_pointer]
    call  cd_calculate_cache
    add   esi,8
    mov   edi,1
.hdreadcache:
;    cmp   dword [esi+4],0       ; empty
;    je    .nohdcache
    cmp   [esi],eax		; correct sector
    je	  .yeshdcache
.nohdcache:
    add   esi,8
    inc   edi
    dec   ecx
    jnz   .hdreadcache
    call  find_empty_slot_CD_cache	 ; ret in edi

    push  edi
    push  eax
    call  cd_calculate_cache_2
    shl   edi,11
    add   edi,eax
    mov   [CDDataBuf_pointer],edi
    pop   eax
    pop   edi

    call  ReadCDWRetr_1
    cmp   [DevErrorCode],0
    jne   .exit

    mov   [CDDataBuf_pointer],ebx
    call  cd_calculate_cache_1
    lea   esi,[edi*8+esi]
    mov   [esi],eax		; sector number
;    mov   dword [esi+4],1       ; hd read - mark as same as in hd
.yeshdcache:
    mov   esi,edi
    shl   esi,11    ;9
    push  eax
    call  cd_calculate_cache_2
    add   esi,eax
    pop   eax
    mov   edi,ebx   ;[CDDataBuf_pointer]
    mov   ecx,512   ;/4
    cld
    rep   movsd 		; move data
.exit:
    popad
    ret

ReadCDWRetr_1:
	pushad

; ����, ���� ������� �� ��������� ������� ��� ��
; ��������� ���������� �������
	mov	  ECX,MaxRetr
@@NextRetr:
; ������ �������
	call  ReadCD
	cmp	  [DevErrorCode],0
	je	  @@End_4

	or    ecx,ecx		;{SPraid.simba} (for cd load)
	jz    @@End_4
	dec   ecx

	cmp   [timer_ticks_enable],0
	jne   @f
	mov   eax,NoTickWaitTime
.wait:
	dec   eax
	cmp   eax,0
	je    @@NextRetr
	jmp	  .wait
@@:
; �������� �� 2,5 �������
;       mov     EAX,[timer_ticks]
;       add     EAX,50  ;250
;@@Wait:
;       call    change_task
;       cmp     EAX,[timer_ticks]
;       ja      @@Wait
	loop  @@NextRetr
@@End_4:
	popad
	ret


;   ������������� ���������, �������������� ����������
;             �������� ������ � ������ PIO

; ����������� ���������� ����� �������� �������
; ���������� �� �������� ������� (� �����)
MaxCDWaitTime equ 1000 ;200 ;10 ������

; ������� ������ ��� ������������ �������� �������
PacketCommand:	 rb 12	;DB 12 DUP (?)
; ������� ������ ��� ������ ������ �� ���������
;CDDataBuf       DB 4096 DUP (0)
; ������ ������������ ����� ������ � ������
CDBlockSize	DW ?
; ����� ������������ ������� ������
CDSectorAddress: DD ?
; ����� ������ ��������� �������� � ������
TickCounter_1 DD 0
; ����� ������ �������� ���������� ����������
WURStartTime DD 0
; ��������� ������ ��� ����������
CDDataBuf_pointer dd 0

;****************************************************
;*    ������� ���������� ATAPI �������� �������,    *
;* ����������������� �������� ������ ������� ������ *
;*     �������� 2048 ���� �� ���������� � �����     *
;* ������� ��������� ���������� ����� ����������    *
;* ���������:                                       *
;* ChannelNumber - ����� ������;                    *
;* DiskNumber - ����� ����� �� ������;              *
;* PacketCommand - 12-������� ��������� �����;      *
;* CDBlockSize - ������ ������������ ����� ������.  *
;****************************************************
SendPacketDatCommand:
	pushad
	mov   [DevErrorCode],0
; ������ ����� CHS
	mov	[ATAAddressMode],0
; ������� ATA-������� �������� �������� �������
	mov	[ATAFeatures],0
	mov	[ATASectorCount],0
	mov	[ATASectorNumber],0
	; ��������� ������ ������������� �����
	mov	AX,[CDBlockSize]
	mov	[ATACylinder],AX
	mov	[ATAHead],0
	mov	[ATACommand],0A0h
	call	SendCommandToHDD_1
	cmp	[DevErrorCode],0 ;��������� ��� ������
	jne	@@End_8    ;���������, �������� ��� ������

; �������� ���������� ��������� � ������
; �������� �������
	mov	DX,[ATABasePortAddr]
	add	DX,7	 ;���� 1�7h
	mov	ecx,NoTickWaitTime
@@WaitDevice0:
	cmp	[timer_ticks_enable],0
	jne	@f
	dec	ecx
	cmp	ecx,0
	je	@@Err1_1
	jmp	.test
@@:
	call	change_task
	; ��������� ����� ���������� �������
	mov	EAX,[timer_ticks]
	sub	EAX,[TickCounter_1]
	cmp	EAX,BSYWaitTime
	ja	@@Err1_1   ;������ ����-����
	; ��������� ����������
.test:
	in	AL,DX
	test	AL,80h	 ;��������� ������� BSY
	jnz	@@WaitDevice0
	test	AL,08h	 ;��������� ������� DRQ
	jz	@@WaitDevice0
	test	AL,1	 ;��������� ������� ERR
	jnz	@@Err6
; ������� �������� �������
	cli
	mov	DX,[ATABasePortAddr]
	mov	AX,[PacketCommand]
	out	DX,AX
	mov	AX,[PacketCommand+2]
	out	DX,AX
	mov	AX,[PacketCommand+4]
	out	DX,AX
	mov	AX,[PacketCommand+6]
	out	DX,AX
	mov	AX,[PacketCommand+8]
	out	DX,AX
	mov	AX,[PacketCommand+10]
	out	DX,AX
	sti
; �������� ���������� ������
	mov	DX,[ATABasePortAddr]
	add	DX,7   ;���� 1�7h
	mov	ecx,NoTickWaitTime
@@WaitDevice1:
	cmp	[timer_ticks_enable],0
	jne	@f
	dec	ecx
	cmp	ecx,0
	je	@@Err1_1
	jmp	.test_1
@@:
	call	change_task
	; ��������� ����� ���������� �������
	mov	EAX,[timer_ticks]
	sub	EAX,[TickCounter_1]
	cmp	EAX,MaxCDWaitTime
	ja	@@Err1_1   ;������ ����-����
	; ��������� ����������
.test_1:
	in	AL,DX
	test	AL,80h	 ;��������� ������� BSY
	jnz	@@WaitDevice1
	test	AL,08h	 ;��������� ������� DRQ
	jz	@@WaitDevice1
	test	AL,1	 ;��������� ������� ERR
	jnz	@@Err6_temp
; ������� ���� ������ �� �����������
	mov	EDI,[CDDataBuf_pointer] ;0x7000  ;CDDataBuf
	; ��������� ����� �������� ������ �����������
	mov	DX,[ATABasePortAddr] ;���� 1x0h
	; ��������� � ������� ������ ����� � ������
	xor	ecx,ecx
	mov	CX,[CDBlockSize]
	; ��������� ������ ����� � 16-��������� ������
	shr	CX,1 ;��������� ������ ����� �� 2
	; ������� ���� ������
	cli
	cld
	rep	insw
	sti
	; �������� ���������� ������ ������
	jmp @@End_8

; �������� ��� ������
@@Err1_1:
	mov	[DevErrorCode],1
	jmp @@End_8
@@Err6_temp:
	mov	[DevErrorCode],7
	jmp @@End_8
@@Err6:
	mov	[DevErrorCode],6
@@End_8:
	popad
	ret



;***********************************************
;*  ������� ���������� ATAPI �������� �������, *
;*     �� ����������������� �������� ������    *
;* ������� ��������� ���������� �����          *
;* ���������� ���������:                       *
;* ChannelNumber - ����� ������;               *
;* DiskNumber - ����� ����� �� ������;         *
;* PacketCommand - 12-������� ��������� �����. *
;***********************************************
SendPacketNoDatCommand:
	pushad
    mov   [DevErrorCode],0
; ������ ����� CHS
	mov	[ATAAddressMode],0
; ������� ATA-������� �������� �������� �������
	mov	[ATAFeatures],0
	mov	[ATASectorCount],0
	mov	[ATASectorNumber],0
	mov	[ATACylinder],0
	mov	[ATAHead],0
	mov	[ATACommand],0A0h
	call	SendCommandToHDD_1
	cmp	[DevErrorCode],0 ;��������� ��� ������
	jne	@@End_9  ;���������, �������� ��� ������
; �������� ���������� ��������� � ������
; �������� �������
	mov	DX,[ATABasePortAddr]
	add	DX,7   ;���� 1�7h
@@WaitDevice0_1:
	call	change_task
	; ��������� ����� ��������
	mov	EAX,[timer_ticks]
	sub	EAX,[TickCounter_1]
	cmp	EAX,BSYWaitTime
	ja	@@Err1_3   ;������ ����-����
	; ��������� ����������
	in	AL,DX
	test	AL,80h	 ;��������� ������� BSY
	jnz	@@WaitDevice0_1
	test	AL,1	 ;��������� ������� ERR
	jnz	@@Err6_1
	test	AL,08h	 ;��������� ������� DRQ
	jz	@@WaitDevice0_1
; ������� �������� �������
;        cli
	mov	DX,[ATABasePortAddr]
	mov	AX,word [PacketCommand]
	out	DX,AX
	mov	AX,word [PacketCommand+2]
	out	DX,AX
	mov	AX,word [PacketCommand+4]
	out	DX,AX
	mov	AX,word [PacketCommand+6]
	out	DX,AX
	mov	AX,word [PacketCommand+8]
	out	DX,AX
	mov	AX,word [PacketCommand+10]
	out	DX,AX
;        sti
    cmp [ignore_CD_eject_wait],1
    je	@@End_9
; �������� ������������� ������ �������
	mov	DX,[ATABasePortAddr]
	add	DX,7   ;���� 1�7h
@@WaitDevice1_1:
	call	change_task
	; ��������� ����� ���������� �������
	mov	EAX,[timer_ticks]
	sub	EAX,[TickCounter_1]
	cmp	EAX,MaxCDWaitTime
	ja	@@Err1_3   ;������ ����-����
	; ������� ������������ ����������
	in	AL,DX
	test	AL,80h	 ;��������� ������� BSY
	jnz	@@WaitDevice1_1
	test	AL,1	 ;��������� ������� ERR
	jnz	@@Err6_1
	test	AL,40h	 ;��������� ������� DRDY
	jz	@@WaitDevice1_1
	jmp @@End_9

; �������� ��� ������
@@Err1_3:
	mov	[DevErrorCode],1
	jmp @@End_9
@@Err6_1:
	mov	[DevErrorCode],6
@@End_9:
	popad
	ret

;****************************************************
;*          ������� ������� ��������� �����         *
;* ������� ��������� ���������� ����� ����������    *
;* ����������:                                      *
;* ChannelNumber - ����� ������ (1 ��� 2);          *
;* DiskNumber - ����� ����� (0 ��� 1);              *
;* ATAFeatures - "�����������";                     *
;* ATASectorCount - ���������� ��������;            *
;* ATASectorNumber - ����� ���������� �������;      *
;* ATACylinder - ����� ���������� ��������;         *
;* ATAHead - ����� ��������� �������;               *
;* ATAAddressMode - ����� ��������� (0-CHS, 1-LBA); *
;* ATACommand - ��� �������.                        *
;* ����� ��������� ���������� �������:              *
;* � ATABasePortAddr - ������� ����� HDD;           *
;* � DevErrorCode - ����.                           *
;* ��� ������������� ������ � DevErrorCode �����    *
;* ��������� ��� ������.                            *
;****************************************************
SendCommandToHDD_1:
	pushad
    mov   [DevErrorCode],0
; ��������� �������� ���� ������
	cmp	[ATAAddressMode],1
	ja	@@Err2_4
; ��������� ������������ ������ ������
	mov	BX,[ChannelNumber]
	cmp	BX,1
	jb	@@Err3_4
	cmp	BX,2
	ja	@@Err3_4
; ���������� ������� �����
	dec	BX
	shl	BX,1
	movzx	ebx,bx
	mov	AX,[ebx+StandardATABases]
	mov	[ATABasePortAddr],AX
; �������� ���������� HDD � ������ �������
	; ������� ������ ����
	mov	DX,[ATABasePortAddr]
	add	DX,6	;����� �������� �������
	mov	AL,[DiskNumber]
	cmp	AL,1	;��������� ������ �����
	ja	@@Err4_4
	shl	AL,4
	or	AL,10100000b
	out	DX,AL
	; �������, ���� ���� �� ����� �����
	inc	DX
	mov	eax,[timer_ticks]
	mov	[TickCounter_1],eax
	mov	ecx,NoTickWaitTime
@@WaitHDReady_2:
	cmp    [timer_ticks_enable],0
	jne    @f
	dec    ecx
	cmp    ecx,0
	je     @@Err1_4
	jmp    .test
@@:
	call	change_task
	; ��������� ����� ��������
	mov	eax,[timer_ticks]
	sub	eax,[TickCounter_1]
	cmp	eax,BSYWaitTime ;300    ;������� 3 ���.
	ja	@@Err1_4   ;������ ����-����
	; ��������� ������� ���������
.test:
	in	AL,DX
	; ��������� ��������� ������� BSY
	test	AL,80h
	jnz	@@WaitHDReady_2
	; ��������� ��������� ������� DRQ
	test	AL,08h
	jnz	@@WaitHDReady_2

; ��������� ������� � �������� �����������
	cli
	mov	DX,[ATABasePortAddr]
	inc	DX	;������� "������������"
	mov	AL,[ATAFeatures]
	out	DX,AL
	inc	DX	;������� ��������
	mov	AL,[ATASectorCount]
	out	DX,AL
	inc	DX	;������� ������ �������
	mov	AL,[ATASectorNumber]
	out	DX,AL
	inc	DX	;����� �������� (������� ����)
	mov	AX,[ATACylinder]
	out	DX,AL
	inc	DX	;����� �������� (������� ����)
	mov	AL,AH
	out	DX,AL
	inc	DX	;����� �������/����� �����
	mov	AL,[DiskNumber]
	shl	AL,4
	cmp	[ATAHead],0Fh ;��������� ����� �������
	ja	@@Err5_4
	or	AL,[ATAHead]
	or	AL,10100000b
	mov	AH,[ATAAddressMode]
	shl	AH,6
	or	AL,AH
	out	DX,AL
; ������� �������
	mov	AL,[ATACommand]
	inc	DX	;������� ������
	out	DX,AL
	sti
; �������� ������� ������
	mov	[DevErrorCode],0
	jmp @@End_10
; �������� ��� ������
@@Err1_4:
	mov	[DevErrorCode],1
	jmp @@End_10
@@Err2_4:
	mov	[DevErrorCode],2
	jmp @@End_10
@@Err3_4:
	mov	[DevErrorCode],3
	jmp @@End_10
@@Err4_4:
	mov	[DevErrorCode],4
	jmp @@End_10
@@Err5_4:
	mov	[DevErrorCode],5
; ���������� ������ ���������
@@End_10:
;        sti
	popad
	ret

;*************************************************
;*    �������� ���������� ���������� � ������    *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
WaitUnitReady:
	pusha
; ��������� ����� ������ ��������
	mov	EAX,[timer_ticks]
	mov	[WURStartTime],EAX
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������������ ������� TEST UNIT READY
	mov	[PacketCommand],word 00h
; ���� �������� ���������� ����������
	mov	ecx,NoTickWaitTime
@@SendCommand:
	; ������ ������� �������� ����������
	call	SendPacketNoDatCommand
	cmp	[timer_ticks_enable],0
	jne	@f
	cmp	[DevErrorCode],0
	je	@@End_11
	dec	ecx
	cmp	ecx,0
	je	.Error
	jmp	@@SendCommand
@@:
	call	change_task
	; ��������� ��� ������
	cmp	[DevErrorCode],0
	je	@@End_11
	; ��������� ����� �������� ����������
	mov	EAX,[timer_ticks]
	sub	EAX,[WURStartTime]
	cmp	EAX,MaxCDWaitTime
	jb	@@SendCommand
.Error:
	; ������ ����-����
	mov	[DevErrorCode],1
@@End_11:
	popa
	ret

;*************************************************
;*            ��������� ����� �����              *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
prevent_medium_removal:
	pusha
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������ ��� �������
	mov  [PacketCommand],byte 0x1E
; ������ ��� �������
    mov  [PacketCommand+4],byte 11b
; ������ �������
	call SendPacketNoDatCommand
	mov  eax,ATAPI_IDE0_lock
	add  eax,[cdpos]
	dec  eax
	mov  [eax],byte 1
	popa
	ret

;*************************************************
;*            ��������� ����� �����              *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
allow_medium_removal:
	pusha
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������ ��� �������
	mov  [PacketCommand],byte 0x1E
; ������ ��� �������
    mov  [PacketCommand+4],byte 00b
; ������ �������
	call SendPacketNoDatCommand
	mov  eax,ATAPI_IDE0_lock
	add  eax,[cdpos]
	dec  eax
	mov  [eax],byte 0
	popa
	ret

;*************************************************
;*         ��������� �������� � ��������         *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
LoadMedium:
	pusha
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������������ ������� START/STOP UNIT
	; ������ ��� �������
	mov	[PacketCommand],word 1Bh
	; ������ �������� �������� ��������
	mov	[PacketCommand+4],word 00000011b
; ������ �������
	call	SendPacketNoDatCommand
	popa
	ret

;*************************************************
;*         ������� �������� �� ���������         *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
EjectMedium:
	pusha
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������������ ������� START/STOP UNIT
	; ������ ��� �������
	mov	[PacketCommand],word 1Bh
	; ������ �������� ���������� ��������
	mov	[PacketCommand+4],word 00000010b
; ������ �������
	call	SendPacketNoDatCommand
	popa
	ret

;*************************************************
;* ��������� ������� ������� ������ ����������   *
;*                     �����                     *
;* ������� ��������� ���������� ����� ���������� *
;* ����������:                                   *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
check_ATAPI_device_event:
	pusha
    mov  eax,[timer_ticks]
    sub  eax,[timer_ATAPI_check]
    cmp  eax,100
    jb	 .end_1
    mov  al,[DRIVE_DATA+1]
    and al,11b
    cmp al,10b
    jz	.ide3
.ide2_1:
    mov  al,[DRIVE_DATA+1]
    and al,1100b
    cmp al,1000b
    jz	.ide2
.ide1_1:
    mov  al,[DRIVE_DATA+1]
    and al,110000b
    cmp al,100000b
    jz	.ide1
.ide0_1:
    mov  al,[DRIVE_DATA+1]
    and al,11000000b
    cmp al,10000000b
    jz	.ide0
.end:

    sti
    mov  eax,[timer_ticks]
    mov  [timer_ATAPI_check],eax
.end_1:
	popa
	ret

.ide3:
    cli
    cmp  [ATAPI_IDE3_lock],1
    jne  .ide2_1
    cmp  [IDE_Channel_2],0
    jne  .ide1_1
    cmp  [cd_status],0
    jne  .end
	mov  [IDE_Channel_2],1
    call reserve_ok2
	mov  [ChannelNumber],2
	mov  [DiskNumber],1
	mov	 [cdpos],4
	call GetEvent_StatusNotification
	cmp  [CDDataBuf+4],byte 1
	je   .eject_ide3
	call syscall_cdaudio.free
    jmp  .ide2_1
.eject_ide3:
    call .eject
	call syscall_cdaudio.free
    jmp  .ide2_1

.ide2:
    cli
    cmp  [ATAPI_IDE2_lock],1
    jne  .ide1_1
    cmp  [IDE_Channel_2],0
    jne  .ide1_1
    cmp  [cd_status],0
    jne  .end
	mov  [IDE_Channel_2],1
    call  reserve_ok2
	mov  [ChannelNumber],2
	mov  [DiskNumber],0
	mov	[cdpos],3
	call GetEvent_StatusNotification
	cmp  [CDDataBuf+4],byte 1
	je   .eject_ide2
	call syscall_cdaudio.free
    jmp  .ide1_1
.eject_ide2:
    call .eject
	call syscall_cdaudio.free
    jmp  .ide1_1

.ide1:
    cli
    cmp  [ATAPI_IDE1_lock],1
    jne  .ide0_1
    cmp  [IDE_Channel_1],0
    jne  .end
    cmp  [cd_status],0
    jne  .end
	mov  [IDE_Channel_1],1
    call reserve_ok2
	mov  [ChannelNumber],1
	mov  [DiskNumber],1
	mov	[cdpos],2
	call GetEvent_StatusNotification
	cmp  [CDDataBuf+4],byte 1
	je   .eject_ide1
	call syscall_cdaudio.free
    jmp  .ide0_1
.eject_ide1:
    call .eject
	call syscall_cdaudio.free
    jmp  .ide0_1

.ide0:
    cli
    cmp  [ATAPI_IDE0_lock],1
    jne  .end
    cmp  [IDE_Channel_1],0
    jne  .end
    cmp  [cd_status],0
    jne  .end
	mov  [IDE_Channel_1],1
    call reserve_ok2
	mov  [ChannelNumber],1
	mov  [DiskNumber],0
	mov	[cdpos],1
	call GetEvent_StatusNotification
	cmp  [CDDataBuf+4],byte 1
	je   .eject_ide0
	call syscall_cdaudio.free
    jmp  .end
.eject_ide0:
    call .eject
	call syscall_cdaudio.free
    jmp  .end

.eject:
	call clear_CD_cache
	call allow_medium_removal
	mov  [ignore_CD_eject_wait],1
	call EjectMedium
	mov  [ignore_CD_eject_wait],0
	ret

timer_ATAPI_check dd 0
ATAPI_IDE0_lock db 0
ATAPI_IDE1_lock db 0
ATAPI_IDE2_lock db 0
ATAPI_IDE3_lock db 0
ignore_CD_eject_wait db 0

;*************************************************
;* �������� ��������� � ������� ��� ���������    *
;*                  ����������                   *
;* ������� ��������� ���������� ����� ���������� *
;* ����������:                                   *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
GetEvent_StatusNotification:
	pusha
	mov	[CDDataBuf_pointer],CDDataBuf
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������ ��� �������
	mov	[PacketCommand],byte 4Ah
	mov	[PacketCommand+1],byte 00000001b
; ������ ������ ������ ���������
	mov	[PacketCommand+4],byte 00010000b
; ������ ���������� �������
	mov	[PacketCommand+7],byte 8h
	mov	[PacketCommand+8],byte 0h
; ������ �������
	call	SendPacketDatCommand
	popa
	ret

;*************************************************
; ��������� ���������� �� TOC
;* ������� ��������� ���������� ����� ���������� *
;* ����������:                                   *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
Read_TOC:
	pusha
	mov	[CDDataBuf_pointer],CDDataBuf
; �������� ����� �������� �������
	call  clear_packet_buffer
; ������������ �������� ������� ��� ����������
; ������� ������
	mov	  [PacketCommand],byte 0x43
	; ������ ������
	mov	  [PacketCommand+2],byte 1
; ������ ���������� �������
	mov	[PacketCommand+7],byte 0xFF
	mov	[PacketCommand+8],byte 0h
; ������ �������
	call  SendPacketDatCommand
	popa
	ret

;*************************************************
;* ���������� ����� ���������� �������� �� ����� *
;* ������� ��������� ���������� ����� ���������� *
;* ����������:                                   *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;*************************************************
;ReadCapacity:
;       pusha
;; �������� ����� �������� �������
;       call  clear_packet_buffer
;; ������ ������ ������ � ������
;       mov     [CDBlockSize],8
;; ������������ ������� READ CAPACITY
;       mov     [PacketCommand],word 25h
;; ������ �������
;       call    SendPacketDatCommand
;       popa
;       ret

clear_packet_buffer:
; �������� ����� �������� �������
	mov	[PacketCommand],dword 0
	mov	[PacketCommand+4],dword 0
	mov	[PacketCommand+8],dword 0
	ret
