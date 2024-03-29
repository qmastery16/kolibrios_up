$Revision: 473 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;**************************************************
;*      ����� ���� �� ���������������� ������     *
;* ��������� �������������� ���������� ���������� *
;* COMPortNum � COMPortBaseAddr ��� ������������  *
;* ��������� ����������� ����������               *
;**************************************************
; ����� ��������� ������ ������� �������� �����������.
; ��������� � ��������� Mario79

Detect_COM_Mouse:
        pusha
        call   MSMouseSearch
        cmp    AL,'M'
        jne    @f
        mov    [com1_mouse_detected],1
        mov     [irq_owner+4*4], 1      ; IRQ4 owner is System
        mov     eax, [RESERVED_PORTS]
        inc     eax
        mov     [RESERVED_PORTS], eax
        shl     eax, 4
        mov     [RESERVED_PORTS+eax+0], dword 1
        mov     [RESERVED_PORTS+eax+4], dword 0x3F8
        mov     [RESERVED_PORTS+eax+8], dword 0x3FF
        mov    esi,boot_setmouse_type+22
        call  boot_log
     @@:
        sub    [COMPortBaseAddr],100h
        call   MSMouseSearch
        cmp    AL,'M'
        jne    @f
        mov    [com2_mouse_detected],1
        mov     [irq_owner+3*4], 1      ; IRQ3 owner is System
        mov     eax, [RESERVED_PORTS]
        inc     eax
        mov     [RESERVED_PORTS], eax
        shl     eax, 4
        mov     [RESERVED_PORTS+eax+0], dword 1
        mov     [RESERVED_PORTS+eax+4], dword 0x2F8
        mov     [RESERVED_PORTS+eax+8], dword 0x2FF
        mov    esi,boot_setmouse_type+44
        call  boot_log
     @@:
        popa
        jmp     end_detecting_mouse

MSMouseSearch:
        ; ����� ���� ����� COM-�����
MouseSearch:
        ; ������������� ��������
        ; ������/�������� 1200 ���
        mov     DX,[COMPortBaseAddr]
        add     DX,3
        in      AL,DX
        or      AL,80h  ;���������� ��� DLAB
        out     DX,AL
        mov     DX,[COMPortBaseAddr]
        mov     AL,60h  ;1200 ���
        out     DX,AL
        inc     DX
        mov     AL,0
        out     DX,AL
        ; ���������� ����� ����� 7 ���, 1 �������� ���,
        ; �������� �� ��������������
        mov     DX,[COMPortBaseAddr]
        add     DX,3
        mov     AL,00000010b
        out     DX,AL
        ; ��������� ��� ���������
        mov     DX,[COMPortBaseAddr]
        inc     DX
        mov     AL,0
        out     DX,AL
; ���������, ��� ���������� ���������� � �������
; ����� ���� MSMouse
        ; ��������� ������� ���� � ���������
        mov     DX,[COMPortBaseAddr]
        add     EDX,4   ;������� ���������� �������
        mov     AL,0    ;�������� DTR, RTS � OUT2
        out     DX,AL
        ; ������� 5 "�����" (0,2 �)
        mov     ecx, 0xFFFF
        loop    $
        ; �������� ������� ����
        mov     al, 1
        out     dx, al
        mov     ecx, 0xFFFF
        loop    $
        ; �������� ������� ������
        sub     edx, 4
        in      AL,DX
        add     edx, 4
        mov     AL,11b  ;���������� DTR � RTS
        out     DX,AL
        mov     ecx, 0x1FFFF
; ���� ������ �����
WaitData:
        ; ������� ��� 10 "�����"
         dec  ecx
         cmp  ecx,0
         je     NoMouse
        ; ��������� ������� ������������������ �����
        mov     DX,[COMPortBaseAddr]
        add     DX,5
        in      AL,DX
        test    AL,1   ;������ ������?
        jz      WaitData
        ; ������ ������
        mov     DX,[COMPortBaseAddr]
        in      AL,DX
NoMouse:
        ret

iglobal
COMPortBaseAddr dw 3F8h
;COMPortNum dw 0
endg

iglobal
boot_setmouse_type  db   'Detected - PS2  mouse',0
                    db   'Detected - COM1 mouse',0
                    db   'Detected - COM2 mouse',0
endg

end_detecting_mouse:

