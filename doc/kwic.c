/* Simple KWIC generator, uses stdin/stdout only. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Every symbol must be no more than this long. */

#define SYMLEN 40

#define EXTRALEN 20

#define MAXSYMBOLS 1000

char *arr;

char *prefix = NULL;

char *suffix = NULL;

int pad = 0;

int dots = 0;

int longest = 0;

int breakup = 0;

int
compare_names(const void *n1arg, const void *n2arg)
{
    int rslt;
    const char *n1 = (const char *)n1arg;
    const char *n2 = (const char *)n2arg;
    const char *n1a;
    const char *n2a;

    rslt = strcmp(n1 + SYMLEN, n2 + SYMLEN);
    if (rslt != 0)
      return rslt;
    /* If the part of the names that are the keyword match,
       go back and compare the entire names. */
    n1a = strrchr(n1 + SYMLEN, ' ');
    n1a = (n1a ? n1a + 1 : n1);
    n2a = strrchr(n2 + SYMLEN, ' ');
    n2a = (n2a ? n2a + 1 : n2);
    return strcmp(n1a, n2a);
}

int
main(int argc, char **argv)
{
    int i, j, linesize, bufsize, maxi, blanks;
    char name[SYMLEN + 1 + EXTRALEN + 1], *extra, *str;

    /* Parse a couple args if supplied. */
    for (i = 1; i < argc; ++i) {
	if (strcmp(argv[i], "-prefix") == 0) {
	    prefix = argv[++i];
	} else if (strcmp(argv[i], "-suffix") == 0) {
	    suffix = argv[++i];
	} else if (strcmp(argv[i], "-breakup") == 0) {
	    breakup = 1;
	} else if (strcmp(argv[i], "-dots") == 0) {
	    dots = 1;
	} else if (strcmp(argv[i], "-pad") == 0) {
	    pad = 1;
	}
    }
    linesize = 2 * SYMLEN + 1 + EXTRALEN + 1;
    bufsize = MAXSYMBOLS * 5 * linesize;
    arr = (char *) malloc(bufsize * sizeof(char));
    for (i = 0; i < bufsize; ++i)
      arr[i] = ' ';
    i = 0;
    while (fgets(name, SYMLEN+1+EXTRALEN+1, stdin) != NULL) {
	if (name[strlen(name)-1] == '\n')
	  name[strlen(name)-1] = '\0';
	extra = strchr(name, ' ');
	if (extra != NULL) {
	    /* Replace the blank with an EOS, point after it. */
	    *extra = '\0';
	    ++extra;
	    if (strlen(extra) > EXTRALEN) {
		extra[EXTRALEN-1] = '\0';
	    }
	}
	if (strlen(name) > SYMLEN) {
	    fprintf(stderr, "Symbol %s too long; ignoring\n", name);
	    continue;
	}
	/* See if this is the longest name so far. */
	if (strlen(name) > longest)
	  longest = strlen(name);
	/* Put out the name undisplaced. */
	memcpy(arr + i * linesize + SYMLEN, name, strlen(name));
	*(arr + i * linesize + 2 * SYMLEN) = '\0';
	*(arr + i * linesize + 2 * SYMLEN + 1) = '\0';
	if (extra != NULL)
	  strcpy(arr + i * linesize + 2 * SYMLEN + 1, extra);
	++i;
	/* Scan down the name looking for hyphens. */
	for (j = 0; name[j] != '\0'; ++j) {
	    if (name[j] == '-') {
		/* Put out the name shifted left by position of hyphen. */
		memcpy(arr + i * linesize + SYMLEN - j-1, name, strlen(name));
		*(arr + i * linesize + 2 * SYMLEN) = '\0';
		*(arr + i * linesize + 2 * SYMLEN + 1) = '\0';
		if (extra != NULL)
		  strcpy(arr + i * linesize + 2 * SYMLEN + 1, extra);
		++i;
	    }
	}
    }
    /* Sort everything. */
    maxi = i;
    qsort(arr, maxi, linesize, compare_names);
    for (j = 0; j < MAXSYMBOLS; ++j) {
	if (arr[j] == '\n')
	  arr[j] = '&';
    }
    blanks = 99;
    for (i = 0; i < maxi; ++i) {
	str = arr + i * linesize + SYMLEN - longest;
	for (j = 0; j < longest; ++j)
	  if (str[j] != ' ')
	    break;
	if (j < blanks)
	  blanks = j;
    }
    /* Dump out all the lines. */
    if (breakup)
      fputs("@smallexample\n", stdout);
    for (i = 0; i < maxi; ++i) {
	if (prefix != NULL)
	  fputs(prefix, stdout);
	str = arr + i * linesize + SYMLEN - longest;
	*(str + 2 * longest) = '\0';
	if (dots) {
	    for (j = strlen(str) - 1; j >= 0; --j) {
		if (str[j] != ' ')
		  break;
		if (j % 2 == 0 && str[j-1] == ' ' && str[j+1] == ' ')
		  str[j] = '.';
	    }
	}
	fputs(str + blanks, stdout);
	if (suffix != NULL)
	  fputs(suffix, stdout);
	if (pad) {
	    for (j = 0; j < pad; ++j)
	      fputs(" ", stdout);
	}
	fputs(arr + i * linesize + 2 * SYMLEN + 1, stdout);
	fputs("\n", stdout);
	if (breakup && i > 0 && i % 50 == 0) {
	    fputs("@end smallexample\n", stdout);
	    fputs("@smallexample\n", stdout);
	}
    }
    if (breakup)
      fputs("@end smallexample\n", stdout);
    exit(0);
}
