;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 593 $


;******************************************************
; ����� �������� HDD � CD
; ����� ��������� ������ ������� �������� �����������.
; ��������� � ��������� Mario79
;******************************************************

;****************************************************
;*                 ����� HDD � CD                   *
;****************************************************
FindHDD:
        mov     [ChannelNumber],1
        mov     [DiskNumber],0
        call    FindHDD_3
;        mov     ax,[Sector512+176]
;        mov     [DRIVE_DATA+6],ax
;        mov     ax,[Sector512+126]
;        mov     [DRIVE_DATA+8],ax
;        mov     ax,[Sector512+128]
;        mov     [DRIVE_DATA+8],ax
        mov     [DiskNumber],1
        call    FindHDD_3
;        mov     al,[Sector512+176]
;        mov     [DRIVE_DATA+7],al
        inc     [ChannelNumber]
        mov     [DiskNumber],0
        call    FindHDD_3
;        mov     al,[Sector512+176]
;        mov     [DRIVE_DATA+8],al
        mov     [DiskNumber],1
        call    FindHDD_1
;        mov     al,[Sector512+176]
;        mov     [DRIVE_DATA+9],al

        jmp     EndFindHDD

FindHDD_1:
        call    ReadHDD_ID
        cmp     [DevErrorCode],0
        jne     FindHDD_2
        cmp     [Sector512+6],word 16
        ja      FindHDD_2
        cmp     [Sector512+12],word 255
        ja      FindHDD_2
        inc     byte [DRIVE_DATA+1]
        jmp     FindHDD_2_2
   FindHDD_2:
        call    DeviceReset
        cmp     [DevErrorCode],0
        jne     FindHDD_2_2
        call    ReadCD_ID
        cmp     [DevErrorCode],0
        jne      FindHDD_2_2
        inc     byte [DRIVE_DATA+1]
        inc     byte [DRIVE_DATA+1]
   FindHDD_2_2:
        ret

FindHDD_3:
        call    FindHDD_1
        shl     byte [DRIVE_DATA+1],2
        ret


; ����� ������������ ������� � ������ LBA
SectorAddress   DD ?

;*************************************************
;*     ������ �������������� �������� �����      *
;* ������� ��������� ���������� ����� ���������� *
;* ����������:                                   *
;* ChannelNumber - ����� ������ (1 ��� 2);       *
;* DiskNumber - ����� ����� �� ������ (0 ��� 1). *
;* ����������������� ���� ������ �����������     *
;* � ������ Sector512.                           *
;*************************************************
ReadHDD_ID:
; ������ ����� CHS
        mov     [ATAAddressMode],0
; ������� ������� ������������� ����������
        mov     [ATAFeatures],0
        mov     [ATAHead],0
        mov     [ATACommand],0ECh
        call    SendCommandToHDD
        cmp     [DevErrorCode],0 ;��������� ��� ������
        jne     @@End  ;���������, �������� ��� ������
        mov     DX,[ATABasePortAddr]
        add     DX,7     ;����� �������� ��������
        mov  ecx,0xffff
@@WaitCompleet:
        ; ��������� ����� ���������� �������
        dec  ecx
        cmp  ecx,0
        je     @@Error1   ;������ ����-����
        ; ��������� ����������
        in      AL,DX
        test    AL,80h   ;��������� ������� BSY
        jnz     @@WaitCompleet
        test    AL,1     ;��������� ������� ERR
        jnz     @@Error6
        test    AL,08h   ;��������� ������� DRQ
        jz      @@WaitCompleet
; ������� ���� ������ �� �����������
;        mov     AX,DS
;        mov     ES,AX
        mov     EDI,Sector512  ;offset Sector512
        mov     DX,[ATABasePortAddr] ;������� ������
        mov     CX,256   ;����� ����������� ����
        rep     insw     ;������� ���� ������
        jmp @@End
; �������� ��� ������
@@Error1:
        mov     [DevErrorCode],1
        jmp @@End
@@Error6:
        mov     [DevErrorCode],6
@@End:  ret



; ����������� ������� ������ ������� 1 � 2
StandardATABases DW 1F0h, 170h
; ����� ������
ChannelNumber   DW ?
; ����� �����
DiskNumber      DB ?
; ������� ����� ������ ������ ����������� ATA
ATABasePortAddr DW ?
; ��������� ATA-�������
ATAFeatures     DB ? ;�����������
ATASectorCount  DB ? ;���������� �������������� ��������
ATASectorNumber DB ? ;����� ���������� �������
ATACylinder     DW ? ;����� ���������� ��������
ATAHead         DB ? ;����� ��������� �������
ATAAddressMode  DB ? ;����� ��������� (0 - CHS, 1 - LBA)
ATACommand      DB ? ;��� �������, ���������� ����������
; ��� ������ (0 - ��� ������, 1 - �������� ����������
; �������� ��������, 2 - �������� ��� ������ ���������,
; 3 - �������� ����� ������, 4 - �������� ����� �����,
; 5 - �������� ����� �������, 6 - ������ ��� ����������
; �������)
DevErrorCode DB ?

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
SendCommandToHDD:
; ��������� �������� ���� ������
        cmp     [ATAAddressMode],1
        ja      @@Err2
; ��������� ������������ ������ ������
        mov     BX,[ChannelNumber]
        cmp     BX,1
        jb      @@Err3
        cmp     BX,2
        ja      @@Err3
; ���������� ������� �����
        dec     BX
        shl     BX,1
        movzx   ebx,bx
        mov     AX,[ebx+StandardATABases]
        mov     [ATABasePortAddr],AX
; �������� ���������� HDD � ������ �������
        ; ������� ������ ����
        mov     DX,[ATABasePortAddr]
        add     DX,6    ;����� �������� �������
        mov     AL,[DiskNumber]
        cmp     AL,1    ;��������� ������ �����
        ja      @@Err4
        shl     AL,4
        or      AL,10100000b
        out     DX,AL
        ; �������, ���� ���� �� ����� �����
        inc     DX
        mov  ecx,0xfff
;        mov     eax,[timer_ticks]
;        mov     [TickCounter_1],eax
@@WaitHDReady:
        ; ��������� ����� �������
        dec  ecx
        cmp  ecx,0
        je     @@Err1
;        mov     eax,[timer_ticks]
;        sub     eax,[TickCounter_1]
;        cmp     eax,300    ;������� 300 �����
;        ja      @@Err1   ;������ ����-����
        ; ��������� ������� ��������
        in      AL,DX
        ; ��������� ��������� ������� BSY
        test    AL,80h
        jnz     @@WaitHDReady
        ; ��������� ��������� ������� DRQ
        test    AL,08h
        jnz     @@WaitHDReady
; ��������� ������� � �������� �����������
        cli
        mov     DX,[ATABasePortAddr]
        inc     DX      ;������� "������������"
        mov     AL,[ATAFeatures]
        out     DX,AL
        inc     DX      ;������� ��������
        mov     AL,[ATASectorCount]
        out     DX,AL
        inc     DX      ;������� ������ �������
        mov     AL,[ATASectorNumber]
        out     DX,AL
        inc     DX      ;����� �������� (������� ����)
        mov     AX,[ATACylinder]
        out     DX,AL
        inc     DX      ;����� �������� (������� ����)
        mov     AL,AH
        out     DX,AL
        inc     DX      ;����� �������/����� �����
        mov     AL,[DiskNumber]
        shl     AL,4
        cmp     [ATAHead],0Fh ;��������� ����� �������
        ja      @@Err5
        or      AL,[ATAHead]
        or      AL,10100000b
        mov     AH,[ATAAddressMode]
        shl     AH,6
        or      AL,AH
        out     DX,AL
; ������� �������
        mov     AL,[ATACommand]
        inc     DX      ;������� ������
        out     DX,AL
        sti
; �������� ������� ������
        mov     [DevErrorCode],0
        jmp @@End_2
; �������� ��� ������
@@Err1: mov     [DevErrorCode],1
        jmp @@End_2
@@Err2: mov     [DevErrorCode],2
        jmp @@End_2
@@Err3: mov     [DevErrorCode],3
        jmp @@End_2
@@Err4: mov     [DevErrorCode],4
        jmp @@End_2
@@Err5: mov     [DevErrorCode],5
; ���������� ������ ���������
@@End_2:
        ret

;*************************************************
;*     ������ �������������� ���������� ATAPI    *
;* ������� ��������� ���������� ����� ���������� *
;* ���������:                                    *
;* ChannelNumber - ����� ������;                 *
;* DiskNumber - ����� ����� �� ������.           *
;* ����������������� ���� ������ �����������     *
;* � ������ Sector512.                           *
;*************************************************
ReadCD_ID:
; ������ ����� CHS
        mov     [ATAAddressMode],0
; ������� ������� ������������� ����������
        mov     [ATAFeatures],0
        mov     [ATASectorCount],0
        mov     [ATASectorNumber],0
        mov     [ATACylinder],0
        mov     [ATAHead],0
        mov     [ATACommand],0A1h
        call    SendCommandToHDD
        cmp     [DevErrorCode],0 ;��������� ��� ������
        jne     @@End_1  ;���������, �������� ��� ������
; ������� ���������� ������ HDD
        mov     DX,[ATABasePortAddr]
        add     DX,7   ;���� 1�7h
        mov  ecx,0xffff
@@WaitCompleet_1:
        ; ��������� ����
        dec  ecx
        cmp  ecx,0
        je     @@Error1_1  ;������ ����-����
        ; ��������� ����������
        in      AL,DX
        test    AL,80h   ;��������� ������� BSY
        jnz     @@WaitCompleet_1
        test    AL,1     ;��������� ������� ERR
        jnz     @@Error6_1
        test    AL,08h   ;��������� ������� DRQ
        jz      @@WaitCompleet_1
; ������� ���� ������ �� �����������
;        mov     AX,DS
;        mov     ES,AX
        mov     EDI,Sector512  ;offset Sector512
        mov     DX,[ATABasePortAddr] ;���� 1x0h
        mov     CX,256 ;����� ����������� ����
        rep     insw
        jmp @@End_1
; �������� ��� ������
@@Error1_1:
        mov     [DevErrorCode],1
        jmp @@End_1
@@Error6_1:
        mov     [DevErrorCode],6
@@End_1:
        ret

;*************************************************
;*                ����� ����������               *
;* ������� ��������� ���������� ����� ���������� *
;* ����������:                                   *
;* ChannelNumber - ����� ������ (1 ��� 2);       *
;* DiskNumber - ����� ����� (0 ��� 1).           *
;*************************************************
DeviceReset:
; ��������� ������������ ������ ������
        mov     BX,[ChannelNumber]
        cmp     BX,1
        jb      @@Err3_2
        cmp     BX,2
        ja      @@Err3_2
; ���������� ������� �����
        dec     BX
        shl     BX,1
        movzx   ebx,bx
        mov     DX,[ebx+StandardATABases]
        mov     [ATABasePortAddr],DX
; ������� ������ ����
        add     DX,6    ;����� �������� �������
        mov     AL,[DiskNumber]
        cmp     AL,1    ;��������� ������ �����
        ja      @@Err4_2
        shl     AL,4
        or      AL,10100000b
        out     DX,AL
; ������� ������� "�����"
        mov     AL,08h
        inc     DX      ;������� ������
        out     DX,AL
        mov     ecx,0x80000
@@WaitHDReady_1:
        ; ��������� ����� �������
        dec     ecx
        cmp     ecx,0
        je      @@Err1_2 ;������ ����-����
        ; ��������� ������� ��������
        in      AL,DX
        ; ��������� ��������� ������� BSY
        test    AL,80h
        jnz     @@WaitHDReady_1
; �������� ������� ������
        mov     [DevErrorCode],0
        jmp @@End_3
; ��������� ������
@@Err1_2: mov     [DevErrorCode],1
        jmp @@End_3
@@Err3_2: mov     [DevErrorCode],3
        jmp @@End_3
@@Err4_2: mov     [DevErrorCode],4
; �������� ��� ������
@@End_3:
        ret

EndFindHDD:

