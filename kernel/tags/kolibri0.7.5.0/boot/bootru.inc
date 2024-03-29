;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;=================================================================
;
;                           BOOT DATA
;
;=================================================================

$Revision: 751 $


d80x25_bottom:
          db       186,' Kolibri OS �᭮���� �� Menuet OS � �� �।��⠢��� '
          db       '������� ���a�⨩.        ',186
          db       186,' ���஡��� ᬮ��� � 䠩�� COPYING.TXT              '
          db       '                         ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm   db      " APM x.x ", 0
novesa    db      "���������: EGA/CGA",13,10,0
s_vesa    db      "����� VESA: "
  .ver    db      "?.?",13,10,0

gr_mode   db      "�롥�� �����०��: ",13,10,0
vrrmprint db      "�ᯮ�짮���� VRR? (���� ���஢ ��� 60 ��"
          db      " ⮫쪮 ��� ���室��:",13,10
          db      186," 1024*768>800*600 � 800*600>640*480) [1-��, 2-���]: ",0
;ask_dma   db      "�ᯮ�짮���� DMA ��� ����㯠 � HDD? [1-��, 2-⮫쪮 �⥭��, 3-���]: ",0
ask_bd     db      "�������� ��᪨, ������ �१ BIOS � ०��� V86? [1-��, 2-���]: ",0
bdev      db      "����㧨�� ��ࠧ �� [1-��᪥�; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-�ᯮ�짮���� 㦥 ����㦥��� ��ࠧ;"
          db      13,10,186,"                    "
          db      "4-ᮧ���� ���� ��ࠧ]: ",0
prnotfnd  db      "�訡�� - �����०�� �� ������.",0
not386    db      "�訡�� - �ॡ���� ������ 386+.",0
fatalsel  db      "�訡�� - ��࠭�� �����०�� �� �����ন������.",0
pres_key  db      "������� ���� �������, ��� ���室� � �롮� ०����.",0
badsect   db      13,10,186," �訡�� - ��᪥� ���०����. ���஡�� �����.",0
memmovefailed db  13,10,186," �訡�� - Int 0x15 move failed.",0
okt       db      " ... OK"
linef     db      13,10,0
diskload  db      "����㧪� ��᪥��: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0
start_msg db      "������ [abcd] ��� ��������� ����஥�, [Enter] ��� �த������� ����㧪�",13,10,0
time_msg  db      " ��� �������� "
time_str  db      " 5 ᥪ㭤 "
          db      " �� ��⮬���᪮�� �த�������",13,10,0
current_cfg_msg db "����騥 ����ன��:",13,10,0
curvideo_msg db   " [a] �����०��: ",0


mode0     db      "320x200, EGA/CGA 256 梥⮢",13,10,0
mode9     db      "640x480, VGA 16 梥⮢",13,10,0  

usebd_msg db      " [b] �������� ��᪨, ������ �१ BIOS:",0
on_msg    db      " ���",13,10,0
off_msg   db      " �몫",13,10,0
readonly_msg db   " ⮫쪮 �⥭��",13,10,0
vrrm_msg  db      " [c] �ᯮ�짮����� VRR:",0
preboot_device_msg db " [d] ��ࠧ ��᪥��: ",0
preboot_device_msgs dw 0,pdm1,pdm2,pdm3,pdm4
pdm1      db      "������� ��᪥�",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "�ᯮ�짮���� 㦥 ����㦥��� ��ࠧ",13,10,0
pdm4      db      "ᮧ���� ���� ��ࠧ",13,10,0
loading_msg db    "���� ����㧪� KolibriOS...",0
save_quest db     "��������� ⥪�騥 ����ன��? [y/n]: ",0
loader_block_error db "�訡�� � ������ ��砫쭮�� �����稪�, �த������� ����������.",0


_st db 186,'                   ���������������������������������Ŀ  ',13,10,0
_r1 db 186,'                   �  320x200  EGA/CGA 256 梥⮢  � �  ',13,10,0
_r2 db 186,'                   �  640x480  VGA 16 梥⮢       � �  ',13,10,0
_rs db 186,'                   �  ????x????@??  SVGA VESA      � �  ',13,10,0
_bt db 186,'                   �����������������������������������  ',13,10,0


remark1 db      "���祭�� �� 㬮�砭�� ��࠭� ��� 㤮��⢠ ����設�⢠, �� �� ���.",0
remark2 db      "�᫨ � ��� LCD-������, �⪫��� VRR � �㭪� [c] - �� ��� �� �㦥�.",0
remark3 db      "�᫨ � ��� �� ��㧨��� ��⥬�, ���஡�� �⪫���� �㭪� [b].",0
remarks dw      remark1, remark2, remark3
num_remarks = 3
