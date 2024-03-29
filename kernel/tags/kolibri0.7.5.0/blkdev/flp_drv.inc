;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 593 $


;**********************************************************
;  ���������������� ������ � ������������ ������� �����
;**********************************************************
; ����� ��������� ������  ������� �������� �����������.
; ��������� � ��������� Mario79

;give_back_application_data:  ; ��������� ����������
;     mov edi,[TASK_BASE]
;     mov edi,[edi+TASKDATA.mem_start]
;     add edi,ecx
give_back_application_data_1:
     mov esi,FDD_BUFF   ;FDD_DataBuffer  ;0x40000
     xor ecx,ecx
     mov cx,128
     cld
     rep movsd
     ret

;take_data_from_application:   ; ����� �� ���������
;     mov esi,[TASK_BASE]
;     mov esi,[esi+TASKDATA.mem_start]
;     add esi,ecx
take_data_from_application_1:
     mov edi,FDD_BUFF   ;FDD_DataBuffer  ;0x40000
     xor ecx,ecx
     mov cx,128
     cld
     rep movsd
     ret

; ���� ���������� �������� � ������������ (FDC_Status)
FDC_Normal         equ 0 ;���������� ����������
FDC_TimeOut        equ 1 ;������ ����-����
FDC_DiskNotFound   equ 2 ;� ��������� ��� �����
FDC_TrackNotFound  equ 3 ;������� �� �������
FDC_SectorNotFound equ 4 ;������ �� ������

; ������������ �������� ��������� ������� (��������
; �������� ������������� ���������� ������������
; ������������� ������� ����� ������� 1,44 ��)
MAX_Track   equ 79
MAX_Head    equ  1
MAX_Sector  equ 18

uglobal
; ������� ����� �������
TickCounter dd ?
; ��� ���������� �������� � ������������ ����
FDC_Status  DB ?
; ���� ���������� �� ����
FDD_IntFlag DB ?
; ������ ������ ��������� �������� � ����
FDD_Time    DD ?
; ����� ���������
FDD_Type    db 0
; ���������� �������
FDD_Track   DB ?
FDD_Head    DB ?
FDD_Sector  DB ?

; ���� ���������� ��������
FDC_ST0 DB ?
FDC_ST1 DB ?
FDC_ST2 DB ?
FDC_C   DB ?
FDC_H   DB ?
FDC_R   DB ?
FDC_N   DB ?
; ������� ���������� �������� �����
ReadRepCounter  DB ?
; ������� ���������� �������� ������������
RecalRepCounter DB ?
endg
; ������� ������ ��� �������� ������������ �������
;FDD_DataBuffer:  times 512 db 0   ;DB 512 DUP (?)
fdd_motor_status db 0
timer_fdd_motor  dd 0

;*************************************
;* ������������� ������ ��� ��� ���� *
;*************************************
Init_FDC_DMA:
        pushad
        mov al,0
        out 0x0c,al     ; reset the flip-flop to a known state.
        mov al,6                ; mask channel 2 so we can reprogram it.
        out 0x0a,al
        mov al,[dmamode] ; 0x46 -> Read from floppy - 0x4A Write to floppy
        out 0x0b,al
        mov al,0
        out 0x0c,al     ; reset the flip-flop to a known state.
        mov eax,0xD000
        out 0x04,al     ; set the channel 2 starting address to 0
        shr eax,8
        out 0x04,al
        shr eax,8
        out 0x81,al
        mov al,0
        out 0x0c, al    ; reset flip-flop
        mov al, 0xff    ;set count (actual size -1)
        out 0x5, al
        mov al,0x1  ;[dmasize]       ;(0x1ff = 511 / 0x23ff =9215)
        out 0x5,al
        mov al,2
        out 0xa,al
        popad
        ret

;***********************************
;* �������� ���� � ���� ������ FDC *
;* ���������:                      *
;* AL - ��������� ����.            *
;***********************************
FDCDataOutput:
;        pusha
        push eax ecx edx
        mov     AH,AL     ;��������� ���� � AH
; �������� ���������� ��������� �����������
        mov     [FDC_Status],FDC_Normal
; ��������� ���������� ����������� � ������ ������
        mov     DX,3F4h   ;(���� ��������� FDC)
        mov     ecx, 0x10000 ;���������� ������� ����-����
@@TestRS:
        in      AL,DX     ;��������� ������� RS
        and     AL,0C0h   ;�������� ������� 6 � 7
        cmp     AL,80h    ;��������� ������� 6 � 7
        je      @@OutByteToFDC
        loop    @@TestRS
; ������ ����-����
        mov     [FDC_Status],FDC_TimeOut
        jmp @@End_5
; ������� ���� � ���� ������
@@OutByteToFDC:
        inc     DX
        mov     AL,AH
        out     DX,AL
@@End_5:
;        popa
        pop edx ecx eax
        ret

;******************************************
;*   ��������� ���� �� ����� ������ FDC   *
;* ��������� �� ����� ������� ����������. *
;* �������� ������:                       *
;* AL - ��������� ����.                   *
;******************************************
FDCDataInput:
        push    ECX
        push    DX
; �������� ���������� ��������� �����������
        mov     [FDC_Status],FDC_Normal
; ��������� ���������� ����������� � �������� ������
        mov     DX,3F4h   ;(���� ��������� FDC)
        xor     CX,CX     ;���������� ������� ����-����
@@TestRS_1:
        in      AL,DX     ;��������� ������� RS
        and     AL,0C0h   ;������� ������� 6 � 7
        cmp     AL,0C0h   ;��������� ������� 6 � 7
        je      @@GetByteFromFDC
        loop    @@TestRS_1
; ������ ����-����
        mov     [FDC_Status],FDC_TimeOut
        jmp @@End_6
; ������ ���� �� ����� ������
@@GetByteFromFDC:
        inc     DX
        in      AL,DX
@@End_6:  pop     DX
        pop     ECX
        ret

;*********************************************
;* ���������� ���������� �� ����������� ���� *
;*********************************************
FDCInterrupt:
; ���������� ���� ���������
        mov     [FDD_IntFlag],1
        ret


;******************************************
;* ���������� ����� ���������� ���������� *
;*             ����                       *
;******************************************
SetUserInterrupts:
         mov     [fdc_irq_func],FDCInterrupt
         ret

;*******************************************
;* �������� ���������� �� ����������� ���� *
;*******************************************
WaitFDCInterrupt:
        pusha
; �������� ���� ��������� ��������
        mov     [FDC_Status],FDC_Normal
; �������� ���� ���������
        mov     [FDD_IntFlag],0
; �������� ������� �����
        mov     eax,[timer_ticks]
        mov     [TickCounter],eax
; ������� ��������� ����� ���������� ����
@@TestRS_2:
        cmp     [FDD_IntFlag],0
        jnz     @@End_7           ;���������� ���������
        call    change_task
        mov     eax,[timer_ticks]
        sub     eax,[TickCounter]
        cmp     eax,50  ;25   ;5 ;������� 5 �����
        jb      @@TestRS_2
;        jl      @@TestRS_2
; ������ ����-����
        mov     [FDC_Status],FDC_TimeOut
;        mov   [flp_status],0
@@End_7:  popa
        ret

;*********************************
;* �������� ����� ��������� "A:" *
;*********************************
FDDMotorON:
        pusha
;        cmp     [fdd_motor_status],1
;        je      fdd_motor_on
        mov     al,[flp_number]
        cmp     [fdd_motor_status],al
        je      fdd_motor_on
; ���������� ����� ����������� ����
        mov     DX,3F2h ;���� ���������� �����������
        mov     AL,0
        out     DX,AL
; ������� � �������� ����� ���������
        cmp     [flp_number],1
        jne     FDDMotorON_B
;        call    FDDMotorOFF_B
        mov     AL,1Ch    ; Floppy A
        jmp     FDDMotorON_1
FDDMotorON_B:
;        call    FDDMotorOFF_A
        mov     AL,2Dh    ; Floppy B
FDDMotorON_1:
        out     DX,AL
; �������� ������� �����
        mov     eax,[timer_ticks]
        mov     [TickCounter],eax
; ������� 0,5 �
@@dT:
        call    change_task
        mov     eax,[timer_ticks]
        sub     eax,[TickCounter]
        cmp     eax,50  ;10
        jb      @@dT
        cmp     [flp_number],1
        jne     fdd_motor_on_B
        mov     [fdd_motor_status],1
        jmp     fdd_motor_on
fdd_motor_on_B:
        mov     [fdd_motor_status],2
fdd_motor_on:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************
;*  ���������� ��������� �������         *
;*****************************************
save_timer_fdd_motor:
        mov     eax,[timer_ticks]
        mov     [timer_fdd_motor],eax
        ret

;*****************************************
;*  �������� �������� ���������� ������  *
;*****************************************
check_fdd_motor_status:
        cmp     [fdd_motor_status],0
        je      end_check_fdd_motor_status_1
        mov     eax,[timer_ticks]
        sub     eax,[timer_fdd_motor]
        cmp     eax,500
        jb      end_check_fdd_motor_status
        call    FDDMotorOFF
        mov     [fdd_motor_status],0
end_check_fdd_motor_status_1:
        mov     [flp_status],0
end_check_fdd_motor_status:
        ret

;**********************************
;* ��������� ����� ���������      *
;**********************************
FDDMotorOFF:
        push    AX
        push    DX
        cmp     [flp_number],1
        jne     FDDMotorOFF_1
        call    FDDMotorOFF_A
        jmp     FDDMotorOFF_2
FDDMotorOFF_1:
        call    FDDMotorOFF_B
FDDMotorOFF_2:
        pop     DX
        pop     AX
        ; ����� ������ ����������� � ����� � ������������ ����������
        mov    [root_read],0
        mov    [flp_fat],0
        ret

FDDMotorOFF_A:
        mov     DX,3F2h ;���� ���������� �����������
        mov     AL,0Ch  ; Floppy A
        out     DX,AL
        ret

FDDMotorOFF_B:
        mov     DX,3F2h ;���� ���������� �����������
        mov     AL,5h  ; Floppy B
        out     DX,AL
        ret

;*******************************
;* ������������ ��������� "A:" *
;*******************************
RecalibrateFDD:
        pusha
        call    save_timer_fdd_motor
; ������ ������� "������������"
        mov     AL,07h
        call    FDCDataOutput
        mov     AL,00h
        call    FDCDataOutput
; ������� ���������� ��������
        call    WaitFDCInterrupt
;        cmp    [FDC_Status],0
;        je    no_fdc_status_error
;        mov   [flp_status],0
;no_fdc_status_error:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************************
;*                    ����� �������                  *
;* ��������� ���������� ����� ���������� ����������: *
;* FDD_Track - ����� ������� (0-79);                 *
;* FDD_Head - ����� ������� (0-1).                   *
;* ��������� �������� ��������� � FDC_Status.        *
;*****************************************************
SeekTrack:
        pusha
        call    save_timer_fdd_motor
; ������ ������� "�����"
        mov     AL,0Fh
        call    FDCDataOutput
        ; �������� ���� ������ �������/���������
        mov     AL,[FDD_Head]
        shl     AL,2
        call    FDCDataOutput
        ; �������� ���� ������ �������
        mov     AL,[FDD_Track]
        call    FDCDataOutput
; ������� ���������� ��������
        call    WaitFDCInterrupt
        cmp     [FDC_Status],FDC_Normal
        jne     @@Exit
; ��������� ��������� ������
        mov     AL,08h
        call    FDCDataOutput
        call    FDCDataInput
        mov     [FDC_ST0],AL
        call    FDCDataInput
        mov     [FDC_C],AL
; ��������� ��������� ������
        ; ����� ��������?
        test    [FDC_ST0],100000b
        je      @@Err
        ; �������� ���� ������?
        mov     AL,[FDC_C]
        cmp     AL,[FDD_Track]
        jne     @@Err
        ; ����� ������� ��������� � ��������?
        mov     AL,[FDC_ST0]
        and     AL,100b
        shr     AL,2
        cmp     AL,[FDD_Head]
        jne     @@Err
        ; �������� ��������� �������
        mov     [FDC_Status],FDC_Normal
        jmp @@Exit
@@Err:  ; ���� �� ������
        mov     [FDC_Status],FDC_TrackNotFound
;        mov   [flp_status],0
@@Exit:
        call    save_timer_fdd_motor
        popa
        ret

;*******************************************************
;*               ������ ������� ������                 *
;* ��������� ���������� ����� ���������� ����������:   *
;* FDD_Track - ����� ������� (0-79);                   *
;* FDD_Head - ����� ������� (0-1);                     *
;* FDD_Sector - ����� ������� (1-18).                  *
;* ��������� �������� ��������� � FDC_Status.          *
;* � ������ ��������� ���������� �������� ������       *
;* ���������� ������� ����� �������� � FDD_DataBuffer. *
;*******************************************************
ReadSector:
        pushad
        call    save_timer_fdd_motor
; ���������� �������� �������� 500 �����/�
        mov     AX,0
        mov     DX,03F7h
        out     DX,AL
; ���������������� ����� ������� ������� � ������
        mov     [dmamode],0x46
        call    Init_FDC_DMA
; ������ ������� "������ ������"
        mov     AL,0E6h  ;������ � �������������� ������
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        shl     AL,2
        call    FDCDataOutput
        mov     AL,[FDD_Track]
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        call    FDCDataOutput
        mov     AL,[FDD_Sector]
        call    FDCDataOutput
        mov     AL,2    ;��� ������� ������� (512 ����)
        call    FDCDataOutput
        mov     AL,18  ;+1; 3Fh  ;����� �������� �� �������
        call    FDCDataOutput
        mov     AL,1Bh  ;�������� GPL
        call    FDCDataOutput
        mov     AL,0FFh ;�������� DTL
        call    FDCDataOutput
; ������� ���������� �� ���������� ��������
        call    WaitFDCInterrupt
        cmp     [FDC_Status],FDC_Normal
        jne     @@Exit_1
; ��������� ������ ���������� ��������
        call    GetStatusInfo
        test    [FDC_ST0],11011000b
        jnz     @@Err_1
        mov     [FDC_Status],FDC_Normal
        jmp @@Exit_1
@@Err_1:  mov     [FDC_Status],FDC_SectorNotFound
;        mov   [flp_status],0
@@Exit_1:
        call    save_timer_fdd_motor
        popad
        ret

;*******************************************************
;*   ������ ������� (� ����������� �������� ��� ����)  *
;* ��������� ���������� ����� ���������� ����������:   *
;* FDD_Track - ����� ������� (0-79);                   *
;* FDD_Head - ����� ������� (0-1);                     *
;* FDD_Sector - ����� ������� (1-18).                  *
;* ��������� �������� ��������� � FDC_Status.          *
;* � ������ ��������� ���������� �������� ������       *
;* ���������� ������� ����� �������� � FDD_DataBuffer. *
;*******************************************************
ReadSectWithRetr:
        pusha
; �������� ������� ���������� �������� ������������
        mov     [RecalRepCounter],0
@@TryAgain:
; �������� ������� ���������� �������� �����
        mov     [ReadRepCounter],0
@@ReadSector_1:
        call    ReadSector
        cmp     [FDC_Status],0
        je      @@Exit_2
        cmp     [FDC_Status],1
        je      @@Err_3
        ; ����������� ���������� �����
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter],3
        jb      @@ReadSector_1
        ; ����������� ���������� ������������
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter],3
        jb      @@TryAgain
;        mov   [flp_status],0
@@Exit_2:
        popa
        ret
@@Err_3:
        mov   [flp_status],0
        popa
        ret

;*******************************************************
;*               ������ ������� ������                 *
;* ��������� ���������� ����� ���������� ����������:   *
;* FDD_Track - ����� ������� (0-79);                   *
;* FDD_Head - ����� ������� (0-1);                     *
;* FDD_Sector - ����� ������� (1-18).                  *
;* ��������� �������� ��������� � FDC_Status.          *
;* � ������ ��������� ���������� �������� ������       *
;* ���������� FDD_DataBuffer ����� �������� � ������.  *
;*******************************************************
WriteSector:
        pushad
        call    save_timer_fdd_motor
; ���������� �������� �������� 500 �����/�
        mov     AX,0
        mov     DX,03F7h
        out     DX,AL
; ���������������� ����� ������� ������� � ������
        mov     [dmamode],0x4A
        call    Init_FDC_DMA
; ������ ������� "������ ������"
        mov     AL,0xC5  ;0x45  ;������ � �������������� ������
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        shl     AL,2
        call    FDCDataOutput
        mov     AL,[FDD_Track]
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        call    FDCDataOutput
        mov     AL,[FDD_Sector]
        call    FDCDataOutput
        mov     AL,2    ;��� ������� ������� (512 ����)
        call    FDCDataOutput
        mov     AL,18; 3Fh  ;����� �������� �� �������
        call    FDCDataOutput
        mov     AL,1Bh  ;�������� GPL
        call    FDCDataOutput
        mov     AL,0FFh ;�������� DTL
        call    FDCDataOutput
; ������� ���������� �� ���������� ��������
        call    WaitFDCInterrupt
        cmp     [FDC_Status],FDC_Normal
        jne     @@Exit_3
; ��������� ������ ���������� ��������
        call    GetStatusInfo
        test    [FDC_ST0],11000000b  ;11011000b
        jnz     @@Err_2
        mov     [FDC_Status],FDC_Normal
        jmp @@Exit_3
@@Err_2:  mov     [FDC_Status],FDC_SectorNotFound
@@Exit_3:
        call    save_timer_fdd_motor
        popad
        ret

;*******************************************************
;*   ������ ������� (� ����������� �������� ��� ����)  *
;* ��������� ���������� ����� ���������� ����������:   *
;* FDD_Track - ����� ������� (0-79);                   *
;* FDD_Head - ����� ������� (0-1);                     *
;* FDD_Sector - ����� ������� (1-18).                  *
;* ��������� �������� ��������� � FDC_Status.          *
;* � ������ ��������� ���������� �������� ������       *
;* ���������� FDD_DataBuffer ����� �������� � ������.  *
;*******************************************************
WriteSectWithRetr:
        pusha
; �������� ������� ���������� �������� ������������
        mov     [RecalRepCounter],0
@@TryAgain_1:
; �������� ������� ���������� �������� �����
        mov     [ReadRepCounter],0
@@WriteSector_1:
        call    WriteSector
        cmp     [FDC_Status],0
        je      @@Exit_4
        cmp     [FDC_Status],1
        je      @@Err_4
        ; ����������� ���������� �����
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter],3
        jb      @@WriteSector_1
        ; ����������� ���������� ������������
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter],3
        jb      @@TryAgain_1
@@Exit_4:
        popa
        ret
@@Err_4:
        mov   [flp_status],0
        popa
        ret

;*********************************************
;* �������� ���������� � ���������� �������� *
;*********************************************
GetStatusInfo:
        push    AX
        call    FDCDataInput
        mov     [FDC_ST0],AL
        call    FDCDataInput
        mov     [FDC_ST1],AL
        call    FDCDataInput
        mov     [FDC_ST2],AL
        call    FDCDataInput
        mov     [FDC_C],AL
        call    FDCDataInput
        mov     [FDC_H],AL
        call    FDCDataInput
        mov     [FDC_R],AL
        call    FDCDataInput
        mov     [FDC_N],AL
        pop     AX
        ret

