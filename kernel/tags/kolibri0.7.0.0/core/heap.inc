$Revision: 465 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

struc MEM_BLOCK
{  .next_block  dd ?
   .prev_block  dd ? ;+4
   .list_fd     dd ? ;+8
   .list_bk     dd ? ;+12
   .base        dd ? ;+16
   .size        dd ? ;+20
   .flags       dd ? ;+24
   .handle      dd ? ;+28
}

MEM_LIST_OFFSET equ  8
FREE_BLOCK      equ  4
USED_BLOCK      equ  8

virtual at 0
  MEM_BLOCK MEM_BLOCK
end virtual

MEM_BLOCK_SIZE equ 8*4

block_next   equ MEM_BLOCK.next_block
block_prev   equ MEM_BLOCK.prev_block
list_fd      equ MEM_BLOCK.list_fd
list_bk      equ MEM_BLOCK.list_bk
block_base   equ MEM_BLOCK.base
block_size   equ MEM_BLOCK.size
block_flags  equ MEM_BLOCK.flags

macro calc_index op
{          shr op, 12
           dec op
           cmp op, 63
           jna @f
           mov op, 63
@@:
}

macro remove_from_list op
{          mov edx, [op+list_fd]
           mov ecx, [op+list_bk]
           test edx, edx
           jz @f
           mov [edx+list_bk], ecx
@@:
           test ecx, ecx
           jz @f
           mov [ecx+list_fd], edx
@@:
           mov [op+list_fd],0
           mov [op+list_bk],0
}

macro remove_from_free op
{
           remove_from_list op

           mov eax, [op+block_size]
           calc_index eax
           cmp [mem_block_list+eax*4], op
           jne @f
           mov [mem_block_list+eax*4], edx
@@:
           cmp [mem_block_list+eax*4], 0
           jne @f
           btr [mem_block_mask], eax
@@:
}

macro remove_from_used op
{
           mov edx, [op+list_fd]
           mov ecx, [op+list_bk]
           mov [edx+list_bk], ecx
           mov [ecx+list_fd], edx
           mov [op+list_fd], 0
           mov [op+list_bk], 0
}

align 4
proc init_kernel_heap

           mov ecx, 64/4
           mov edi, mem_block_list
           xor eax, eax
           cld
           rep stosd

           mov ecx, 512/4
           mov edi, mem_block_map
           not eax
           rep stosd

           mov [mem_block_start], mem_block_map
           mov [mem_block_end], mem_block_map+512
           mov [mem_block_arr], HEAP_BASE

           mov eax, mem_used.fd-MEM_LIST_OFFSET
           mov [mem_used.fd], eax
           mov [mem_used.bk], eax

           stdcall alloc_pages, dword 32
           mov ecx, 32
           mov edx, eax
           mov edi, HEAP_BASE
.l1:
           stdcall map_page,edi,edx,PG_SW
           add edi, 0x1000
           add edx, 0x1000
           dec ecx
           jnz .l1

           mov edi, HEAP_BASE
           mov ebx, HEAP_BASE+MEM_BLOCK_SIZE
           xor eax, eax
           mov [edi+block_next], ebx
           mov [edi+block_prev], eax
           mov [edi+list_fd], eax
           mov [edi+list_bk], eax
           mov [edi+block_base], HEAP_BASE
           mov [edi+block_size], 4096*MEM_BLOCK_SIZE
           mov [edi+block_flags], USED_BLOCK

           mov [ebx+block_next], eax
           mov [ebx+block_prev], eax
           mov [ebx+list_fd], eax
           mov [ebx+list_bk], eax
           mov [ebx+block_base], HEAP_BASE+4096*MEM_BLOCK_SIZE

           mov ecx, [MEM_AMOUNT]
           sub ecx, HEAP_BASE + 4096*MEM_BLOCK_SIZE
           mov [heap_size], ecx
           mov [heap_free], ecx
           mov [ebx+block_size], ecx
           mov [ebx+block_flags], FREE_BLOCK

           mov [mem_block_mask], eax
           mov [mem_block_mask+4],0x80000000

           mov [mem_block_list+63*4], ebx
           mov byte [mem_block_map], 0xFC
           and [heap_mutex], 0
           mov [heap_blocks], 4095
           mov [free_blocks], 4095
           ret
endp

; param
;  eax= required size
;
; retval
;  edi= memory block descriptor
;  ebx= descriptor index

align 4
get_block:
           mov ecx, eax
           shr ecx, 12
           dec ecx
           cmp ecx, 63
           jle .get_index
           mov ecx, 63
.get_index:
           lea esi, [mem_block_mask]
           xor ebx, ebx
           or edx, -1

           cmp ecx, 32
           jb .bit_test

           sub ecx, 32
           add ebx, 32
           add esi, 4
.bit_test:
           shl edx, cl
           and edx, [esi]
.find:
           bsf edi, edx
           jz .high_mask
           add ebx, edi
           mov edi, [mem_block_list+ebx*4]
.check_size:
           cmp eax, [edi+block_size]
           ja .next
           ret

.high_mask:
           add esi, 4
           cmp esi, mem_block_mask+8
           jae .err
           add ebx, 32
           mov edx, [esi]
           jmp .find
.next:
           mov edi, [edi+list_fd]
           test edi, edi
           jnz .check_size
.err:
           xor edi, edi
           ret

align 4
proc alloc_mem_block

           mov ebx, [mem_block_start]
           mov ecx, [mem_block_end]
.l1:
           bsf eax,[ebx];
           jnz found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           xor eax,eax
           ret

found:
           btr [ebx], eax
           mov [mem_block_start],ebx
           sub ebx, mem_block_map
           lea eax,[eax+ebx*8]
           shl eax, 5
           add eax, [mem_block_arr]
           dec [free_blocks]
           ret
endp

proc free_mem_block
           mov dword [eax], 0
           mov dword [eax+4], 0
           mov dword [eax+8], 0
           mov dword [eax+12], 0
           mov dword [eax+16], 0
;           mov dword [eax+20], 0
           mov dword [eax+24], 0
           mov dword [eax+28], 0

           sub eax, [mem_block_arr]
           shr eax, 5

           mov ebx, mem_block_map
           bts [ebx], eax
           inc [free_blocks]
           shr eax, 3
           and eax, not 3
           add eax, ebx
           cmp [mem_block_start], eax
           ja @f
           ret
@@:
           mov [mem_block_start], eax
	   ret
.err:
           xor eax, eax
	   ret
endp

align 4
proc alloc_kernel_space stdcall, size:dword
           local block_ind:DWORD

           mov eax, [size]
           add eax, 4095
           and eax, not 4095
           mov [size], eax

           mov ebx, heap_mutex
           call wait_mutex    ;ebx

           cmp eax, [heap_free]
           ja .error

           call get_block ; eax
           test edi, edi
           jz .error

           cmp [edi+block_flags], FREE_BLOCK
           jne .error

           mov [block_ind], ebx   ;index of allocated block

           mov eax, [edi+block_size]
           cmp eax, [size]
           je .m_eq_size

           call alloc_mem_block
           and eax, eax
           jz .error

           mov esi, eax           ;esi - splitted block

           mov [esi+block_next], edi
           mov eax, [edi+block_prev]
           mov [esi+block_prev], eax
           mov [edi+block_prev], esi
           mov [esi+list_fd], 0
           mov [esi+list_bk], 0
           and eax, eax
           jz @f
           mov [eax+block_next], esi
@@:
           mov ebx, [edi+block_base]
           mov [esi+block_base], ebx
           mov edx, [size]
           mov [esi+block_size], edx
           add [edi+block_base], edx
           sub [edi+block_size], edx

           mov eax, [edi+block_size]
           shr eax, 12
           sub eax, 1
           cmp eax, 63
           jna @f
           mov eax, 63
@@:
           cmp eax, [block_ind]
           je .m_eq_ind

           remove_from_list edi

           mov ecx, [block_ind]
           mov [mem_block_list+ecx*4], edx

           test edx, edx
           jnz @f
           btr [mem_block_mask], ecx
@@:
           mov edx, [mem_block_list+eax*4]
           mov [edi+list_fd], edx
           test edx, edx
           jz @f
           mov [edx+list_bk], edi
@@:
           mov [mem_block_list+eax*4], edi
           bts [mem_block_mask], eax
.m_eq_ind:
           mov ecx, mem_used.fd-MEM_LIST_OFFSET
           mov edx, [ecx+list_fd]
           mov [esi+list_fd], edx
           mov [esi+list_bk], ecx
           mov [ecx+list_fd], esi
           mov [edx+list_bk], esi

           mov [esi+block_flags], USED_BLOCK
           mov eax, [esi+block_base]
           mov ebx, [size]
           sub [heap_free], ebx
           and [heap_mutex], 0
           ret
.m_eq_size:
           remove_from_list edi
           mov [mem_block_list+ebx*4], edx
           and edx, edx
           jnz @f
           btr [mem_block_mask], ebx
@@:
           mov ecx, mem_used.fd-MEM_LIST_OFFSET
           mov edx, [ecx+list_fd]
           mov [edi+list_fd], edx
           mov [edi+list_bk], ecx
           mov [ecx+list_fd], edi
           mov [edx+list_bk], edi

           mov [edi+block_flags], USED_BLOCK
           mov eax, [edi+block_base]
           mov ebx, [size]
           sub [heap_free], ebx
           and [heap_mutex], 0
           ret
.error:
           xor eax, eax
           mov [heap_mutex], eax
           ret
endp

align 4
proc free_kernel_space stdcall uses ebx ecx edx esi edi, base:dword

           mov ebx, heap_mutex
           call wait_mutex    ;ebx

           mov eax, [base]
           mov esi, [mem_used.fd]
@@:
           cmp esi, mem_used.fd-MEM_LIST_OFFSET
           je .fail

           cmp [esi+block_base], eax
           je .found
           mov esi, [esi+list_fd]
           jmp @b
.found:
           cmp [esi+block_flags], USED_BLOCK
           jne .fail

           mov eax, [esi+block_size]
           add [heap_free], eax

           mov edi, [esi+block_next]
           test edi, edi
           jz .prev

           cmp [edi+block_flags], FREE_BLOCK
           jne .prev

           remove_from_free edi

           mov edx, [edi+block_next]
           mov [esi+block_next], edx
           test edx, edx
           jz @f

           mov [edx+block_prev], esi
@@:
           mov ecx, [edi+block_size]
           add [esi+block_size], ecx

           mov eax, edi
           call free_mem_block
.prev:
           mov edi, [esi+block_prev]
           test edi, edi
           jz .insert

           cmp [edi+block_flags], FREE_BLOCK
           jne .insert

           remove_from_used esi

           mov edx, [esi+block_next]
           mov [edi+block_next], edx
           test edx, edx
           jz @f
           mov [edx+block_prev], edi
@@:
           mov eax, esi
           call free_mem_block

           mov ecx, [edi+block_size]
           mov eax, [esi+block_size]
           add eax, ecx
           mov [edi+block_size], eax

           calc_index eax
           calc_index ecx
           cmp eax, ecx
           je .m_eq

           push ecx
           remove_from_list edi
           pop ecx

           cmp [mem_block_list+ecx*4], edi
           jne @f
           mov [mem_block_list+ecx*4], edx
@@:
           cmp [mem_block_list+ecx*4], 0
           jne @f
           btr [mem_block_mask], ecx
@@:
           mov esi, [mem_block_list+eax*4]
           mov [mem_block_list+eax*4], edi
           mov [edi+list_fd], esi
           test esi, esi
           jz @f
           mov [esi+list_bk], edi
@@:
           bts [mem_block_mask], eax
.m_eq:
           xor eax, eax
           mov [heap_mutex], eax
           dec eax
           ret
.insert:
           remove_from_used esi

           mov eax, [esi+block_size]
           calc_index eax

           mov edi, [mem_block_list+eax*4]
           mov [mem_block_list+eax*4], esi
           mov [esi+list_fd], edi
           test edi, edi
           jz @f
           mov [edi+list_bk], esi
@@:
           bts [mem_block_mask], eax
           mov [esi+block_flags],FREE_BLOCK
           xor eax, eax
           mov [heap_mutex], eax
           dec eax
           ret
.fail:
           xor eax, eax
           mov [heap_mutex], eax
           ret
endp

align 4
proc kernel_alloc stdcall, size:dword
           locals
             lin_addr    dd ?
             pages_count dd ?
           endl

           mov eax, [size]
           add eax, 4095
           and eax, not 4095;
           mov [size], eax
           and eax, eax
           jz .err
           mov ebx, eax
           shr ebx, 12
           mov [pages_count], ebx

           stdcall alloc_kernel_space, eax
           test eax, eax
           jz .err
           mov [lin_addr], eax

           mov ecx, [pages_count]
           mov edx, eax
           mov ebx, ecx

           shr ecx, 3
           jz .next

           and ebx, not 7
           push ebx
           stdcall alloc_pages, ebx
           pop ecx                   ; yes ecx!!!
           and eax, eax
           jz .err

           mov edi, eax
           mov edx, [lin_addr]
@@:
           stdcall map_page,edx,edi,dword PG_SW
           add edx, 0x1000
           add edi, 0x1000
           dec ecx
           jnz @B
.next:
           mov ecx, [pages_count]
           and ecx, 7
           jz .end
@@:
           push ecx
           call alloc_page
           pop ecx
           test eax, eax
           jz .err

           stdcall map_page,edx,eax,dword PG_SW
           add edx, 0x1000
           dec ecx
           jnz @B
.end:
           mov eax, [lin_addr]
           ret
.err:
           xor eax, eax
           ret
endp

align 4
proc kernel_free stdcall, base:dword
           push ebx esi

           mov ebx, heap_mutex
           call wait_mutex    ;ebx

           mov eax, [base]
           mov esi, [mem_used.fd]
@@:
           cmp esi, mem_used.fd-MEM_LIST_OFFSET
           je .fail

           cmp [esi+block_base], eax
           je .found
           mov esi, [esi+list_fd]
           jmp @b
.found:
           cmp [esi+block_flags], USED_BLOCK
           jne .fail

           and [heap_mutex], 0

           push ecx
           mov ecx, [esi+block_size];
           shr ecx, 12
           call release_pages   ;eax, ecx
           pop ecx
           stdcall free_kernel_space, [base]
           pop esi ebx
           ret
.fail:
           and [heap_mutex], 0
           pop esi ebx
           ret
endp

restore block_next
restore block_prev
restore block_list
restore block_base
restore block_size
restore block_flags

;;;;;;;;;;;;;;      USER     ;;;;;;;;;;;;;;;;;

HEAP_TOP  equ 0x5FC00000

align 4
proc init_heap

           mov ebx,[current_slot]
           mov eax, [ebx+APPDATA.heap_top]
           test eax, eax
           jz @F
           sub eax,[ebx+APPDATA.heap_base]
           sub eax, 4096
           ret
@@:
           mov esi, [ebx+APPDATA.mem_size]
           add esi, 4095
           and esi, not 4095
           mov [ebx+APPDATA.mem_size], esi
           mov eax, HEAP_TOP
           mov [ebx+APPDATA.heap_base], esi
           mov [ebx+APPDATA.heap_top], eax

           sub eax, esi
       ;    add esi, new_app_base
           shr esi, 10
           mov ecx, eax
           sub eax, 4096
           or ecx, FREE_BLOCK
           mov [page_tabs+esi], ecx
           ret
.exit:
           xor eax, eax
           ret
endp

align 4
proc user_alloc stdcall, alloc_size:dword

           mov ecx, [alloc_size]
           add ecx, (4095+4096)
           and ecx, not 4095

           mov ebx, [current_slot]
           mov esi, dword [ebx+APPDATA.heap_base]  ; heap_base
           mov edi, dword [ebx+APPDATA.heap_top]   ; heap_top
l_0:
           cmp esi, edi
           jae m_exit

           mov ebx, esi
           shr ebx, 12
           mov eax, [page_tabs+ebx*4]
           test eax, FREE_BLOCK
           jz test_used
           and eax, 0xFFFFF000
           cmp eax, ecx    ;alloc_size
           jb  m_next
	   jz  @f

           mov edx, esi
           add edx, ecx
           sub eax, ecx;
           or eax, FREE_BLOCK
           shr edx, 12
           mov [page_tabs+edx*4], eax
@@:
           or ecx, USED_BLOCK
           mov [page_tabs+ebx*4], ecx
           shr ecx, 12
           dec ecx
           inc ebx
@@:
           mov dword [page_tabs+ebx*4], 2
           inc ebx
           dec ecx
           jnz @B

           mov     edx, [current_slot]
           mov     ebx, [alloc_size]
           add     ebx, 0xFFF
           and     ebx, not 0xFFF
           add     ebx, [edx+APPDATA.mem_size]
           call    update_mem_size

           mov eax, esi
           add eax, 4096
           ret
m_next:
           add esi, eax
           jmp l_0
test_used:
           test eax, USED_BLOCK
           jz m_exit

           and eax, 0xFFFFF000
           add esi, eax
           jmp l_0
m_exit:
           xor eax, eax
           ret
endp

align 4
proc user_free stdcall, base:dword

           mov esi, [base]
           test esi, esi
           jz .exit

           xor ebx, ebx
           sub esi, 4096
           shr esi, 12
           mov eax, [page_tabs+esi*4]
           test eax, USED_BLOCK
           jz .not_used

           and eax, not 4095
           mov ecx, eax
           or eax, FREE_BLOCK
           mov [page_tabs+esi*4], eax
           inc esi
           sub ecx, 4096
           shr ecx, 12
           mov ebx, ecx
.release:
           xor eax, eax
           xchg eax, [page_tabs+esi*4]
           test eax, 1
           jz @F
           call free_page
           mov eax, esi
           shl eax, 12
           invlpg [eax]
@@:
           inc esi
           dec ecx
           jnz .release
.not_used:
           mov edx, [current_slot]
           mov esi, dword [edx+APPDATA.heap_base]
           mov edi, dword [edx+APPDATA.heap_top]
           sub ebx, [edx+APPDATA.mem_size]
           neg ebx
           call update_mem_size
           call user_normalize
           ret
.exit:
           xor eax, eax
           inc eax
           ret
endp

user_normalize:
; in: esi=heap_base, edi=heap_top
; out: eax=0 <=> OK
; destroys: ebx,edx,esi,edi
           shr esi, 12
           shr edi, 12
@@:
           mov eax, [page_tabs+esi*4]
           test eax, USED_BLOCK
           jz .test_free
           shr eax, 12
           add esi, eax
           jmp @B
.test_free:
           test eax, FREE_BLOCK
           jz .err
           mov edx, eax
           shr edx, 12
           add edx, esi
           cmp edx, edi
           jae .exit

           mov ebx, [page_tabs+edx*4]
           test ebx, USED_BLOCK
           jz .next_free

           shr ebx, 12
           add edx, ebx
           mov esi, edx
           jmp @B
.next_free:
           test ebx, FREE_BLOCK
           jz .err
           and dword [page_tabs+edx*4], 0
           add eax, ebx
           and eax, not 4095
           or eax, FREE_BLOCK
           mov [page_tabs+esi*4], eax
           jmp @B
.exit:
           xor eax, eax
           inc eax
           ret
.err:
           xor eax, eax
           ret

user_realloc:
; in: eax = pointer, ebx = new size
; out: eax = new pointer or NULL
        test    eax, eax
        jnz     @f
; realloc(NULL,sz) - same as malloc(sz)
        push    ebx
        call    user_alloc
        ret
@@:
        push    ecx edx
        lea     ecx, [eax - 0x1000]
        shr     ecx, 12
        mov     edx, [page_tabs+ecx*4]
        test    edx, USED_BLOCK
        jnz     @f
; attempt to realloc invalid pointer
.ret0:
        pop     edx ecx
        xor     eax, eax
        ret
@@:
        add     ebx, 0x1FFF
        shr     edx, 12
        shr     ebx, 12
; edx = allocated size, ebx = new size
        add     edx, ecx
        add     ebx, ecx
        cmp     edx, ebx
        jb      .realloc_add
; release part of allocated memory
.loop:
        cmp     edx, ebx
        jz      .release_done
        dec     edx
        xor     eax, eax
        xchg    eax, [page_tabs+edx*4]
        test    al, 1
        jz      .loop
        call    free_page
        mov     eax, edx
        shl     eax, 12
        invlpg  [eax]
        jmp     .loop
.release_done:
        sub     ebx, ecx
        cmp     ebx, 1
        jnz     .nofreeall
        mov     eax, [page_tabs+ecx*4]
        and     eax, not 0xFFF
        mov     edx, [current_slot]
        mov     ebx, [APPDATA.mem_size+edx]
        sub     ebx, eax
        add     ebx, 0x1000
        or      al, FREE_BLOCK
        mov     [page_tabs+ecx*4], eax
        push    esi edi
        mov     esi, [APPDATA.heap_base+edx]
        mov     edi, [APPDATA.heap_top+edx]
        call    update_mem_size
        call    user_normalize
        pop     edi esi
        jmp     .ret0   ; all freed
.nofreeall:
        sub     edx, ecx
        shl     ebx, 12
        or      ebx, USED_BLOCK
        xchg    [page_tabs+ecx*4], ebx
        shr     ebx, 12
        sub     ebx, edx
        push    ebx ecx edx
        mov     edx, [current_slot]
        shl     ebx, 12
        sub     ebx, [APPDATA.mem_size+edx]
        neg     ebx
        call    update_mem_size
        pop     edx ecx ebx
        lea     eax, [ecx+1]
        shl     eax, 12
        push    eax
        add     ecx, ebx
        add     edx, ecx
        shl     ebx, 12
        jz      .ret
        push    esi
        mov     esi, [current_slot]
        mov     esi, [APPDATA.heap_top+esi]
        shr     esi, 12
@@:
        cmp     edx, esi
        jae     .merge_done
        mov     eax, [page_tabs+edx*4]
        test    al, USED_BLOCK
        jz      .merge_done
        and     dword [page_tabs+edx*4], 0
        and     eax, not 0xFFF
        add     ebx, eax
        add     edx, eax
        jmp     @b
.merge_done:
        pop     esi
        or      ebx, FREE_BLOCK
        mov     [page_tabs+ecx*4], ebx
.ret:
        pop     eax edx ecx
        ret
.realloc_add:
; get some additional memory
        mov     eax, [current_slot]
        mov     eax, [APPDATA.heap_top+eax]
        shr     eax, 12
        cmp     edx, eax
        jae     .cant_inplace
        mov     eax, [page_tabs+edx*4]
        shr     eax, 12
        add     eax, edx
        cmp     eax, ebx
        jb      .cant_inplace
        sub     eax, ebx
        jz      @f
        shl     eax, 12
        or      al, FREE_BLOCK
        mov     [page_tabs+ebx*4], eax
@@:
        mov     eax, ebx
        sub     eax, ecx
        shl     eax, 12
        or      al, USED_BLOCK
        mov     [page_tabs+ecx*4], eax
        lea     eax, [ecx+1]
        shl     eax, 12
        push    eax
        push    edi
        lea     edi, [page_tabs+edx*4]
        mov     eax, 2
        sub     ebx, edx
        mov     ecx, ebx
        cld
        rep     stosd
        pop     edi
        mov     edx, [current_slot]
        shl     ebx, 12
        add     ebx, [APPDATA.mem_size+edx]
        call    update_mem_size
        pop     eax edx ecx
        ret
.cant_inplace:
        push    esi edi
        mov     eax, [current_slot]
        mov     esi, [APPDATA.heap_base+eax]
        mov     edi, [APPDATA.heap_top+eax]
        shr     esi, 12
        shr     edi, 12
        sub     ebx, ecx
.find_place:
        cmp     esi, edi
        jae     .place_not_found
        mov     eax, [page_tabs+esi*4]
        test    al, FREE_BLOCK
        jz      .next_place
        shr     eax, 12
        cmp     eax, ebx
        jae     .place_found
        add     esi, eax
        jmp     .find_place
.next_place:
        shr     eax, 12
        add     esi, eax
        jmp     .find_place
.place_not_found:
        pop     edi esi
        jmp     .ret0
.place_found:
        sub     eax, ebx
        jz      @f
        push    esi
        add     esi, eax
        shl     eax, 12
        or      al, FREE_BLOCK
        mov     [page_tabs+esi*4], eax
        pop     esi
@@:
        mov     eax, ebx
        shl     eax, 12
        or      al, USED_BLOCK
        mov     [page_tabs+esi*4], eax
        inc     esi
        mov     eax, esi
        shl     eax, 12
        push    eax
        mov     eax, [page_tabs+ecx*4]
        and     eax, not 0xFFF
        or      al, FREE_BLOCK
        sub     edx, ecx
        mov     [page_tabs+ecx*4], eax
        inc     ecx
@@:
        xor     eax, eax
        xchg    eax, [page_tabs+ecx*4]
        mov     [page_tabs+esi*4], eax
	mov     eax, ecx
	shl     eax, 12
	invlpg  [eax]
        inc     ecx
        inc     esi
        dec     ebx
        dec     edx
        jnz     @b
        push    ebx
        mov     edx, [current_slot]
        shl     ebx, 12
        add     ebx, [APPDATA.mem_size+edx]
        call    update_mem_size
        pop     ebx
@@:
        mov     dword [page_tabs+esi*4], 2
        inc     esi
        dec     ebx
        jnz     @b
        pop     eax edi esi edx ecx
        ret

if 0
align 4
proc alloc_dll
           pushf
           cli
           bsf eax, [dll_map]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [dll_map], eax
           popf
           shl eax, 5
           add eax, dll_tab
           ret
endp

align 4
proc alloc_service
           pushf
           cli
           bsf eax, [srv_map]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [srv_map], eax
           popf
           shl eax,0x02
           lea eax,[srv_tab+eax+eax*8]   ;srv_tab+eax*36
           ret
endp

end if
