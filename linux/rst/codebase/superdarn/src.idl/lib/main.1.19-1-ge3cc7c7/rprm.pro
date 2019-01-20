; rprm.pro
; ========
; Author: R.J.Barnes
; 
; $License$
;
; Public Functions
; ----------------
;
; RadarMakeRadarPrm          
;
; ---------------------------------------------------------------



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;+
; NAME:
;       RadarMakeRadarPrm
;
; PURPOSE:
;       Create a structure to store the radar operating 
;       parameters.
;       
;
; CALLING SEQUENCE:
;       RadarMakeRadarPrm,prm
;
;       This procedure creates a structure to store the rawACF data,
;       the structure is returns in rawdata.
;
;
;-----------------------------------------------------------------
;


pro RadarMakeRadarPrm,prm

  PULSE_SIZE=64
  LAG_SIZE=96
  MAX_RANGE=300

  prm={RadarPrm, $
         revision: {rbstr, major: 0B, minor: 0B}, $ 
         origin: { ogstr, code: 0B, time: ' ', command: ' '}, $
         cp: 0, $
         stid: 0, $
         time: {tmstr, yr:0, $
                      mo:0, $
                      dy:0, $
                      hr:0, $
                      mt:0, $
                      sc:0, $
                      us:0L  $
               }, $
         txpow: 0, $
         nave: 0, $
         atten: 0, $
         lagfr: 0, $
         smsep: 0, $
         ercod: 0, $
         stat: {ststr, agc:0, lopwr:0}, $
         noise: {nsstr, search:0.0, mean:0.0}, $
         channel: 0, $
         bmnum: 0, $
         bmazm: 0.0, $
         scan: 0, $
         rxrise: 0, $
         intt: {itstr, sc:0, us:0L}, $
         txpl: 0, $
         mpinc: 0, $
         mppul: 0, $
         mplgs: 0, $
         nrang: 0, $
         frang: 0, $
         rsep: 0, $
         xcf: 0, $
         tfreq: 0, $
         offset: 0, $
         mxpwr: 0L, $
         lvmax: 0L, $
         pulse: intarr(PULSE_SIZE), $
         lag: intarr(LAG_SIZE,2), $
         combf: '' $
     }
end
