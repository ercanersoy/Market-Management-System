; ============================================================================
;  MACROS.INC - Assembler helper macros
; ============================================================================
;
;  The 8086/8088 only has short (8-bit displacement) conditional jumps.
;  FASM would silently emit the 80386 "near" form (0Fh 8xh) when a target
;  is out of range, which crashes on an 8088.  These macros force the short
;  form so that an out-of-range conditional jump becomes an assembly error.

irp cc, o,no,b,c,nae,nb,ae,nc,e,z,ne,nz,be,na,nbe,a,s,ns,p,pe,np,po,l,nge,nl,ge,le,ng,nle,g
{
        macro j#cc target \{ j\#cc short target \}
}
