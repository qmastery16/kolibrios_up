$Revision: 537 $
;-------------------------------------------------------------------------
;Loading configuration from ini file
;    {SPraid.simba}
;-------------------------------------------------------------------------

conf_path_sect: db 'path',0

conf_fname db '/sys/sys.conf',0

; set soke kernel configuration
proc set_kernel_conf
locals
  par db 30 dup(?)
endl
  pushad
  ;[gui]
  ;mouse_speed
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, ugui, ugui_mouse_speed, eax,30, ugui_mouse_speed_def        
  pop eax
  stdcall strtoint,eax
        mov     [mouse_speed_factor], ax
  
  ;mouse_delay
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, ugui, ugui_mouse_delay, eax,30, ugui_mouse_delay_def
  pop eax
  stdcall strtoint,eax
        mov     [mouse_delay], eax

  ;[dev]
  ;sb16
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, udev, udev_sb16, eax,30, udev_sb16_def
  pop eax
  stdcall strtoint,eax
        cmp     eax, 0x100
        jb      @f
        cmp     eax, 0x10000
        jae     @f
        mov     [sb16], eax
@@:
  
  ;sound_dma
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, udev, udev_sound_dma, eax,30, udev_sound_dma_def
  pop eax
  stdcall strtoint,eax
        cmp     eax, 3
        ja      @f
        mov     [sound_dma], eax
@@:
  
  ;midibase
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, udev, udev_midibase, eax,30, udev_midibase_def
  pop eax
  stdcall strtoint,eax
        cmp     eax, 0x100
        jb      @f
        cmp     eax, 0x10000
        jae     @f
        mov     [midi_base], ax
        mov     [mididp], eax
        inc     eax
        mov     [midisp], eax
@@:
  
  popad
  ret
endp

ugui db 'gui',0
ugui_mouse_speed db 'mouse_speed',0
ugui_mouse_speed_def db '2',0
ugui_mouse_delay db 'mouse_delay',0
ugui_mouse_delay_def db '0x00A',0

udev db 'dev',0
udev_sb16 db 'sb16',0
udev_sb16_def db '0x220',0
udev_sound_dma db 'sound_dma',0
udev_sound_dma_def db '1',0
udev_midibase db 'midibase',0
udev_midibase_def db '0x320',0

;set up netvork configuration
proc set_network_conf
locals
  par db 30 dup(?)
endl
  pushad
  
  ;[net]
  ;active
  lea eax,[par]
  invoke ini.get_int,conf_fname, unet, unet_active, 0
  or eax,eax
  jz .do_not_set_net
        mov     eax, [stack_config]
        and     eax, 0xFFFFFF80
        add     eax, 3
        mov     [stack_config], eax
        call    ash_eth_enable
  
  ;addr
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, unet, unet_addr, eax,30, unet_def
  pop eax
  stdcall do_inet_adr,eax
        mov     [stack_ip], eax
  
  ;mask
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, unet, unet_mask, eax,30, unet_def
  pop eax
  stdcall do_inet_adr,eax
        mov     [subnet_mask], eax
  
  ;gate
  lea eax,[par]
  push eax
  invoke ini.get_str,conf_fname, unet, unet_gate, eax,30, unet_def
  pop eax
  stdcall do_inet_adr,eax
        mov     [gateway_ip], eax
.do_not_set_net: 
  popad
  ret


endp

unet db 'net',0
unet_active db 'active',0
unet_addr db 'addr',0
unet_mask db 'mask',0
unet_gate db 'gate',0
unet_def db 0

; convert string to DWord
proc strtoint stdcall,strs
  pushad
  
  mov eax,[strs]
  inc eax
  mov bl,[eax]
  cmp bl,'x'
  je .hex
  cmp bl,'X'
  je .hex
  jmp .dec
.hex:
  inc eax
  stdcall strtoint_hex,eax
  jmp .exit
.dec:
  dec eax
  stdcall strtoint_dec,eax
.exit:
  mov [esp+28],eax
  popad
  ret  
endp

; convert string to DWord for decimal value
proc strtoint_dec stdcall,strs
  pushad
  xor edx,edx
  ; ���� ����
  mov esi,[strs]
@@:
  lodsb
  or al,al
  jnz @b
  mov ebx,esi
  mov esi,[strs]
  dec ebx
  sub ebx,esi
  mov ecx,1

@@:
  dec ebx
  or ebx,ebx
  jz @f
  imul ecx,ecx,10  ; ���冷�
  jmp @b
@@:

 xchg ebx,ecx

  
  xor ecx,ecx
  

@@:  
  xor eax,eax
  lodsb
  cmp al,0
  je .eend
  
  sub al,30h
  imul ebx
  add ecx,eax
  push ecx
  xchg eax,ebx
  mov ecx,10
  div ecx
  xchg eax,ebx
  pop ecx
  jmp @b
  
.eend:
  mov [esp+28],ecx
  popad
  ret
endp

;convert string to DWord for hex value
proc strtoint_hex stdcall,strs
  pushad
  xor edx,edx

  mov esi,[strs]
  mov ebx,1
  add esi,1

@@:
  lodsb
  or al,al
  jz @f
  shl ebx,4
  jmp @b
@@:
  xor ecx,ecx
  mov esi,[strs]

@@:  
  xor eax,eax
  lodsb
  cmp al,0
  je .eend
  
  cmp al,'a'
  jae .bm
  cmp al,'A'
  jae .bb
  jmp .cc
.bm:    ; 57h
  sub al,57h
  jmp .do
 
.bb:    ; 37h
  sub al,37h
  jmp .do

.cc:    ; 30h
  sub al,30h
  
.do:  
  imul ebx
  add ecx,eax
  shr ebx,4

  jmp @b

.eend:
  mov [esp+28],ecx
  popad
  ret
endp    


; convert string to DWord for IP addres
proc do_inet_adr stdcall,strs
  pushad
  
  mov esi,[strs]
  mov ebx,0
.next:
  push esi
@@:
  lodsb
  or al,al
  jz @f
  cmp al,'.'
  jz @f
  jmp @b
@@:
  mov cl, al
  mov [esi-1],byte 0
  ;pop eax
  call strtoint_dec
  rol eax,24
  ror ebx,8
  add ebx,eax
  or cl,cl
  jz @f
  jmp .next
@@:
  mov [esp+28],ebx
  popad
  ret
endp
