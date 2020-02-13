/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sun Jul 12 06:47:45 EDT 2009 */

#include "codelet-rdft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_hc2cdft_c -fma -reorder-insns -schedule-for-pipeline -simd -compact -variables 4 -pipeline-latency 8 -trivial-stores -variables 32 -no-generate-bytw -n 16 -dit -name hc2cfdftv_16 -include hc2cfv.h */

/*
 * This function contains 103 FP additions, 96 FP multiplications,
 * (or, 53 additions, 46 multiplications, 50 fused multiply/add),
 * 92 stack variables, 4 constants, and 32 memory accesses
 */
#include "hc2cfv.h"

static void hc2cfdftv_16(R *Rp, R *Ip, R *Rm, R *Im, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DVK(KP500000000, +0.500000000000000000000000000000000000000000000);
     DVK(KP923879532, +0.923879532511286756128183189396788286822416626);
     DVK(KP707106781, +0.707106781186547524400844362104849039284835938);
     DVK(KP414213562, +0.414213562373095048801688724209698078569671875);
     INT m;
     for (m = mb, W = W + ((mb - 1) * ((TWVL / VL) * 30)); m < me; m = m + VL, Rp = Rp + (VL * ms), Ip = Ip + (VL * ms), Rm = Rm - (VL * ms), Im = Im - (VL * ms), W = W + (TWVL * 30), MAKE_VOLATILE_STRIDE(rs)) {
	  V T8, Tc, TQ, TZ, T1J, T1x, T12, TH, T1I, T1q, Tp, TJ, Te, Tf, Td;
	  V TN, Tj, Tk, Ti, TK, Tg, TO, Tl, TL, T1r, Th, TR, T1y, T1s, Tq;
	  V TM, T1z, T1N, T1t, T10, Tr, T13, TS, T1K, T1A, T1E, T1u, T1f, T11, T1c;
	  V Ts, T1d, T14, T1g, TT;
	  {
	       V T3, Tw, TF, TW, Tz, TA, Ty, TX, T7, Tu, T1, T2, Tv, TD, TE;
	       V TC, TV, T5, T6, T4, Tt, TB, TY, T1o, T1v, Tx, Ta, Tb, T9, TP;
	       V T1w, TG, T1p, Tn, To, Tm, TI;
	       T1 = LD(&(Rp[0]), ms, &(Rp[0]));
	       T2 = LD(&(Rm[0]), -ms, &(Rm[0]));
	       Tv = LDW(&(W[0]));
	       TD = LD(&(Rp[WS(rs, 2)]), ms, &(Rp[0]));
	       TE = LD(&(Rm[WS(rs, 2)]), -ms, &(Rm[0]));
	       TC = LDW(&(W[TWVL * 8]));
	       TV = LDW(&(W[TWVL * 6]));
	       T5 = LD(&(Rp[WS(rs, 4)]), ms, &(Rp[0]));
	       T6 = LD(&(Rm[WS(rs, 4)]), -ms, &(Rm[0]));
	       T3 = VFMACONJ(T2, T1);
	       Tw = VZMULIJ(Tv, VFNMSCONJ(T2, T1));
	       T4 = LDW(&(W[TWVL * 14]));
	       Tt = LDW(&(W[TWVL * 16]));
	       TF = VZMULIJ(TC, VFNMSCONJ(TE, TD));
	       TW = VZMULJ(TV, VFMACONJ(TE, TD));
	       Tz = LD(&(Rp[WS(rs, 6)]), ms, &(Rp[0]));
	       TA = LD(&(Rm[WS(rs, 6)]), -ms, &(Rm[0]));
	       Ty = LDW(&(W[TWVL * 24]));
	       TX = LDW(&(W[TWVL * 22]));
	       T7 = VZMULJ(T4, VFMACONJ(T6, T5));
	       Tu = VZMULIJ(Tt, VFNMSCONJ(T6, T5));
	       Ta = LD(&(Rp[WS(rs, 1)]), ms, &(Rp[WS(rs, 1)]));
	       Tb = LD(&(Rm[WS(rs, 1)]), -ms, &(Rm[WS(rs, 1)]));
	       T9 = LDW(&(W[TWVL * 2]));
	       TP = LDW(&(W[TWVL * 4]));
	       TB = VZMULIJ(Ty, VFNMSCONJ(TA, Tz));
	       TY = VZMULJ(TX, VFMACONJ(TA, Tz));
	       T1o = VADD(T3, T7);
	       T8 = VSUB(T3, T7);
	       T1v = VADD(Tw, Tu);
	       Tx = VSUB(Tu, Tw);
	       Tc = VZMULJ(T9, VFMACONJ(Tb, Ta));
	       TQ = VZMULIJ(TP, VFNMSCONJ(Tb, Ta));
	       T1w = VADD(TF, TB);
	       TG = VSUB(TB, TF);
	       T1p = VADD(TW, TY);
	       TZ = VSUB(TW, TY);
	       Tn = LD(&(Rp[WS(rs, 3)]), ms, &(Rp[WS(rs, 1)]));
	       To = LD(&(Rm[WS(rs, 3)]), -ms, &(Rm[WS(rs, 1)]));
	       Tm = LDW(&(W[TWVL * 10]));
	       TI = LDW(&(W[TWVL * 12]));
	       T1J = VSUB(T1w, T1v);
	       T1x = VADD(T1v, T1w);
	       T12 = VFMA(LDK(KP414213562), Tx, TG);
	       TH = VFNMS(LDK(KP414213562), TG, Tx);
	       T1I = VSUB(T1o, T1p);
	       T1q = VADD(T1o, T1p);
	       Tp = VZMULJ(Tm, VFMACONJ(To, Tn));
	       TJ = VZMULIJ(TI, VFNMSCONJ(To, Tn));
	       Te = LD(&(Rp[WS(rs, 5)]), ms, &(Rp[WS(rs, 1)]));
	       Tf = LD(&(Rm[WS(rs, 5)]), -ms, &(Rm[WS(rs, 1)]));
	       Td = LDW(&(W[TWVL * 18]));
	       TN = LDW(&(W[TWVL * 20]));
	       Tj = LD(&(Rp[WS(rs, 7)]), ms, &(Rp[WS(rs, 1)]));
	       Tk = LD(&(Rm[WS(rs, 7)]), -ms, &(Rm[WS(rs, 1)]));
	       Ti = LDW(&(W[TWVL * 26]));
	       TK = LDW(&(W[TWVL * 28]));
	  }
	  Tg = VZMULJ(Td, VFMACONJ(Tf, Te));
	  TO = VZMULIJ(TN, VFNMSCONJ(Tf, Te));
	  Tl = VZMULJ(Ti, VFMACONJ(Tk, Tj));
	  TL = VZMULIJ(TK, VFNMSCONJ(Tk, Tj));
	  T1r = VADD(Tc, Tg);
	  Th = VSUB(Tc, Tg);
	  TR = VSUB(TO, TQ);
	  T1y = VADD(TQ, TO);
	  T1s = VADD(Tl, Tp);
	  Tq = VSUB(Tl, Tp);
	  TM = VSUB(TJ, TL);
	  T1z = VADD(TL, TJ);
	  T1N = VSUB(T1s, T1r);
	  T1t = VADD(T1r, T1s);
	  T10 = VSUB(Tq, Th);
	  Tr = VADD(Th, Tq);
	  T13 = VFNMS(LDK(KP414213562), TM, TR);
	  TS = VFMA(LDK(KP414213562), TR, TM);
	  T1K = VSUB(T1y, T1z);
	  T1A = VADD(T1y, T1z);
	  T1E = VADD(T1q, T1t);
	  T1u = VSUB(T1q, T1t);
	  T1f = VFMA(LDK(KP707106781), T10, TZ);
	  T11 = VFNMS(LDK(KP707106781), T10, TZ);
	  T1c = VFNMS(LDK(KP707106781), Tr, T8);
	  Ts = VFMA(LDK(KP707106781), Tr, T8);
	  T1d = VSUB(T12, T13);
	  T14 = VADD(T12, T13);
	  T1g = VSUB(TS, TH);
	  TT = VADD(TH, TS);
	  {
	       V T1O, T1L, T1F, T1B, T1k, T1e, T19, T15, T1l, T1h, T18, TU, T1T, T1P, T1S;
	       V T1M, T1H, T1G, T1D, T1C, T1m, T1n, T1j, T1i, T1a, T1b, T17, T16, T1U, T1V;
	       V T1R, T1Q;
	       T1O = VSUB(T1K, T1J);
	       T1L = VADD(T1J, T1K);
	       T1F = VADD(T1x, T1A);
	       T1B = VSUB(T1x, T1A);
	       T1k = VFNMS(LDK(KP923879532), T1d, T1c);
	       T1e = VFMA(LDK(KP923879532), T1d, T1c);
	       T19 = VFNMS(LDK(KP923879532), T14, T11);
	       T15 = VFMA(LDK(KP923879532), T14, T11);
	       T1l = VFNMS(LDK(KP923879532), T1g, T1f);
	       T1h = VFMA(LDK(KP923879532), T1g, T1f);
	       T18 = VFNMS(LDK(KP923879532), TT, Ts);
	       TU = VFMA(LDK(KP923879532), TT, Ts);
	       T1T = VFNMS(LDK(KP707106781), T1O, T1N);
	       T1P = VFMA(LDK(KP707106781), T1O, T1N);
	       T1S = VFNMS(LDK(KP707106781), T1L, T1I);
	       T1M = VFMA(LDK(KP707106781), T1L, T1I);
	       T1H = VCONJ(VMUL(LDK(KP500000000), VADD(T1F, T1E)));
	       T1G = VMUL(LDK(KP500000000), VSUB(T1E, T1F));
	       T1D = VCONJ(VMUL(LDK(KP500000000), VFNMSI(T1B, T1u)));
	       T1C = VMUL(LDK(KP500000000), VFMAI(T1B, T1u));
	       T1m = VMUL(LDK(KP500000000), VFNMSI(T1l, T1k));
	       T1n = VCONJ(VMUL(LDK(KP500000000), VFMAI(T1l, T1k)));
	       T1j = VMUL(LDK(KP500000000), VFMAI(T1h, T1e));
	       T1i = VCONJ(VMUL(LDK(KP500000000), VFNMSI(T1h, T1e)));
	       T1a = VCONJ(VMUL(LDK(KP500000000), VFNMSI(T19, T18)));
	       T1b = VMUL(LDK(KP500000000), VFMAI(T19, T18));
	       T17 = VCONJ(VMUL(LDK(KP500000000), VFMAI(T15, TU)));
	       T16 = VMUL(LDK(KP500000000), VFNMSI(T15, TU));
	       T1U = VMUL(LDK(KP500000000), VFNMSI(T1T, T1S));
	       T1V = VCONJ(VMUL(LDK(KP500000000), VFMAI(T1T, T1S)));
	       T1R = VMUL(LDK(KP500000000), VFMAI(T1P, T1M));
	       T1Q = VCONJ(VMUL(LDK(KP500000000), VFNMSI(T1P, T1M)));
	       ST(&(Rm[WS(rs, 7)]), T1H, -ms, &(Rm[WS(rs, 1)]));
	       ST(&(Rp[0]), T1G, ms, &(Rp[0]));
	       ST(&(Rm[WS(rs, 3)]), T1D, -ms, &(Rm[WS(rs, 1)]));
	       ST(&(Rp[WS(rs, 4)]), T1C, ms, &(Rp[0]));
	       ST(&(Rp[WS(rs, 5)]), T1m, ms, &(Rp[WS(rs, 1)]));
	       ST(&(Rm[WS(rs, 4)]), T1n, -ms, &(Rm[0]));
	       ST(&(Rp[WS(rs, 3)]), T1j, ms, &(Rp[WS(rs, 1)]));
	       ST(&(Rm[WS(rs, 2)]), T1i, -ms, &(Rm[0]));
	       ST(&(Rm[WS(rs, 6)]), T1a, -ms, &(Rm[0]));
	       ST(&(Rp[WS(rs, 7)]), T1b, ms, &(Rp[WS(rs, 1)]));
	       ST(&(Rm[0]), T17, -ms, &(Rm[0]));
	       ST(&(Rp[WS(rs, 1)]), T16, ms, &(Rp[WS(rs, 1)]));
	       ST(&(Rp[WS(rs, 6)]), T1U, ms, &(Rp[0]));
	       ST(&(Rm[WS(rs, 5)]), T1V, -ms, &(Rm[WS(rs, 1)]));
	       ST(&(Rp[WS(rs, 2)]), T1R, ms, &(Rp[0]));
	       ST(&(Rm[WS(rs, 1)]), T1Q, -ms, &(Rm[WS(rs, 1)]));
	  }
     }
}

static const tw_instr twinstr[] = {
     VTW(1, 1),
     VTW(1, 2),
     VTW(1, 3),
     VTW(1, 4),
     VTW(1, 5),
     VTW(1, 6),
     VTW(1, 7),
     VTW(1, 8),
     VTW(1, 9),
     VTW(1, 10),
     VTW(1, 11),
     VTW(1, 12),
     VTW(1, 13),
     VTW(1, 14),
     VTW(1, 15),
     {TW_NEXT, VL, 0}
};

static const hc2c_desc desc = { 16, "hc2cfdftv_16", twinstr, &GENUS, {53, 46, 50, 0} };

void X(codelet_hc2cfdftv_16) (planner *p) {
     X(khc2c_register) (p, hc2cfdftv_16, &desc, HC2C_VIA_DFT);
}
#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_hc2cdft_c -simd -compact -variables 4 -pipeline-latency 8 -trivial-stores -variables 32 -no-generate-bytw -n 16 -dit -name hc2cfdftv_16 -include hc2cfv.h */

/*
 * This function contains 103 FP additions, 56 FP multiplications,
 * (or, 99 additions, 52 multiplications, 4 fused multiply/add),
 * 101 stack variables, 5 constants, and 32 memory accesses
 */
#include "hc2cfv.h"

static void hc2cfdftv_16(R *Rp, R *Ip, R *Rm, R *Im, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DVK(KP707106781, +0.707106781186547524400844362104849039284835938);
     DVK(KP353553390, +0.353553390593273762200422181052424519642417969);
     DVK(KP500000000, +0.500000000000000000000000000000000000000000000);
     DVK(KP382683432, +0.382683432365089771728459984030398866761344562);
     DVK(KP923879532, +0.923879532511286756128183189396788286822416626);
     INT m;
     for (m = mb, W = W + ((mb - 1) * ((TWVL / VL) * 30)); m < me; m = m + VL, Rp = Rp + (VL * ms), Ip = Ip + (VL * ms), Rm = Rm - (VL * ms), Im = Im - (VL * ms), W = W + (TWVL * 30), MAKE_VOLATILE_STRIDE(rs)) {
	  V T1D, T1E, T1R, TP, T1b, Ta, T1w, T18, T1x, T1z, T1A, T1G, T1H, T1S, Tx;
	  V T13, T10, T1a, T1, T3, TA, TM, TL, TN, T6, T8, TC, TH, TG, TI;
	  V T2, Tz, TK, TJ, T7, TB, TF, TE, TD, TO, T4, T9, T5, T15, T17;
	  V T14, T16;
	  T1 = LD(&(Rp[0]), ms, &(Rp[0]));
	  T2 = LD(&(Rm[0]), -ms, &(Rm[0]));
	  T3 = VCONJ(T2);
	  Tz = LDW(&(W[0]));
	  TA = VZMULIJ(Tz, VSUB(T3, T1));
	  TM = LD(&(Rp[WS(rs, 6)]), ms, &(Rp[0]));
	  TK = LD(&(Rm[WS(rs, 6)]), -ms, &(Rm[0]));
	  TL = VCONJ(TK);
	  TJ = LDW(&(W[TWVL * 24]));
	  TN = VZMULIJ(TJ, VSUB(TL, TM));
	  T6 = LD(&(Rp[WS(rs, 4)]), ms, &(Rp[0]));
	  T7 = LD(&(Rm[WS(rs, 4)]), -ms, &(Rm[0]));
	  T8 = VCONJ(T7);
	  TB = LDW(&(W[TWVL * 16]));
	  TC = VZMULIJ(TB, VSUB(T8, T6));
	  TH = LD(&(Rp[WS(rs, 2)]), ms, &(Rp[0]));
	  TF = LD(&(Rm[WS(rs, 2)]), -ms, &(Rm[0]));
	  TG = VCONJ(TF);
	  TE = LDW(&(W[TWVL * 8]));
	  TI = VZMULIJ(TE, VSUB(TG, TH));
	  T1D = VADD(TA, TC);
	  T1E = VADD(TI, TN);
	  T1R = VSUB(T1D, T1E);
	  TD = VSUB(TA, TC);
	  TO = VSUB(TI, TN);
	  TP = VFNMS(LDK(KP382683432), TO, VMUL(LDK(KP923879532), TD));
	  T1b = VFMA(LDK(KP382683432), TD, VMUL(LDK(KP923879532), TO));
	  T4 = VADD(T1, T3);
	  T5 = LDW(&(W[TWVL * 14]));
	  T9 = VZMULJ(T5, VADD(T6, T8));
	  Ta = VMUL(LDK(KP500000000), VSUB(T4, T9));
	  T1w = VADD(T4, T9);
	  T14 = LDW(&(W[TWVL * 6]));
	  T15 = VZMULJ(T14, VADD(TH, TG));
	  T16 = LDW(&(W[TWVL * 22]));
	  T17 = VZMULJ(T16, VADD(TM, TL));
	  T18 = VSUB(T15, T17);
	  T1x = VADD(T15, T17);
	  {
	       V Tf, TR, Tv, TY, Tk, TT, Tq, TW, Tc, Te, Td, Tb, TQ, Ts, Tu;
	       V Tt, Tr, TX, Th, Tj, Ti, Tg, TS, Tn, Tp, To, Tm, TV, Tl, Tw;
	       V TU, TZ;
	       Tc = LD(&(Rp[WS(rs, 1)]), ms, &(Rp[WS(rs, 1)]));
	       Td = LD(&(Rm[WS(rs, 1)]), -ms, &(Rm[WS(rs, 1)]));
	       Te = VCONJ(Td);
	       Tb = LDW(&(W[TWVL * 2]));
	       Tf = VZMULJ(Tb, VADD(Tc, Te));
	       TQ = LDW(&(W[TWVL * 4]));
	       TR = VZMULIJ(TQ, VSUB(Te, Tc));
	       Ts = LD(&(Rp[WS(rs, 3)]), ms, &(Rp[WS(rs, 1)]));
	       Tt = LD(&(Rm[WS(rs, 3)]), -ms, &(Rm[WS(rs, 1)]));
	       Tu = VCONJ(Tt);
	       Tr = LDW(&(W[TWVL * 10]));
	       Tv = VZMULJ(Tr, VADD(Ts, Tu));
	       TX = LDW(&(W[TWVL * 12]));
	       TY = VZMULIJ(TX, VSUB(Tu, Ts));
	       Th = LD(&(Rp[WS(rs, 5)]), ms, &(Rp[WS(rs, 1)]));
	       Ti = LD(&(Rm[WS(rs, 5)]), -ms, &(Rm[WS(rs, 1)]));
	       Tj = VCONJ(Ti);
	       Tg = LDW(&(W[TWVL * 18]));
	       Tk = VZMULJ(Tg, VADD(Th, Tj));
	       TS = LDW(&(W[TWVL * 20]));
	       TT = VZMULIJ(TS, VSUB(Tj, Th));
	       Tn = LD(&(Rp[WS(rs, 7)]), ms, &(Rp[WS(rs, 1)]));
	       To = LD(&(Rm[WS(rs, 7)]), -ms, &(Rm[WS(rs, 1)]));
	       Tp = VCONJ(To);
	       Tm = LDW(&(W[TWVL * 26]));
	       Tq = VZMULJ(Tm, VADD(Tn, Tp));
	       TV = LDW(&(W[TWVL * 28]));
	       TW = VZMULIJ(TV, VSUB(Tp, Tn));
	       T1z = VADD(Tf, Tk);
	       T1A = VADD(Tq, Tv);
	       T1G = VADD(TR, TT);
	       T1H = VADD(TW, TY);
	       T1S = VSUB(T1H, T1G);
	       Tl = VSUB(Tf, Tk);
	       Tw = VSUB(Tq, Tv);
	       Tx = VMUL(LDK(KP353553390), VADD(Tl, Tw));
	       T13 = VMUL(LDK(KP707106781), VSUB(Tw, Tl));
	       TU = VSUB(TR, TT);
	       TZ = VSUB(TW, TY);
	       T10 = VFMA(LDK(KP382683432), TU, VMUL(LDK(KP923879532), TZ));
	       T1a = VFNMS(LDK(KP923879532), TU, VMUL(LDK(KP382683432), TZ));
	  }
	  {
	       V T1U, T20, T1X, T21, T1Q, T1T, T1V, T1W, T1Y, T23, T1Z, T22, T1C, T1M, T1J;
	       V T1N, T1y, T1B, T1F, T1I, T1K, T1P, T1L, T1O, T12, T1g, T1d, T1h, Ty, T11;
	       V T19, T1c, T1e, T1j, T1f, T1i, T1m, T1s, T1p, T1t, T1k, T1l, T1n, T1o, T1q;
	       V T1v, T1r, T1u;
	       T1Q = VMUL(LDK(KP500000000), VSUB(T1w, T1x));
	       T1T = VMUL(LDK(KP353553390), VADD(T1R, T1S));
	       T1U = VADD(T1Q, T1T);
	       T20 = VSUB(T1Q, T1T);
	       T1V = VSUB(T1A, T1z);
	       T1W = VMUL(LDK(KP707106781), VSUB(T1S, T1R));
	       T1X = VMUL(LDK(KP500000000), VBYI(VADD(T1V, T1W)));
	       T21 = VMUL(LDK(KP500000000), VBYI(VSUB(T1W, T1V)));
	       T1Y = VCONJ(VSUB(T1U, T1X));
	       ST(&(Rm[WS(rs, 1)]), T1Y, -ms, &(Rm[WS(rs, 1)]));
	       T23 = VADD(T20, T21);
	       ST(&(Rp[WS(rs, 6)]), T23, ms, &(Rp[0]));
	       T1Z = VADD(T1U, T1X);
	       ST(&(Rp[WS(rs, 2)]), T1Z, ms, &(Rp[0]));
	       T22 = VCONJ(VSUB(T20, T21));
	       ST(&(Rm[WS(rs, 5)]), T22, -ms, &(Rm[WS(rs, 1)]));
	       T1y = VADD(T1w, T1x);
	       T1B = VADD(T1z, T1A);
	       T1C = VADD(T1y, T1B);
	       T1M = VSUB(T1y, T1B);
	       T1F = VADD(T1D, T1E);
	       T1I = VADD(T1G, T1H);
	       T1J = VADD(T1F, T1I);
	       T1N = VBYI(VSUB(T1I, T1F));
	       T1K = VCONJ(VMUL(LDK(KP500000000), VSUB(T1C, T1J)));
	       ST(&(Rm[WS(rs, 7)]), T1K, -ms, &(Rm[WS(rs, 1)]));
	       T1P = VMUL(LDK(KP500000000), VADD(T1M, T1N));
	       ST(&(Rp[WS(rs, 4)]), T1P, ms, &(Rp[0]));
	       T1L = VMUL(LDK(KP500000000), VADD(T1C, T1J));
	       ST(&(Rp[0]), T1L, ms, &(Rp[0]));
	       T1O = VCONJ(VMUL(LDK(KP500000000), VSUB(T1M, T1N)));
	       ST(&(Rm[WS(rs, 3)]), T1O, -ms, &(Rm[WS(rs, 1)]));
	       Ty = VADD(Ta, Tx);
	       T11 = VMUL(LDK(KP500000000), VADD(TP, T10));
	       T12 = VADD(Ty, T11);
	       T1g = VSUB(Ty, T11);
	       T19 = VSUB(T13, T18);
	       T1c = VSUB(T1a, T1b);
	       T1d = VMUL(LDK(KP500000000), VBYI(VADD(T19, T1c)));
	       T1h = VMUL(LDK(KP500000000), VBYI(VSUB(T1c, T19)));
	       T1e = VCONJ(VSUB(T12, T1d));
	       ST(&(Rm[0]), T1e, -ms, &(Rm[0]));
	       T1j = VADD(T1g, T1h);
	       ST(&(Rp[WS(rs, 7)]), T1j, ms, &(Rp[WS(rs, 1)]));
	       T1f = VADD(T12, T1d);
	       ST(&(Rp[WS(rs, 1)]), T1f, ms, &(Rp[WS(rs, 1)]));
	       T1i = VCONJ(VSUB(T1g, T1h));
	       ST(&(Rm[WS(rs, 6)]), T1i, -ms, &(Rm[0]));
	       T1k = VSUB(T10, TP);
	       T1l = VADD(T18, T13);
	       T1m = VMUL(LDK(KP500000000), VBYI(VSUB(T1k, T1l)));
	       T1s = VMUL(LDK(KP500000000), VBYI(VADD(T1l, T1k)));
	       T1n = VSUB(Ta, Tx);
	       T1o = VMUL(LDK(KP500000000), VADD(T1b, T1a));
	       T1p = VSUB(T1n, T1o);
	       T1t = VADD(T1n, T1o);
	       T1q = VADD(T1m, T1p);
	       ST(&(Rp[WS(rs, 5)]), T1q, ms, &(Rp[WS(rs, 1)]));
	       T1v = VCONJ(VSUB(T1t, T1s));
	       ST(&(Rm[WS(rs, 2)]), T1v, -ms, &(Rm[0]));
	       T1r = VCONJ(VSUB(T1p, T1m));
	       ST(&(Rm[WS(rs, 4)]), T1r, -ms, &(Rm[0]));
	       T1u = VADD(T1s, T1t);
	       ST(&(Rp[WS(rs, 3)]), T1u, ms, &(Rp[WS(rs, 1)]));
	  }
     }
}

static const tw_instr twinstr[] = {
     VTW(1, 1),
     VTW(1, 2),
     VTW(1, 3),
     VTW(1, 4),
     VTW(1, 5),
     VTW(1, 6),
     VTW(1, 7),
     VTW(1, 8),
     VTW(1, 9),
     VTW(1, 10),
     VTW(1, 11),
     VTW(1, 12),
     VTW(1, 13),
     VTW(1, 14),
     VTW(1, 15),
     {TW_NEXT, VL, 0}
};

static const hc2c_desc desc = { 16, "hc2cfdftv_16", twinstr, &GENUS, {99, 52, 4, 0} };

void X(codelet_hc2cfdftv_16) (planner *p) {
     X(khc2c_register) (p, hc2cfdftv_16, &desc, HC2C_VIA_DFT);
}
#endif				/* HAVE_FMA */
