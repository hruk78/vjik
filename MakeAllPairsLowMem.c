/*!
 * Copyright 2019 XGBoost contributors
 *
 * \MakePairs.c
 * \brief Generate pairs to predict
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#include <time.h>

typedef struct TStCol {
	size_t size;
	char** StrCol1;
	char** StrCol2;
} TStCol;
////////////////////////////////////////////////////////////
typedef struct StrColIntMtrx {
	size_t size_h;
	size_t size_l;
	int** IntMtrx;
	char** StrCol;
} StrColIntMtrx;
//////
////////////////////////////////////////////////////////////
typedef struct ArraySet {
	int raws;
	int cols;
	//int** IntMtrx;
	//int* Labels;
	float** IntMtrx;
	float* Labels;
	
} ArraySet;
//////
/////////////////////////////////////////////////////////////////////////////
int cmpfunc(const void * pa, const void * pb) 
{
const char** a = *(const char ***)pa;
const char** b = *(const char ***)pb;
return strcmp(*a,*b);
}
////////////////////////////////////////////////////////////////////////
int cmpfunc2(const void* key, const void* b)
{
    const char* bb = *((const char**)b);
    return strcmp((const char*)key, bb);
}
////////////////////////////////////////////////////////////
//a_str - string to split, a_delim - delimiter, 
//Returns size - returns parts amount, returns array of tokens
char** str_split(char* a_str, const char a_delim, int *size)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);
	*size = count;

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
////////////////////////////////////////////////////////////////////////////////////
//fp - file to read, sp1Stolbec - column of key label strings, sp1Data - array[][] of int
//columns, row - max possible cols and raws
int countTableElemFromFile(const char* Name, int *columns, int *rows)
{
	FILE* fp = fopen(Name, "r");
	if (fp == NULL) 
	{
		printf("countTableElemFromFile %s failed to open \n",Name);
		//exit(EXIT_FAILURE);
	}
	int result=1;
	int amount_str=0;
	char* line = NULL;
	size_t len = 0;
	int len_str=0;
	int len_str_max=0;
	while ((getline(&line, &len, fp)) != -1) 
	{

		//printf("%s", line); 	//string to parts
		char** tokens;

		//printf("line=[%s]\n\n", line); 	//length of the string
		amount_str++;
		tokens = str_split(line, '\t', &len_str);
		//remeber max line length
		if (len_str > len_str_max) len_str_max=len_str;
		//printf("Size of String %d/n",len_str); 	//printf("Amount of String %d/n",amount_str);

		//free memory
		if (tokens)
		{
			int i;
			for (i = 0; *(tokens + i); i++)
			{
				free(*(tokens + i));
			}
			free(tokens);
		}
		//if (line) free(line);
		//end of string to parts
	}
		
	*columns=len_str_max;
	*rows=amount_str;
	fclose(fp);
	return result;
}

////////////////////////////////////////////////////////////////////////////////////
//fp - file to read, sp1Stolbec - column of key label strings, sp1Data - array[][] of int
//columns, row - Real cols and raws. Raws may be smaller 1,2.
int ReadKeyColAndIntTableFromFile(const char* Name, char** sp1Stopbec, int ** sp1Data,int *columns, int *rows)
{
	FILE* fp = fopen(Name, "r");
	if (fp == NULL) exit(EXIT_FAILURE);
	int result=1;
	int amount_str=0;
	char* line = NULL;
	size_t len = 0;
	int len_str=0;
	int len_str_max=0;
	if (!sp1Stopbec || !sp1Data) return 0;
	while ((getline(&line, &len, fp)) != -1) 
	{
		char** tokens;
		tokens = str_split(line, '\t', &len_str);

		//remember max line length
		if (len_str > len_str_max) len_str_max=len_str;

		//read to arrays allocate and free memory
		if (tokens)
		{
			int i;
			len_str_max=0;
			for (i = 0; *(tokens + i); i++)
			{
				if (i==0) 
				{ 
					sp1Stopbec[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
					if (sp1Stopbec[amount_str])
					{
						strcpy(sp1Stopbec[amount_str], *(tokens + i));
					} 
					else
					{
						//return 0;
					}
				}
				else
				{
					(sp1Data[amount_str][i-1])=atoi(*(tokens + i));
					len_str_max++;
				}

				free(*(tokens + i));
			}
			free(tokens);
		}
		amount_str++;

	}
	(*columns)=len_str_max;
	(*rows)=amount_str;
	//printf("READ of FILE to memory was good");
	fclose(fp);
	return result;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//fp - file to read, sp1Stolbec - column of key label strings, sp1Data - array[][] of int
//columns, row - Real cols and raws. Raws may be smaller 1,2.
int ReadKeyColAndDropIntFromFile(const char* Name, char** sp1Stopbec, int *rows)
{
	FILE* fp = fopen(Name, "r");
	if (fp == NULL) exit(EXIT_FAILURE);
	int result=1;
	int amount_str=0;
	char* line = NULL;
	size_t len = 0;
	if (!sp1Stopbec) return 0;

	while ((getline(&line, &len, fp)) != -1) 
	{
		//char** tokens;
		char tokens[128]={0};
		//tokens = str_split(line, '\t', &len_str);
		
		char *istr;
		istr = strchr(line,'\t');
		memcpy(tokens, line, istr-line);
		tokens[istr-line] = '\0';
		//read to arrays allocate and free memory
		if (tokens) asprintf(&(sp1Stopbec[amount_str]),"%s",tokens); else return 0;
		amount_str++;
	}
	free(line);
	(*rows)=amount_str;
	fclose(fp);
	return result;
}
////////////////

////////////////////////////////////////////////////////////////////////////////////
int WriteKeyColAndIntTableToFile(const char* Name, char** sp1Stopbec, int ** sp1Data,int *len_str_max, int *amount_str)
{
	FILE* mf=fopen(Name,"w");
	

	if (mf == NULL) return 0;
	//else printf ("open for write done\n");

	//fprintf (mf,"write file test");
	int a=0;
	int b=0;
	for (a=0;a<*amount_str;a++)
	{
		fprintf(mf,"%s\t",sp1Stopbec[a]);
		//printf("%s\t",sp1Stopbec[a]);

		for (b=0;b<*len_str_max;b++)
		{
			if (b<*len_str_max-1)
			{
				fprintf(mf,"%d\t",(sp1Data[a][b]));
				//printf("[%d][%d]=%d ",a,b,(sp1Data[a][b]));
			}
			else
			{
				fprintf(mf,"%d",(sp1Data[a][b]));
				//printf("[%d][%d]=%d_end",a,b,(sp1Data[a][b]));
			}


		}
		fprintf(mf,"\n");
		//printf("\n");
	}


	//printf ("Write done\n");


	fclose (mf);
	//printf ("File Closed\n");
	return 1;
}
//////////////////////////////////////////////////////////////////////////
int ReadTwoKeyColAndFloatColFromFile(const char* Name, char** StrCol1, char** StrCol2, float *FloatData, int *columns, int *rows)
{
	FILE* fp = fopen(Name, "r");
	if (fp == NULL) exit(EXIT_FAILURE);
	int result=1;
	int amount_str=0;
	char* line = NULL;
	size_t len = 0;
	int len_str=0;
	int len_str_max=0;
	if (!StrCol1 || !StrCol2) return 0;
	while ((getline(&line, &len, fp)) != -1) 
	{
		char** tokens;
		tokens = str_split(line, '\t', &len_str);

		//remember max line length
		if (len_str > len_str_max) len_str_max=len_str;

		//read to arrays allocate and free memory
		if (tokens)
		{
			int i;
			len_str_max=0;
			for (i = 0; *(tokens + i); i++)
			{
				if (i==0) 
				{ 
					//string
					StrCol1[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
					if (StrCol1[amount_str])
					{
						strcpy(StrCol1[amount_str], *(tokens + i));
					} 
				}
				else if (i==1)
				{
					//string
					StrCol2[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
					if (StrCol2[amount_str])
					{
						strcpy(StrCol2[amount_str], *(tokens + i));
					} 
				}
				else if (i==2)
				{
					FloatData[amount_str]=strtod(*(tokens + i),NULL);
					//float 2.552
				}

				free(*(tokens + i));
			}
			free(tokens);
		}
		amount_str++;

	}
	(*columns)=len_str_max;
	(*rows)=amount_str;
	//printf("READ of FILE to memory was good");
	fclose(fp);
	return result;
}
/////////////////////////////////////////////////////////////////////////
int ReadTwoKeyColFromFile(const char* Name, char** StrCol1, char** StrCol2,int *columns, int *rows)
{
	FILE* fp = fopen(Name, "r");
	if (fp == NULL) exit(EXIT_FAILURE);
	int result=1;
	int amount_str=0;
	char* line = NULL;
	size_t len = 0;
	int len_str=0;
	int len_str_max=0;
	if (!StrCol1 || !StrCol2) return 0;
	while ((getline(&line, &len, fp)) != -1) 
	{
		char** tokens;
		tokens = str_split(line, '\t', &len_str);

		//remember max line length
		if (len_str > len_str_max) len_str_max=len_str;

		//read to arrays allocate and free memory
		if (tokens)
		{
			int i;
			len_str_max=0;
			for (i = 0; *(tokens + i); i++)
			{
				if (i==0) 
				{ 
					StrCol1[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
					if (StrCol1[amount_str])
					{
						strcpy(StrCol1[amount_str], *(tokens + i));
					} 
				}
				else if (i==1)
				{
					StrCol2[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
					if (StrCol2[amount_str])
					{
						strcpy(StrCol2[amount_str], *(tokens + i));
					} 
				}

				free(*(tokens + i));
			}
			free(tokens);
		}
		amount_str++;

	}
	(*columns)=len_str_max;
	(*rows)=amount_str;
	//printf("READ of FILE to memory was good");
	fclose(fp);
	return result;
}
/////////////////////////////////////////////////////////////////////////
//StrCol1, StrCol2 must be of equal amount of str
int WriteTwoKeyColToFile(const char* Name, char** StrCol1, char** StrCol2, int *amount_str)
{
	FILE* mf=fopen(Name,"w");
	if (mf == NULL) return 0;
	//else printf ("open for write done\n");

	int a=0;
	for (a=0;a<*amount_str;a++)
	{
		fprintf(mf,"%s\t",StrCol1[a]);
		fprintf(mf,"%s\n",StrCol2[a]);
		//printf("%s\t",StrCol1[a]);
		//printf("%s\n",StrCol2[a]);
	}
	fclose (mf);
	return 1;
}
///////////////////////////////////////////////////////////////////////////
int WriteTwoKeyColAndFloatColToFile(const char* Name, char** StrCol1, char** StrCol2, float *FloatData, int *amount_str)
{
	FILE* mf=fopen(Name,"w");
	if (mf == NULL) return 0;
	//else printf ("open for write done\n");

	int a=0;
	for (a=0;a<*amount_str;a++)
	{
		fprintf(mf,"%s\t",StrCol1[a]);
		fprintf(mf,"%s\t",StrCol2[a]);
		fprintf(mf,"%.4f\n", FloatData[a]);
	}
	fclose (mf);
	return 1;
}
///////////////////////////////////////////////////////////////////////////
int MakeListOfDir(char* DirName,char **nameList)
{
	DIR           *d;
	struct dirent *dir;
	int count = 0;
	int index = 0;
	d = opendir(DirName);

	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if( strcmp( dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 )
				continue;
			if( dir->d_type == DT_DIR )
			{
				nameList[count] = (char*) malloc (strlen(dir->d_name)+1);
				strcpy(nameList[count],dir->d_name);
				printf("<DIR> %s\n", dir->d_name);
				count++;
			}
			else
			{
				//it is not directory do nothing
				//printf("%s\n", dir->d_name);
				//strcpy(name[count],dir->d_name);
			}

		}

		closedir(d);
	}
	return count;
}

///returns number of printed rows
int PrintTwoStringStructure( struct TStCol *src)
{
	int i = 0;
	for (i = 0; i<src->size; i++)
	{
		printf("%s[%d] ",src->StrCol1[i],i); 
		printf("%s[%d]\n",src->StrCol2[i],i); 
	}
	return i;
}
/////////////////////////////////////////////////////////////////////////
int FAppendTwoStringStructure(const char* Name, struct TStCol *src)
{
	FILE* mf=fopen(Name,"a");
	if (mf == NULL) return 0;
	//else printf ("open for write done\n");

	int i = 0;
	for (i = 0; i<src->size; i++)
	{
		//fprintf(mf,"%s[%d] ",src->StrCol1[i],i); 
		//fprintf(mf,"%s[%d]\n",src->StrCol2[i],i); 
		fprintf(mf,"%s\t",src->StrCol1[i],i); 
		fprintf(mf,"%s\n",src->StrCol2[i],i); 
	}
	//fprintf(mf,"\nEnd List Element\n"); 
	fclose (mf);
	return i;
}
/////////////////////////////////////////////////////////////////////////
//int sizeof dst or rows
int TStColCopy(struct TStCol *src, struct TStCol *dst)
{
	dst->StrCol1 = (char**)malloc(sizeof(char*) * (src->size));
	dst->StrCol2 = (char**)malloc(sizeof(char*) * (src->size));
	for (int i = 0; i< src->size; i++)
	{
			asprintf(&(dst->StrCol1[i]),"%s",src->StrCol1[i]); 
			asprintf(&(dst->StrCol2[i]),"%s",src->StrCol2[i]); 
	}
	dst->size=src->size;
	return sizeof(*dst);
}
int FreeTStCol(struct TStCol *dst)
{
	for (size_t i = 0; i< dst->size; i++)
	{
			free(dst->StrCol1[i]); 
			free(dst->StrCol2[i]); 
	}
	free(dst->StrCol1);
	free(dst->StrCol2);;
	dst->size=0;
	return sizeof(dst->size);
}
///////////////////////////////////////////////
int FreeArraySet(struct ArraySet *dst)
{
	free(dst->Labels);
	free(dst->IntMtrx);
	dst->raws=0;
	dst->cols=0;
	return sizeof(dst);
}
/*
typedef struct ArraySet {
	int raws;
	int cols;
	int** IntMtrx;
	int* Labels;
} ArraySet;
typedef struct TStCol {
	int size;
	char** StrCol1;
	char** StrCol2;
} TStCol;
*/

/////////////////////////////////////////////////////////////////////////
//int sizeof dst or rows
//it is supposed that the row number is already counted, but it is possible
//to implement this function for adding new elements to structure
int CopyTwoStringToStruct(char** StrCol1, char** StrCol2, int *amount_str, struct TStCol *dst)
{
	int add= *amount_str; 
	dst->StrCol1 = (char**)realloc(NULL, sizeof(char*) * (add));
	dst->StrCol2 = (char**)realloc(NULL, sizeof(char*) * (add));
	dst->size=amount_str;
	
	for (int i = 0; i<*amount_str; i++)
	{
		asprintf(&(dst->StrCol1[i]),"%s",StrCol1[i]); 
		asprintf(&(dst->StrCol2[i]),"%s",StrCol2[i]); 
	}
	dst->size=amount_str;
	return sizeof(*dst);
}
//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
char* DeleteLastN(char *str)
{
	char* pch=NULL;
	char* toTrim = (char*)malloc((strlen(str)+1)*sizeof(char*));
	strcpy(toTrim,str);
	pch=strchr(toTrim,'\n');
	if (pch)
	{ 
		memmove(pch, pch+1, strlen(pch));
		//printf("found ");
	}				
	strcpy(str,toTrim);
	return str;
}
/////////////////////////////////////////////////////////////////////////
//int sizeof dst or rows
//it is supposed that the row number is already counted, but it is possible
//to implement this function for adding new elements to structure
int ApndTStColTSColl(char** StrCol1, char** StrCol2, int amount_str, struct TStCol *dst)
{
	int start=dst->size;
	//printf("int start=dst->size %d\n",start);
	int add=amount_str;
	//printf("add=amount_str %d\n",add);
	if (dst->StrCol1 && dst->StrCol2 && dst->size)
	{	
		dst->StrCol1 = (char**)realloc(dst->StrCol1, sizeof(char*)*(start+add));
		dst->StrCol2 = (char**)realloc(dst->StrCol2, sizeof(char*)*(start+add));
		dst->size=start+add;
		//printf("\nRealloc\n");
	}
	else
	{	
		dst->StrCol1 = (char**)malloc(sizeof(char*)*(add));
		dst->StrCol2 = (char**)malloc(sizeof(char*)*(add));
		dst->size=add;
		start=0;
		//printf("\nFirst malloc\n");
	}

	for (int i = 0; i<add; i++)
	{
		asprintf(&(dst->StrCol1[i+start]),"%s",StrCol1[i]); 
		DeleteLastN(dst->StrCol1[i+start]);
		asprintf(&(dst->StrCol2[i+start]),"%s",StrCol2[i]); 
		//printf(" strings are transmitting[%d] from StrCol1[%d] ",(i+start),i);
	}
	return sizeof(*dst);
}
////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//Merdge two TStCol structures
//If destination is empty it MUST be initialized by zeros!!!!!!!!!!!!!
//arbitrary dst->size leads to segmentation fault 
int MrgeTwoTStCol(struct TStCol *src, struct TStCol *dst)
{
	int start=dst->size;
	int add=src->size;
	//printf("int start=dst->size %d\n",start);
	//printf("add=amount_str %d\n",add);
	if (!(src && src->StrCol1 && src->StrCol2 && src->size)) return 0;

	if (dst->StrCol1 && dst->StrCol2 && dst->size)
	{	
		dst->StrCol1 = (char**)realloc(dst->StrCol1, sizeof(char*)*(start+add));
		dst->StrCol2 = (char**)realloc(dst->StrCol2, sizeof(char*)*(start+add));
		dst->size=start+add;
		//printf("\nRealloc\n");
	}
	else
	{	
		dst->StrCol1 = (char**)malloc(sizeof(char*)*(add));
		dst->StrCol2 = (char**)malloc(sizeof(char*)*(add));
		dst->size=add;
		start=0;
		//printf("\nFirst malloc\n");
	}

	for (int i = 0; i<add; i++)
	{
		asprintf(&(dst->StrCol1[i+start]),"%s",src->StrCol1[i]); 
		asprintf(&(dst->StrCol2[i+start]),"%s",src->StrCol2[i]); 
		//printf(" strings are transmitting[%d] from StrCol1[%d] ",(i+start),i);
	}
	return sizeof(*dst);
}
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////
//struct StrColIntMtrx s; Matrix of Int hXl
struct StrColIntMtrx *pStrColIntMtrx(struct StrColIntMtrx *s,char **sp1Stopbec, int **sp1Data, int size_l, int size_h)
{
	//struct StrColIntMtrx *s;
	s->size_h=size_h;
	s->size_l=size_l;
	s->StrCol=malloc(sizeof(char*) * (size_h));
	s->IntMtrx=malloc(size_h*sizeof(int*) + size_h*size_l*sizeof(int));
	if (sp1Stopbec && sp1Data)
	{	
	char* pc = s->IntMtrx;
	pc += size_h*sizeof(int*);
	for (int i=0; i < size_h; i++) 
	{
		s->IntMtrx[i] = pc + i*size_l*sizeof(int);
	}
	//StrCol initializing
	for (int i=0; i<size_h; i++)
	{
		asprintf(&(s->StrCol[i]),"%s",sp1Stopbec[i]); 
		for (int j=0; j<size_l; j++)
		{
			s->IntMtrx[i][j]=sp1Data[i][j];
		}
	}
	}
	if (s->StrCol && s->IntMtrx && size_h && size_l)
	{
		return s;
	}
}
///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////
//This takes ***pppChr of sorted char**, and takes position of the first unsorted element char** Start
//to replace int row according to the new sorted position ot strCol
//struct StrColIntMtrx s; Matrix of Int hXl, s must be created, and filled with zeros.
struct StrColIntMtrx *pPStrColIntMtrx(struct StrColIntMtrx *s,char ***pppChr,char** Start, int **sp1Data, int size_l, int size_h)
{
	//struct StrColIntMtrx *s;
	s->size_h=size_h;
	s->size_l=size_l;
	s->StrCol=malloc(sizeof(char*) * (size_h));
	s->IntMtrx=malloc(size_h*sizeof(int*) + size_h*size_l*sizeof(int));
	if (pppChr && sp1Data)
	{	
	char* pc = s->IntMtrx;
	pc += size_h*sizeof(int*);
	for (int i=0; i < size_h; i++) 
	{
		s->IntMtrx[i] = pc + i*size_l*sizeof(int);
	}
	//StrCol initializing
	for (int i=0; i<size_h; i++)
	{
		asprintf(&(s->StrCol[i]),"%s",*(pppChr)[i]); 
		for (int j=0; j<size_l; j++)
		{
			s->IntMtrx[i][j]=sp1Data[(int)(pppChr[i]-Start)][j];
		}
	}
	}
	if (s->StrCol && s->IntMtrx && size_h && size_l)
	{
		return s;
	}
}
///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////
//This sorts unsorted element char** Start strings accending and repleses int raw according to.
//to replace int row according to the new sorted position ot strCol
//struct StrColIntMtrx s; Matrix of Int hXl, s must be created, and filled with zeros.
struct StrColIntMtrx *pStrSortedColIntMtrx(struct StrColIntMtrx *s,char** Start, int **sp1Data, int size_l, int size_h)
{
	//sort string column accending
	/////////////////////////////////////
	char*** pVal;
	int n=size_h;
	pVal=malloc(n*sizeof(char**));
	for( int i = 0; i < n; i++)
	{
		pVal[i] =Start+i;
	}
	qsort(pVal, n, sizeof(char*), cmpfunc);
	/////////////////////////////////////
	
	//initializing s structure	
	s->size_h=size_h;
	s->size_l=size_l;
	s->StrCol=malloc(sizeof(char*) * (size_h));
	s->IntMtrx=malloc(size_h*sizeof(int*) + size_h*size_l*sizeof(int));
	if (pVal && sp1Data)
	{	
		char* pc = s->IntMtrx;
		pc += size_h*sizeof(int*);
		for (int i=0; i < size_h; i++) 
		{
			s->IntMtrx[i] = pc + i*size_l*sizeof(int);
		}
		//StrCol initializing by acceding sorted pVal *Start, Start is array of unsorted char*
		for (int i=0; i<size_h; i++)
		{
			asprintf(&(s->StrCol[i]),"%s",*(pVal)[i]); 
			//changing rows according to the sorted str
			for (int j=0; j<size_l; j++)
			{	
				s->IntMtrx[i][j]=sp1Data[(int)(pVal[i]-Start)][j];
			}
		}
	}
	free(pVal);
	if (s->StrCol && s->IntMtrx && size_h && size_l)
	{
		return s;
	}
}
//if spdata is zeoro and we need only sp1stolbec in structure
struct StrColIntMtrx *pStrSortedColIntMtrxEmpty(struct StrColIntMtrx *s,char** Start, int size_h)
{
	//sort string column accending
	/////////////////////////////////////
	//size_l=0
	//int **sp1Data contains no data, but empty str
	char*** pVal;
	int n=size_h;
	pVal=malloc(n*sizeof(char**));
	for( int i = 0; i < n; i++)
	{
		pVal[i] =Start+i;
	}
	qsort(pVal, n, sizeof(char*), cmpfunc);
	/////////////////////////////////////
	
	//initializing s structure	
	s->size_h=size_h; 
	s->size_l=0; //0
	s->StrCol=malloc(sizeof(char*) * (size_h));
	if (pVal)
	{	
		//StrCol initializing by acceding sorted pVal *Start, Start is array of unsorted char*
		for (int i=0; i<size_h; i++)
		{
			asprintf(&(s->StrCol[i]),"%s",*(pVal)[i]); 
			//changing rows according to the sorted str
		}
	}
	free(pVal);
	if (s->StrCol && size_h)
	{
		return s;
	}
	else return NULL;
}

///////////////////////////////////
struct StrColIntMtrx *MrgeTwoStrIntMtrx(struct StrColIntMtrx *src, struct StrColIntMtrx *dst)
{
	int start=dst->size_h;
	int add=src->size_h;
	int size_h=add+start;
	int size_l=dst->size_l;
	//if (!(src && src->StrCol && src->IntMtrx && src->size_h && src->size_l)) return 0;
	//if (src->size_l != dst->size_l && dst->size_l ) return 0;

	if (dst->StrCol && dst->size_h)
	{	

		//create dst copy		
		StrColIntMtrx s;
		memset(&s,0,sizeof(StrColIntMtrx));
		StrColIntMtrx* ps=&s;
		ps=pStrColIntMtrx(ps,dst->StrCol,dst->IntMtrx,dst->size_l,dst->size_h);

//FPrintStrColIntMtrx("Test1Matr", ps)

		//StrCol
		dst->StrCol=(char**)realloc(dst->StrCol,sizeof(char*)*(add+start));
		
		//StrCol initializing
		//First destination Str is already in place, so copy only ones from src

		for (int i=0; i<start; i++)
		{
			asprintf(&(dst->StrCol[i]),"%s",ps->StrCol[i]); 
			//printf("%s[%d]",dst->StrCol[i+start],(i+start)); 
		}
		for (int i=0; i<add; i++)
		{
			asprintf(&(dst->StrCol[i+start]),"%s",src->StrCol[i]); 
			//printf("%s[%d]",dst->StrCol[i+start],(i+start)); 
		}
		
		//IntMtrx
		dst->IntMtrx=realloc(dst->IntMtrx,(size_h*sizeof(int*) + size_h*size_l*sizeof(int)));
		char* pc = dst->IntMtrx;
		pc += (add+start)*sizeof(int*);		
		for (int i=0; i < add+start; i++) 
		{
				dst->IntMtrx[i] = pc + i*size_l*sizeof(int);
		}
		//printf("630\n");
		for (int i=0; i < start; i++) 
		{	
			//printf("\nrow=%d",i);
			for (int j=0; j<size_l; j++)
			{
				dst->IntMtrx[i][j]=ps->IntMtrx[i][j];
			}
		}
		for (int i=start; i < size_h; i++) 
		{	
			//printf("\nrow=%d",i);
			for (int j=0; j<size_l; j++)
			{
				dst->IntMtrx[i][j]=src->IntMtrx[i-start][j];
			}
		}

		//new size_h 
		dst->size_h=start+add;
		//free ps
		FreeStrColIntMtrx(ps);



	}
	else
	{	
		StrColIntMtrx *s=pStrColIntMtrx(s,src->StrCol,src->IntMtrx,src->size_l,src->size_h);
		dst=s;
		size_l=dst->size_l;
		size_h=dst->size_h;
	}
	//if (dst->StrCol && dst->IntMtrx && size_h && size_l)
	//{
		return dst;
	//} 
	//else return NULL;
}
//////////////////////////////////////////////////////////////////////////
int FreeStrColIntMtrx(struct StrColIntMtrx *dst)
{
	for (int i=0; i < dst->size_h; i++)
		for (int j=0; j < dst->size_l; j++)
			{
				if (dst->IntMtrx[i][j]) free(dst->IntMtrx[i][j]);
			}
	
	if (dst->IntMtrx) free(dst->IntMtrx);
	for (int i = 0; i< dst->size_h; i++)
	{
		if (dst->StrCol[i]) free(dst->StrCol[i]); 
	}
	free(dst->StrCol);
	dst->size_l=0;
	dst->size_h=0;
	return 1;
}
/////////////////////////////////////////////////////////////////////////
int FPrintStrColIntMtrx(const char* Name, struct StrColIntMtrx *src)
{
	FILE* mf=fopen(Name,"a");
	if (mf == NULL) return 0;
	//else printf ("open for write done\n");
	for (int i=0; i < src->size_h; i++)
	{
		fprintf(mf,"%s\t",src->StrCol[i]);
		//printf("%s[%d]\t",src->StrCol[i],i);
		for (int j=0; j < src->size_l; j++)
		{
			if (j < src->size_l-1)
			{
				fprintf(mf,"%d\t",(src->IntMtrx[i][j]));
				//printf("%d[%d][%d]\t",(src->IntMtrx[i][j]),i,j);
			}
			else
			{
				fprintf(mf,"%d",(src->IntMtrx[i][j]));
				//printf("%d[%d][%d]",(src->IntMtrx[i][j]),i,j);
			}
		}
		fprintf(mf,"\n");
	}
	fclose (mf);
	return 1;
}
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
int FPrintLblIntMtrx(const char* Name, struct ArraySet *src)
{
	FILE* mf=fopen(Name,"w");
	if (mf == NULL) return 0;
	//else printf ("open for write done\n");
	for (int i=0; i < src->raws; i++)
	{
		fprintf(mf,"%d\t",src->Labels[i]);
		//printf("%s[%d]\t",src->StrCol[i],i);
		for (int j=0; j < src->cols; j++)
		{
			if (j < src->cols-1)
			{
				fprintf(mf,"%d\t",(src->IntMtrx[i][j]));
				//printf("%d[%d][%d]\t",(src->IntMtrx[i][j]),i,j);
			}
			else
			{
				fprintf(mf,"%d",(src->IntMtrx[i][j]));
				//printf("%d[%d][%d]",(src->IntMtrx[i][j]),i,j);
			}
		}
		fprintf(mf,"\n");
	}
	fclose (mf);
	return 1;
}
//Find key in src->StrCol and gives its row number in IntMtrx 
//src->StrCol must be accending sorted
//returns -1 if there is no occurences
int FindStrInKeyStrColIntMtrx(const char* key, struct StrColIntMtrx *src)
{
	char** pItem = NULL;
	if (!src || !key || !src->StrCol) return -1;
	//asprintf(&key,"%s",sp1Stopbec[amount_str-5]); 
	//printf("Let us find %s\n",key);
	pItem = (char **)bsearch (key, src->StrCol, src->size_h, sizeof(char*), cmpfunc2);
    if (pItem != NULL)
        return (int)(pItem-src->StrCol);
    else
        return -1;
	//how to get find IntMtrx row consistent key
	//for (i=0; i<sp2Espr->size_l; i++)
	    //printf("%d[%d,%d] ",sp2Espr->IntMtrx[(int)(pItem-sp2Espr->StrCol)][i], (int)(pItem-sp2Espr->StrCol), i);
	//printf("%s ",*pItem);
	//printf("%d ",(int)(pItem-sp2Espr->StrCol));
	//return 1;
}
///////////////////////////////////////////////////////////////////////////
int ReadKnownTwoKeyColFromFile(const char* Name, char** StrCol1, char** StrCol2,int *columns, int *rows,struct StrColIntMtrx *src1,struct StrColIntMtrx *src2)
{
	FILE* fp = fopen(Name, "r");
	if (fp == NULL) exit(EXIT_FAILURE);
	int result=1;
	int amount_str=0;
	int dropped_str=0;
	char* line = NULL;
	char* toSearchFor=NULL;
	size_t len = 0;
	int len_str=0;
	int len_str_max=0;
	int ItemInSrc1=0, ItemInSrc2=0;
	if (!StrCol1 || !StrCol2) return 0;
	while ((getline(&line, &len, fp)) != -1) 
	{
		char** tokens;
		tokens = str_split(line, '\t', &len_str);

		//remember max line length
		if (len_str > len_str_max) len_str_max=len_str;

		//read to arrays allocate and free memory
		if (tokens)
		{
			int i;
			len_str_max=0;
			for (i = 0; *(tokens + i); i++)
			{
					toSearchFor = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
					strcpy(toSearchFor, *(tokens + i));
					DeleteLastN(toSearchFor);
					ItemInSrc1=FindStrInKeyStrColIntMtrx(toSearchFor, src1);
					ItemInSrc2=FindStrInKeyStrColIntMtrx(toSearchFor, src2);
				if (ItemInSrc1 > -1 || ItemInSrc2 > -1)
				{
					if (i==0) 
					{ 
						StrCol1[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
						if (StrCol1[amount_str])
						{
							strcpy(StrCol1[amount_str], *(tokens + i));
						} 
					}
					else if (i==1)
					{
						StrCol2[amount_str] = (char*)malloc((strlen(*(tokens + i))+1)*sizeof(char*));
						if (StrCol2[amount_str])
						{
							strcpy(StrCol2[amount_str], *(tokens + i));
						} 
					}
				} 
				else 
				{
					//printf("Doesn`t added %s %d %d ",StrCol1[amount_str], ItemInSrc1,ItemInSrc2);
					dropped_str++;
				}

				free(*(tokens + i));
				free(toSearchFor);
			}
			free(tokens);
		}
		amount_str++;

	}
	(*columns)=len_str_max;
	(*rows)=amount_str-dropped_str;
	printf("Added Rows %d ",(*rows));
	//printf("READ of FILE to memory was good");
	fclose(fp);
	return result;
}
/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int WriteTwoKeyToFile(const char* Name,int columns, size_t rows, TStCol* gNamsToPredict, StrColIntMtrx *src1,StrColIntMtrx *src2)
{
	//Name new file
	//rows must be 1 000 000
	//columns - 2
	
	size_t rws=0;  //number of raws were added
	//printf("1036\n");
	int result=1;
	int dropped_str=0;
	char* toSearchFor=NULL;
	char* temp=NULL;
	size_t Npair=(src1->size_h)*(src2->size_h);

	//allocate memory for two string arrays
	
	//gNamsToPredict->StrCol1 = (char**)malloc(sizeof(char*)*(Npair-rows));
	//gNamsToPredict->StrCol2 = (char**)malloc(sizeof(char*)*(Npair-rows));
	//gNamsToPredict->size=Npair-rows;
	gNamsToPredict->StrCol1 = (char**)malloc(sizeof(char*)*(src2->size_h));	
	gNamsToPredict->StrCol2 = (char**)malloc(sizeof(char*)*(src2->size_h));
	gNamsToPredict->size=src2->size_h;
	if ( gNamsToPredict || gNamsToPredict->StrCol1 || gNamsToPredict->StrCol2)
	{	
		for(int ii = 0; ii < src1->size_h ; ii++)
		{
			for(int jj = 0; jj < src2->size_h ; jj++)
			{	
					asprintf(&(gNamsToPredict->StrCol1[jj]),"%s",src1->StrCol[ii]);
					asprintf(&(gNamsToPredict->StrCol2[jj]),"%s",src2->StrCol[jj]);
			}
			result=FAppendTwoStringStructure(Name, gNamsToPredict);
			for(int jj = 0; jj < src2->size_h ; jj++)
			{	
					free(gNamsToPredict->StrCol1[jj]);
					free(gNamsToPredict->StrCol2[jj]);
			}
		}
	} 
	return result;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int FillTrainSet(TStCol* ListOfTrain, TStCol* trainSet, StrColIntMtrx *sp1Espr, StrColIntMtrx *sp2Espr, char* labels)
{		
	int Item=-1;
	char* IntStr=NULL;
	char** arLabel=malloc(sizeof(char*));
	asprintf(&arLabel[0],"%s",labels); 
	//asprintf(&arLabel[0],"0"); 
	char* Result=NULL;
	char* temp=NULL;
		int count=0;
		for (int i = 0; i<ListOfTrain->size; i++)
		{
			//printf("iter %d ",i);	
			//printf("Let us find %s -",ListOfTrain.StrCol1[i]);
			Item=-1;
			Item = FindStrInKeyStrColIntMtrx(ListOfTrain->StrCol1[i], sp1Espr);
			if (Item > -1)
			{
			    //printf("It is %d place",Item);
				for (int ii=0; ii<sp1Espr->size_l; ii++)
				{
					//printf("%d[%d,%d] ",sp1Espr->IntMtrx[Item][i],Item,i);
					//printf("TEST");
					if (IntStr)
					{
						//new copy to repair memory leak 05022020 AVM
						asprintf(&temp,"%s",IntStr);
						free(IntStr);
						asprintf(&IntStr,"%s\t%d",temp,sp1Espr->IntMtrx[Item][ii]); 
						free(temp);
					}
					else
						asprintf(&IntStr,"%d",sp1Espr->IntMtrx[Item][ii]); 
					//printf("%s ",IntStr);
					//getchar();
				}
				//ApndTStColTSColl(&(ListOfTrain.StrCol1[i]), &IntStr, 1, &trainSet);
				//free(IntStr);
				//IntStr=NULL;
				//PrintTwoStringStructure(&trainSet);
			}
			else
			    printf ("- doesn't exist\n");
			
			/////find str2 in sp2Espr
			Item=-1;
			Item = FindStrInKeyStrColIntMtrx(ListOfTrain->StrCol2[i], sp2Espr);
			if (Item > -1)
			{
			    //printf("It is %d place",Item);
				for (int ii=0; ii<sp2Espr->size_l; ii++)
				{
					if (IntStr)
					{
						//new copy to repair memory leak 05022020 AVM
						asprintf(&temp,"%s",IntStr);
						free(IntStr);
						asprintf(&IntStr,"%s\t%d",temp,sp2Espr->IntMtrx[Item][ii]); 
						free(temp);
					}
					else
					asprintf(&IntStr,"%d",sp2Espr->IntMtrx[Item][ii]); 
					//printf("%s ",IntStr);
				//getchar();
				}
				//ApndTStColTSColl(&(ListOfTrain.StrCol2[i]), &IntStr, 1, &trainSet);
				//arLabel[0]=labels;
				//arLabel[1]='\0';
				
				ApndTStColTSColl(arLabel, &IntStr, 1, trainSet);
				count++;
				//PrintTwoStringStructure(&trainSet);
			}
			else
			    printf ("- doesn't exist\n");
							
			free(IntStr);
			IntStr=NULL;
		}
		free(arLabel[0]);
		free(arLabel);
	return count;
} 
/////////////////////////////////////////////////////////////////////////////
ArraySet* MakeArrayTrainSet(TStCol* src, ArraySet* dst)
{		
	int raws=src->size;
	int cols=0;
	//dst->raws=src->size;
	
	//int result=1;
	//int amount_str=0;
	char* line = NULL;
	size_t len = 0;
	int len_str=0;
	int len_str_max=0;
	
	//count elements
	for (int i=0; i < raws; i++)
	{

			//printf("%s[%d]\t",src->StrCol[i],i);
			//printf("%s", line); 	//string to parts
			char** tokens;
			asprintf(&line,"%s",src->StrCol2[i]);
			//printf("line=[%s]\n\n", line); 	//length of the string
			tokens = str_split(line, '\t', &len_str);
		
			//free memory
			if (tokens)
			{	cols=0;
				int ii;
				for (ii = 0; *(tokens + ii); ii++)
				{
					cols++;
					free(*(tokens + ii));
				}
				free(tokens);

			}
			free(line);
	}
	
	//allocate memory
	dst->Labels=malloc(sizeof(float) * raws);
	dst->IntMtrx=malloc(raws*sizeof(float*) + raws*cols*sizeof(float));
	char* pc = dst->IntMtrx;
	pc += raws*sizeof(float*);
	for (int jj=0; jj < raws; jj++) 
	{
		dst->IntMtrx[jj] = pc + jj*cols*sizeof(float);
	}
	//if (dst->IntMtrx && src->StrCol2)
	{	
		for (int i=0; i < raws; i++) 
		{
			//dst->Labels[i] = atoi(src->StrCol1[i]);
			dst->Labels[i] = atof(src->StrCol1[i]);
			char** tokens;
			asprintf(&line,"%s",src->StrCol2[i]);
			tokens = str_split(line, '\t', &len_str);
			//remeber max line length
			if (tokens)
			{
				int ii;
				for (ii = 0; *(tokens + ii); ii++)
				{
					//(dst->IntMtrx[i][ii])=atoi(*(tokens + ii));
					dst->IntMtrx[i][ii]=atof(*(tokens + ii));
					free(*(tokens + ii));
				}
				free(tokens);
			}
			free(line);
			
		}
	}
	dst->raws=raws;
	dst->cols=cols;
	return dst;

} 
/////////////////////////////////////////////////////////////////////////////
void* getDataFromRun(int argc, char** argv)
{
	/*
	
    for( int i = 0; i < argc; ++i ) {
        //printf( "argv[ %d ] = %s\n", i, argv[ i ] );
		switch (i+1)
		{
			case 1: int = argv[0];
				break;
			case 2: int eta = argv[1]
				break;
			case 3: 
				break;
			case 4: 
				break;
			case 5: 
				break;
			case 6: 
				break;
			case 7: 
				break;
			case 8: 
				break;
			case 9: 
				break;
			case 10: 
				break;
			case 11: 
				break;
			case 12: 
				break;
			case 13: 
				break;
			case 14: 
				break;
			case 15: 
				break;
			case 16: 	
				break;
			default: 
		}
		
    }

	/*

eta = sys.argv[2]
subsample = sys.argv[3]
colsample_bytree = sys.argv[4]
colsample_bylevel = sys.argv[5]
min_child_weight = sys.argv[6]
gamma = sys.argv[7]
alpha = sys.argv[8]
lambdaParam = sys.argv[9]
numIterations = sys.argv[10]
eval_metric = sys.argv[11]
scale_pos_weight = sys.argv[12]
modelNam = sys.argv[13]
species1File = sys.argv[14]
species2File = sys.argv[15]
foldsDir = sys.argv[16]
negFoldsDir = sys.argv[17]
isHeader1 = sys.argv[18]
isHeader2 = sys.argv[19]
	*/
	
}
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) 
{
	//initialisation
	
		char* species1File;
		char* species2File;
		char* predictPairsFile;
		char* modelDir;
		char* PredDir;
		char* isHeader1;
		char* isHeader2;
		int MinIter,MaxIter; //ot kakoi i do kakoi iteracii $iter
		int Nthread; //kolichesvo processov

		size_t PredNum; // how much strings for prediction to write to dmatrix
	

	{		
		asprintf(&species1File, "%s", argv[1]);
		asprintf(&species2File, "%s", argv[2]);
		asprintf(&predictPairsFile, "%s", argv[3]);
		
		MinIter=atoi(argv[4]);
		MaxIter=atoi(argv[5]); 
				
		for( int i = 0; i < argc; ++i ) 
			printf( "argv[ %d ] = %s\n", i, argv[ i ] );
	}
	//fflush(NULL);
	//getchar();
	char cwd[PATH_MAX];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
   } else {
       perror("getcwd() error");
   }

	char* modelNameWithDir;
	//char* sNthread;
	//asprintf(&sNthread,"%d",Nthread); 
	int amount_str=0;  //rows
	int len_str_max=0; //max columns for the case of not rectangle table

	countTableElemFromFile(species1File, &len_str_max, &amount_str);
	
	/////////////////////////////////////////////////
	//create array of char* for first name column size

	printf("Allocation for Sp1Stolbes\n");
	char** sp1Stopbec = 0;
	sp1Stopbec = (char**)malloc(sizeof(char*) * (amount_str));
	if (sp1Stopbec) printf("Allocation for Sp1Stolbes is done");

	//////////////////////////////////////////////////////////////////////////
	//read file to Sp1Stolbec and sp1Data/////////////////////////////////////
	//ReadKeyColAndIntTableFromFile("ATH_TMAP_TGR_star.txt", sp1Stopbec, sp1Data, &len_str_max, &amount_str);
	//ReadKeyColAndIntTableFromFile(species1File, sp1Stopbec, sp1Data, &len_str_max, &amount_str);
	ReadKeyColAndDropIntFromFile(species1File, sp1Stopbec,&amount_str);

	//read file 2 ATH_TMAP_TGR_star.txt
	int amount_str2=0;  //rows
	int len_str_max2=0; //max columns for the case of not rectangle table

	//read file/////////////////////////////////////
	//file to count, a result: amount_str - rows, len_str_max - columns
	//countTableElemFromFile("AMB_TMAP_TGR_star.txt", &len_str_max2, &amount_str2);
	countTableElemFromFile(species2File, &len_str_max2, &amount_str2);
	//printf("Read %s... Rows(amount_str)=%d Columns(len_str_max)=%d\n",species2File,amount_str,len_str_max);
	
	/////////////////////////////////////////////////
	//create array of char* for first name column size

	printf("Allocation for Sp1Stolbes\n");
	char** sp1Stopbec2 = 0;
	sp1Stopbec2 = (char**)malloc(sizeof(char*) * (amount_str2));
	if (sp1Stopbec2) printf("Allocation for Sp1Stolbes is done");

	//////////////////////////////////////////////////////////////////////////
	ReadKeyColAndDropIntFromFile(species2File, sp1Stopbec2,&amount_str2);
	
	//test new structure
	StrColIntMtrx sp1Esp,sp2Esp;

	memset(&sp1Esp, 0, sizeof(StrColIntMtrx)); 
	memset(&sp2Esp, 0, sizeof(StrColIntMtrx));

	StrColIntMtrx *sp1Espr=&sp1Esp,*sp2Espr=&sp2Esp;
	sp1Espr=pStrSortedColIntMtrxEmpty(sp1Espr,sp1Stopbec,amount_str);
	sp2Espr=pStrSortedColIntMtrxEmpty(sp2Espr,sp1Stopbec2,amount_str2);


	TStCol gNamsToPredict;
	memset(&gNamsToPredict, 0, sizeof(gNamsToPredict));
	int amount_str7=0;  //rows
	int len_str_max7=0; //max columns for the case of not rectangle table

	//check if exist and create
	struct stat st = {0};

	
	printf("entering to WriteTwoKeyToFile");
	//make all possible pairs
	WriteTwoKeyToFile(predictPairsFile,2,0,&gNamsToPredict,sp1Espr,sp2Espr);


//disbled temporary for simple pairs generted debug purposes
	FreeTStCol(&gNamsToPredict);

	//free test and train
	//last iteration after cycle for is done
	//free memory to use it again
	printf("Free memory start...\n");
	free(species1File);
	free(species2File);
	free(predictPairsFile);
	FreeStrColIntMtrx(sp2Espr);
	FreeStrColIntMtrx(sp1Espr);
	for (int j=0;j<amount_str;j++)
		{
	   	  free(sp1Stopbec[j]);
	    }
	  free(sp1Stopbec);

	   for (int j=0;j<amount_str2;j++)
	     {
	   	  free(sp1Stopbec2[j]);
	     }
	  free(sp1Stopbec2);
	  return 0;
}
