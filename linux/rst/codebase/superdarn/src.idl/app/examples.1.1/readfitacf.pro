; readfitacf.pro
; ==============
; Author: R.J.Barnes
; 
; $License$
;
; A Simply demonstration program for reading rawacf files


pro readfitacf

; Open the fit index file and load it into the inx structure 

  openr,inp,'test.fitinx',/GET_LUN,/STDIO,/SWAP_IF_BIG_ENDIAN
  s=FitLoadInx(inp,inx)
  free_lun,inp
 


; Open the raw file for read only 
 
  inp=FitOpen('test.fitacf',/read)
 

;  Search for a specific time in the file

  s=FitSeek(inp,2002,12,19,0,30,10,inx,atme=atme)
  while FitRead(inp,prm,fit) ne -1 do begin
     print, prm.time.hr,prm.time.mt,prm.time.sc
     plot,fit.v[0:prm.nrang-1]
     stop
  endwhile
  free_lun,inp

end


