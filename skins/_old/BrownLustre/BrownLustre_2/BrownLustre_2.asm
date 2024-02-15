include 'skin.inc'

SKIN_PARAMS \
  height	  = bmp_base.height,\	  ; skin height
  margins	  = [6:1:43:1],\	  ; margins [left:top:right:bottom]
  colors active   = [binner=0x612000:\	  ; border inner color
		     bouter=0x612000:\	  ; border outer color
		     bframe=0x842B00],\   ; border frame color
  colors inactive = [binner=0x6C4532:\	  ; border inner color
		     bouter=0x6C4532:\	  ; border outer color
		     bframe=0x844C32],\   ; border frame color
  dtp		  = 'BrownLustre_2.DTP'	  ; dtp colors

SKIN_BUTTONS \
  close    = [-23:6][12:12],\		  ; buttons coordinates
  minimize = [-42:6][12:12]		  ; [left:top][width:height]

SKIN_BITMAPS \
  left active	= bmp_left,\		  ; skin bitmaps pointers
  left inactive = bmp_left1,\
  oper active	= bmp_oper,\
  oper inactive = bmp_oper1,\
  base active	= bmp_base,\
  base inactive = bmp_base1

BITMAP bmp_left ,'active/left.bmp'		 ; skin bitmaps
BITMAP bmp_oper ,'active/oper.bmp'
BITMAP bmp_base ,'active/base.bmp'
BITMAP bmp_left1,'inactive/left.bmp'
BITMAP bmp_oper1,'inactive/oper.bmp'
BITMAP bmp_base1,'inactive/base.bmp'

;================================;
; Created by Rock_maniak_forever ;
;================================;


