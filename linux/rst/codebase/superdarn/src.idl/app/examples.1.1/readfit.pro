; readfit.pro
; ===========
; Author: R.J.Barnes
; 
; $License$
;
; A Simply demonstration program for reading fit files


pro readfit

  
; Open the fit file

  fitfp=OldFitOpen('test.fit','test.inx')


;  Search for a specific time in the file

  s=OldFitSeek(fitfp,2002,12,19,1,30,10,atme=atme)

  while OldFitRead(fitfp,prm,fit) ne -1 do begin
     print, prm.time.hr,prm.time.mt,prm.time.sc
     plot,fit.v[0:prm.nrang-1]
     stop
 endwhile
 s=OldFitClose(fitfp)
end


