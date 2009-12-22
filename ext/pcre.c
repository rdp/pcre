#include "ruby.h"
#include "regex.h"

#include <stdio.h>
#include <string.h>
#include <pcre.h>

#define OVECCOUNT 30    /* should be a multiple of 3 */

static VALUE rb_ePCREError;

static char *
str_cat(char *dest,char *src, int len)
{
  int slen = strlen(dest);
  if(len<=0) return dest;
  REALLOC_N(dest, char, slen+len+1);
  strncpy(dest+slen,src,len);
  dest[slen+len] = '\0';
  return dest;
}

VALUE pcre_match(char *pattern,char *subject,int option)
{
VALUE result;
pcre *re;
const char *error;
int erroffset;
int ovector[OVECCOUNT];
int rc, i;


/* Compile the regular expression in the first argument */

re = pcre_compile(
  pattern,              /* the pattern */
  option,                    /* default options */
  &error,               /* for error message */
  &erroffset,           /* for error offset */
  NULL);                /* use default character tables */

/* Compilation failed: print the error message and exit */

if (re == NULL)
  {
  rb_raise(rb_ePCREError,"PCRE compilation failed at offset %d: %s", erroffset, error);
  return Qnil;
  }

/* Compilation succeeded: match the subject in the second argument */

rc = pcre_exec(
  re,                   /* the compiled pattern */
  NULL,                 /* no extra data - we didn't study the pattern */
  subject,              /* the subject string */
  (int)strlen(subject), /* the length of the subject */
  0,                    /* start at offset 0 in the subject */
  0,                    /* default options */
  ovector,              /* output vector for substring information */
  OVECCOUNT);           /* number of elements in the output vector */

/* Matching failed: handle error cases */

if (rc < 0)
  {
  switch(rc)
    {
    case PCRE_ERROR_NOMATCH: break;
    /*
    Handle other special cases if you like
    */
    default: rb_raise(rb_ePCREError,"Matching error %d\n", rc);
    }
  return Qnil;
  }

/* Match succeded */

/* The output vector wasn't big enough */

if (rc == 0)
  {
  rc = OVECCOUNT/3;
/*  printf("ovector only has room for %d captured substrings\n", rc - 1); */
  }

/* Show substrings stored in the output vector */
result = rb_ary_new2(rc);

for (i = 0; i < rc; i++)
  {
  char *substring_start = subject + ovector[2*i];
  int substring_length = ovector[2*i+1] - ovector[2*i];
  rb_ary_push(result,rb_str_new(substring_start,substring_length));
  }

return result;
}

VALUE pcre_match_all(char *pattern,char *subject,int option)
{
VALUE result, res2;
pcre *re;
const char *error;
int erroffset;
int ovector[OVECCOUNT];
int rc, i, offset;


/* Compile the regular expression in the first argument */

re = pcre_compile(
  pattern,              /* the pattern */
  option,                    /* default options */
  &error,               /* for error message */
  &erroffset,           /* for error offset */
  NULL);                /* use default character tables */

/* Compilation failed: print the error message and exit */

if (re == NULL)
  {
  rb_raise(rb_ePCREError,"PCRE compilation failed at offset %d: %s", erroffset, error);
  return Qnil;
  }

result = Qnil;

/* Compilation succeeded: match the subject in the second argument */

for(offset=0;offset<strlen(subject);) {
  rc = pcre_exec(
  re,                   /* the compiled pattern */
  NULL,                 /* no extra data - we didn't study the pattern */
  subject,              /* the subject string */
  (int)strlen(subject), /* the length of the subject */
  offset,               /* start at offset  in the subject */
  0,                    /* default options */
  ovector,              /* output vector for substring information */
  OVECCOUNT);           /* number of elements in the output vector */

/* Matching failed: handle error cases */

  if (rc < 0)
  {
  switch(rc)
    {
    case PCRE_ERROR_NOMATCH: break;
    /*
    Handle other special cases if you like
    */
    default: rb_raise(rb_ePCREError,"Matching error %d\n", rc);
    }
  return result;
  }

/* Match succeded */

/* The output vector wasn't big enough */

  if (rc == 0)
  {
  rc = OVECCOUNT/3;
/*  printf("ovector only has room for %d captured substrings\n", rc - 1); */
  }

/* Show substrings stored in the output vector */
  if(result==Qnil) 
	result = rb_ary_new();

  res2 = rb_ary_new2(rc);

  for (i = 0; i < rc; i++)
  {
  char *substring_start = subject + ovector[2*i];
  int substring_length = ovector[2*i+1] - ovector[2*i];
  rb_ary_push(res2,rb_str_new(substring_start,substring_length));
  }
  rb_ary_push(result,res2);
  offset = ovector[1];
}

return result;
}


VALUE
pcre_replace(char *pattern,char *replacement,char *subject,int option)
{
char *subj,*repl;
VALUE result;
pcre *re;
const char *error;
int erroffset;
int ovector[OVECCOUNT];
int rc, i, j, k, offset, len, ilen, rlen, slen;

if(strlen(replacement)==0 || strlen(pattern)==0)
    return rb_str_new2(subject);

/* Compile the regular expression in the first argument */

re = pcre_compile(
  pattern,              /* the pattern */
  option,                    /* default options */
  &error,               /* for error message */
  &erroffset,           /* for error offset */
  NULL);                /* use default character tables */

/* Compilation failed: print the error message and exit */

if (re == NULL)
  {
  rb_raise(rb_ePCREError,"PCRE compilation failed at offset %d: %s", erroffset, error);
  return Qnil;
  }

slen = strlen(subject);
subj = ALLOC_N(char,slen+1);
*subj = '\0';
rlen = strlen(replacement);
repl = ALLOC_N(char,rlen+1);

/* Compilation succeeded: match the subject in the second argument */
rc = pcre_exec(
  re,                   /* the compiled pattern */
  NULL,                 /* no extra data - we didn't study the pattern */
  subject,              /* the subject string */
  (int)slen, /* the length of the subject */
  0,                    /* start at offset in the subject */
  0,                    /* default options */
  ovector,              /* output vector for substring information */
  OVECCOUNT);           /* number of elements in the output vector */

/* Matching failed: handle error cases */

if (rc < 0)
  {
  switch(rc)
    {
    case PCRE_ERROR_NOMATCH:
	subj = str_cat(subj,subject,slen);
	result = rb_str_new2(subj);
        free(subj);
        free(repl);
	return result;
	break;
    /*
    Handle other special cases if you like
    */
    default: rb_raise(rb_ePCREError,"Matching error %d\n", rc);
    }
  free(subj);
  free(repl);
  return Qnil;
  }

/* Match succeded */

/* The output vector wasn't big enough */

  if (rc == 0)
  {
    rc = OVECCOUNT/3;
  /*  printf("ovector only has room for %d captured substrings\n", rc - 1); */
  }

  /* Show substrings stored in the output vector */
 /*
  for(i=0;i<rc;i++) {
      printf("%d %d %s\n",i,ovector[2*i],subj+ovector[2*i]);
  }
  */

  for(k=0,len=0;replacement[k];) {
    if(replacement[k]=='\\') {
      if(replacement[k+1]=='&') {
        ilen = ovector[1] - ovector[0];
        if(rlen<len+ilen && ilen>2) {
            REALLOC_N(repl, char, len+ilen);
            rlen = len+ilen;
        }
        strncpy(repl+len,subject+ovector[0],ilen);
        len += ilen;
        k+=2;
      } else if(replacement[k+1]>='1' && replacement[k+1]<='9') {
        j = replacement[k+1] - '0';
        if(j<rc) {
        ilen = ovector[2*j+1] - ovector[2*j];
        if(rlen<len+ilen && ilen>2) {
            REALLOC_N(repl, char, len+ilen);
            rlen = len+ilen;
        }
        strncpy(repl+len,subject+ovector[2*j],ilen);
        len += ilen;
        k+=2;
        }
      }
    }
    else {
        repl[len++] = replacement[k++];
    }
  }
  repl[len] = '\0';

  subj = str_cat(subj, subject, ovector[0]);
  subj = str_cat(subj, repl, len);
  subj = str_cat(subj,subject+ovector[1],slen-ovector[1]);
  result = rb_str_new2(subj);
  free(subj);
  free(repl);
  return result;
}

VALUE
pcre_replace_all(char *pattern,char *replacement,char *subject,int option)
{
char *subj,*repl;
VALUE result;
pcre *re;
const char *error;
int erroffset;
int ovector[OVECCOUNT];
int rc, i, j, k, offset, len, ilen, rlen, slen;

if(strlen(replacement)==0 || strlen(pattern)==0)
    return rb_str_new2(subject);

/* Compile the regular expression in the first argument */

re = pcre_compile(
  pattern,              /* the pattern */
  option,                    /* default options */
  &error,               /* for error message */
  &erroffset,           /* for error offset */
  NULL);                /* use default character tables */

/* Compilation failed: print the error message and exit */

if (re == NULL)
  {
  rb_raise(rb_ePCREError,"PCRE compilation failed at offset %d: %s", erroffset, error);
  return Qnil;
  }

slen = strlen(subject);
subj = ALLOC_N(char,slen+1);
*subj = '\0';
rlen = strlen(replacement);
repl = ALLOC_N(char,rlen+1);

/* Compilation succeeded: match the subject in the second argument */
for(offset=0;offset<slen;) {
rc = pcre_exec(
  re,                   /* the compiled pattern */
  NULL,                 /* no extra data - we didn't study the pattern */
  subject,              /* the subject string */
  (int)slen, /* the length of the subject */
  offset,                    /* start at offset in the subject */
  0,                    /* default options */
  ovector,              /* output vector for substring information */
  OVECCOUNT);           /* number of elements in the output vector */

/* Matching failed: handle error cases */

if (rc < 0)
  {
  switch(rc)
    {
    case PCRE_ERROR_NOMATCH:
	subj = str_cat(subj,subject+offset,slen-offset);
	result = rb_str_new2(subj);
        free(subj);
        free(repl);
	return result;
	break;
    /*
    Handle other special cases if you like
    */
    default: rb_raise(rb_ePCREError,"Matching error %d\n", rc);
    }
  free(subj);
  free(repl);
  return Qnil;
  }

/* Match succeded */

/* The output vector wasn't big enough */

  if (rc == 0)
  {
    rc = OVECCOUNT/3;
  /*  printf("ovector only has room for %d captured substrings\n", rc - 1); */
  }

  /* Show substrings stored in the output vector */
 /*
  for(i=0;i<rc;i++) {
      printf("%d %d %s\n",i,ovector[2*i],subj+ovector[2*i]);
  }
  */

  for(k=0,len=0;replacement[k];) {
    if(replacement[k]=='\\') {
      if(replacement[k+1]=='&') {
	ilen = ovector[1] - ovector[0];
        if(rlen<len+ilen && ilen>2) {
	    REALLOC_N(repl, char, len+ilen);
	    rlen = len+ilen;
	}
	strncpy(repl+len,subject+ovector[0],ilen);
        len += ilen;
	k+=2;
      } else if(replacement[k+1]>='1' && replacement[k+1]<='9') {
	j = replacement[k+1] - '0';
	if(j<rc) {
	ilen = ovector[2*j+1] - ovector[2*j];
        if(rlen<len+ilen && ilen>2) {
	    REALLOC_N(repl, char, len+ilen);
	    rlen = len+ilen;
	}
	strncpy(repl+len,subject+ovector[2*j],ilen);
        len += ilen;
	k+=2;
	}
      }
    }
    else {
	repl[len++] = replacement[k++];
    }
  }
  repl[len] = '\0';
/*
printf("subj = [%s],repl = [%s]\n",subj,repl);
*/
  if(offset<ovector[0])
      subj = str_cat(subj, subject+offset, ovector[0]-offset);
  /*
printf("subj2 = [%s],repl = [%s]\n",subj,repl);
*/
  subj = str_cat(subj, repl, len);
  offset = ovector[1];

}

result = rb_str_new2(subj);
free(subj);
free(repl);

return result;
}

VALUE
rb_pcre_match(int argc,VALUE *argv,VALUE obj)
{
    int option = 0;
    if(argc==2) option = FIX2INT(argv[1]);
    if(TYPE(argv[0])==T_REGEXP) {
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_MULTILINE)
	  option |= PCRE_MULTILINE;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_IGNORECASE)
	  option |= PCRE_CASELESS;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_EXTENDED)
	  option |= PCRE_EXTENDED;
        return pcre_match(RREGEXP(argv[0])->str,RSTRING(obj)->ptr,option);
    }
    else
      return pcre_match(RSTRING(argv[0])->ptr,RSTRING(obj)->ptr,option);
}

VALUE
rb_pcre_match_all(int argc,VALUE *argv,VALUE obj)
{
    int option = 0;
    if(argc==2) option = FIX2INT(argv[1]);
    if(TYPE(argv[0])==T_REGEXP) {
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_MULTILINE)
	  option |= PCRE_MULTILINE;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_IGNORECASE)
	  option |= PCRE_CASELESS;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_EXTENDED)
	  option |= PCRE_EXTENDED;
        return pcre_match_all(RREGEXP(argv[0])->str,RSTRING(obj)->ptr,option);
    }
    else
      return pcre_match_all(RSTRING(argv[0])->ptr,RSTRING(obj)->ptr,option);
}

VALUE
rb_pcre_replace(int argc,VALUE *argv,VALUE obj)
{
    int option = 0;
    if(argc==3) option = FIX2INT(argv[2]);
    if(TYPE(argv[0])==T_REGEXP) {
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_MULTILINE)
	  option |= PCRE_MULTILINE;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_IGNORECASE)
	  option |= PCRE_CASELESS;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_EXTENDED)
	  option |= PCRE_EXTENDED;
        return pcre_replace(RREGEXP(argv[0])->str,RSTRING(argv[1])->ptr,
		RSTRING(obj)->ptr,option);
    }
    else
      return pcre_replace(RSTRING(argv[0])->ptr,RSTRING(argv[1])->ptr,
	      RSTRING(obj)->ptr,option);
}

VALUE
rb_pcre_replace_all(int argc,VALUE *argv,VALUE obj)
{
    int option = 0;
    if(argc==4) option = FIX2INT(argv[3]);
    if(TYPE(argv[0])==T_REGEXP) {
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_MULTILINE)
	  option |= PCRE_MULTILINE;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_IGNORECASE)
	  option |= PCRE_CASELESS;
        if (RREGEXP(argv[0])->ptr->options & RE_OPTION_EXTENDED)
	  option |= PCRE_EXTENDED;
        return pcre_replace_all(RREGEXP(argv[0])->str,RSTRING(argv[1])->ptr,
		RSTRING(obj)->ptr,option);
    }
    else
      return pcre_replace_all(RSTRING(argv[0])->ptr,RSTRING(argv[1])->ptr,
	      RSTRING(obj)->ptr,option);
}

Init_pcre()
{
    rb_define_method(rb_cString,"pcre_match",rb_pcre_match,-1);
    rb_define_method(rb_cString,"pcre_match_all",rb_pcre_match_all,-1);
    rb_define_method(rb_cString,"pcre_sub",rb_pcre_replace,-1);
    rb_define_method(rb_cString,"pcre_gsub",rb_pcre_replace_all,-1);
    rb_define_global_const("PCRE_CASELESS",INT2FIX(PCRE_CASELESS));
    rb_define_global_const("PCRE_MULTILINE",INT2FIX(PCRE_MULTILINE));
    rb_define_global_const("PCRE_DOTALL",INT2FIX(PCRE_DOTALL));
    rb_define_global_const("PCRE_EXTENDED",INT2FIX(PCRE_EXTENDED));
    rb_define_global_const("PCRE_ANCHORED",INT2FIX(PCRE_ANCHORED));
    rb_define_global_const("PCRE_DOLLAR_ENDONLY",INT2FIX(PCRE_DOLLAR_ENDONLY));
    rb_define_global_const("PCRE_EXTRA",INT2FIX(PCRE_EXTRA));
    rb_define_global_const("PCRE_NOTBOL",INT2FIX(PCRE_NOTBOL));
    rb_define_global_const("PCRE_NOTEOL",INT2FIX(PCRE_NOTEOL));
    rb_define_global_const("PCRE_UNGREEDY",INT2FIX(PCRE_UNGREEDY));
    rb_define_global_const("PCRE_NOTEMPTY",INT2FIX(PCRE_NOTEMPTY));
    rb_define_global_const("PCRE_UTF8",INT2FIX(PCRE_UTF8));
    rb_ePCREError = rb_define_class("PCREError", rb_eStandardError);
}

