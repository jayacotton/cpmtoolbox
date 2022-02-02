/*
	Sort/Merge from Software Tools

	Last Modified : 21 September 1982

	Converted from Software Tools RATFOR to BDS C by Leor Zolman
	Sep 2, 1982

	Usage: sort <infile> <outfile>

	Main variables have been made external; this is pretty much in
	line with the RATFOR call-by-name convention anyway.

	Requires lots of disk space, up to around three times the length
	of the file file being sorted. This program is intended for files
	bigger than memory; simpler, faster sorts can be implemented for
	really short files (like ALPH.C)
		-leor

	Compile & Link:
		A>cc sort.c -e2800 -o
		A>l2 sort
	(or...)
		A>cc sort.c -e2900 -o
		A>clink sort

*/


#include <bdscio.h>

#define VERBOSE 1		/* give running account of file activity */

#define MAXLINE 200		/* longest line we want to deal with */
#define NBUFS 7			/* Max number of open buffered files */
#define MAXPTR  3000		/* Max number of lines (set for dict) */
#define MERGEORDER (NBUFS-1)	/* Max # of intermediate files to merge */
#define NAMESIZE 20		/* Max Filename size */
#define LOGPTR 13		/* smallest value >= log (base 2) of  MAXPTR */
#define EOS '\0'		/* string termination character */
#define FILE struct _buf

#define stderr 4
#define fputc putc

char name[NAMESIZE], name2[NAMESIZE + 10];
FILE buffers[NBUFS + 1];	/* up to NBUFS general-purp. buffered files */
FILE *infil[MERGEORDER + 1];	/* tmp file ptrs for sort operation */
unsigned linptr[MAXPTR + 1], nlines;
int temp;
unsigned maxtext;		/* max # of chars in main text buffer */
char *linbuf;			/* text area starts after this variable */

main(argc,argv)
char **argv;
{

	FILE *infile, *outfile;		/* main input and output streams */
	FILE *tmpfile;

	int makfil(), min(), gopen(), gremov();
	int gtext();
	unsigned high, lim, low, t;

	linbuf = endext();		/* start of text buffer area */
	maxtext = topofmem() - endext() - 500;

	tmpfile = buffers[0];

	if (argc != 3)
		exit(puts("Usage: sort <infile> <outfile>\n"));

	infile = buffers[1];
	if (fopen(argv[1], infile) == ERROR)
	{
		puts("Can't open "); puts(argv[1]); exit(-1);
	}

#if VERBOSE
	fputs("Beginning initial formation run\n",stderr);
#endif

	high = 0;		/* Initial formation of runs:	*/
	do {
		t = gtext(infile);
		quick(nlines);		
		high++;
		makfil(high,tmpfile);
		ptext(tmpfile);
		fclout(tmpfile);
	} while (t != NULL);

	fclose(infile);		/* free up the input file buffer */

#if VERBOSE
	fputs("Beginning merge operation\n",stderr);
#endif

	for (low = 1; low < high; low += MERGEORDER)	/* merge */
	{
		lim = min(low + MERGEORDER - 1, high);
		gopen(low, lim);		/* open files */
		high++;
		makfil(high, tmpfile);
		merge(lim - low + 1, tmpfile);
		fclout(tmpfile);	/* terminate, flush and close file */
		gremov(low, lim);
	}

			/* Now write the sorted output file: */

#if VERBOSE
	fputs("Merge complete. Writing output file.\n",stderr);
#endif

	gname(high, name);	/* create name of result file */
	infile = buffers[0];

	if (fopen(name,infile) == ERROR)
	{
		puts("Something's screwy; I can't open ");
		puts(name);
		exit(-1);
	}

	outfile = buffers[1];
	while (fcreat(argv[2],outfile) == ERROR)
	{
		puts("Can't create "); puts(argv[2]);
		puts("\nEnter another name to call the output: ");
		gets(name2);
		argv[2] = &name2;
	}

	while (fgets(linbuf,infile))
	{
		fputs(linbuf,outfile);
		fputs("\r\n",outfile);
	}
	fclout(outfile);
	fabort(infile->_fd);
	unlink(name);
}


fclout(obuf)
FILE *obuf;
{
	putc(CPMEOF,obuf);
	fflush(obuf);
	fclose(obuf);
}


/*
 * Quick: Quicksort for character lines
 */

quick(nlines)
unsigned nlines;
{
	unsigned i,j, lv[LOGPTR + 1], p, pivlin, uv[LOGPTR + 1];
	int compar();

	lv[1] = 1;
	uv[1] = nlines;
	p = 1;
	while (p > 0)
		if (lv[p] >= uv[p])	/* only 1 element in this subset */
			p--;		/* pop stack			*/
		else
		{
			i = lv[p] - 1;
			j = uv[p];
			pivlin = linptr[j];	/* pivot line		*/
			while (i < j)
			{
			   for (i++; compar(linptr[i],pivlin) < 0; i++)
				;
			   for (j--; j > i; j--)
				if (compar(linptr[j], pivlin) <= 0)
					break;
			   if (i < j)	/* out of order pair		*/
			   {
				temp = linptr[i];
				linptr[i] = linptr[j];
				linptr[j] = temp;
			   }
			}
		j = uv[p];		/* move pivot to position 1 	*/

		temp = linptr[i];
		linptr[i] = linptr[j];
		linptr[j] = temp;

		if ( (i - lv[p]) < (uv[p] - i))
		  {
			lv[p + 1] = lv[p];
			uv[p + 1] = i - 1;
			lv[p] = i + 1;
		  }
		else
		  {
			lv[p + 1] = i + 1;
			uv[p + 1] = uv[p];
			uv[p] = i - 1;
		  }
		p++;
		}			
	return;
}			


/*
 * Compar: Compare two strings; return 0 if equal, -1 if first is
 *	   lexically smaller, or 1 if first is bigger
 */

int compar(lp1, lp2)
unsigned lp1, lp2;
{
	unsigned i, j;

	for (i = lp1, j = lp2; linbuf[i] == linbuf[j]; i++,j++)
	{
		if (linbuf[i] == EOS)
			return 0;
	}

	return (linbuf[i] < linbuf[j]) ? -1 : 1;
}

/*
 * Ptext: output text lines from linbuf onto the buffered output file given
 */

ptext(outfil)
FILE *outfil;
{
	int i;

	for (i = 1; i <= nlines; i++) {
		if (fputs(&linbuf[linptr[i]], outfil) == ERROR) {
			fputs("Error writing output file..disk full?\n",
					stderr);
			exit(-1);
		}
		fputc('\0', outfil);	/* terminate the line */
	}
	return 0;
}


/*
 * Gtext: Get text lines from the buffered input file provided, and
 * 	  place them into linbuf
 */

int gtext(infile)
FILE *infile;
{
	unsigned lbp, len;

	nlines = 0;
	lbp = 1;
	do {
		if ( (len = fgets(&linbuf[lbp], infile)) == NULL)
			break;
		len = strlen(&linbuf[lbp]);
		linbuf[lbp + len - 1] = '\0';
		nlines++;
		linptr[nlines] = lbp;
		lbp += len;	/* drop '\n', but keep NULL at end of string */
	} while ( lbp < (maxtext - MAXLINE) && nlines < MAXPTR);

	return len;		/* return 0 if done with file */
}


/*
 * Makfil: Make a temporary file having suffix 'n' and open it for
 * 	   output via the supplied buffer
 */

FILE *makfil(n,obuf)	/* make temp file having suffix 'n' */
int n;
FILE *obuf;
{
	FILE *fp;
	char name[20];
	gname(n,name);
	if (fcreat(name,obuf) == ERROR) {
		puts("Can't create "); puts(name); exit(-1);
	}
	return 0;
}


/*
 * Gname: Make temporary filename having suffix 'n'
 */

char *gname(n,name)
char *name;
int n;
{
	char tmptext[10];

	strcpy(name,"TEMP");		/* create "TEMPn.$$$"	*/
	strcat(name,itoa(tmptext,n));
	strcat(name,".$$$");
	return name;		/* return a pointer to it	*/
}


/*
 * Itoa: convert integer value n into ASCII representation at strptr,
 * 	 and return a pointer to it
 */

char *itoa(strptr,n)
char *strptr;
int n;
{
	int length;

	if (n < 0)
	{
		*strptr++ = '-';
		strptr++;
		n = -n;
	}

	if (n < 10)
	{
		*strptr++ = (n + '0');
		*strptr = '\0';
		return strptr - 1;
	}
	else
	{
		length = strlen(itoa(strptr, n/10));
		itoa(&strptr[length], n % 10);
		return strptr;
	}
}


/*
 * Gopen: open group of files low...lim
 */

gopen(low, lim)
int lim, low;
{
	int i;

#if VERBOSE
	fprintf(stderr,"Opening temp files %d-%d\n",low,lim);
#endif

	for (i = 1; i <= (lim - low + 1); i++)
	{
		gname(low + i - 1, name);
		if (fopen(name, buffers[i]) == ERROR)
		{
			puts("Can't open: "); puts(name); exit(-1);;
		}
		infil[i] = &buffers[i];
	}
}


/*
 * Remove group of files low...lim
 *		(should use "close" instead of "fabort" for MP/M II)
 */

gremov(low, lim)
int lim, low;
{
	int i;

#if VERBOSE
	fprintf(stderr,"Removing temp files %d-%d\n",low,lim);
#endif
	for (i = 1; i <= (lim - low + 1); i++)
	{
		fabort(infil[i]->_fd);		/* forget about the file */
		gname(low + i - 1, name);
		unlink(name);			/* and physically remove it */
	}
}

/*
 * Fputs: special version that doesn't epand LF's and aborts on error
 */
 
fputs(s,iobuf)
char *s;
FILE *iobuf;
{
	char c;
	while (c = *s++)
		if (putc(c,iobuf) == ERROR)
		{
			fputs("Error on file output\n",stderr);
			exit(-1);
		}
	return OK;
}


/*
 * Merge: merge infil[1]...infil[nfiles] onto outfil
 */

merge(nfiles, outfil)
FILE *outfil;
{
	char *fgets();
	int i, inf, lbp, lp1, nf;

	lbp = 1;
	nf = 0;
	for (i = 1; i <= nfiles; i++)  /* get one line from each file */
		if (fgets(&linbuf[lbp], infil[i]) != NULL)
		{
			nf++;
			linptr[nf] = lbp;
			lbp += MAXLINE;	/* leave room for largest line */
		}

	quick(nf);		/* make initial heap */

	while (nf > 0) {
		lp1 = linptr[1];
		fputs(&linbuf[lp1], outfil);				
		fputc('\0', outfil);
		inf = lp1 / MAXLINE + 1;	/* compute file index */
		if (fgets(&linbuf[lp1],infil[inf]) == NULL)
		{
			linptr[1] = linptr[nf];
			nf--;
		}
		reheap(nf);

	}
	return;
}

		
/*
 * Reheap: propogate linbuf[linptr[1]] to proper place in heap
 */

reheap(nf)
unsigned nf;
{
	unsigned i,j;

	for (i = 1; (i + i) <= nf; i = j)
	{
		j = i + i;
		if (j < nf && compar(linptr[j], linptr[j + 1]) > 0)
			j++;
		if (compar(linptr[i], linptr[j]) <= 0)
			break;

		temp = linptr[i];
		linptr[i] = linptr[j];
		linptr[j] = temp;
	}
	return;
}

/*
 * Just like regular library version, except that NULL is also
 * taken as a string terminator.
 */

char *fgets(s,iobuf)
char *s;
struct _buf *iobuf;
{
	int count, c;
	char *cptr;
	count = MAXLINE;
	cptr = s;
	if ( (c = getc(iobuf)) == CPMEOF || c == EOF) return NULL;

	do {
		if ((*cptr++ = c) == '\n') {
		  if (cptr>s+1 && *(cptr-2) == '\r')
			*(--cptr - 1) = '\n';
		  break;
		}
		if (!c) break;
	 } while (count-- && (c=getc(iobuf)) != EOF && c != CPMEOF);

	if (c == CPMEOF) ungetc(c,iobuf);	/* push back control-Z */
	*cptr = '\0';
	return s;
}
