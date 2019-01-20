; radartest.pro
; =============
; Author: R.J.Barnes
; 
; $License$
;
; A Simply demonstration program for converting radar coordinates


pro radartest


; Open the data table, this should be defined in the environment
; variable SD_RADAR

  fname=getenv('SD_RADAR')
  openr,inp,fname,/get_lun

; Load the data tables

  network=RadarLoad(inp)
  free_lun,inp

; Load up the hardware data, the environment variable SD_HDWPATH
; should point to the directory containing the hardware files


  s=RadarLoadHardware(network,path=getenv('SD_HDWPATH'))

; Find the radar information about the radar with ID code 3

  r=RadarGetRadar(network,3)


; Get the hardware information for the date in 1998

  site=RadarYMDHMSGetSite(r,1998,6,1,0,0,0)
  
; do the transform

  lat=0.0D
  lon=0.0D
  s=RadarPos(0,8,10,site,180,45,0,300.0,rho,lat,lon)
  
  print, lat,lon

  

end
