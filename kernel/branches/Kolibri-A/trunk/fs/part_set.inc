;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 1419 $


;*************************************************************
;* 13.02.2010 Find all partition and check supported FS
;* 12.07.2007 Check all 4 entry of MBR and EMBR
;* 29.04.2006 Elimination of hangup after the
;*             expiration hd_wait_timeout -  Mario79
;* 28.01.2006 find all Fat16/32 partition in all input point
;*            to MBR - Mario79
;*************************************************************

uglobal
align 4

;******************************************************
; Please do not change this place - variables  in text
; Mario79
; START place
;******************************************************
PARTITION_START      dd 0x3f
PARTITION_END        dd 0
fs_type              db 0       ; 1=NTFS, 2=EXT2/3, 16=FAT16, 32=FAT32
align 4

fs_dependent_data_start:
; FATxx data

SECTORS_PER_FAT      dd 0x1f3a
NUMBER_OF_FATS       dd 0x2
SECTORS_PER_CLUSTER  dd 0x8
BYTES_PER_SECTOR     dd 0x200   ; Note: if BPS <> 512 need lots of changes
ROOT_CLUSTER         dd 2       ; first rootdir cluster
FAT_START            dd 0       ; start of fat table
ROOT_START           dd 0       ; start of rootdir (only fat16)
ROOT_SECTORS         dd 0       ; count of rootdir sectors (only fat16)
DATA_START           dd 0       ; start of data area (=first cluster 2)
LAST_CLUSTER         dd 0       ; last availabe cluster
ADR_FSINFO           dd 0       ; used only by fat32

fatRESERVED          dd 0x0FFFFFF6
fatBAD               dd 0x0FFFFFF7
fatEND               dd 0x0FFFFFF8
fatMASK              dd 0x0FFFFFFF

fatStartScan         dd 2

fs_dependent_data_end:
file_system_data_size = $ - PARTITION_START
if file_system_data_size > 96
ERROR: sizeof(file system data) too big!
end if

virtual at fs_dependent_data_start
; NTFS data
ntfs_data:
.sectors_per_cluster    dd      ?
.mft_cluster            dd      ?
.mftmirr_cluster        dd      ?
.frs_size               dd      ?       ; FRS size in bytes
.iab_size               dd      ?       ; IndexAllocationBuffer size in bytes
.frs_buffer             dd      ?
.iab_buffer             dd      ?
.mft_retrieval          dd      ?
.mft_retrieval_size     dd      ?
.mft_retrieval_alloc    dd      ?
.mft_retrieval_end      dd      ?
.cur_index_size         dd      ?
.cur_index_buf          dd      ?
if $ > fs_dependent_data_end
ERROR: increase sizeof(fs_dependent_data)!
end if
end virtual

virtual at fs_dependent_data_start
; EXT2 data
ext2_data:
    .log_block_size                 dd ?
    .block_size                     dd ?
    .count_block_in_block           dd ?
    .blocks_per_group               dd ?
    .inodes_per_group               dd ?
    .global_desc_table              dd ?
    .root_inode                     dd ?	; pointer to root inode in memory
    .inode_size                     dd ?
    .count_pointer_in_block         dd ?	;  block_size / 4
    .count_pointer_in_block_square  dd ?	; (block_size / 4)**2
    .ext2_save_block                dd ?    ; ���� �� ��������� 1 ��楤���
    .ext2_temp_block                dd ?    ; ���� ��� ������ ��楤��
    .ext2_save_inode                dd ?    ; inode �� ��������� ��楤���
    .ext2_temp_inode                dd ?    ; inode ��� ������ ��楤��
    .sb                             dd ?    ; superblock
    .groups_count                   dd ?
if $ > fs_dependent_data_end
ERROR: increase sizeof(fs_dependent_data)!
end if
end virtual

;***************************************************************************
; End place
; Mario79
;***************************************************************************
endg
iglobal

  partition_types:              ; list of fat16/32 partitions
    db    0x04                  ; DOS: fat16 <32M
    db    0x06                  ; DOS: fat16 >32M
    db    0x0b                  ; WIN95: fat32
    db    0x0c                  ; WIN95: fat32, LBA-mapped
    db    0x0e                  ; WIN95: fat16, LBA-mapped
    db    0x14                  ; Hidden DOS: fat16 <32M
    db    0x16                  ; Hidden DOS: fat16 >32M
    db    0x1b                  ; Hidden WIN95: fat32
    db    0x1c                  ; Hidden WIN95: fat32, LBA-mapped
    db    0x1e                  ; Hidden WIN95: fat16, LBA-mapped
    db    0xc4                  ; DRDOS/secured: fat16 <32M
    db    0xc6                  ; DRDOS/secured: fat16 >32M
    db    0xcb                  ; DRDOS/secured: fat32
    db    0xcc                  ; DRDOS/secured: fat32, LBA-mapped
    db    0xce                  ; DRDOS/secured: fat16, LBA-mapped
    db    0xd4                  ; Old Multiuser DOS secured: fat16 <32M
    db    0xd6                  ; Old Multiuser DOS secured: fat16 >32M
    db    0x07                  ; NTFS
    db    0x27                  ; NTFS, hidden
    db    0x83                  ; Linux native file system (ext2fs)
  partition_types_end:


  extended_types:               ; list of extended partitions
    db    0x05                  ; DOS: extended partition
    db    0x0f                  ; WIN95: extended partition, LBA-mapped
    db    0xc5                  ; DRDOS/secured: extended partition
    db    0xd5                  ; Old Multiuser DOS secured: extended partition
  extended_types_end:

endg

; Partition chain used:
; MBR <---------------------
; |                         |
; |-> PARTITION1          |
; |-> EXTENDED PARTITION -        ;not need be second partition
; |-> PARTITION3
; |-> PARTITION4

set_PARTITION_variables:
set_FAT32_variables:        ;deprecated
    and   [problem_partition], 0
    call  reserve_hd1
    call  reserve_hd_channel

    pushad

    cmp   dword [hdpos],0
    je    problem_hd

    xor   ecx,ecx               ; partition count
    ;or    edx,-1                ; flag for partition
    xor   eax,eax               ; address MBR
    xor   ebp,ebp               ; extended partition start

new_mbr:
    test  ebp,ebp               ; is there extended partition? (MBR or EMBR)
    jnz   extended_already_set  ; yes
    xchg  ebp,eax               ; no. set it now

extended_already_set:
    add   eax,ebp               ; mbr=mbr+0, ext_part=ext_start+relat_start
    mov   ebx,buffer
    call  hd_read
    cmp  [hd_error],0
    jne  problem_hd

    cmp   word [ebx+0x1fe],0xaa55 ; is it valid boot sector?
    jnz   end_partition_chain
    push  eax                      ; push only one time
    cmp   dword [ebx+0x1be+0xc],0 ; skip over empty partition
    jnz    test_primary_partition_0
    cmp   dword [ebx+0x1be+0xc+16],0
    jnz    test_primary_partition_1
    cmp   dword [ebx+0x1be+0xc+16+16],0
    jnz    test_primary_partition_2
    cmp   dword [ebx+0x1be+0xc+16+16+16],0
    jnz    test_primary_partition_3
    pop   eax
    jmp   end_partition_chain

test_primary_partition_0:
    mov   al,[ebx+0x1be+4]      ; get primary partition type
    call  scan_partition_types
    jnz   test_primary_partition_1      ; no. skip over

    inc   ecx
    cmp   ecx,[known_part]       ; is it wanted partition?
    jnz   test_primary_partition_1      ; no

        pop     eax
        ;mov     edx, eax                ; start sector
        add     eax, [ebx+0x1be+8]      ; add relative start
        ;mov     [PARTITON_START],edx
        ;push    edx
        mov     edx, [ebx+0x1be+12]     ; length
        ;add     edx, eax                ; add length
        ;dec     edx                     ; PARTITION_END is inclusive
        ;mov     [PARTITION_END], edx    ; note that this can be changed
                                        ; when file system data will be available
        mov     cl, [ebx+0x1be+4]       ; fs_type
        ;mov     [fs_type], dl           ; save for FS recognizer (separate FAT vs NTFS)
        ;pop     edx
        jmp     hd_and_partition_ok

test_primary_partition_1:
    mov   al,[ebx+0x1be+4+16]      ; get primary partition type
    call  scan_partition_types
    jnz   test_primary_partition_2        ; no. skip over

    inc   ecx
    cmp   ecx,[known_part]       ; is it wanted partition?
    jnz   test_primary_partition_2        ; no

        pop     eax
        add     eax, [ebx+0x1be+8+16]
        mov     edx, [ebx+0x1be+12+16]
        mov     cl, [ebx+0x1be+4+16]
        jmp     hd_and_partition_ok

        ;mov     edx, eax
        ;add     edx, [ebx+0x1be+8+16]
        ;push    edx
        ;add     edx, [ebx+0x1be+12+16]
        ;dec     edx
        ;mov     [PARTITION_END], edx
        ;mov     al, [ebx+0x1be+4+16]
        ;mov     [fs_type], dl
        ;pop     edx

test_primary_partition_2:
    mov   al,[ebx+0x1be+4+16+16]      ; get primary partition type
    call  scan_partition_types
    jnz   test_primary_partition_3        ; no. skip over

    inc   ecx
    cmp   ecx,[known_part]       ; is it wanted partition?
    jnz   test_primary_partition_3        ; no

        pop     eax
        add     eax, [ebx+0x1be+8+16+16]
        mov     edx, [ebx+0x1be+12+16+16]
        mov     cl, [ebx+0x1be+4+16+16]
        jmp     hd_and_partition_ok
        ;mov     edx, eax
        ;add     edx, [ebx+0x1be+8+16+16]
        ;push    edx
        ;add     edx, [ebx+0x1be+12+16+16]
        ;dec     edx
        ;mov     [PARTITION_END], edx
        ;mov     al, [ebx+0x1be+4+16+16]
        ;mov     [fs_type], dl
        ;pop     edx

test_primary_partition_3:
    mov   al,[ebx+0x1be+4+16+16+16]      ; get primary partition type
    call  scan_partition_types
    jnz   test_ext_partition_0        ; no. skip over

    inc   ecx
    cmp   ecx,[known_part]       ; is it wanted partition?
    jnz   test_ext_partition_0  ; no

        pop     eax
        add     eax, [ebx+0x1be+8+16+16+16]
        mov     edx, [ebx+0x1be+12+16+16+16]
        mov     cl, [ebx+0x1be+4+16+16+16]
        jmp     hd_and_partition_ok

        ;mov     edx, eax
        ;add     edx, [ebx+0x1be+8+16+16+16]
        ;push    edx
        ;add     edx, [ebx+0x1be+12+16+16+16]
        ;dec     edx
        ;mov     [PARTITION_END], edx
        ;mov     al, [ebx+0x1be+4+16+16+16]
        ;mov     [fs_type], dl
        ;pop     edx

test_ext_partition_0:
    pop   eax               ; ���� �모�뢠�� �� �⥪�
    mov   al,[ebx+0x1be+4]   ; get extended partition type
    call  scan_extended_types
    jnz   test_ext_partition_1

    mov   eax,[ebx+0x1be+8]     ; add relative start
    test  eax,eax               ; is there extended partition?
    jnz   new_mbr               ; yes. read it

test_ext_partition_1:
    mov   al,[ebx+0x1be+4+16]   ; get extended partition type
    call  scan_extended_types
    jnz   test_ext_partition_2

    mov   eax,[ebx+0x1be+8+16]  ; add relative start
    test  eax,eax               ; is there extended partition?
    jnz   new_mbr               ; yes. read it

test_ext_partition_2:
    mov   al,[ebx+0x1be+4+16+16]   ; get extended partition type
    call  scan_extended_types
    jnz   test_ext_partition_3

    mov   eax,[ebx+0x1be+8+16+16]     ; add relative start
    test  eax,eax               ; is there extended partition?
    jnz   new_mbr               ; yes. read it

test_ext_partition_3:
    mov   al,[ebx+0x1be+4+16+16+16]   ; get extended partition type
    call  scan_extended_types
    jnz   end_partition_chain   ; no. end chain

    mov   eax,[ebx+0x1be+8+16+16+16]  ; get start of extended partition
    test  eax,eax               ; is there extended partition?
    jnz   new_mbr               ; yes. read it

end_partition_chain:
    ;mov   [partition_count],ecx

    ;cmp   edx,-1                ; found wanted partition?
    ;jnz   hd_and_partition_ok   ; yes. install it
    ;jmp   problem_partition_or_fat
problem_hd:
    or    [problem_partition], 2
    jmp   return_from_part_set


scan_partition_types:
    push  ecx
    mov   edi,partition_types
    mov   ecx,partition_types_end-partition_types
    cld
    repne scasb                 ; is partition type ok?
    pop   ecx
    ret

scan_extended_types:
    push  ecx
    mov   edi,extended_types
    mov   ecx,extended_types_end-extended_types
    cld
    repne scasb                 ; is it extended partition?
    pop   ecx
    ret

problem_fat_dec_count:          ; bootsector is missing or another problem
;    dec   [partition_count]     ; remove it from partition_count

problem_partition_or_fat:
    or    [problem_partition],1

return_from_part_set:
    popad
    ;mov   [fs_type],0
    call  free_hd_channel
    mov   [hd1_status],0        ; free
    ret

hd_and_partition_ok:

;eax = PARTITION_START edx=PARTITION_LENGTH cl=fs_type
    mov   [fs_type], cl
    ;mov   eax,edx
    mov   [PARTITION_START],eax
    add   edx, eax
    dec   edx
    mov   [PARTITION_END], edx

   ;     mov     edx, [PARTITION_END]
   ;     sub     edx, eax
   ;     inc     edx     ; edx = length of partition ��祬 ��� ���??

;    mov   [hd_setup],1
    mov   ebx,buffer
    call  hd_read               ; read boot sector of partition
        cmp     [hd_error], 0
        jz      boot_read_ok
        cmp     [fs_type], 7
        jnz     problem_fat_dec_count
; NTFS duplicates bootsector:
; NT4/2k/XP+ saves bootsector copy in the end of disk
; NT 3.51 saves bootsector copy in the middle of disk
        and     [hd_error], 0
        mov     eax, [PARTITION_END]
        call    hd_read
        cmp     [hd_error], 0
        jnz     @f
        call    ntfs_test_bootsec
        jnc     boot_read_ok
@@:
        and     [hd_error], 0
        mov     eax, edx
        shr     eax, 1
        add     eax, [PARTITION_START]
        call    hd_read
        cmp     [hd_error], 0
        jnz     problem_fat_dec_count   ; no chance...
boot_read_ok:

; if we are running on NTFS, check bootsector

        call    ntfs_test_bootsec      ; test ntfs
        jnc     ntfs_setup

        call    ext2_test_superblock   ; test ext2fs
        jnc     ext2_setup

        mov     eax, [PARTITION_START]	;ext2 test changes [buffer]
        call    hd_read
        cmp     [hd_error], 0
        jnz     problem_fat_dec_count

    cmp   word [ebx+0x1fe],0xaa55 ; is it valid boot sector?
    jnz   problem_fat_dec_count

    movzx eax,word [ebx+0xe]    ; sectors reserved
    add   eax,[PARTITION_START]
    mov   [FAT_START],eax       ; fat_start = partition_start + reserved

    movzx eax,byte [ebx+0xd]    ; sectors per cluster
    test  eax,eax
    jz    problem_fat_dec_count
    mov   [SECTORS_PER_CLUSTER],eax

    movzx ecx,word [ebx+0xb]    ; bytes per sector
    cmp   ecx,0x200
    jnz   problem_fat_dec_count
    mov   [BYTES_PER_SECTOR],ecx

    movzx eax,word [ebx+0x11]   ; count of rootdir entries (=0 fat32)
    mov   edx,32
    mul   edx
    dec   ecx
    add   eax,ecx               ; round up if not equal count
    inc   ecx                   ; bytes per sector
    div   ecx
    mov   [ROOT_SECTORS],eax    ; count of rootdir sectors

    movzx eax,word [ebx+0x16]   ; sectors per fat <65536
    test  eax,eax
    jnz   fat16_fatsize
    mov   eax,[ebx+0x24]        ; sectors per fat
  fat16_fatsize:
    mov   [SECTORS_PER_FAT],eax

    movzx eax,byte [ebx+0x10]   ; number of fats
    test  eax,eax               ; if 0 it's not fat partition
    jz    problem_fat_dec_count
    mov   [NUMBER_OF_FATS],eax
    imul  eax,[SECTORS_PER_FAT]
    add   eax,[FAT_START]
    mov   [ROOT_START],eax      ; rootdir = fat_start + fat_size * fat_count
    add   eax,[ROOT_SECTORS]    ; rootdir sectors should be 0 on fat32
    mov   [DATA_START],eax      ; data area = rootdir + rootdir_size

    movzx eax,word [ebx+0x13]   ; total sector count <65536
    test  eax,eax
    jnz   fat16_total
    mov   eax,[ebx+0x20]        ; total sector count
  fat16_total:
    add   eax,[PARTITION_START]
    dec   eax
    mov   [PARTITION_END],eax
    inc   eax
    sub   eax,[DATA_START]      ; eax = count of data sectors
    xor   edx,edx
    div   dword [SECTORS_PER_CLUSTER]
    inc   eax
    mov   [LAST_CLUSTER],eax
    dec   eax                   ; cluster count
    mov   [fatStartScan],2

    ; limits by Microsoft Hardware White Paper v1.03
    cmp   eax,4085              ; 0xff5
    jb    problem_fat_dec_count ; fat12 not supported
    cmp   eax,65525             ; 0xfff5
    jb    fat16_partition

fat32_partition:
    mov   eax,[ebx+0x2c]        ; rootdir cluster
    mov   [ROOT_CLUSTER],eax
    movzx eax,word [ebx+0x30]   ; fs info sector
    add   eax,[PARTITION_START]
    mov   [ADR_FSINFO],eax
    call  hd_read
    mov   eax,[ebx+0x1ec]
    cmp   eax,-1
    jz    @f
    mov   [fatStartScan],eax
@@:

    popad

    mov   [fatRESERVED],0x0FFFFFF6
    mov   [fatBAD],0x0FFFFFF7
    mov   [fatEND],0x0FFFFFF8
    mov   [fatMASK],0x0FFFFFFF
    mov   [fs_type],32         ; Fat32
    call  free_hd_channel
    mov   [hd1_status],0        ; free
    ret

fat16_partition:
    xor   eax,eax
    mov   [ROOT_CLUSTER],eax

    popad

    mov   [fatRESERVED],0x0000FFF6
    mov   [fatBAD],0x0000FFF7
    mov   [fatEND],0x0000FFF8
    mov   [fatMASK],0x0000FFFF
    mov   [fs_type],16         ; Fat16
    call  free_hd_channel
    mov   [hd1_status],0        ; free
    ret

