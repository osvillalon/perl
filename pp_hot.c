/*    pp_hot.c
 *
 *    Copyright (c) 1991-1997, Larry Wall
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * Then he heard Merry change the note, and up went the Horn-cry of Buckland,
 * shaking the air.
 *
 *            Awake!  Awake!  Fear, Fire, Foes!  Awake!
 *                     Fire, Foes!  Awake!
 */

#include "EXTERN.h"
#include "perl.h"

/* Hot code. */

PP(pp_const)
{
    djSP;
    XPUSHs(cSVOP->op_sv);
    RETURN;
}

PP(pp_nextstate)
{
    curcop = (COP*)op;
    TAINT_NOT;		/* Each statement is presumed innocent */
    stack_sp = stack_base + cxstack[cxstack_ix].blk_oldsp;
    FREETMPS;
    return NORMAL;
}

PP(pp_gvsv)
{
    djSP;
    EXTEND(SP,1);
    if (op->op_private & OPpLVAL_INTRO)
	PUSHs(save_scalar(cGVOP->op_gv));
    else
	PUSHs(GvSV(cGVOP->op_gv));
    RETURN;
}

PP(pp_null)
{
    return NORMAL;
}

PP(pp_pushmark)
{
    PUSHMARK(stack_sp);
    return NORMAL;
}

PP(pp_stringify)
{
    djSP; dTARGET;
    STRLEN len;
    char *s;
    s = SvPV(TOPs,len);
    sv_setpvn(TARG,s,len);
    SETTARG;
    RETURN;
}

PP(pp_gv)
{
    djSP;
    XPUSHs((SV*)cGVOP->op_gv);
    RETURN;
}

PP(pp_and)
{
    djSP;
    if (!SvTRUE(TOPs))
	RETURN;
    else {
	--SP;
	RETURNOP(cLOGOP->op_other);
    }
}

PP(pp_sassign)
{
    djSP; dPOPTOPssrl;
    MAGIC *mg;

    if (op->op_private & OPpASSIGN_BACKWARDS) {
	SV *temp;
	temp = left; left = right; right = temp;
    }
    if (tainting && tainted && !SvTAINTED(left))
	TAINT_NOT;
    PUTBACK;
    SvSetMagicSV(right, left);
    SPAGAIN;
    SETs(right);
    RETURN;
}

PP(pp_cond_expr)
{
    djSP;
    if (SvTRUEx(POPs))
	RETURNOP(cCONDOP->op_true);
    else
	RETURNOP(cCONDOP->op_false);
}

PP(pp_unstack)
{
    I32 oldsave;
    TAINT_NOT;		/* Each statement is presumed innocent */
    stack_sp = stack_base + cxstack[cxstack_ix].blk_oldsp;
    FREETMPS;
    oldsave = scopestack[scopestack_ix - 1];
    LEAVE_SCOPE(oldsave);
    return NORMAL;
}

PP(pp_concat)
{
  djSP; dATARGET; tryAMAGICbin(concat,opASSIGN);
  {
    dPOPTOPssrl;
    STRLEN len;
    char *s;
    if (TARG != left) {
	s = SvPV(left,len);
	sv_setpvn(TARG,s,len);
    }
    else if (SvGMAGICAL(TARG))
	mg_get(TARG);
    else if (!SvOK(TARG) && SvTYPE(TARG) <= SVt_PVMG) {
	sv_setpv(TARG, "");	/* Suppress warning. */
	s = SvPV_force(TARG, len);
    }
    s = SvPV(right,len);
    if (SvOK(TARG))
	sv_catpvn(TARG,s,len);
    else
	sv_setpvn(TARG,s,len);	/* suppress warning */
    SETTARG;
    RETURN;
  }
}

PP(pp_padsv)
{
    djSP; dTARGET;
    XPUSHs(TARG);
    if (op->op_flags & OPf_MOD) {
	if (op->op_private & OPpLVAL_INTRO)
	    SAVECLEARSV(curpad[op->op_targ]);
        else if (op->op_private & OPpDEREF) {
	    PUTBACK;
	    vivify_ref(curpad[op->op_targ], op->op_private & OPpDEREF);
	    SPAGAIN;
	}
    }
    RETURN;
}

PP(pp_readline)
{
    last_in_gv = (GV*)(*stack_sp--);
    return do_readline();
}

PP(pp_eq)
{
    djSP; tryAMAGICbinSET(eq,0); 
    {
      dPOPnv;
      SETs(boolSV(TOPn == value));
      RETURN;
    }
}

PP(pp_preinc)
{
    djSP;
    if (SvREADONLY(TOPs) || SvTYPE(TOPs) > SVt_PVLV)
	croak(no_modify);
    if (SvIOK(TOPs) && !SvNOK(TOPs) && !SvPOK(TOPs) &&
    	SvIVX(TOPs) != IV_MAX)
    {
	++SvIVX(TOPs);
	SvFLAGS(TOPs) &= ~(SVp_NOK|SVp_POK);
    }
    else
	sv_inc(TOPs);
    SvSETMAGIC(TOPs);
    return NORMAL;
}

PP(pp_or)
{
    djSP;
    if (SvTRUE(TOPs))
	RETURN;
    else {
	--SP;
	RETURNOP(cLOGOP->op_other);
    }
}

PP(pp_add)
{
    djSP; dATARGET; tryAMAGICbin(add,opASSIGN); 
    {
      dPOPTOPnnrl_ul;
      SETn( left + right );
      RETURN;
    }
}

PP(pp_aelemfast)
{
    djSP;
    AV *av = GvAV((GV*)cSVOP->op_sv);
    SV** svp = av_fetch(av, op->op_private, op->op_flags & OPf_MOD);
    EXTEND(SP, 1);
    PUSHs(svp ? *svp : &sv_undef);
    RETURN;
}

PP(pp_join)
{
    djSP; dMARK; dTARGET;
    MARK++;
    do_join(TARG, *MARK, MARK, SP);
    SP = MARK;
    SETs(TARG);
    RETURN;
}

PP(pp_pushre)
{
    djSP;
#ifdef DEBUGGING
    /*
     * We ass_u_me that LvTARGOFF() comes first, and that two STRLENs
     * will be enough to hold an OP*.
     */
    SV* sv = sv_newmortal();
    sv_upgrade(sv, SVt_PVLV);
    LvTYPE(sv) = '/';
    Copy(&op, &LvTARGOFF(sv), 1, OP*);
    XPUSHs(sv);
#else
    XPUSHs((SV*)op);
#endif
    RETURN;
}

/* Oversized hot code. */

PP(pp_print)
{
    djSP; dMARK; dORIGMARK;
    GV *gv;
    IO *io;
    register PerlIO *fp;
    MAGIC *mg;

    if (op->op_flags & OPf_STACKED)
	gv = (GV*)*++MARK;
    else
	gv = defoutgv;
    if (SvRMAGICAL(gv) && (mg = mg_find((SV*)gv, 'q'))) {
	if (MARK == ORIGMARK) {
	    /* If using default handle then we need to make space to 
	     * pass object as 1st arg, so move other args up ...
	     */
	    MEXTEND(SP, 1);
	    ++MARK;
	    Move(MARK, MARK + 1, (SP - MARK) + 1, SV*);
	    ++SP;
	}
	PUSHMARK(MARK - 1);
	*MARK = mg->mg_obj;
	PUTBACK;
	ENTER;
	perl_call_method("PRINT", G_SCALAR);
	LEAVE;
	SPAGAIN;
	MARK = ORIGMARK + 1;
	*MARK = *SP;
	SP = MARK;
	RETURN;
    }
    if (!(io = GvIO(gv))) {
	if (dowarn) {
	    SV* sv = sv_newmortal();
            gv_fullname3(sv, gv, Nullch);
            warn("Filehandle %s never opened", SvPV(sv,na));
        }

	SETERRNO(EBADF,RMS$_IFI);
	goto just_say_no;
    }
    else if (!(fp = IoOFP(io))) {
	if (dowarn)  {
	    SV* sv = sv_newmortal();
            gv_fullname3(sv, gv, Nullch);
	    if (IoIFP(io))
		warn("Filehandle %s opened only for input", SvPV(sv,na));
	    else
		warn("print on closed filehandle %s", SvPV(sv,na));
	}
	SETERRNO(EBADF,IoIFP(io)?RMS$_FAC:RMS$_IFI);
	goto just_say_no;
    }
    else {
	MARK++;
	if (ofslen) {
	    while (MARK <= SP) {
		if (!do_print(*MARK, fp))
		    break;
		MARK++;
		if (MARK <= SP) {
		    if (PerlIO_write(fp, ofs, ofslen) == 0 || PerlIO_error(fp)) {
			MARK--;
			break;
		    }
		}
	    }
	}
	else {
	    while (MARK <= SP) {
		if (!do_print(*MARK, fp))
		    break;
		MARK++;
	    }
	}
	if (MARK <= SP)
	    goto just_say_no;
	else {
	    if (orslen)
		if (PerlIO_write(fp, ors, orslen) == 0 || PerlIO_error(fp))
		    goto just_say_no;

	    if (IoFLAGS(io) & IOf_FLUSH)
		if (PerlIO_flush(fp) == EOF)
		    goto just_say_no;
	}
    }
    SP = ORIGMARK;
    PUSHs(&sv_yes);
    RETURN;

  just_say_no:
    SP = ORIGMARK;
    PUSHs(&sv_undef);
    RETURN;
}

PP(pp_rv2av)
{
    djSP; dPOPss;
    AV *av;

    if (SvROK(sv)) {
      wasref:
	av = (AV*)SvRV(sv);
	if (SvTYPE(av) != SVt_PVAV)
	    DIE("Not an ARRAY reference");
	if (op->op_flags & OPf_REF) {
	    PUSHs((SV*)av);
	    RETURN;
	}
    }
    else {
	if (SvTYPE(sv) == SVt_PVAV) {
	    av = (AV*)sv;
	    if (op->op_flags & OPf_REF) {
		PUSHs((SV*)av);
		RETURN;
	    }
	}
	else {
	    GV *gv;
	    
	    if (SvTYPE(sv) != SVt_PVGV) {
		char *sym;

		if (SvGMAGICAL(sv)) {
		    mg_get(sv);
		    if (SvROK(sv))
			goto wasref;
		}
		if (!SvOK(sv)) {
		    if (op->op_flags & OPf_REF ||
		      op->op_private & HINT_STRICT_REFS)
			DIE(no_usym, "an ARRAY");
		    if (dowarn)
			warn(warn_uninit);
		    if (GIMME == G_ARRAY)
			RETURN;
		    RETPUSHUNDEF;
		}
		sym = SvPV(sv,na);
		if (op->op_private & HINT_STRICT_REFS)
		    DIE(no_symref, sym, "an ARRAY");
		gv = (GV*)gv_fetchpv(sym, TRUE, SVt_PVAV);
	    } else {
		gv = (GV*)sv;
	    }
	    av = GvAVn(gv);
	    if (op->op_private & OPpLVAL_INTRO)
		av = save_ary(gv);
	    if (op->op_flags & OPf_REF) {
		PUSHs((SV*)av);
		RETURN;
	    }
	}
    }

    if (GIMME == G_ARRAY) {
	I32 maxarg = AvFILL(av) + 1;
	EXTEND(SP, maxarg);
	Copy(AvARRAY(av), SP+1, maxarg, SV*);
	SP += maxarg;
    }
    else {
	dTARGET;
	I32 maxarg = AvFILL(av) + 1;
	PUSHi(maxarg);
    }
    RETURN;
}

PP(pp_rv2hv)
{
    djSP; dTOPss;
    HV *hv;

    if (SvROK(sv)) {
      wasref:
	hv = (HV*)SvRV(sv);
	if (SvTYPE(hv) != SVt_PVHV)
	    DIE("Not a HASH reference");
	if (op->op_flags & OPf_REF) {
	    SETs((SV*)hv);
	    RETURN;
	}
    }
    else {
	if (SvTYPE(sv) == SVt_PVHV) {
	    hv = (HV*)sv;
	    if (op->op_flags & OPf_REF) {
		SETs((SV*)hv);
		RETURN;
	    }
	}
	else {
	    GV *gv;
	    
	    if (SvTYPE(sv) != SVt_PVGV) {
		char *sym;

		if (SvGMAGICAL(sv)) {
		    mg_get(sv);
		    if (SvROK(sv))
			goto wasref;
		}
		if (!SvOK(sv)) {
		    if (op->op_flags & OPf_REF ||
		      op->op_private & HINT_STRICT_REFS)
			DIE(no_usym, "a HASH");
		    if (dowarn)
			warn(warn_uninit);
		    if (GIMME == G_ARRAY) {
			SP--;
			RETURN;
		    }
		    RETSETUNDEF;
		}
		sym = SvPV(sv,na);
		if (op->op_private & HINT_STRICT_REFS)
		    DIE(no_symref, sym, "a HASH");
		gv = (GV*)gv_fetchpv(sym, TRUE, SVt_PVHV);
	    } else {
		gv = (GV*)sv;
	    }
	    hv = GvHVn(gv);
	    if (op->op_private & OPpLVAL_INTRO)
		hv = save_hash(gv);
	    if (op->op_flags & OPf_REF) {
		SETs((SV*)hv);
		RETURN;
	    }
	}
    }

    if (GIMME == G_ARRAY) { /* array wanted */
	*stack_sp = (SV*)hv;
	return do_kv(ARGS);
    }
    else {
	dTARGET;
	if (HvFILL(hv))
	    sv_setpvf(TARG, "%ld/%ld",
		      (long)HvFILL(hv), (long)HvMAX(hv) + 1);
	else
	    sv_setiv(TARG, 0);
	SETTARG;
	RETURN;
    }
}

PP(pp_aassign)
{
    djSP;
    SV **lastlelem = stack_sp;
    SV **lastrelem = stack_base + POPMARK;
    SV **firstrelem = stack_base + POPMARK + 1;
    SV **firstlelem = lastrelem + 1;

    register SV **relem;
    register SV **lelem;

    register SV *sv;
    register AV *ary;

    I32 gimme;
    HV *hash;
    I32 i;
    int magic;

    delaymagic = DM_DELAY;		/* catch simultaneous items */

    /* If there's a common identifier on both sides we have to take
     * special care that assigning the identifier on the left doesn't
     * clobber a value on the right that's used later in the list.
     */
    if (op->op_private & OPpASSIGN_COMMON) {
        for (relem = firstrelem; relem <= lastrelem; relem++) {
            /*SUPPRESS 560*/
            if (sv = *relem) {
		TAINT_NOT;	/* Each item is independent */
                *relem = sv_mortalcopy(sv);
	    }
        }
    }

    relem = firstrelem;
    lelem = firstlelem;
    ary = Null(AV*);
    hash = Null(HV*);
    while (lelem <= lastlelem) {
	TAINT_NOT;		/* Each item stands on its own, taintwise. */
	sv = *lelem++;
	switch (SvTYPE(sv)) {
	case SVt_PVAV:
	    ary = (AV*)sv;
	    magic = SvMAGICAL(ary) != 0;
	    
	    av_clear(ary);
	    av_extend(ary, lastrelem - relem);
	    i = 0;
	    while (relem <= lastrelem) {	/* gobble up all the rest */
		SV **didstore;
		sv = NEWSV(28,0);
		assert(*relem);
		sv_setsv(sv,*relem);
		*(relem++) = sv;
		didstore = av_store(ary,i++,sv);
		if (magic) {
		    if (SvSMAGICAL(sv))
			mg_set(sv);
		    if (!didstore)
			SvREFCNT_dec(sv);
		}
		TAINT_NOT;
	    }
	    break;
	case SVt_PVHV: {
		SV *tmpstr;

		hash = (HV*)sv;
		magic = SvMAGICAL(hash) != 0;
		hv_clear(hash);

		while (relem < lastrelem) {	/* gobble up all the rest */
		    HE *didstore;
		    if (*relem)
			sv = *(relem++);
		    else
			sv = &sv_no, relem++;
		    tmpstr = NEWSV(29,0);
		    if (*relem)
			sv_setsv(tmpstr,*relem);	/* value */
		    *(relem++) = tmpstr;
		    didstore = hv_store_ent(hash,sv,tmpstr,0);
		    if (magic) {
			if (SvSMAGICAL(tmpstr))
			    mg_set(tmpstr);
			if (!didstore)
			    SvREFCNT_dec(tmpstr);
		    }
		    TAINT_NOT;
		}
		if (relem == lastrelem) {
		    if (*relem) {
			HE *didstore;
			if (dowarn) {
			    if (relem == firstrelem &&
				SvROK(*relem) &&
				( SvTYPE(SvRV(*relem)) == SVt_PVAV ||
				  SvTYPE(SvRV(*relem)) == SVt_PVHV ) )
				warn("Reference found where even-sized list expected");
			    else
				warn("Odd number of elements in hash assignment");
			}
			tmpstr = NEWSV(29,0);
			didstore = hv_store_ent(hash,*relem,tmpstr,0);
			if (magic) {
			    if (SvSMAGICAL(tmpstr))
				mg_set(tmpstr);
			    if (!didstore)
				SvREFCNT_dec(tmpstr);
			}
			TAINT_NOT;
		    }
		    relem++;
		}
	    }
	    break;
	default:
	    if (SvTHINKFIRST(sv)) {
		if (SvREADONLY(sv) && curcop != &compiling) {
		    if (sv != &sv_undef && sv != &sv_yes && sv != &sv_no)
			DIE(no_modify);
		    if (relem <= lastrelem)
			relem++;
		    break;
		}
		if (SvROK(sv))
		    sv_unref(sv);
	    }
	    if (relem <= lastrelem) {
		sv_setsv(sv, *relem);
		*(relem++) = sv;
	    }
	    else
		sv_setsv(sv, &sv_undef);
	    SvSETMAGIC(sv);
	    break;
	}
    }
    if (delaymagic & ~DM_DELAY) {
	if (delaymagic & DM_UID) {
#ifdef HAS_SETRESUID
	    (void)setresuid(uid,euid,(Uid_t)-1);
#else
#  ifdef HAS_SETREUID
	    (void)setreuid(uid,euid);
#  else
#    ifdef HAS_SETRUID
	    if ((delaymagic & DM_UID) == DM_RUID) {
		(void)setruid(uid);
		delaymagic &= ~DM_RUID;
	    }
#    endif /* HAS_SETRUID */
#    ifdef HAS_SETEUID
	    if ((delaymagic & DM_UID) == DM_EUID) {
		(void)seteuid(uid);
		delaymagic &= ~DM_EUID;
	    }
#    endif /* HAS_SETEUID */
	    if (delaymagic & DM_UID) {
		if (uid != euid)
		    DIE("No setreuid available");
		(void)setuid(uid);
	    }
#  endif /* HAS_SETREUID */
#endif /* HAS_SETRESUID */
	    uid = (int)getuid();
	    euid = (int)geteuid();
	}
	if (delaymagic & DM_GID) {
#ifdef HAS_SETRESGID
	    (void)setresgid(gid,egid,(Gid_t)-1);
#else
#  ifdef HAS_SETREGID
	    (void)setregid(gid,egid);
#  else
#    ifdef HAS_SETRGID
	    if ((delaymagic & DM_GID) == DM_RGID) {
		(void)setrgid(gid);
		delaymagic &= ~DM_RGID;
	    }
#    endif /* HAS_SETRGID */
#    ifdef HAS_SETEGID
	    if ((delaymagic & DM_GID) == DM_EGID) {
		(void)setegid(gid);
		delaymagic &= ~DM_EGID;
	    }
#    endif /* HAS_SETEGID */
	    if (delaymagic & DM_GID) {
		if (gid != egid)
		    DIE("No setregid available");
		(void)setgid(gid);
	    }
#  endif /* HAS_SETREGID */
#endif /* HAS_SETRESGID */
	    gid = (int)getgid();
	    egid = (int)getegid();
	}
	tainting |= (uid && (euid != uid || egid != gid));
    }
    delaymagic = 0;

    gimme = GIMME_V;
    if (gimme == G_VOID)
	SP = firstrelem - 1;
    else if (gimme == G_SCALAR) {
	dTARGET;
	SP = firstrelem;
	SETi(lastrelem - firstrelem + 1);
    }
    else {
	if (ary || hash)
	    SP = lastrelem;
	else
	    SP = firstrelem + (lastlelem - firstlelem);
	lelem = firstlelem + (relem - firstrelem);
	while (relem <= SP)
	    *relem++ = (lelem <= lastlelem) ? *lelem++ : &sv_undef;
    }
    RETURN;
}

PP(pp_match)
{
    djSP; dTARG;
    register PMOP *pm = cPMOP;
    register char *t;
    register char *s;
    char *strend;
    I32 global;
    I32 savematch;
    char *truebase;
    register REGEXP *rx = pm->op_pmregexp;
    bool rxtainted;
    I32 gimme = GIMME;
    STRLEN len;
    I32 minmatch = 0;
    I32 oldsave = savestack_ix;
    I32 update_minmatch = 1;

    if (op->op_flags & OPf_STACKED)
	TARG = POPs;
    else {
	TARG = GvSV(defgv);
	EXTEND(SP,1);
    }
    s = SvPV(TARG, len);
    strend = s + len;
    if (!s)
	DIE("panic: do_match");
    rxtainted = ((pm->op_pmdynflags & PMdf_TAINTED) ||
		 (tainted && (pm->op_pmflags & PMf_RETAINT)));
    TAINT_NOT;

    if (pm->op_pmdynflags & PMdf_USED) {
	if (gimme == G_ARRAY)
	    RETURN;
	RETPUSHNO;
    }

    if (!rx->prelen && curpm) {
	pm = curpm;
	rx = pm->op_pmregexp;
    }
    truebase = t = s;
    if (global = pm->op_pmflags & PMf_GLOBAL) {
	rx->startp[0] = 0;
	if (SvTYPE(TARG) >= SVt_PVMG && SvMAGIC(TARG)) {
	    MAGIC* mg = mg_find(TARG, 'g');
	    if (mg && mg->mg_len >= 0) {
		rx->endp[0] = rx->startp[0] = s + mg->mg_len; 
		minmatch = (mg->mg_flags & MGf_MINMATCH);
		update_minmatch = 0;
	    }
	}
    }
    if (!rx->nparens && !global)
	gimme = G_SCALAR;			/* accidental array context? */
    savematch = sawampersand | ((gimme != G_ARRAY) && !global && rx->nparens);
    if (pm->op_pmflags & (PMf_MULTILINE|PMf_SINGLELINE)) {
	SAVEINT(multiline);
	multiline = pm->op_pmflags & PMf_MULTILINE;
    }

play_it_again:
    if (global && rx->startp[0]) {
	t = s = rx->endp[0];
	if ((s + rx->minlen) > strend)
	    goto nope;
	if (update_minmatch++)
	    minmatch = (s == rx->startp[0]);
    }
    if (pm->op_pmshort) {
	if (pm->op_pmflags & PMf_SCANFIRST) {
	    if (SvSCREAM(TARG)) {
		if (screamfirst[BmRARE(pm->op_pmshort)] < 0)
		    goto nope;
		else if (!(s = screaminstr(TARG, pm->op_pmshort)))
		    goto nope;
		else if (pm->op_pmflags & PMf_ALL)
		    goto yup;
	    }
	    else if (!(s = fbm_instr((unsigned char*)s,
	      (unsigned char*)strend, pm->op_pmshort)))
		goto nope;
	    else if (pm->op_pmflags & PMf_ALL)
		goto yup;
	    if (s && rx->regback >= 0) {
		++BmUSEFUL(pm->op_pmshort);
		s -= rx->regback;
		if (s < t)
		    s = t;
	    }
	    else
		s = t;
	}
	else if (!multiline) {
	    if (*SvPVX(pm->op_pmshort) != *s
		|| (pm->op_pmslen > 1
		    && memNE(SvPVX(pm->op_pmshort), s, pm->op_pmslen)))
		goto nope;
	}
	if (!rx->naughty && --BmUSEFUL(pm->op_pmshort) < 0) {
	    SvREFCNT_dec(pm->op_pmshort);
	    pm->op_pmshort = Nullsv;	/* opt is being useless */
	}
    }
    if (pregexec(rx, s, strend, truebase, minmatch,
		 SvSCREAM(TARG) ? TARG : Nullsv, savematch))
    {
	curpm = pm;
	if (pm->op_pmflags & PMf_ONCE)
	    pm->op_pmdynflags |= PMdf_USED;
	goto gotcha;
    }
    else
	goto ret_no;
    /*NOTREACHED*/

  gotcha:
    rx->exec_tainted |= rxtainted;
    TAINT_IF(rx->exec_tainted);
    if (gimme == G_ARRAY) {
	I32 iters, i, len;

	iters = rx->nparens;
	if (global && !iters)
	    i = 1;
	else
	    i = 0;
	EXTEND(SP, iters + i);
	EXTEND_MORTAL(iters + i);
	for (i = !i; i <= iters; i++) {
	    PUSHs(sv_newmortal());
	    /*SUPPRESS 560*/
	    if ((s = rx->startp[i]) && rx->endp[i] ) {
		len = rx->endp[i] - s;
		sv_setpvn(*SP, s, len);
	    }
	}
	if (global) {
	    truebase = rx->subbeg;
	    strend = rx->subend;
	    if (rx->startp[0] && rx->startp[0] == rx->endp[0])
		++rx->endp[0];
	    goto play_it_again;
	}
	LEAVE_SCOPE(oldsave);
	RETURN;
    }
    else {
	if (global) {
	    MAGIC* mg = 0;
	    if (SvTYPE(TARG) >= SVt_PVMG && SvMAGIC(TARG))
		mg = mg_find(TARG, 'g');
	    if (!mg) {
		sv_magic(TARG, (SV*)0, 'g', Nullch, 0);
		mg = mg_find(TARG, 'g');
	    }
	    if (rx->startp[0]) {
		mg->mg_len = rx->subskip + (rx->endp[0] - rx->subbeg);
		if (rx->startp[0] == rx->endp[0])
		    mg->mg_flags |= MGf_MINMATCH;
		else
		    mg->mg_flags &= ~MGf_MINMATCH;
	    }
	}
	LEAVE_SCOPE(oldsave);
	RETPUSHYES;
    }

yup:
    rx->exec_tainted |= rxtainted;
    TAINT_IF(rx->exec_tainted);
    ++BmUSEFUL(pm->op_pmshort);
    curpm = pm;
    if (pm->op_pmflags & PMf_ONCE)
	pm->op_pmdynflags |= PMdf_USED;
    Safefree(rx->subbase);
    rx->subbase = Nullch;
    if (global) {
	rx->subskip = 0;
	rx->subbeg = truebase;
	rx->subend = strend;
	rx->startp[0] = s;
	rx->endp[0] = s + SvCUR(pm->op_pmshort);
	goto gotcha;
    }
    if (sawampersand) {
	char *tmps;
	char *svptr;
	STRLEN svlen;

	if (sawampersand > 1) {
	    svptr = t;
	    svlen = strend - t;
	}
	else {
	    svptr = s;
	    svlen = SvCUR(pm->op_pmshort);
	}
	rx->subskip = svptr - t;
	tmps = rx->subbase = savepvn(svptr, svlen);
	rx->subbeg = tmps;
	rx->subend = tmps + svlen;
	tmps = rx->startp[0] = tmps + (s - svptr);
	rx->endp[0] = tmps + SvCUR(pm->op_pmshort);
    }
    LEAVE_SCOPE(oldsave);
    RETPUSHYES;

nope:
    if (pm->op_pmshort)
	++BmUSEFUL(pm->op_pmshort);

ret_no:
    if (global && !(pm->op_pmflags & PMf_CONTINUE)) {
	if (SvTYPE(TARG) >= SVt_PVMG && SvMAGIC(TARG)) {
	    MAGIC* mg = mg_find(TARG, 'g');
	    if (mg)
		mg->mg_len = -1;
	}
    }
    LEAVE_SCOPE(oldsave);
    if (gimme == G_ARRAY)
	RETURN;
    RETPUSHNO;
}

OP *
do_readline()
{
    dSP; dTARGETSTACKED;
    register SV *sv;
    STRLEN tmplen = 0;
    STRLEN offset;
    PerlIO *fp;
    register IO *io = GvIO(last_in_gv);
    register I32 type = op->op_type;
    I32 gimme = GIMME_V;
    MAGIC *mg;

    if (SvRMAGICAL(last_in_gv) && (mg = mg_find((SV*)last_in_gv, 'q'))) {
	PUSHMARK(SP);
	XPUSHs(mg->mg_obj);
	PUTBACK;
	ENTER;
	perl_call_method("READLINE", gimme);
	LEAVE;
	SPAGAIN;
	if (gimme == G_SCALAR)
	    SvSetMagicSV_nosteal(TARG, TOPs);
	RETURN;
    }
    fp = Nullfp;
    if (io) {
	fp = IoIFP(io);
	if (!fp) {
	    if (IoFLAGS(io) & IOf_ARGV) {
		if (IoFLAGS(io) & IOf_START) {
		    IoFLAGS(io) &= ~IOf_START;
		    IoLINES(io) = 0;
		    if (av_len(GvAVn(last_in_gv)) < 0) {
			do_open(last_in_gv,"-",1,FALSE,0,0,Nullfp);
			sv_setpvn(GvSV(last_in_gv), "-", 1);
			SvSETMAGIC(GvSV(last_in_gv));
			fp = IoIFP(io);
			goto have_fp;
		    }
		}
		fp = nextargv(last_in_gv);
		if (!fp) { /* Note: fp != IoIFP(io) */
		    (void)do_close(last_in_gv, FALSE); /* now it does*/
		    IoFLAGS(io) |= IOf_START;
		}
	    }
	    else if (type == OP_GLOB) {
		SV *tmpcmd = NEWSV(55, 0);
		SV *tmpglob = POPs;
		ENTER;
		SAVEFREESV(tmpcmd);
#ifdef VMS /* expand the wildcards right here, rather than opening a pipe, */
           /* since spawning off a process is a real performance hit */
		{
#include <descrip.h>
#include <lib$routines.h>
#include <nam.h>
#include <rmsdef.h>
		    char rslt[NAM$C_MAXRSS+1+sizeof(unsigned short int)] = {'\0','\0'};
		    char vmsspec[NAM$C_MAXRSS+1];
		    char *rstr = rslt + sizeof(unsigned short int), *begin, *end, *cp;
		    char tmpfnam[L_tmpnam] = "SYS$SCRATCH:";
		    $DESCRIPTOR(dfltdsc,"SYS$DISK:[]*.*;");
		    PerlIO *tmpfp;
		    STRLEN i;
		    struct dsc$descriptor_s wilddsc
		       = {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
		    struct dsc$descriptor_vs rsdsc
		       = {sizeof rslt, DSC$K_DTYPE_VT, DSC$K_CLASS_VS, rslt};
		    unsigned long int cxt = 0, sts = 0, ok = 1, hasdir = 0, hasver = 0, isunix = 0;

		    /* We could find out if there's an explicit dev/dir or version
		       by peeking into lib$find_file's internal context at
		       ((struct NAM *)((struct FAB *)cxt)->fab$l_nam)->nam$l_fnb
		       but that's unsupported, so I don't want to do it now and
		       have it bite someone in the future. */
		    strcat(tmpfnam,tmpnam(NULL));
		    cp = SvPV(tmpglob,i);
		    for (; i; i--) {
		       if (cp[i] == ';') hasver = 1;
		       if (cp[i] == '.') {
		           if (sts) hasver = 1;
		           else sts = 1;
		       }
		       if (cp[i] == '/') {
		          hasdir = isunix = 1;
		          break;
		       }
		       if (cp[i] == ']' || cp[i] == '>' || cp[i] == ':') {
		           hasdir = 1;
		           break;
		       }
		    }
		    if ((tmpfp = PerlIO_open(tmpfnam,"w+","fop=dlt")) != NULL) {
		        ok = ((wilddsc.dsc$a_pointer = tovmsspec(SvPVX(tmpglob),vmsspec)) != NULL);
		        if (ok) wilddsc.dsc$w_length = (unsigned short int) strlen(wilddsc.dsc$a_pointer);
		        while (ok && ((sts = lib$find_file(&wilddsc,&rsdsc,&cxt,
		                                    &dfltdsc,NULL,NULL,NULL))&1)) {
		            end = rstr + (unsigned long int) *rslt;
		            if (!hasver) while (*end != ';') end--;
		            *(end++) = '\n';  *end = '\0';
		            for (cp = rstr; *cp; cp++) *cp = _tolower(*cp);
		            if (hasdir) {
		              if (isunix) trim_unixpath(rstr,SvPVX(tmpglob),1);
		              begin = rstr;
		            }
		            else {
		                begin = end;
		                while (*(--begin) != ']' && *begin != '>') ;
		                ++begin;
		            }
		            ok = (PerlIO_puts(tmpfp,begin) != EOF);
		        }
		        if (cxt) (void)lib$find_file_end(&cxt);
		        if (ok && sts != RMS$_NMF &&
		            sts != RMS$_DNF && sts != RMS$_FNF) ok = 0;
		        if (!ok) {
		            if (!(sts & 1)) {
		              SETERRNO((sts == RMS$_SYN ? EINVAL : EVMSERR),sts);
		            }
		            PerlIO_close(tmpfp);
		            fp = NULL;
		        }
		        else {
		           PerlIO_rewind(tmpfp);
		           IoTYPE(io) = '<';
		           IoIFP(io) = fp = tmpfp;
		           IoFLAGS(io) &= ~IOf_UNTAINT;  /* maybe redundant */
		        }
		    }
		}
#else /* !VMS */
#ifdef DOSISH
#ifdef OS2
		sv_setpv(tmpcmd, "for a in ");
		sv_catsv(tmpcmd, tmpglob);
		sv_catpv(tmpcmd, "; do echo \"$a\\0\\c\"; done |");
#else
		sv_setpv(tmpcmd, "perlglob ");
		sv_catsv(tmpcmd, tmpglob);
		sv_catpv(tmpcmd, " |");
#endif /* !OS2 */
#else /* !DOSISH */
#if defined(CSH)
		sv_setpvn(tmpcmd, cshname, cshlen);
		sv_catpv(tmpcmd, " -cf 'set nonomatch; glob ");
		sv_catsv(tmpcmd, tmpglob);
		sv_catpv(tmpcmd, "' 2>/dev/null |");
#else
		sv_setpv(tmpcmd, "echo ");
		sv_catsv(tmpcmd, tmpglob);
#if 'z' - 'a' == 25
		sv_catpv(tmpcmd, "|tr -s ' \t\f\r' '\\012\\012\\012\\012'|");
#else
		sv_catpv(tmpcmd, "|tr -s ' \t\f\r' '\\n\\n\\n\\n'|");
#endif
#endif /* !CSH */
#endif /* !DOSISH */
		(void)do_open(last_in_gv, SvPVX(tmpcmd), SvCUR(tmpcmd),
			      FALSE, 0, 0, Nullfp);
		fp = IoIFP(io);
#endif /* !VMS */
		LEAVE;
	    }
	}
	else if (type == OP_GLOB)
	    SP--;
    }
    if (!fp) {
	if (dowarn && io && !(IoFLAGS(io) & IOf_START))
	    warn("Read on closed filehandle <%s>", GvENAME(last_in_gv));
	if (gimme == G_SCALAR) {
	    (void)SvOK_off(TARG);
	    PUSHTARG;
	}
	RETURN;
    }
  have_fp:
    if (gimme == G_SCALAR) {
	sv = TARG;
	if (SvROK(sv))
	    sv_unref(sv);
	(void)SvUPGRADE(sv, SVt_PV);
	tmplen = SvLEN(sv);	/* remember if already alloced */
	if (!tmplen)
	    Sv_Grow(sv, 80);	/* try short-buffering it */
	if (type == OP_RCATLINE)
	    offset = SvCUR(sv);
	else
	    offset = 0;
    }
    else {
	sv = sv_2mortal(NEWSV(57, 80));
	offset = 0;
    }
    for (;;) {
	if (!sv_gets(sv, fp, offset)) {
	    PerlIO_clearerr(fp);
	    if (IoFLAGS(io) & IOf_ARGV) {
		fp = nextargv(last_in_gv);
		if (fp)
		    continue;
		(void)do_close(last_in_gv, FALSE);
		IoFLAGS(io) |= IOf_START;
	    }
	    else if (type == OP_GLOB) {
		if (do_close(last_in_gv, FALSE) & ~0xFF)
		    warn("internal error: glob failed");
	    }
	    if (gimme == G_SCALAR) {
		(void)SvOK_off(TARG);
		PUSHTARG;
	    }
	    RETURN;
	}
	/* This should not be marked tainted if the fp is marked clean */
	if (!(IoFLAGS(io) & IOf_UNTAINT)) {
	    TAINT;
	    SvTAINTED_on(sv);
	}
	IoLINES(io)++;
	SvSETMAGIC(sv);
	XPUSHs(sv);
	if (type == OP_GLOB) {
	    char *tmps;

	    if (SvCUR(sv) > 0 && SvCUR(rs) > 0) {
		tmps = SvEND(sv) - 1;
		if (*tmps == *SvPVX(rs)) {
		    *tmps = '\0';
		    SvCUR(sv)--;
		}
	    }
	    for (tmps = SvPVX(sv); *tmps; tmps++)
		if (!isALPHA(*tmps) && !isDIGIT(*tmps) &&
		    strchr("$&*(){}[]'\";\\|?<>~`", *tmps))
			break;
	    if (*tmps && Stat(SvPVX(sv), &statbuf) < 0) {
		(void)POPs;		/* Unmatched wildcard?  Chuck it... */
		continue;
	    }
	}
	if (gimme == G_ARRAY) {
	    if (SvLEN(sv) - SvCUR(sv) > 20) {
		SvLEN_set(sv, SvCUR(sv)+1);
		Renew(SvPVX(sv), SvLEN(sv), char);
	    }
	    sv = sv_2mortal(NEWSV(58, 80));
	    continue;
	}
	else if (gimme == G_SCALAR && !tmplen && SvLEN(sv) - SvCUR(sv) > 80) {
	    /* try to reclaim a bit of scalar space (only on 1st alloc) */
	    if (SvCUR(sv) < 60)
		SvLEN_set(sv, 80);
	    else
		SvLEN_set(sv, SvCUR(sv)+40);	/* allow some slop */
	    Renew(SvPVX(sv), SvLEN(sv), char);
	}
	RETURN;
    }
}

PP(pp_enter)
{
    djSP;
    register PERL_CONTEXT *cx;
    I32 gimme = OP_GIMME(op, -1);

    if (gimme == -1) {
	if (cxstack_ix >= 0)
	    gimme = cxstack[cxstack_ix].blk_gimme;
	else
	    gimme = G_SCALAR;
    }

    ENTER;

    SAVETMPS;
    PUSHBLOCK(cx, CXt_BLOCK, SP);

    RETURN;
}

PP(pp_helem)
{
    djSP;
    HE* he;
    SV *keysv = POPs;
    HV *hv = (HV*)POPs;
    U32 lval = op->op_flags & OPf_MOD;
    U32 defer = op->op_private & OPpLVAL_DEFER;

    if (SvTYPE(hv) != SVt_PVHV)
	RETPUSHUNDEF;
    he = hv_fetch_ent(hv, keysv, lval && !defer, 0);
    if (lval) {
	if (!he || HeVAL(he) == &sv_undef) {
	    SV* lv;
	    SV* key2;
	    if (!defer)
		DIE(no_helem, SvPV(keysv, na));
	    lv = sv_newmortal();
	    sv_upgrade(lv, SVt_PVLV);
	    LvTYPE(lv) = 'y';
	    sv_magic(lv, key2 = newSVsv(keysv), 'y', Nullch, 0);
	    SvREFCNT_dec(key2);	/* sv_magic() increments refcount */
	    LvTARG(lv) = SvREFCNT_inc(hv);
	    LvTARGLEN(lv) = 1;
	    PUSHs(lv);
	    RETURN;
	}
	if (op->op_private & OPpLVAL_INTRO) {
	    if (HvNAME(hv) && isGV(HeVAL(he)))
		save_gp((GV*)HeVAL(he), !(op->op_flags & OPf_SPECIAL));
	    else
		save_helem(hv, keysv, &HeVAL(he));
	}
	else if (op->op_private & OPpDEREF)
	    vivify_ref(HeVAL(he), op->op_private & OPpDEREF);
    }
    PUSHs(he ? HeVAL(he) : &sv_undef);
    RETURN;
}

PP(pp_leave)
{
    djSP;
    register PERL_CONTEXT *cx;
    register SV **mark;
    SV **newsp;
    PMOP *newpm;
    I32 gimme;

    if (op->op_flags & OPf_SPECIAL) {
	cx = &cxstack[cxstack_ix];
	cx->blk_oldpm = curpm;	/* fake block should preserve $1 et al */
    }

    POPBLOCK(cx,newpm);

    gimme = OP_GIMME(op, -1);
    if (gimme == -1) {
	if (cxstack_ix >= 0)
	    gimme = cxstack[cxstack_ix].blk_gimme;
	else
	    gimme = G_SCALAR;
    }

    TAINT_NOT;
    if (gimme == G_VOID)
	SP = newsp;
    else if (gimme == G_SCALAR) {
	MARK = newsp + 1;
	if (MARK <= SP)
	    if (SvFLAGS(TOPs) & (SVs_PADTMP|SVs_TEMP))
		*MARK = TOPs;
	    else
		*MARK = sv_mortalcopy(TOPs);
	else {
	    MEXTEND(mark,0);
	    *MARK = &sv_undef;
	}
	SP = MARK;
    }
    else if (gimme == G_ARRAY) {
	/* in case LEAVE wipes old return values */
	for (mark = newsp + 1; mark <= SP; mark++) {
	    if (!(SvFLAGS(*mark) & (SVs_PADTMP|SVs_TEMP))) {
		*mark = sv_mortalcopy(*mark);
		TAINT_NOT;	/* Each item is independent */
	    }
	}
    }
    curpm = newpm;	/* Don't pop $1 et al till now */

    LEAVE;

    RETURN;
}

PP(pp_iter)
{
    djSP;
    register PERL_CONTEXT *cx;
    SV* sv;
    AV* av;

    EXTEND(SP, 1);
    cx = &cxstack[cxstack_ix];
    if (cx->cx_type != CXt_LOOP)
	DIE("panic: pp_iter");

    av = cx->blk_loop.iterary;
    if (cx->blk_loop.iterix >= (av == curstack ? cx->blk_oldsp : AvFILL(av)))
	RETPUSHNO;

    SvREFCNT_dec(*cx->blk_loop.itervar);

    if (sv = AvARRAY(av)[++cx->blk_loop.iterix])
	SvTEMP_off(sv);
    else
	sv = &sv_undef;
    if (av != curstack && SvIMMORTAL(sv)) {
	SV *lv = cx->blk_loop.iterlval;
	if (lv && SvREFCNT(lv) > 1) {
	    SvREFCNT_dec(lv);
	    lv = Nullsv;
	}
	if (lv)
	    SvREFCNT_dec(LvTARG(lv));
	else {
	    lv = cx->blk_loop.iterlval = NEWSV(26, 0);
	    sv_upgrade(lv, SVt_PVLV);
	    LvTYPE(lv) = 'y';
	    sv_magic(lv, Nullsv, 'y', Nullch, 0);
	}
	LvTARG(lv) = SvREFCNT_inc(av);
	LvTARGOFF(lv) = cx->blk_loop.iterix;
	LvTARGLEN(lv) = (UV) -1;
	sv = (SV*)lv;
    }

    *cx->blk_loop.itervar = SvREFCNT_inc(sv);
    RETPUSHYES;
}

PP(pp_subst)
{
    djSP; dTARG;
    register PMOP *pm = cPMOP;
    PMOP *rpm = pm;
    register SV *dstr;
    register char *s;
    char *strend;
    register char *m;
    char *c;
    register char *d;
    STRLEN clen;
    I32 iters = 0;
    I32 maxiters;
    register I32 i;
    bool once;
    bool rxtainted;
    char *orig;
    I32 savematch;
    register REGEXP *rx = pm->op_pmregexp;
    STRLEN len;
    int force_on_match = 0;
    I32 oldsave = savestack_ix;

    /* known replacement string? */
    dstr = (pm->op_pmflags & PMf_CONST) ? POPs : Nullsv;
    if (op->op_flags & OPf_STACKED)
	TARG = POPs;
    else {
	TARG = GvSV(defgv);
	EXTEND(SP,1);
    }
    if (SvREADONLY(TARG)
	|| (SvTYPE(TARG) > SVt_PVLV
	    && !(SvTYPE(TARG) == SVt_PVGV && SvFAKE(TARG))))
	croak(no_modify);
    PUTBACK;
    s = SvPV(TARG, len);
    if (!SvPOKp(TARG) || SvTYPE(TARG) == SVt_PVGV)
	force_on_match = 1;
    rxtainted = ((pm->op_pmdynflags & PMdf_TAINTED) ||
		 (tainted && (pm->op_pmflags & PMf_RETAINT)));
    if (tainted)
	rxtainted |= 2;
    TAINT_NOT;

  force_it:
    if (!pm || !s)
	DIE("panic: do_subst");

    strend = s + len;
    maxiters = (strend - s) + 10;

    if (!rx->prelen && curpm) {
	pm = curpm;
	rx = pm->op_pmregexp;
    }
    if (pm->op_pmflags & (PMf_MULTILINE|PMf_SINGLELINE)) {
	SAVEINT(multiline);
	multiline = pm->op_pmflags & PMf_MULTILINE;
    }
    orig = m = s;
    if (pm->op_pmshort) {
	if (pm->op_pmflags & PMf_SCANFIRST) {
	    if (SvSCREAM(TARG)) {
		if (screamfirst[BmRARE(pm->op_pmshort)] < 0)
		    goto nope;
		else if (!(s = screaminstr(TARG, pm->op_pmshort)))
		    goto nope;
	    }
	    else if (!(s = fbm_instr((unsigned char*)s, (unsigned char*)strend,
	      pm->op_pmshort)))
		goto nope;
	    if (s && rx->regback >= 0) {
		++BmUSEFUL(pm->op_pmshort);
		s -= rx->regback;
		if (s < m)
		    s = m;
	    }
	    else
		s = m;
	}
	else if (!multiline) {
	    if (*SvPVX(pm->op_pmshort) != *s
		|| (pm->op_pmslen > 1
		    && memNE(SvPVX(pm->op_pmshort), s, pm->op_pmslen)))
		goto nope;
	}
	if (!rx->naughty && --BmUSEFUL(pm->op_pmshort) < 0) {
	    SvREFCNT_dec(pm->op_pmshort);
	    pm->op_pmshort = Nullsv;	/* opt is being useless */
	}
    }

    /* only replace once? */
    once = !(rpm->op_pmflags & PMf_GLOBAL);

    /* known replacement string? */
    c = dstr ? SvPV(dstr, clen) : Nullch;

    /* need to save results? */
    savematch = sawampersand | (rx->nparens != 0);
    /* save whole string, even prefix, for complex replacement */
    if (!c)
	savematch |= 16;

    /* can do inplace substitution? */
    if (c && clen <= rx->minlen && !savematch) {
	if (! pregexec(rx, s, strend, orig, 0,
		       SvSCREAM(TARG) ? TARG : Nullsv, 0)) {
	    PUSHs(&sv_no);
	    LEAVE_SCOPE(oldsave);
	    RETURN;
	}
	if (force_on_match) {
	    force_on_match = 0;
	    s = SvPV_force(TARG, len);
	    goto force_it;
	}
	d = s;
	curpm = pm;
	SvSCREAM_off(TARG);	/* disable possible screamer */
	if (once) {
	    rxtainted |= rx->exec_tainted;
	    m = rx->startp[0];
	    d = rx->endp[0];
	    s = orig;
	    if (m - s > strend - d) {  /* faster to shorten from end */
		if (clen) {
		    Copy(c, m, clen, char);
		    m += clen;
		}
		i = strend - d;
		if (i > 0) {
		    Move(d, m, i, char);
		    m += i;
		}
		*m = '\0';
		SvCUR_set(TARG, m - s);
	    }
	    /*SUPPRESS 560*/
	    else if (i = m - s) {	/* faster from front */
		d -= clen;
		m = d;
		sv_chop(TARG, d-i);
		s += i;
		while (i--)
		    *--d = *--s;
		if (clen)
		    Copy(c, m, clen, char);
	    }
	    else if (clen) {
		d -= clen;
		sv_chop(TARG, d);
		Copy(c, d, clen, char);
	    }
	    else {
		sv_chop(TARG, d);
	    }
	    TAINT_IF(rxtainted & 1);
	    PUSHs(&sv_yes);
	}
	else {
	    do {
		if (iters++ > maxiters)
		    DIE("Substitution loop");
		rxtainted |= rx->exec_tainted;
		m = rx->startp[0];
		i = rx->subskip + (m - rx->subbeg) - (s - orig);
		if (i) {
		    if (s != d)
			Move(s, d, i, char);
		    d += i;
		}
		if (clen) {
		    Copy(c, d, clen, char);
		    d += clen;
		}
		s = orig + rx->subskip + (rx->endp[0] - rx->subbeg);
	    } while (pregexec(rx, s, strend, orig, s == m, Nullsv, 0));
		      /* don't match same null twice ^^ */
	    if (s != d) {
		i = strend - s;
		SvCUR_set(TARG, d - SvPVX(TARG) + i);
		Move(s, d, i+1, char);		/* include the NUL */
	    }
	    TAINT_IF(rxtainted & 1);
	    PUSHs(sv_2mortal(newSViv((I32)iters)));
	}
	(void)SvPOK_only(TARG);
	TAINT_IF(rxtainted);
	SvSETMAGIC(TARG);
	SvTAINT(TARG);
	LEAVE_SCOPE(oldsave);
	RETURN;
    }

    if (pregexec(rx, s, strend, orig, 0,
		 SvSCREAM(TARG) ? TARG : Nullsv, savematch)) {
	if (force_on_match) {
	    force_on_match = 0;
	    s = SvPV_force(TARG, len);
	    goto force_it;
	}
	rxtainted |= rx->exec_tainted;
	dstr = NEWSV(25, sv_len(TARG));
	sv_setpvn(dstr, m, s-m);
	curpm = pm;
	if (!c) {
	    register PERL_CONTEXT *cx;
	    PUSHSUBST(cx);
	    RETURNOP(cPMOP->op_pmreplroot);
	}
	do {
	    if (iters++ > maxiters)
		DIE("Substitution loop");
	    rxtainted |= rx->exec_tainted;
	    if (savematch > 1 && rx->subbase && rx->subbase != orig) {
		m = s;
		s = orig;
		orig = rx->subbase;
		s = orig + (m - s);
		strend = s + (strend - m);
		savematch = 0;		/* let's not do THAT again */
	    }
	    m = rx->startp[0];
	    sv_catpvn(dstr, s, rx->subskip + (m - rx->subbeg) - (s - orig));
	    s = orig + rx->subskip + (rx->endp[0] - rx->subbeg);
	    if (clen)
		sv_catpvn(dstr, c, clen);
	    if (once)
		break;
	} while (pregexec(rx, s, strend, orig, s == m, Nullsv, savematch));
	sv_catpvn(dstr, s, strend - s);

	(void)SvOOK_off(TARG);
	Safefree(SvPVX(TARG));
	SvPVX(TARG) = SvPVX(dstr);
	SvCUR_set(TARG, SvCUR(dstr));
	SvLEN_set(TARG, SvLEN(dstr));
	SvPVX(dstr) = 0;
	sv_free(dstr);

	TAINT_IF(rxtainted & 1);
	SPAGAIN;
	PUSHs(sv_2mortal(newSViv((I32)iters)));

	(void)SvPOK_only(TARG);
	TAINT_IF(rxtainted);
	SvSETMAGIC(TARG);
	SvTAINT(TARG);
	LEAVE_SCOPE(oldsave);
	RETURN;
    }
    goto ret_no;

nope:
    ++BmUSEFUL(pm->op_pmshort);

ret_no:
    PUSHs(&sv_no);
    LEAVE_SCOPE(oldsave);
    RETURN;
}

PP(pp_grepwhile)
{
    djSP;

    if (SvTRUEx(POPs))
	stack_base[markstack_ptr[-1]++] = stack_base[*markstack_ptr];
    ++*markstack_ptr;
    LEAVE;					/* exit inner scope */

    /* All done yet? */
    if (stack_base + *markstack_ptr > SP) {
	I32 items;
	I32 gimme = GIMME_V;

	LEAVE;					/* exit outer scope */
	(void)POPMARK;				/* pop src */
	items = --*markstack_ptr - markstack_ptr[-1];
	(void)POPMARK;				/* pop dst */
	SP = stack_base + POPMARK;		/* pop original mark */
	if (gimme == G_SCALAR) {
	    dTARGET;
	    XPUSHi(items);
	}
	else if (gimme == G_ARRAY)
	    SP += items;
	RETURN;
    }
    else {
	SV *src;

	ENTER;					/* enter inner scope */
	SAVESPTR(curpm);

	src = stack_base[*markstack_ptr];
	SvTEMP_off(src);
	DEFSV = src;

	RETURNOP(cLOGOP->op_other);
    }
}

PP(pp_leavesub)
{
    djSP;
    SV **mark;
    SV **newsp;
    PMOP *newpm;
    I32 gimme;
    register PERL_CONTEXT *cx;
    struct block_sub cxsub;

    POPBLOCK(cx,newpm);
    POPSUB1(cx);	/* Delay POPSUB2 until stack values are safe */
 
    TAINT_NOT;
    if (gimme == G_SCALAR) {
	MARK = newsp + 1;
	if (MARK <= SP)
	    *MARK = SvTEMP(TOPs) ? TOPs : sv_mortalcopy(TOPs);
	else {
	    MEXTEND(MARK, 0);
	    *MARK = &sv_undef;
	}
	SP = MARK;
    }
    else if (gimme == G_ARRAY) {
	for (MARK = newsp + 1; MARK <= SP; MARK++) {
	    if (!SvTEMP(*MARK)) {
		*MARK = sv_mortalcopy(*MARK);
		TAINT_NOT;	/* Each item is independent */
	    }
	}
    }
    PUTBACK;
    
    POPSUB2();		/* Stack values are safe: release CV and @_ ... */
    curpm = newpm;	/* ... and pop $1 et al */

    LEAVE;
    return pop_return();
}

PP(pp_entersub)
{
    djSP; dPOPss;
    GV *gv;
    HV *stash;
    register CV *cv;
    register PERL_CONTEXT *cx;
    I32 gimme;
    bool hasargs = (op->op_flags & OPf_STACKED) != 0;

    if (!sv)
	DIE("Not a CODE reference");
    switch (SvTYPE(sv)) {
    default:
	if (!SvROK(sv)) {
	    char *sym;

	    if (sv == &sv_yes) {		/* unfound import, ignore */
		if (hasargs)
		    SP = stack_base + POPMARK;
		RETURN;
	    }
	    if (SvGMAGICAL(sv)) {
		mg_get(sv);
		sym = SvPOKp(sv) ? SvPVX(sv) : Nullch;
	    }
	    else
		sym = SvPV(sv, na);
	    if (!sym)
		DIE(no_usym, "a subroutine");
	    if (op->op_private & HINT_STRICT_REFS)
		DIE(no_symref, sym, "a subroutine");
	    cv = perl_get_cv(sym, TRUE);
	    break;
	}
	cv = (CV*)SvRV(sv);
	if (SvTYPE(cv) == SVt_PVCV)
	    break;
	/* FALL THROUGH */
    case SVt_PVHV:
    case SVt_PVAV:
	DIE("Not a CODE reference");
    case SVt_PVCV:
	cv = (CV*)sv;
	break;
    case SVt_PVGV:
	if (!(cv = GvCVu((GV*)sv)))
	    cv = sv_2cv(sv, &stash, &gv, TRUE);
	break;
    }

    ENTER;
    SAVETMPS;

  retry:
    if (!cv)
	DIE("Not a CODE reference");

    if (!CvROOT(cv) && !CvXSUB(cv)) {
	GV* autogv;
	SV* sub_name;

	/* anonymous or undef'd function leaves us no recourse */
	if (CvANON(cv) || !(gv = CvGV(cv)))
	    DIE("Undefined subroutine called");
	/* autoloaded stub? */
	if (cv != GvCV(gv)) {
	    cv = GvCV(gv);
	    goto retry;
	}
	/* should call AUTOLOAD now? */
	if ((autogv = gv_autoload4(GvSTASH(gv), GvNAME(gv), GvNAMELEN(gv),
				   FALSE)))
	{
	    cv = GvCV(autogv);
	    goto retry;
	}
	/* sorry */
	sub_name = sv_newmortal();
	gv_efullname3(sub_name, gv, Nullch);
	DIE("Undefined subroutine &%s called", SvPVX(sub_name));
    }

    gimme = GIMME_V;
    if ((op->op_private & OPpENTERSUB_DB) && GvCV(DBsub) && !CvNODEBUG(cv)) {
	SV *dbsv = GvSV(DBsub);

	if (!PERLDB_SUB_NN) {
	    GV *gv = CvGV(cv);

	    save_item(dbsv);
	    if ( (CvFLAGS(cv) & (CVf_ANON | CVf_CLONED))
		 || strEQ(GvNAME(gv), "END") 
		 || ((GvCV(gv) != cv) && /* Could be imported, and old sub redefined. */
		     !( (SvTYPE(sv) == SVt_PVGV) && (GvCV((GV*)sv) == cv)
			&& (gv = (GV*)sv) ))) {
		/* Use GV from the stack as a fallback. */
		/* GV is potentially non-unique, or contain different CV. */
		sv_setsv(dbsv, newRV((SV*)cv));
	    }
	    else {
		gv_efullname3(dbsv, gv, Nullch);
	    }
	} else {
	    SvUPGRADE(dbsv, SVt_PVIV);
	    SvIOK_on(dbsv);
	    SAVEIV(SvIVX(dbsv));
	    SvIVX(dbsv) = (IV)cv;		/* Do it the quickest way  */
	}
	if (CvXSUB(cv)) 
	    curcopdb = curcop;
	cv = GvCV(DBsub);
	if (!cv)
	    DIE("No DBsub routine");
    }

    if (CvXSUB(cv)) {
	if (CvOLDSTYLE(cv)) {
	    I32 (*fp3)_((int,int,int));
	    dMARK;
	    register I32 items = SP - MARK;
					/* We dont worry to copy from @_. */
	    while (SP > mark) {
		SP[1] = SP[0];
		SP--;
	    }
	    stack_sp = mark + 1;
	    fp3 = (I32(*)_((int,int,int)))CvXSUB(cv);
	    items = (*fp3)(CvXSUBANY(cv).any_i32, 
			   MARK - stack_base + 1,
			   items);
	    stack_sp = stack_base + items;
	}
	else {
	    I32 markix = TOPMARK;

	    PUTBACK;

	    if (!hasargs) {
		/* Need to copy @_ to stack. Alternative may be to
		 * switch stack to @_, and copy return values
		 * back. This would allow popping @_ in XSUB, e.g.. XXXX */
		AV* av;
		I32 items;
		av = GvAV(defgv);
		items = AvFILLp(av) + 1;   /* @_ is not tieable */

		if (items) {
		    /* Mark is at the end of the stack. */
		    EXTEND(SP, items);
		    Copy(AvARRAY(av), SP + 1, items, SV*);
		    SP += items;
		    PUTBACK ;		    
		}
	    }
	    if (curcopdb) {		/* We assume that the first
					   XSUB in &DB::sub is the
					   called one. */
		SAVESPTR(curcop);
		curcop = curcopdb;
		curcopdb = NULL;
	    }
	    /* Do we need to open block here? XXXX */
	    (void)(*CvXSUB(cv))(cv);

	    /* Enforce some sanity in scalar context. */
	    if (gimme == G_SCALAR && ++markix != stack_sp - stack_base ) {
		if (markix > stack_sp - stack_base)
		    *(stack_base + markix) = &sv_undef;
		else
		    *(stack_base + markix) = *stack_sp;
		stack_sp = stack_base + markix;
	    }
	}
	LEAVE;
	return NORMAL;
    }
    else {
	dMARK;
	register I32 items = SP - MARK;
	AV* padlist = CvPADLIST(cv);
	SV** svp = AvARRAY(padlist);
	push_return(op->op_next);
	PUSHBLOCK(cx, CXt_SUB, MARK);
	PUSHSUB(cx);
	CvDEPTH(cv)++;
	if (CvDEPTH(cv) < 2)
	    (void)SvREFCNT_inc(cv);
	else {	/* save temporaries on recursion? */
	    if (CvDEPTH(cv) == 100 && dowarn 
		  && !(PERLDB_SUB && cv == GvCV(DBsub)))
		sub_crush_depth(cv);
	    if (CvDEPTH(cv) > AvFILLp(padlist)) {
		AV *av;
		AV *newpad = newAV();
		SV **oldpad = AvARRAY(svp[CvDEPTH(cv)-1]);
		I32 ix = AvFILLp((AV*)svp[1]);
		svp = AvARRAY(svp[0]);
		for ( ;ix > 0; ix--) {
		    if (svp[ix] != &sv_undef) {
			char *name = SvPVX(svp[ix]);
			if ((SvFLAGS(svp[ix]) & SVf_FAKE) /* outer lexical? */
			    || *name == '&')		  /* anonymous code? */
			{
			    av_store(newpad, ix, SvREFCNT_inc(oldpad[ix]));
			}
			else {				/* our own lexical */
			    if (*name == '@')
				av_store(newpad, ix, sv = (SV*)newAV());
			    else if (*name == '%')
				av_store(newpad, ix, sv = (SV*)newHV());
			    else
				av_store(newpad, ix, sv = NEWSV(0,0));
			    SvPADMY_on(sv);
			}
		    }
		    else {
			av_store(newpad, ix, sv = NEWSV(0,0));
			SvPADTMP_on(sv);
		    }
		}
		av = newAV();		/* will be @_ */
		av_extend(av, 0);
		av_store(newpad, 0, (SV*)av);
		AvFLAGS(av) = AVf_REIFY;
		av_store(padlist, CvDEPTH(cv), (SV*)newpad);
		AvFILLp(padlist) = CvDEPTH(cv);
		svp = AvARRAY(padlist);
	    }
	}
	SAVESPTR(curpad);
	curpad = AvARRAY((AV*)svp[CvDEPTH(cv)]);
	if (hasargs) {
	    AV* av = (AV*)curpad[0];
	    SV** ary;

	    if (AvREAL(av)) {
		av_clear(av);
		AvREAL_off(av);
	    }
	    cx->blk_sub.savearray = GvAV(defgv);
	    cx->blk_sub.argarray = av;
	    GvAV(defgv) = (AV*)SvREFCNT_inc(av);
	    ++MARK;

	    if (items > AvMAX(av) + 1) {
		ary = AvALLOC(av);
		if (AvARRAY(av) != ary) {
		    AvMAX(av) += AvARRAY(av) - AvALLOC(av);
		    SvPVX(av) = (char*)ary;
		}
		if (items > AvMAX(av) + 1) {
		    AvMAX(av) = items - 1;
		    Renew(ary,items,SV*);
		    AvALLOC(av) = ary;
		    SvPVX(av) = (char*)ary;
		}
	    }
	    Copy(MARK,AvARRAY(av),items,SV*);
	    AvFILLp(av) = items - 1;
	    
	    while (items--) {
		if (*MARK)
		    SvTEMP_off(*MARK);
		MARK++;
	    }
	}
#if 0
	DEBUG_L(PerlIO_printf(PerlIO_stderr(),
			      "%p entersub returning %p\n", thr, CvSTART(cv)));
#endif
	RETURNOP(CvSTART(cv));
    }
}

void
sub_crush_depth(cv)
CV* cv;
{
    if (CvANON(cv))
	warn("Deep recursion on anonymous subroutine");
    else {
	SV* tmpstr = sv_newmortal();
	gv_efullname3(tmpstr, CvGV(cv), Nullch);
	warn("Deep recursion on subroutine \"%s\"", SvPVX(tmpstr));
    }
}

PP(pp_aelem)
{
    djSP;
    SV** svp;
    I32 elem = POPi;
    AV* av = (AV*)POPs;
    U32 lval = op->op_flags & OPf_MOD;
    U32 defer = (op->op_private & OPpLVAL_DEFER) && (elem > AvFILL(av));

    if (elem > 0)
	elem -= curcop->cop_arybase;
    if (SvTYPE(av) != SVt_PVAV)
	RETPUSHUNDEF;
    svp = av_fetch(av, elem, lval && !defer);
    if (lval) {
	if (!svp || *svp == &sv_undef) {
	    SV* lv;
	    if (!defer)
		DIE(no_aelem, elem);
	    lv = sv_newmortal();
	    sv_upgrade(lv, SVt_PVLV);
	    LvTYPE(lv) = 'y';
	    sv_magic(lv, Nullsv, 'y', Nullch, 0);
	    LvTARG(lv) = SvREFCNT_inc(av);
	    LvTARGOFF(lv) = elem;
	    LvTARGLEN(lv) = 1;
	    PUSHs(lv);
	    RETURN;
	}
	if (op->op_private & OPpLVAL_INTRO)
	    save_aelem(av, elem, svp);
	else if (op->op_private & OPpDEREF)
	    vivify_ref(*svp, op->op_private & OPpDEREF);
    }
    PUSHs(svp ? *svp : &sv_undef);
    RETURN;
}

void
vivify_ref(sv, to_what)
SV* sv;
U32 to_what;
{
    if (SvGMAGICAL(sv))
	mg_get(sv);
    if (!SvOK(sv)) {
	if (SvREADONLY(sv))
	    croak(no_modify);
	if (SvTYPE(sv) < SVt_RV)
	    sv_upgrade(sv, SVt_RV);
	else if (SvTYPE(sv) >= SVt_PV) {
	    (void)SvOOK_off(sv);
	    Safefree(SvPVX(sv));
	    SvLEN(sv) = SvCUR(sv) = 0;
	}
	switch (to_what) {
	case OPpDEREF_SV:
	    SvRV(sv) = NEWSV(355,0);
	    break;
	case OPpDEREF_AV:
	    SvRV(sv) = (SV*)newAV();
	    break;
	case OPpDEREF_HV:
	    SvRV(sv) = (SV*)newHV();
	    break;
	}
	SvROK_on(sv);
	SvSETMAGIC(sv);
    }
}

PP(pp_method)
{
    djSP;
    SV* sv;
    SV* ob;
    GV* gv;
    HV* stash;
    char* name;
    char* packname;
    STRLEN packlen;

    if (SvROK(TOPs)) {
	sv = SvRV(TOPs);
	if (SvTYPE(sv) == SVt_PVCV) {
	    SETs(sv);
	    RETURN;
	}
    }

    name = SvPV(TOPs, na);
    sv = *(stack_base + TOPMARK + 1);
    
    if (SvGMAGICAL(sv))
        mg_get(sv);
    if (SvROK(sv))
	ob = (SV*)SvRV(sv);
    else {
	GV* iogv;

	packname = Nullch;
	if (!SvOK(sv) ||
	    !(packname = SvPV(sv, packlen)) ||
	    !(iogv = gv_fetchpv(packname, FALSE, SVt_PVIO)) ||
	    !(ob=(SV*)GvIO(iogv)))
	{
	    if (!packname || !isIDFIRST(*packname))
		DIE("Can't call method \"%s\" %s", name,
		    SvOK(sv)? "without a package or object reference"
			    : "on an undefined value");
	    stash = gv_stashpvn(packname, packlen, TRUE);
	    goto fetch;
	}
	*(stack_base + TOPMARK + 1) = sv_2mortal(newRV((SV*)iogv));
    }

    if (!ob || !SvOBJECT(ob))
	DIE("Can't call method \"%s\" on unblessed reference", name);

    stash = SvSTASH(ob);

  fetch:
    gv = gv_fetchmethod(stash, name);
    if (!gv) {
	char* leaf = name;
	char* sep = Nullch;
	char* p;

	for (p = name; *p; p++) {
	    if (*p == '\'')
		sep = p, leaf = p + 1;
	    else if (*p == ':' && *(p + 1) == ':')
		sep = p, leaf = p + 2;
	}
	if (!sep || ((sep - name) == 5 && strnEQ(name, "SUPER", 5))) {
	    packname = HvNAME(sep ? curcop->cop_stash : stash);
	    packlen = strlen(packname);
	}
	else {
	    packname = name;
	    packlen = sep - name;
	}
	DIE("Can't locate object method \"%s\" via package \"%.*s\"",
	    leaf, (int)packlen, packname);
    }
    SETs(isGV(gv) ? (SV*)GvCV(gv) : (SV*)gv);
    RETURN;
}
