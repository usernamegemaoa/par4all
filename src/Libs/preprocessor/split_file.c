/*
 * $Id$
 *
 * adapted from what can be seen by FC 31/12/96
 * 
 * - static declarations; 
 * - main -> function; 
 * - stdout -> FILE* out;
 * - include unistd added
 * - exit -> return
 * - close ifp
 * - bug labeled end (skipped) in lend()
 * - tab in first columns...
 * - bang comments added
 * - bug name[20] overflow not checked in lname (20 -> 80)
 * - hollerith constants conversion;-)
 * - LINESIZE 80 -> 200...
 * - "PROGRAM MAIN..." added if implicit program name.
 * - extr* stuff dropped.
 * - dir_name for localizing files...
 * - \r skipped
 * - last line may not be \n'ed.
 * - bang comment management added (to avoid the parser)
 *
 * $Log: split_file.c,v $
 * Revision 1.38  1998/07/10 08:17:54  coelho
 * entry-related bug fixed.
 *
 * Revision 1.37  1998/07/09 18:24:18  coelho
 * hack for entry: output file names are preceded by their modules...
 *
 * Revision 1.36  1998/05/29 16:22:27  coelho
 * handler for simply processing files.
 *
 * Revision 1.35  1998/05/29 13:18:58  coelho
 * bang comment skipped... (beurk).
 *
 */

static void hollerith_and_bangcomments(char *);
#define LINESIZE 200

/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Asa Romberger and Jerry Berkman.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char fsplit_copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
char fsplit_sccsid[] = "@(#)fsplit.c	5.5 (Berkeley) 3/12/91";
#endif /* not lint */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 *	usage:		fsplit [-e efile] ... [file]
 *
 *	split single file containing source for several fortran programs
 *		and/or subprograms into files each containing one
 *		subprogram unit.
 *	each separate file will be named using the corresponding subroutine,
 *		function, block data or program name if one is found; otherwise
 *		the name will be of the form mainNNN.f or blkdtaNNN.f .
 *		If a file of that name exists, it is saved in a name of the
 *		form zzz000.f .
 *	If -e option is used, then only those subprograms named in the -e
 *		option are split off; e.g.:
 *			fsplit -esub1 -e sub2 prog.f
 *		isolates sub1 and sub2 in sub1.f and sub2.f.  The space 
 *		after -e is optional.
 *
 *    Modified Feb., 1983 by Jerry Berkman, Computing Services, U.C. Berkeley.
 *		- added comments
 *		- more function types: double complex, character*(*), etc.
 *		- fixed minor bugs
 *		- instead of all unnamed going into zNNN.f, put mains in
 *		  mainNNN.f, block datas in blkdtaNNN.f, dups in zzzNNN.f .
 */

#define BSZ 512
static char buf[BSZ];
static FILE *ifp;
static char *x, *mainp, *blkp;

#define TRUE 1
#define FALSE 0
static struct stat sbuf;

static char *look(), *skiplab(), *functs();
static int scan_name();
static void get_name();

#define trim(p)	while (*p == ' ' || *p == '\t') p++

static int saveit(char * name)
{
    return 1;
}

static char * full_name(char * dir, char * name)
{
    char * full = (char*) malloc(sizeof(char)*(strlen(dir)+strlen(name)+2));
    sprintf(full, "%s/%s", dir, name);
    return full;
}

static void get_name(name)
char *name;
{
	register char *ptr;

	while (stat(name, &sbuf) >= 0) 
	{
	    ptr = name + strlen(name) - 1;
	    while (!isdigit(*ptr--) && ptr>name);
	    for (ptr++; isdigit(*ptr) && ptr>name; ptr--) {
		(*ptr)++;
		if (*ptr <= '9')
		    break;
		*ptr = '0';
	    }
	    if(ptr < name ) {
		fprintf( stderr, "fsplit: ran out of file names\n");
		exit(1);
	    }
	}
}

static int getline()
{
	register char *ptr;

	if (feof(ifp)) return -1;

	for (ptr = buf; ptr < &buf[BSZ]; ) {
		*ptr = getc(ifp);
		/* fix for the last line that may not have a \n.
		 * It is returned however and lend handles it correctly.
		 */
		if (feof(ifp) || *ptr++ == '\n')
		  {
		    *ptr = 0;
		    return (1);
		  }
	}
	while (getc(ifp) != '\n' && feof(ifp) == 0) ;
	fprintf(stderr, "line truncated to %d characters\n", BSZ);
	return (1);
}

static char * skip_comment_if_any(char * lines)
{
    int i = 0;

    while (lines[i]=='c' || lines[i]=='C' || lines[i]=='*' || lines[i]=='!')
    {
	while (lines[i]!='\0' && lines[i]!='\n') i++;
	if (lines[i]=='\n') i++;
    }

    return lines+i;
}

/* return 1 for 'end' alone on card (up to col. 72),  0 otherwise */
static int lend()
{
	register char *p, * lbuf;
	int tab = FALSE;

	lbuf = skip_comment_if_any(buf);

	for (p=lbuf; p<&lbuf[6] && !tab; p++)
	{
	    if (*p=='\0') return 0;
	    if (*p=='\t') tab=TRUE;
	}
	
	if (!tab && (lbuf[5]!=' ' && lbuf[5]!='\t')) 
	    return 0; /* a continuation */
	    
	trim(p);
	if (*p != 'e' && *p != 'E') return(0);
	p++;
	trim(p);
	if (*p != 'n' && *p != 'N') return(0);
	p++;
	trim(p);
	if (*p != 'd' && *p != 'D') return(0);
	p++;
	trim(p);
	if (p - buf >= 72 || *p == '\n' || *p == '\r' || *p == '\0')
		return (1);
	return (0);
}

static int implicit_program; /* FC */
static int implicit_blockdata_name; /* FC */
static int implicit_program_name; /* FC */
static int it_is_a_main; /* FC */

/*		check for keywords for subprograms	
		return 0 if comment card, 1 if found
		name and put in arg string. invent name for unnamed
		block datas and main programs.		*/
static int lname(s)
char *s;
{
	register char *ptr, *p;
	char	line[LINESIZE], *iptr = line, * lbuf;

	implicit_program = 0;
	implicit_blockdata_name = 0;
	implicit_program_name = 0;
	it_is_a_main = 0;

	lbuf = skip_comment_if_any(buf);

	/* first check for comment cards */
	if(lbuf[0]=='c' || lbuf[0]=='C' || lbuf[0]=='*' || lbuf[0]=='!') 
	    return 0;
	ptr = lbuf;
	while (*ptr == ' ' || *ptr == '\t') ptr++;
	if(*ptr == '\n') return(0);

	ptr = skiplab(lbuf);
	if (ptr == 0) return (0);

	/*  copy to buffer and converting to lower case */
	p = ptr;
	while (*p && p <= &lbuf[71] ) {
	   *iptr = isupper(*p) ? tolower(*p) : *p;
	   iptr++;
	   p++;
	}
	*iptr = '\n';

	if ((ptr = look(line, "subroutine")) != 0 ||
	    (ptr = look(line, "entry")) != 0 ||
	    (ptr = look(line, "function")) != 0 ||
	    (ptr = functs(line)) != 0) {
	    if(!scan_name(s, ptr)) 
		strcpy( s, x);
	} else if((ptr = look(line, "program")) != 0) {
	    it_is_a_main = 1;
	    if(!scan_name(s, ptr)) {
		implicit_program_name = 1;
		get_name( mainp);
		strcpy( s, mainp);
	    }
	} else if((ptr = look(line, "blockdata")) != 0) {
	    if(!scan_name(s, ptr)) {
		implicit_blockdata_name = 1;
		get_name( blkp);
		strcpy( s, blkp);
	    }
	} else if((ptr = functs(line)) != 0) {
	    if(!scan_name(s, ptr)) 
		strcpy( s, x);
	} else {
	    implicit_program = 1;
	    it_is_a_main = 1;
	    get_name(mainp);
	    strcpy(s, mainp);
	}

	return(1);
}

static int scan_name(s, ptr)
char *s, *ptr;
{
	char *sptr;

	/* scan off the name */
	trim(ptr);
	sptr = s;
	while (*ptr != '(' && *ptr != '\n') {
		if (*ptr != ' ' && *ptr != '\t' && *ptr != '\r')
			*sptr++ = *ptr;
		ptr++;
	}

	if (sptr == s) return(0);

	*sptr++ = '.';
	*sptr++ = 'f';
	*sptr++ = 0;
	return(1);
}

static char *functs(p)
char *p;
{
        register char *ptr;

/*      look for typed functions such as: real*8 function,
                character*16 function, character*(*) function  */

        if((ptr = look(p,"character")) != 0 ||
           (ptr = look(p,"logical")) != 0 ||
           (ptr = look(p,"real")) != 0 ||
           (ptr = look(p,"integer")) != 0 ||
           (ptr = look(p,"doubleprecision")) != 0 ||
           (ptr = look(p,"complex")) != 0 ||
           (ptr = look(p,"doublecomplex")) != 0 ) {
                while ( *ptr == ' ' || *ptr == '\t' || *ptr == '*'
			|| (*ptr >= '0' && *ptr <= '9')
			|| *ptr == '(' || *ptr == ')') ptr++;
		ptr = look(ptr,"function");
		return(ptr);
	}
        else
                return(0);
}

/* 	if first 6 col. blank, return ptr to col. 7,
	if blanks and then tab, return ptr after tab,
	else return 0 (labelled statement, comment or continuation */
static char *skiplab(p)
char *p;
{
	register char *ptr;

	for (ptr = p; ptr < &p[6]; ptr++) {
		if (*ptr == ' ')
			continue;
		if (*ptr == '\t') {
			ptr++;
			break;
		}
		return (0);
	}
	return (ptr);
}

/* 	return 0 if m doesn't match initial part of s;
	otherwise return ptr to next char after m in s */
static char *look(s, m)
char *s, *m;
{
	register char *sp, *mp;

	sp = s; mp = m;
	while (*mp) {
		trim(sp);
		if (*sp++ != *mp++)
			return (0);
	}
	return (sp);
}

static void put_upper_till_dot_or_end(char * what, FILE * where)
{
    while (*what && *what!='.') putc(toupper(*what++), where);
}

static void print_name(FILE * o, char * name, int n, int upper) /* FC */
{
    name = name + strlen(name) - n - 2;
    while (n-->0) putc(upper? toupper(*name++): *name++, o);
}

#define FREE_STRINGS					\
  if (main_list) free(main_list), main_list = NULL;	\
  if (x) free(x), x = NULL;				\
  if (mainp) free(mainp), mainp = NULL;			\
  if (blkp) free(blkp), blkp = NULL;

int fsplit(char * dir_name, char * file_name, FILE * out)
{
    register FILE *ofp;	/* output file */
    register int rv;	/* 1 if got card in output file, 0 otherwise */
    int nflag,		/* 1 if got name of subprog., 0 otherwise */
	retval, someentry, newname;
   /* ??? 20 -> 80 because not checked... smaller than a line is ok ? FC */
    char name[80]; 
    char tmpname[80];

    /* MALLOC STRINGS 
     */
    char * main_list = full_name(dir_name, ".fsplit_main_list");
    x = full_name(dir_name, "zzz000.f");
    mainp = full_name(dir_name, "main000.f");
    blkp = full_name(dir_name, "data000.f");
	
    if ((ifp = fopen(file_name, "r")) == NULL) {
	fprintf(stderr, "fsplit: cannot open %s\n", file_name);
	FREE_STRINGS; return 0;
    }

    for(;;) {

	/* look for a temp file that doesn't correspond to an existing file */
	get_name(x);
	ofp = fopen(x, "w");
	if (ofp==NULL) {
	    fprintf(stderr, "%s %s -> %s\n", dir_name, file_name, x);
	    fprintf(stderr, "fopen(\"%s\", ...) failed\n", x);
	    abort();
	}

	nflag = 0;
	rv = 0;
	newname = 0;
	someentry = 0;

	while (getline() > 0)
	{
	    hollerith_and_bangcomments(buf); /* FC */

	    if (nflag == 0) /* if no name yet, try and find one */
		nflag = lname(name), newname=nflag;
	    else { /* FC: some hack to deal with entry... */
		someentry = lname(tmpname);
		newname=someentry;
		implicit_program = 0;
		it_is_a_main = 0;
	    }

	    if (it_is_a_main) {
		FILE * fm = fopen(main_list, "a");
		if (fm==NULL) {
		    fprintf(stderr, "fopen(\"%s\", ...) failed\n", main_list);
		    abort();
		}
		if (implicit_program_name==1 || implicit_program==1)
		    print_name(fm, name, 7, 1);
		else
		    put_upper_till_dot_or_end(name, fm);
		putc('\n', fm);
		fclose(fm);
		it_is_a_main = 0;
	    }

	    if (implicit_program==1) /* FC again */ 
	    {
		fprintf(ofp, 
			"! next line added by fsplit() in pips\n"
			"      PROGRAM ");
		print_name(ofp, name, 7, 0);
		putc('\n', ofp);
		implicit_program = 0; /* now we gave it a name! */
	    }

	    if (implicit_blockdata_name==1 || implicit_program_name==1) 
	    {
		fprintf(ofp, 
			"! next line modified by fsplit() in pips\n"
			"      %s ", 
			implicit_program_name==1? "PROGRAM": "BLOCK DATA");
		print_name(ofp, name, 7, 0);
		putc('\n', ofp);
		implicit_blockdata_name = 0;
		implicit_program_name = 0;
	    }
	    else
		fprintf(ofp, "%s", buf);

	    /* a new module name is appended to the current line... */
	    if (newname)
	    {
		if ((someentry && tmpname[0]) || (!someentry && name[0]))
		{
		    put_upper_till_dot_or_end(someentry? tmpname: name, out);
		    putc(' ', out);
		}
		newname = 0;
		someentry = 0;
		tmpname[0] = '\0';
	    }

	    rv = 1;

	    if (lend())		/* look for an 'end' statement */
		break;
	} /* while */
	
	if (fclose(ofp)) {
	    fprintf(stderr, "fclose(ofp) failed\n");
	    exit(2);
	}
	if (rv == 0) {			/* no lines in file, forget the file */
	    unlink(x);
	    retval = 0;
	    if (fclose(ifp)) {
		fprintf(stderr, "fclose(ifp) failed\n");
		exit(2);
	    }
	    FREE_STRINGS; return ( retval );
	}
	if (nflag)			/* rename the file */
	{
	    if(saveit(name)) 
	    {
		if (strncmp(dir_name, name, strlen(dir_name))!=0) 
		{
		    char * full = full_name(dir_name, name);
		    strcpy(name, full);
		    free(full);
		}
		if (strcmp(name, x) == 0) {
		    printf(/* out? */ "%s\n", x);
		}
		else if (stat(name, &sbuf) < 0 ) 
		{
		    link(x, name);
		    unlink(x);
		    fprintf(out, "%s\n", name);
		}
		else 
		    printf("%s already exists, put in %s\n", name, x);
		continue;
	    } 
	    else
		unlink(x);
	    continue;
	}
	fprintf(out, "%s\n", x);
    } /* for(;;) */

    if (fclose(ifp)) {
	fprintf(stderr, "fclose(ifp) failed\n");
	exit(2);
    }
    FREE_STRINGS;
    return 1;
}


/* ADDITION: basic Hollerith constants handling
 * FC 11 Apr 1997
 *
 * bugs:
 *  - under special circonstances, the dilatation of the transformation
 *    may lead continuations to exceed the 19 lines limit. 
 *
 * to improve:
 *  - hack for "real*8 hollerith", but should just forbids start after *?
 *    maybe some other characters?
 */

#define isbegincomment(c) ((c)=='!' || (c)=='*' || (c)=='c' || (c)=='C')
#define issquote(c) ((c)=='\'')
#define isdquote(c) ((c)=='\"')
#define ishH(c) ((c)=='h' || (c)=='H')
#define char2int(c) ((int)((c)-'0'))

/* global state
 */
static int in_squotes=0, in_dquotes=0, in_id=0;

static int blank_line_p(char * line)
{
    if (!line) return 1;
    while (*line)
	if (!isspace(*line++))
	    return 0;
    return 1;
}

static void hollerith_and_bangcomments(char * line)
{
    int i,j,initial, touched=0, bang=0;
    char bangcomment[BSZ];

    bangcomment[0] = '\0';
    
    if (!line) {
	in_squotes=0, in_dquotes=0, in_id=0; /* RESET */
	return;
    }

    if (blank_line_p(line))
	return;

    if (isbegincomment(line[0]))
	return;

    i = (line[0]=='\t')? 1: 6; /* first column to analyze */
    
    for (j=0; j<i; j++)
	if (!line[j]) return;

    if (isspace(line[i-1]))
	in_squotes=0, in_dquotes=0, in_id=0; /* RESET */

    initial=i;

    while (line[i] && initial<72) /* 73.. ignored */
    {
	if (!in_dquotes && issquote(line[i])) 
	    in_squotes = !in_squotes, in_id=0;
	if (!in_squotes && isdquote(line[i])) 
	    in_dquotes = !in_dquotes, in_id=0;
	if (!in_squotes && !in_dquotes)
	{
	    if (isalpha(line[i]))
		in_id=1;
	    else if (!isalnum(line[i]) && !isspace(line[i]) 
		&& line[i]!='*') /* hack for real*8 hollerith */
		in_id=0;
	}

	if (!in_squotes && !in_dquotes && !in_id && isdigit(line[i]))
	{
	    /* looks for [0-9 ]+[hH] 
	     */
	    int len=char2int(line[i]), ni=i;
	    i++, initial++;
	    
	    while (line[i] && initial<72
		   && (isdigit(line[i]) || isspace(line[i])))
	    {
		if (isdigit(line[i]))
		    len=10*len+char2int(line[i]);
		i++, initial++;
	    }

	    if (!line[i] || initial>=72) return;
	    
	    if (ishH(line[i])) /* YEAH, here it is! */
	    {
		char tmp[200];
		int k;

		if (!touched) { /* rm potential 73-80 text */
		    touched=1;
		    line[72]='\n'; 
		    line[73]='\0';			
		}

		j=1;

		tmp[0] = '\''; i++, initial++;
		while (j<200 && line[i] && initial<72 && 
		       line[i]!='\n' && len>0)
		{
		    len--;
		    if (line[i]=='\'')
			tmp[j++]='\'';
		    tmp[j++] = line[i++];
		    initial++;
		}
                
                while (j<199 && len>0) /* padding */
                    tmp[j++]=' ', len--;
		
		tmp[j]='\'';

		/* must insert tmp[<j] in line[ni..]
		 * first, shift the line...
		 */
		
		{
		    int ll = strlen(line), shift = i-(ni+j+1);

		    if (shift>0) /* to the left */
			for (k=0; i+k<=ll; k++) 
			    line[ni+j+1+k] = line[i+k];
		    else /* to the right */
			for (k=ll-i; k>=0; k--)
			    line[ni+j+1+k] = line[i+k];
		}

		i=ni+j+1;

		while(j>=0)
		    line[ni+j]=tmp[j], j--;

	    }
	}
	

	/* bang comment */
	if (!in_squotes && !in_dquotes && line[i]=='!')
	{
	    strcpy(bangcomment,&line[i]);
	    line[i]='\n', line[i+1]='\0'; /* stop while loop */
	    bang=1;
	}

	i++, initial++;
    }

    if (touched) 
    {
	int len = strlen(line); /* the new line may exceed the 72 column */
	/* caution, len includes cr... */
	/* the dilatation cannot exceed one line (?) */
	if (len-1>72) /* then shift and continuation... */
	{
	    for (i=len; i>=72; i--) line[i+7] = line[i];
	    line[72]='\n'; line[73]=' '; line[74]=' ';
	    line[75]=' '; line[76]=' '; line[77]=' '; line[78]='x';
	}
    }

    /* the bang comment is moved to the preceding line.
     */
    if (bang) 
    {
	char tmp[BSZ];
	strcpy(tmp,line);
	strcpy(line,bangcomment);
	strcat(line,tmp);
    }
}

/* processing extracted for includes...
 */
void process_bang_comments_and_hollerith(FILE * in, FILE * out)
{
    ifp = in;
    while (getline()>0) 
    {
	hollerith_and_bangcomments(buf);
	fputs(buf, out);
    }
    ifp = NULL;
}
