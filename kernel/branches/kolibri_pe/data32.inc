;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 2971 $


keymap:

     db   '6',27
     db   '1234567890-=',8,9
     db   'qwertyuiop[]',13
     db   '~asdfghjkl;',39,96,0,'\zxcvbnm,./',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

keymap_shift:
     db   '6',27
     db   '!@#$%^&*()_+',8,9
     db   'QWERTYUIOP{}',13
     db   '~ASDFGHJKL:"~',0,'|ZXCVBNM<>?',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


keymap_alt:
     db   ' ',27
     db   ' @ $  {[]}\ ',8,9
     db   '            ',13
     db   '             ',0,'           ',0,'4',0,' '
     db   '             ',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'ABCD',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'



  boot_memdetect    db	 'Determining amount of memory',0
  boot_fonts	    db	 'Fonts loaded',0
  boot_tss	    db	 'Setting TSSs',0
  boot_cpuid	    db	 'Reading CPUIDs',0
  boot_devices	    db	 'Detecting devices',0
  boot_timer	    db	 'Setting timer',0
  boot_irqs	    db	 'Reprogramming IRQs',0
  boot_setmouse     db	 'Setting mouse',0
  boot_windefs	    db	 'Setting window defaults',0
  boot_bgr	    db	 'Calculating background',0
  boot_resirqports  db	 'Reserving IRQs & ports',0
  boot_setrports    db	 'Setting addresses for IRQs',0
  boot_setostask    db	 'Setting OS task',0
  boot_allirqs	    db	 'Unmasking all IRQs',0
  boot_tsc	    db	 'Reading TSC',0
  boot_cpufreq      db   'CPU frequency is ','    ',' MHz',0
  boot_pal_ega	    db	 'Setting EGA/CGA 320x200 palette',0
  boot_pal_vga	    db	 'Setting VGA 640x480 palette',0
  boot_failed	    db	 'Failed to start first app',0
  boot_mtrr	    db	 'Setting MTRR',0
;if preboot_blogesc
;  boot_tasking      db   'All set - press ESC to start',0
;end if

;new_process_loading db 'K : New Process - loading',13,10,0
;new_process_running db 'K : New Process - done',13,10,0
start_not_enough_memory db 'K : New Process - not enough memory',13,10,0

msg_unresolved db 'unresolved ',0
msg_module     db 'in module ',0
msg_version    db 'incompatible driver version',13,10,0
msg_www        db 'please visit www.kolibrios.org',13,10,0
msg_CR	       db  13,10,0
aSis           db 'SIS',0

intel_str      db "GenuineIntel",0
AMD_str        db "AuthenticAMD",0

;szSound        db 'SOUND',0
;szInfinity     db 'INFINITY',0
szHwMouse	db 'ATI2D',0
szPS2MDriver	db 'PS2MOUSE',0
szCOM_MDriver	db 'COM_MOUSE',0
szUSB		db 'USB',0
szAtiHW         db '/rd/1/drivers/ati2d.drv',0

szSTART        db 'START',0
szEXPORTS      db 'EXPORTS',0
sz_EXPORTS     db '_EXPORTS',0

szIMPORTS      db 'IMPORTS',0

read_firstapp  db '/sys/'
firstapp       db  '@DESKTOP',0

char    db '/rd/1/FONTS/CHAR.MT',0
char2   db '/rd/1/FONTS/CHAR2.MT',0

bootpath       db  '/KOLIBRI    '
bootpath2      db  0
vmode	       db  '/sys/drivers/VMODE.MDR',0
vrr_m	       db  'VRR_M',0
kernel_file    db  'KERNEL  MNT'


align 4

shmem_list:
 .bk           dd shmem_list
 .fd           dd shmem_list

; supported videomodes
mode_1280_1024_32:
  dw 1280,1024,32,60
mode_1280_1024_24:
  dw 1280,1024,24,60
mode_1024_768_32:
  dw 1024,768,32,60
mode_1024_768_24:
  dw 1024,768,24,60
mode_800_600_32:
  dw 800,600,32,60
mode_800_600_24:
  dw 800,600,24,60
mode_640_480_32:
  dw 640,480,32,60
mode_640_480_24:
  dw 640,480,24,60
mode_640_480_16:
  dw 640,480,16,60
mode_320_240_8:
  dw 320,240,8,60

; mike.dld {
db 0
dd servetable-0x10000
draw_line	dd __sys_draw_line
disable_mouse	dd __sys_disable_mouse
draw_pointer	dd __sys_draw_pointer
;//mike.dld, 2006-08-02 [
;drawbar         dd __sys_drawbar
drawbar 	dd __sys_drawbar.forced
;//mike.dld, 2006-08-02 ]
putpixel	dd __sys_putpixel
; } mike.dld


align 4
keyboard   dd 1
syslang    dd 1

boot_y	   dd 10

pci_bios_entry	dd 0
           dw sel_pci_code

if __DEBUG__ eq 1
  include_debug_strings
end if

IncludeIGlobals

align 16
_gdts:

    dw     (gdte - _gdts -1)
    dd     _gdts
	dw     0

tss_l:
	dw     TSS_SIZE-1
	dw     tss and 0xFFFF
	db     (tss shr 16) and 0xFF
	db     10001001b
	dw     (tss shr 16) and 0xFF00

; Attention! Do not change the order of the first four selectors. They are used in Fast System Call
; must be : os_code, os_data, app_code, app_data, ....

os_code_l:                            ;0x10
	dw     0xffff
	dw     0x0000
	db     0x00
	dw     11011111b *256 +10011010b
	db     0x00

os_stacK_l:                           ;0x18
	dw     0xffff
	dw     0x0000
	db     0x00
	dw     11011111b *256 +10010010b
	db     0x00

app_code_l:                           ;0x20
    dw     0xFFFF
    dw     0
    db     0
    db     cpl3
    dw     G32+D32+0xF;

app_data_l:                           ;0x28
    dw     0xFFFF
    dw     0
    db     0
    db     drw3
    dw     G32+D32+0xF;

srv_code_l:                           ;0x30
    dw     0xFFFF
    dw     0
    db     0
    db     cpl1
    dw     G32+D32+0xF;

srv_stack_l:                          ;0x38
    dw     0xFFFF
    dw     0
    db     0
    db     drw1
    dw     G32+D32+0xF;

; ------------- PCI BIOS ------------------

pci_code_32:
	dw 0	     ;lim  0-15
	dw 0	     ;base 0-15
	db 0	     ;base 16-23
	db cpl0      ;type
	db D32	     ;lim 16-19+props
	db 0	     ;base 24-31

pci_data_32:
	dw 0	     ;lim  0-15
	dw 0	     ;base 0-15
	db 0	     ;base 16-23
	db dpl0      ;type
	db D32	     ;lim 16-19+props
	db 0	     ;base 24-31

; --------------- APM ---------------------
apm_code_32:
	dw     0x0f	   ; limit 64kb
	db     0, 0, 0
	dw     11010000b *256 +10011010b
	db     0x00
apm_code_16:
	dw     0x0f
	db     0, 0, 0
	dw     10010000b *256 +10011010b
	db     0x00
apm_data_16:
	dw     0x0f
	db     0, 0, 0
	dw     10010000b *256 +10010010b
	db     0x00
; -----------------------------------------

graph_data_l:

	dw     0x7ff
	dw     0x0000
	db     0x00
	dw     11010000b *256 +11110010b
	db     0x00

code_16_l:
    dw 0xFFFF
    dw 0
    db 1
    db cpl0
    dw 0

endofcode:
gdte:

section '.bss' data writeable align 4096

align 4096

SLOT_BASE:
                rb 64*1024
DRIVE_DATA:
                rb 64*1024
RESERVED_PORTS:
                rb 64*1024
FLOPPY_BUFF:
                rb 16*1024

BUTTON_INFO:
                rb 16*1024
BgrAuxTable:
                rb 32*1024
skin_data:
                rb 32*1024

IDE_DMA:        rb 32*1024
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
window_data:
                rb 8192

CURRENT_TASK:
_current_task:
                rb 8192
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
draw_data:
                rb 4096
CDDataBuf:
                rb 4096

cur_saved_data  rb 4096

_sys_pdbr       rd 1024


fpu_data:
                rb 512

WIN_STACK:
                rb 0x400
WIN_POS:
                rb 0x800

idts            rq 0x42

; device irq owners
irq_owner	  rd 16 ; process id

; on irq read ports

irq00read	  rd 16
irq01read	  rd 16
irq02read	  rd 16
irq03read	  rd 16
irq04read	  rd 16
irq05read	  rd 16
irq06read	  rd 16
irq07read	  rd 16
irq08read	  rd 16
irq09read	  rd 16
irq10read	  rd 16
irq11read	  rd 16
irq12read	  rd 16
irq13read	  rd 16
irq14read	  rd 16
irq15read	  rd 16

irq_tab 	  rd 16

FONT_I        rb 2304    ;16*144
FONT_II       rb 2560    ;16*160

RAMDISK_FAT:  rb 2856*2 +16 ;16*357
FLOPPY_FAT:   rb 2856*2 +16

_z_core           rd 52

mem_block_map	  rb 512

event_map         rb 64
mem_block_list	  rd 64
large_block_list  rd 31
mem_block_mask	  rd 2
large_block_mask  rd 1


_boot_mbi         rd 1

_mem_amount       rd 1
_rd_base          rd 1
_rd_fat           rd 1 ;RAMDISK + 512
_rd_fat_end       rd 1 ;RAMDISK + 512 + 4278
_rd_root          rd 1 ;RAMDISK + 512*18 + 512
_rd_root_end      rd 1 ;RAMDISK + 512*33

heap_mutex        rd 1
heap_size         rd 1
heap_free         rd 1
heap_blocks       rd 1
free_blocks       rd 1

mst MEM_STATE

page_start        rd 1
page_end          rd 1
events            rd 1
event_start       rd 1
event_end         rd 1
event_uid         rd 1
sys_page_map      rd 1

srv.fd            rd 1
srv.bk            rd 1

_HwCursorCreate:
create_cursor	  rd 1

_SelectHwCursor:
select_hw_cursor  rd 1

_SetHwCursor:
set_hw_cursor	  rd 1

_HwCursorRestore:
hw_restore        rd 1

def_cursor        rd 1
current_cursor	  rd 1
hw_cursor         rd 1
cur_saved_base	  rd 1

cur.lock          rd 1   ;1 - lock update, 2- hide
cur.left          rd 1   ;cursor clip box
cur.top           rd 1
cur.right         rd 1
cur.bottom        rd 1
cur.w             rd 1
cur.h             rd 1

ipc_tmp           rd 1
ipc_pdir          rd 1
ipc_ptab          rd 1

proc_mem_map	  rd 1
proc_mem_pdir	  rd 1
proc_mem_tab	  rd 1

tmp_task_pdir	  rd 1
tmp_task_ptab	  rd 1

default_io_map	  rd 1


stall_mcs         rd 1

_current_slot:
current_slot	  rd 1

_display_data     rd 1

_current_thread   rd 1
_k_reenter        rd 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

scr_mode          rd 1

LFBAddress        rd 1
LFBSize           rd 1       ;8Mb hardcoded
ScreenBPP         rd 1
BytesPerScanLine  rd 1

put_pixel         rd 1
get_pixel         rd 1

_screen_width     rd 1
_screen_height    rd 1

Screen_Max_X      rd 1
Screen_Max_Y      rd 1

btn_addr          rd 1

redraw_background rd 1
BgrDrawMode       rd 1
BgrDataWidth      rd 1
BgrDataHeight     rd 1

MOUSE_VISIBLE     rd 1
WIN_TEMP_XY       rd 1
MOUSE_X           rw 1
MOUSE_Y           rw 1
mouse_scroll_v    rd 1
mouse_scroll_h    rd 1

BTN_COUNT         rd 1
BTN_BUFF          rd 1

KEY_COUNT         rd 1
KEY_BUFF          rb 128

btn_down          rd 1
x_under           rd 1
y_under           rd 1

dont_switch       rd 1

sys_shutdown      rd 1

bank_rw           rd 1

CPU_FREQ          rd 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; status
hd1_status		    rd 1  ; 0 - free : other - pid
application_table_status    rd 1  ; 0 - free : other - pid

; device addresses
mididp            rd 1
midisp            rd 1

cdbase            rd 1
cdid              rd 1

hdbase            rd 1    ; for boot 0x1f0
hdid              rd 1
hdpos             rd 1    ; for boot 0x1
fat32part         rd 1    ; for boot 0x1
cdpos             rd 1

;CPUID information
cpu_vendor        rd 3
cpu_sign          rd 1
cpu_info          rd 1
cpu_caps          rd 4


pg_data           PG_DATA
heap_test         rd 1

buttontype        rd 1
windowtypechanged rd 1

hd_entries        rd 1     ;unused ? 0xfe10

;* start code - Mario79

mouse_active	      rd 1
mouse_pause           rd 1
MouseTickCounter      rd 1

;* end code - Mario79

img_background	      rd 1
mem_BACKGROUND	      rd 1
wraw_bacground_select rb 1

cache_ide0:
cache_ide0_pointer            rd 1
cache_ide0_size               rd 1   ; not use
cache_ide0_data_pointer       rd 1
cache_ide0_system_data_size   rd 1   ; not use
cache_ide0_appl_data_size     rd 1   ; not use
cache_ide0_system_data	      rd 1
cache_ide0_appl_data	      rd 1
cache_ide0_system_sad_size    rd 1
cache_ide0_appl_sad_size      rd 1
cache_ide0_search_start       rd 1
cache_ide0_appl_search_start  rd 1

cache_ide1:
cache_ide1_pointer            rd 1
cache_ide1_size               rd 1   ; not use
cache_ide1_data_pointer       rd 1
cache_ide1_system_data_size   rd 1   ; not use
cache_ide1_appl_data_size     rd 1   ; not use
cache_ide1_system_data	      rd 1
cache_ide1_appl_data	      rd 1
cache_ide1_system_sad_size    rd 1
cache_ide1_appl_sad_size      rd 1
cache_ide1_search_start       rd 1
cache_ide1_appl_search_start  rd 1

cache_ide2:
cache_ide2_pointer            rd 1
cache_ide2_size               rd 1   ; not use
cache_ide2_data_pointer       rd 1
cache_ide2_system_data_size   rd 1   ; not use
cache_ide2_appl_data_size     rd 1   ; not use
cache_ide2_system_data	      rd 1
cache_ide2_appl_data	      rd 1
cache_ide2_system_sad_size    rd 1
cache_ide2_appl_sad_size      rd 1
cache_ide2_search_start       rd 1
cache_ide2_appl_search_start  rd 1

cache_ide3:
cache_ide3_pointer            rd 1
cache_ide3_size               rd 1   ; not use
cache_ide3_data_pointer       rd 1
cache_ide3_system_data_size   rd 1   ; not use
cache_ide3_appl_data_size     rd 1   ; not use
cache_ide3_system_data	      rd 1
cache_ide3_appl_data	      rd 1
cache_ide3_system_sad_size    rd 1
cache_ide3_appl_sad_size      rd 1
cache_ide3_search_start       rd 1
cache_ide3_appl_search_start  rd 1

debug_step_pointer            rd 1

hdd_appl_data                 rb 1 ; 0 = system cache, 1 - application cache
cd_appl_data                  rb 1 ; 0 = system cache, 1 - application cache

align 4
lba_read_enabled              rd 1  ; 0 = disabled , 1 = enabled
pci_access_enabled            rd 1  ; 0 = disabled , 1 = enabled
timer_ticks_enable            rb 1     ; for cd driver

align 4
NumBiosDisks                  rd 1
BiosDisksData                 rb 200h
BiosDiskCaches                rb 80h*(cache_ide1-cache_ide0)
BiosDiskPartitions            rd 80h

IncludeUGlobals

