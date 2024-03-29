;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 983 $


align 4
proc alloc_page

           pushfd
           cli
           push ebx
           mov ebx, [page_start]
           mov ecx, [page_end]
.l1:
           bsf eax,[ebx];
           jnz .found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           pop ebx
           popfd
           xor eax,eax
           ret
.found:
           btr [ebx], eax
           mov [page_start],ebx
           sub ebx, sys_pgmap
           lea eax, [eax+ebx*8]
           shl eax, 12
           dec [pg_data.pages_free]
           pop ebx
           popfd
           ret
endp

align 4
proc alloc_pages stdcall, count:dword
           pushfd
           push ebx
           push edi
           cli
           mov eax, [count]
           add eax, 7
           shr eax, 3
           mov [count], eax
           cmp eax, [pg_data.pages_free]
           ja .fail

           mov ecx, [page_start]
           mov ebx, [page_end]
.find:
           mov edx, [count]
           mov edi, ecx
.match:
           cmp byte [ecx], 0xFF
           jne .next
           dec edx
           jz .ok
           inc ecx
           cmp ecx,ebx
           jb .match
.fail:
           xor eax, eax
           pop edi
           pop ebx
           popfd
           ret
.next:
           inc ecx
           cmp ecx, ebx
           jb .find
           pop edi
           pop ebx
           popfd
           xor eax, eax
           ret
.ok:
           sub ecx, edi
           inc ecx
           push esi
           mov esi, edi
           xor eax, eax
           rep stosb
           sub esi, sys_pgmap
           shl esi, 3+12
           mov eax, esi
           mov ebx, [count]
           shl ebx, 3
           sub [pg_data.pages_free], ebx
           pop esi
           pop edi
           pop ebx
           popfd
           ret
endp

align 4
proc map_page stdcall,lin_addr:dword,phis_addr:dword,flags:dword
           push ebx
           mov eax, [phis_addr]
           and eax, not 0xFFF
           or eax, [flags]
           mov ebx, [lin_addr]
           shr ebx, 12
           mov [page_tabs+ebx*4], eax
           mov eax, [lin_addr]
           invlpg [eax]
           pop ebx
           ret
endp

align 4
map_space:    ;not implemented


           ret


align 4
proc free_page
;arg:  eax  page address
           pushfd
           cli
           shr eax, 12                        ;page index
           bts dword [sys_pgmap], eax         ;that's all!
           cmc
           adc [pg_data.pages_free], 0
           shr eax, 3
           and eax, not 3                     ;dword offset from page_map
           add eax, sys_pgmap
           cmp [page_start], eax
           ja @f
           popfd
           ret
@@:
           mov [page_start], eax
           popfd
           ret
endp

proc map_io_mem stdcall, base:dword, size:dword, flags:dword

           push ebx
           push edi
           mov eax, [size]
           add eax, 4095
           and eax, -4096
           mov [size], eax
           stdcall alloc_kernel_space, eax
           test eax, eax
           jz .fail
           push eax

           mov edi, 0x1000
           mov ebx, eax
           mov ecx,[size]
           mov edx, [base]
           shr eax, 12
           shr ecx, 12
           and edx, -4096
           or edx, [flags]
@@:
           mov [page_tabs+eax*4], edx
          ; push eax
          ; invlpg [ebx]
          ; pop eax
           inc eax
           add ebx, edi
           add edx, edi
           loop @B

           pop eax
           mov edx, [base]
           and edx, 4095
           add eax, edx
.fail:
           pop edi
           pop ebx
           ret
endp

; param
;  eax= page base + page flags
;  ebx= linear address
;  ecx= count

align 4
commit_pages:
           push edi
           test ecx, ecx
           jz .fail

           mov edi, ebx
           mov ebx, pg_data.pg_mutex
           call wait_mutex      ;ebx

           mov edx, 0x1000
           mov ebx, edi
           shr ebx, 12
@@:
           mov [page_tabs+ebx*4], eax
          ; push eax
          ; invlpg [edi]
          ; pop eax
           add edi, edx
           add eax, edx
           inc ebx
           dec ecx
           jnz @B
           mov [pg_data.pg_mutex],ecx
.fail:
           pop edi
           ret


; param
;  eax= base
;  ecx= count

align 4
release_pages:

           pushad
           mov ebx, pg_data.pg_mutex
           call wait_mutex      ;ebx

           mov esi, eax
           mov edi, eax

           shr esi, 10
           add esi, page_tabs

           mov ebp, [pg_data.pages_free]
           mov ebx, [page_start]
           mov edx, sys_pgmap
@@:
           xor eax, eax
           xchg eax, [esi]
           push eax
           invlpg [edi]
           pop eax

           test eax, 1
           jz .next

           shr eax, 12
           bts [edx], eax
           cmc
           adc ebp, 0
           shr eax, 3
           and eax, -4
           add eax, edx
           cmp eax, ebx
           jae .next

           mov ebx, eax
.next:
           add edi, 0x1000
           add esi, 4
           dec ecx
           jnz @B
           mov [pg_data.pages_free], ebp
           and [pg_data.pg_mutex],0
           popad
           ret

; param
;  eax= base
;  ecx= count

align 4
unmap_pages:

           push edi

           mov edi, eax
           mov edx, eax

           shr edi, 10
           add edi, page_tabs

           xor eax, eax
@@:
           stosd
           invlpg [edx]
           add edx, 0x1000
           loop @b

           pop edi
           ret


align 4
proc map_page_table stdcall, lin_addr:dword, phis_addr:dword
           push ebx
           mov ebx, [lin_addr]
           shr ebx, 22
           mov eax, [phis_addr]
           and eax, not 0xFFF
           or eax, PG_UW          ;+PG_NOCACHE
           mov dword [master_tab+ebx*4], eax
           mov eax, [lin_addr]
           shr eax, 10
           add eax, page_tabs
           invlpg [eax]
           pop ebx
           ret
endp

align 4
proc init_LFB
           locals
             pg_count dd ?
           endl

           cmp dword [LFBAddress], -1
           jne @f
           mov [BOOT_VAR+0x901c],byte 2
           stdcall kernel_alloc, 0x280000
           mov [LFBAddress], eax
           ret
@@:
           test [SCR_MODE],word 0100000000000000b
           jnz @f
           mov [BOOT_VAR+0x901c],byte 2
           ret
@@:
           call init_mtrr

           mov edx, LFB_BASE
           mov esi, [LFBAddress]
           mov edi, 0x00800000
           mov dword [exp_lfb+4], edx

           shr edi, 12
           mov [pg_count], edi
           shr edi, 10

           bt [cpu_caps], CAPS_PSE
           jnc .map_page_tables
           or esi, PG_LARGE+PG_UW
           mov edx, sys_pgdir+(LFB_BASE shr 20)
@@:
           mov [edx], esi
           add edx, 4
           add esi, 0x00400000
           dec edi
           jnz @B

           bt [cpu_caps], CAPS_PGE
           jnc @F
           or dword [sys_pgdir+(LFB_BASE shr 20)], PG_GLOBAL
@@:
           mov dword [LFBAddress], LFB_BASE
           mov eax, cr3       ;flush TLB
           mov cr3, eax
           ret

.map_page_tables:

@@:
           call alloc_page
           stdcall map_page_table, edx, eax
           add edx, 0x00400000
           dec edi
           jnz @B

           mov eax, [LFBAddress]
           mov edi, page_tabs + (LFB_BASE shr 10)
           or eax, PG_UW
           mov ecx, [pg_count]
           cld
@@:
           stosd
           add eax, 0x1000
           dec ecx
           jnz @B

           mov dword [LFBAddress], LFB_BASE
           mov eax, cr3       ;flush TLB
           mov cr3, eax

           ret
endp

align 4
proc new_mem_resize stdcall, new_size:dword

           mov ebx, pg_data.pg_mutex
           call wait_mutex    ;ebx

           mov edi, [new_size]
           add edi,4095
           and edi,not 4095
           mov [new_size], edi

           mov edx,[current_slot]
           cmp [edx+APPDATA.heap_base],0
           jne .exit

           mov esi, [edx+APPDATA.mem_size]
           add esi, 4095
           and esi, not 4095

           cmp edi, esi
           jae .expand

           shr edi, 12
           shr esi, 12
@@:
           mov eax, [app_page_tabs+edi*4]
           test eax, 1
           jz .next
           mov dword [app_page_tabs+edi*4], 2
           mov ebx, edi
           shl ebx, 12
           push eax
           invlpg [ebx]
           pop eax
           call free_page

.next:     add edi, 1
           cmp edi, esi
           jb @B

.update_size:
           mov     ebx, [new_size]
           call    update_mem_size

           xor eax, eax
           dec [pg_data.pg_mutex]
           ret
.expand:

           push esi
           push edi

           add edi, 0x3FFFFF
           and edi, not(0x3FFFFF)
           add esi, 0x3FFFFF
           and esi, not(0x3FFFFF)

           cmp esi, edi
           jae .grow

           xchg esi, edi

@@:
           call alloc_page
           test eax, eax
           jz .exit

           stdcall map_page_table, edi, eax

           push edi
           shr edi, 10
           add edi, page_tabs
           mov ecx, 1024
           xor eax, eax
           cld
           rep stosd
           pop edi

           add edi, 0x00400000
           cmp edi, esi
           jb @B
.grow:
           pop edi
           pop esi
@@:
           call alloc_page
           test eax, eax
           jz .exit
           stdcall map_page,esi,eax,dword PG_UW

           push edi
           mov edi, esi
           xor eax, eax
           mov ecx, 1024
           cld
           rep stosd
           pop edi

           add esi, 0x1000
           cmp esi, edi
           jb  @B

           jmp .update_size
.exit:
           xor eax, eax
           inc eax
           dec [pg_data.pg_mutex]
           ret
endp

update_mem_size:
; in: edx = slot base
;     ebx = new memory size
; destroys eax,ecx,edx

           mov    [APPDATA.mem_size+edx],ebx
;search threads and update
;application memory size infomation
           mov    ecx,[APPDATA.dir_table+edx]
           mov    eax,2

.search_threads:
;eax = current slot
;ebx = new memory size
;ecx = page directory
           cmp    eax,[TASK_COUNT]
           jg     .search_threads_end
           mov    edx,eax
           shl    edx,5
           cmp    word [CURRENT_TASK+edx+TASKDATA.state],9 ;if slot empty?
           jz     .search_threads_next
           shl    edx,3
           cmp    [SLOT_BASE+edx+APPDATA.dir_table],ecx     ;if it is our thread?
           jnz    .search_threads_next
           mov    [SLOT_BASE+edx+APPDATA.mem_size],ebx     ;update memory size
.search_threads_next:
           inc    eax
           jmp    .search_threads
.search_threads_end:
           ret

; param
;  eax= linear address
;
; retval
;  eax= phisical page address

align 4
get_pg_addr:
           shr eax, 12
           mov eax, [page_tabs+eax*4]
           and eax, 0xFFFFF000
           ret


align 4
proc page_fault_handler

        test    byte [esp+12+2], 2
        jnz     v86_page_fault

        .err_code equ ebp+32
        .err_addr equ ebp-4

           pushad
           mov ebp, esp
           mov eax, cr2
           push eax

           mov ax, app_data
           mov ds, ax
           mov es, ax

           inc [pg_data.pages_faults]

;     push eax
;     push edx
;     mov edx, 0x400   ;bochs
;     mov al,0xff      ;bochs
;     out dx, al       ;bochs
;     pop edx
;     pop eax

           mov ebx, [.err_addr]
           mov eax, [.err_code]

           cmp ebx, OS_BASE
           jb .user_space      ;�������� � ������ ���������� ;

           cmp ebx, page_tabs
           jb .kernel_space    ;�������� � ������ ����

           cmp ebx, kernel_tabs
           jb .alloc;.app_tabs ;������� ������� ���������� ;
                               ;������ �������� ����

           cmp ebx, LFB_BASE
           jb .core_tabs       ;������� ������� ����
                               ;������
.lfb:
                               ;������� LFB
                               ;������
           jmp .fail

align 4
.user_space:
           test eax, PG_MAP
           jnz .err_access     ;�������� ������������
                               ;������ ������� ?

           shr ebx, 12
           mov ecx, ebx
           shr ecx, 10
           mov edx, [master_tab+ecx*4]
           test edx, PG_MAP
           jz .fail            ;������� ������� �� �������
                               ;�������� ����� � ���������

           mov eax, [page_tabs+ebx*4]
           test eax, 2
           jz .fail            ;����� �� �������������� ��� ;
                               ;�������������. ������
.alloc:
           call alloc_page
           test eax, eax
           jz .fail

           stdcall map_page,[ebp-4],eax,dword PG_UW

           mov edi, [ebp-4]
           and edi, 0xFFFFF000
           mov ecx, 1024
           xor eax, eax
           cld
           rep stosd
.exit:
           mov esp, ebp
           popad
           add esp, 4
           iretd

.err_access:
;������� �� ����������
           jmp .fail

.kernel_space:
           test eax, PG_MAP
           jz .fail        ;�������� �� ������������

           test eax, 4     ;U/S
           jnz .fail       ;���������� ���������� � ������
                           ;����
           test eax, 8
           jnz .fail       ;���������� ����������������� ���
                           ;� �������� �������. ��������� � P4/Xeon

;������� ������ � ���������� �������� ����

           cmp ebx, tss._io_map_0
           jb .fail

           cmp ebx, tss._io_map_0+8192
           jae .fail

; io permission map
; copy-on-write protection

           call alloc_page
           test eax, eax
           jz .fail

           push eax
           stdcall map_page,[ebp-4],eax,dword PG_SW
           pop eax
           mov edi, [.err_addr]
           and edi, -4096
           lea esi, [edi+(not tss._io_map_0)+1]; -tss._io_map_0

           mov ebx, esi
           shr ebx, 12
           mov edx, [current_slot]
           or eax, PG_SW
           mov [edx+APPDATA.io_map+ebx*4], eax

           add esi, [default_io_map]
           mov ecx, 4096/4
           cld
           rep movsd
           jmp .exit


;�� ������������. ������

.core_tabs:
.fail:
           mov esp, ebp
           popad
           add esp, 4

;           iretd

           save_ring3_context     ;debugger support

           mov bl, 14
           jmp exc_c
           iretd
endp

align 4
proc map_mem stdcall, lin_addr:dword,pdir:dword,\
                      ofs:dword,buf_size:dword
           mov eax, [buf_size]
           test eax, eax
           jz .exit

           mov eax, [pdir]
           and eax, 0xFFFFF000

           stdcall map_page,[ipc_pdir],eax,PG_UW
           mov ebx, [ofs]
           shr ebx, 22
           mov esi, [ipc_pdir]
           mov edi, [ipc_ptab]
           mov eax, [esi+ebx*4]
           and eax, 0xFFFFF000
           jz .exit
           stdcall map_page,edi,eax,PG_UW
;           inc ebx
;           add edi, 0x1000
;           mov eax, [esi+ebx*4]
;           test eax, eax
;           jz @f
;          and eax, 0xFFFFF000
;           stdcall map_page, edi, eax

@@:        mov edi, [lin_addr]
           and edi, 0xFFFFF000
           mov ecx, [buf_size]
           add ecx, 4095
           shr ecx, 12
           inc ecx

           mov edx, [ofs]
           shr edx, 12
           and edx, 0x3FF
           mov esi, [ipc_ptab]

.map:      mov eax, [esi+edx*4]
           and eax, 0xFFFFF000
           jz  .exit
           stdcall map_page,edi,eax,PG_UW
           dec ecx
           jz  .exit
           add edi, 0x1000
           inc edx
           cmp edx, 0x400
           jnz .map
           inc ebx
           mov eax, [ipc_pdir]
           mov eax, [eax+ebx*4]
           and eax, 0xFFFFF000
           jz  .exit
           stdcall map_page,esi,eax,PG_UW
           xor edx, edx
           jmp .map

.exit:
           ret
endp

align 4
proc map_memEx stdcall, lin_addr:dword,pdir:dword,\
                        ofs:dword,buf_size:dword
           mov eax, [buf_size]
           test eax, eax
           jz .exit

           mov eax, [pdir]
           and eax, 0xFFFFF000

           stdcall map_page,[proc_mem_pdir],eax,dword PG_UW
           mov ebx, [ofs]
           shr ebx, 22
           mov esi, [proc_mem_pdir]
           mov edi, [proc_mem_tab]
           mov eax, [esi+ebx*4]
           and eax, 0xFFFFF000
           test eax, eax
           jz .exit
           stdcall map_page,edi,eax,dword PG_UW

@@:        mov edi, [lin_addr]
           and edi, 0xFFFFF000
           mov ecx, [buf_size]
           add ecx, 4095
           shr ecx, 12
           inc ecx

           mov edx, [ofs]
           shr edx, 12
           and edx, 0x3FF
           mov esi, [proc_mem_tab]

.map:      mov eax, [esi+edx*4]
;           and eax, 0xFFFFF000
;           test eax, eax
;           jz .exit
           stdcall map_page,edi,eax,dword PG_UW
           add edi, 0x1000
           inc edx
           dec ecx
           jnz .map
.exit:
           ret
endp




sys_IPC:
;input:
;  eax=1 - set ipc buffer area
;    ebx=address of buffer
;    ecx=size of buffer
;  eax=2 - send message
;    ebx=PID
;    ecx=address of message
;    edx=size of message

           cmp  eax,1
           jne @f
           call set_ipc_buff
           mov [esp+36], eax
           ret
@@:
           cmp eax, 2
           jne @f
           stdcall sys_ipc_send, ebx, ecx, edx
           mov [esp+36], eax
           ret
@@:
           xor eax, eax
           not eax
           mov [esp+36], eax
           ret

align 4
proc set_ipc_buff

           mov  eax,[current_slot]
           pushf
           cli
           mov  [eax+APPDATA.ipc_start],ebx     ;set fields in extended information area
           mov  [eax+APPDATA.ipc_size],ecx

           add ecx, ebx
           add ecx, 4095
           and ecx, not 4095

.touch:    mov eax, [ebx]
           add ebx, 0x1000
           cmp ebx, ecx
           jb  .touch

           popf
           xor eax, eax
           ret
endp

proc sys_ipc_send stdcall, PID:dword, msg_addr:dword, msg_size:dword
           locals
             dst_slot   dd ?
             dst_offset dd ?
             buf_size   dd ?
             used_buf   dd ?
           endl

           pushf
           cli

           mov  eax, [PID]
           call pid_to_slot
           test eax,eax
           jz   .no_pid

           mov [dst_slot], eax
           shl  eax,8
           mov  edi,[eax+SLOT_BASE+0xa0]  ;is ipc area defined?
           test edi,edi
           jz   .no_ipc_area

           mov ebx, edi
           and ebx, 0xFFF
           mov [dst_offset], ebx

           mov esi, [eax+SLOT_BASE+0xa4]
           mov [buf_size], esi

           mov ecx, [ipc_tmp]
           cmp esi, 0x40000-0x1000 ; size of [ipc_tmp] minus one page
           jbe @f
           push eax esi edi
           add esi,0x1000
           stdcall alloc_kernel_space,esi
           mov ecx, eax
           pop edi esi eax
@@:
           mov [used_buf], ecx
           stdcall map_mem, ecx, [SLOT_BASE+eax+0xB8],\
                             edi, esi

           mov edi, [dst_offset]
           add edi, [used_buf]
           cmp dword [edi], 0
           jnz  .ipc_blocked          ;if dword [buffer]<>0 - ipc blocked now

           mov edx, dword [edi+4]
           lea ebx, [edx+8]
           add ebx, [msg_size]
           cmp ebx, [buf_size]
           ja .buffer_overflow         ;esi<0 - not enough memory in buffer

           mov dword [edi+4], ebx
           mov eax,[TASK_BASE]
           mov eax, [eax+0x04]         ;eax - our PID
           add edi, edx
           mov [edi], eax
           mov ecx, [msg_size]

           mov [edi+4], ecx
           add edi, 8
           mov esi, [msg_addr]
       ;    add esi, new_app_base
           cld
           rep movsb

           mov ebx, [ipc_tmp]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [page_tabs+ebx*4], eax
           invlpg [edx]

           mov ebx, [ipc_pdir]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [page_tabs+ebx*4], eax
           invlpg [edx]

           mov ebx, [ipc_ptab]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [page_tabs+ebx*4], eax
           invlpg [edx]

           mov  eax, [dst_slot]
           shl eax, 8
           or   [eax+SLOT_BASE+0xA8],dword 0x40
           cmp  dword [check_idle_semaphore],20
           jge  .ipc_no_cis

           mov  dword [check_idle_semaphore],5
.ipc_no_cis:
           push 0
           jmp .ret
.no_pid:
           popf
           mov eax, 4
           ret
.no_ipc_area:
           popf
           xor eax, eax
           inc eax
           ret
.ipc_blocked:
           push 2
           jmp .ret
.buffer_overflow:
           push 3
.ret:
           mov eax, [used_buf]
           cmp eax, [ipc_tmp]
           jz @f
           stdcall free_kernel_space,eax
@@:
           pop eax
           popf
           ret
endp

align 4
sysfn_meminfo:

        ;   add ebx, new_app_base
           cmp ebx, OS_BASE
           jae .fail

           mov eax, [pg_data.pages_count]
           mov [ebx], eax
           shl eax, 12
           mov [esp+36], eax
           mov ecx, [pg_data.pages_free]
           mov [ebx+4], ecx
           mov edx, [pg_data.pages_faults]
           mov [ebx+8], edx
           mov esi, [heap_size]
           mov [ebx+12], esi
           mov edi, [heap_free]
           mov [ebx+16], edi
           mov eax, [heap_blocks]
           mov [ebx+20], eax
           mov ecx, [free_blocks]
           mov [ebx+24], ecx
           ret
.fail:
           mov dword [esp+36], -1
           ret

align 4
f68call:
           dd f68.11
           dd f68.12
           dd f68.13
           dd f68.14
           dd f68.15
           dd f68.16
           dd f68.17
           dd f68.18
           dd f68.19
           dd f68.20
           dd f68.21
           dd f68.22
           dd f68.23

align 4
f68:
           cmp  eax,4
           jle  sys_sheduler

           cmp eax, 11
           jb .fail

           cmp eax, 23
           ja .fail

           jmp dword [f68call+eax*4-11*4]
.11:
           call init_heap
           mov [esp+36], eax
           ret
.12:
           stdcall user_alloc, ebx
           mov [esp+36], eax
           ret
.13:
           stdcall user_free, ebx
           mov [esp+36], eax
           ret
.14:
           cmp ebx, OS_BASE
           jae .fail
           stdcall get_event_ex, ebx, ecx
           mov [esp+36], eax
           ret
.15:
           mov ecx, [current_slot]
           mov eax, [ecx+APPDATA.fpu_handler]
           mov [ecx+APPDATA.fpu_handler], ebx
           mov [esp+36], eax
           ret
.16:
           test ebx, ebx
           jz .fail
           cmp ebx, OS_BASE
           jae .fail
           stdcall get_service, ebx
           mov [esp+36], eax
           ret
.17:
           call srv_handlerEx   ;ebx
           mov [esp+36], eax
           ret
.18:
           mov ecx, [current_slot]
           mov eax, [ecx+APPDATA.sse_handler]
           mov [ecx+APPDATA.sse_handler], ebx
           mov [esp+36], eax
           ret
.19:
           cmp ebx, OS_BASE
           jae .fail
           stdcall load_library, ebx
           mov [esp+36], eax
           ret
.20:
           mov     eax, ecx
           call    user_realloc
           mov     [esp+36], eax
           ret
.21:
           cmp ebx, OS_BASE
           jae .fail

           stdcall load_PE, ebx

           test eax, eax
           jz @F

           mov esi, eax
           stdcall eax, DRV_ENTRY

           test eax, eax
           jz @F

           mov [eax+SRV.entry], esi

@@:
           mov [esp+36], eax
           ret
.22:
           cmp ebx, OS_BASE
           jae .fail

           stdcall shmem_open, ebx, ecx, edx
           mov [esp+28], edx
           mov [esp+36], eax
           ret

.23:
           cmp ebx, OS_BASE
           jae .fail

           stdcall shmem_close, ebx
           mov [esp+36], eax
           ret

.fail:
           xor eax, eax
           mov [esp+36], eax
           ret

align 4
proc load_pe_driver stdcall, file:dword

           stdcall load_PE, [file]
           test eax, eax
           jz .fail

           mov esi, eax
           stdcall eax, DRV_ENTRY
           test eax, eax
           jz .fail

           mov [eax+SRV.entry], esi
           ret

.fail:
           xor eax, eax
           ret
endp


align 4
proc init_mtrr

           cmp [BOOT_VAR+0x901c],byte 2
           je  .exit

           bt [cpu_caps], CAPS_MTRR
           jnc .exit

           mov eax, cr0
           or eax, 0x60000000   ;disable caching
           mov cr0, eax
           wbinvd               ;invalidate cache

           mov ecx, 0x2FF
           rdmsr                ;
           push eax

           xor edx, edx
           xor eax, eax
           mov ecx, 0x2FF
           wrmsr                ;disable all MTRR

           mov eax, [MEM_AMOUNT]
; round eax up to next power of 2
           dec eax
           bsr ecx, eax
           mov eax, 2
           shl eax, cl
           stdcall set_mtrr, edx,edx,eax,MEM_WB
           stdcall set_mtrr, 1,[LFBAddress],[LFBSize],MEM_WC
           xor edx, edx
           xor eax, eax
           mov ecx, 0x204
           mov ebx, 6
@@:
           wrmsr                ;disable unused MTRR
           inc ecx
           wrmsr
           inc ecx
           dec ebx
           jnz @b

           wbinvd               ;again invalidate

           pop eax
           or eax, 0x800        ;set default memtype to UC
           and al, 0xF0
           mov ecx, 0x2FF
           wrmsr                ;and enable MTRR

           mov eax, cr0
           and eax, not 0x60000000
           mov cr0, eax         ; enable caching
.exit:
           ret
endp

align 4
proc set_mtrr stdcall, reg:dword,base:dword,size:dword,mem_type:dword

           xor edx, edx
           mov eax, [base]
           or eax, [mem_type]
           mov ecx, [reg]
           lea ecx, [0x200+ecx*2]
           wrmsr

           mov ebx, [size]
           dec ebx
           mov eax, 0xFFFFFFFF
           mov edx, 0x0000000F
           sub eax, ebx
           sbb edx, 0
           or eax, 0x800
           inc ecx
           wrmsr
           ret
endp

align 4
proc stall stdcall, delay:dword
           push ecx
           push edx
           push ebx
           push eax

           mov eax, [delay]
           mul [stall_mcs]
           mov ebx, eax       ;low
           mov ecx, edx       ;high
           rdtsc
           add ebx, eax
           adc ecx,edx
@@:
           rdtsc
           sub eax, ebx
           sbb edx, ecx
           jb @B

           pop eax
           pop ebx
           pop edx
           pop ecx
           ret
endp

align 4
proc create_ring_buffer stdcall, size:dword, flags:dword
           locals
             buf_ptr  dd ?
           endl

           mov eax, [size]
           test eax, eax
           jz .fail

           add eax, eax
           stdcall alloc_kernel_space, eax
           test eax, eax
           jz .fail

           push ebx

           mov [buf_ptr], eax

           mov ebx, [size]
           shr ebx, 12
           push ebx

           stdcall alloc_pages, ebx
           pop ecx

           test eax, eax
           jz .mm_fail

           push edi

           or eax, [flags]
           mov edi, [buf_ptr]
           mov ebx, [buf_ptr]
           mov edx, ecx
           shl edx, 2
           shr edi, 10
@@:
           mov [page_tabs+edi], eax
           mov [page_tabs+edi+edx], eax
           invlpg [ebx]
           invlpg [ebx+0x10000]
           add eax, 0x1000
           add ebx, 0x1000
           add edi, 4
           dec ecx
           jnz @B

           mov eax, [buf_ptr]
           pop edi
           pop ebx
           ret
.mm_fail:
           stdcall free_kernel_space, [buf_ptr]
           xor eax, eax
           pop ebx
.fail:
           ret
endp



