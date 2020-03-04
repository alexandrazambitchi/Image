#include <stdio.h>
#include <stdlib.h>
#include <math.h>
typedef struct 
{
	unsigned char R, G, B;
}RGB;

typedef struct 
{
	unsigned int dim_img, latime_img, inaltime_img, padding;
}header;

typedef struct 
{
	unsigned int coloana, linie, inaltime, latime;
	float corelatie;
	RGB culoare;
}detectie;

void citire_header(char* nume_fisier_sursa, header *date)
{
	FILE *fin;
	fin=fopen(nume_fisier_sursa, "rb");
	if(fin==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_sursa);
		exit(-1);
	}
	fseek(fin, 2, SEEK_SET);
	fread(&date->dim_img, sizeof(unsigned int), 1, fin);
	fseek(fin, 18, SEEK_SET);
	fread(&date->latime_img, sizeof(unsigned int), 1, fin);
	fread(&date->inaltime_img, sizeof(unsigned int), 1, fin);
    if(date->latime_img % 4 != 0)
        date-> padding = 4 - (3 * date->latime_img) % 4;
    else
        date->padding = 0;
    fclose(fin);
}

void citire(char* nume_fisier_sursa, header date, RGB **mat)
{
	FILE *fin;
	unsigned char *pixel;
	pixel=(unsigned char*)malloc(3*sizeof(unsigned char));
	fin=fopen(nume_fisier_sursa, "rb");
	if(fin==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_sursa);
		exit(-1);
	}
	fseek(fin, 54, SEEK_SET);
	int i,j;
	for(i=0;i<date.inaltime_img;i++)
	{
		for(j=0;j<date.latime_img;j++)
		{
			fread(pixel, 3, 1, fin);
			mat[i][j].B=pixel[0];
			mat[i][j].G=pixel[1];
			mat[i][j].R=pixel[2];
		}
		fseek(fin, date.padding, SEEK_CUR);
	}
	fclose(fin);
}

void inversare(RGB **mat, header date)
{
	RGB aux;
	int i, j;
	for (i=0;i<date.inaltime_img/2;i++)
	{
		for(j=0;j<date.latime_img;j++)
		{
			aux=mat[i][j];
			mat[i][j]=mat[date.inaltime_img-1-i][j];
			mat[date.inaltime_img-1-i][j]=aux;
		}
	}
}

void liniarizare(char* nume_fisier_sursa, RGB *imagine_liniarizata, header date)
{
	int i, j, k=0;
	FILE *fin;
	fin=fopen(nume_fisier_sursa, "rb");
	if(fin==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_sursa);
		exit(-1);
	}
	RGB **mat;
	mat=(RGB**)malloc(date.inaltime_img*sizeof(RGB*));
    for(int i=0;i<date.inaltime_img;i++)
    {	
    	mat[i]=(RGB*)malloc(date.latime_img*sizeof(RGB));
    }
    if(mat==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	citire(nume_fisier_sursa, date, mat);
	fclose(fin);
	inversare(mat, date);
	for(i=0;i<date.inaltime_img;i++)
		for(j=0;j<date.latime_img;j++)
		{
			imagine_liniarizata[k].B=mat[i][j].B;
			imagine_liniarizata[k].G=mat[i][j].G;
			imagine_liniarizata[k].R=mat[i][j].R;
			k++;
		}
	for(int i=0;i<date.inaltime_img;i++)
		free(mat[i]);
	free(mat);
}

void copiere_header(char* nume_fisier_destinatie, char* nume_fisier_sursa)
{
	FILE *fout;
	FILE *fin;
	fout=fopen(nume_fisier_destinatie, "wb+");
	fin=fopen(nume_fisier_sursa, "rb");
	if(fin==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_sursa);
		exit(-1);
	}
	if(fout==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_destinatie);
		exit(-1);
	}
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	for (int i=0;i<54;i++)
	{
		fread(&c, 1,1, fin);
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
    fclose(fin);
    fclose(fout);
}

void scriere_cript(RGB *imagine_intermediara, char* nume_fisier_destinatie, char* nume_fisier_sursa, header date)
{
	FILE *fout;
	fout=fopen(nume_fisier_destinatie, "wb+");
	if(fout==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_destinatie);
		exit(-1);
	}
	copiere_header(nume_fisier_destinatie, nume_fisier_sursa);
	fseek(fout, 54, SEEK_SET);
	int i;
	for(i=0;i<date.inaltime_img*date.latime_img;i++)
	{
		fwrite(&imagine_intermediara[i].B, 1, 1, fout);
		fwrite(&imagine_intermediara[i].G, 1, 1, fout);
		fwrite(&imagine_intermediara[i].R, 1, 1, fout);
		if((i+1)%date.latime_img==0)
			for(int n=0;n<date.padding;n++)
				fputc(0, fout);
		fflush(fout);
	}
	fclose(fout);
}

void XORSHIFT32(unsigned int seed, unsigned int n, unsigned int *r)
{
	unsigned int k, rand;
	rand=seed;
	r[0]=seed;
	for(k=1;k<n;k++)
	{
		rand=rand^rand<<13;
		rand=rand^rand>>17;
		rand=rand^rand<<5;
		r[k]=rand;
	}
}

void permutare(unsigned int n, unsigned int *r, unsigned int *perm)
{
	unsigned int k, aux, poz, i=1;
	for(k=0;k<n;k++)
		perm[k]=k;
	for(k=n-1;k>=1;k--)
	{
		poz=r[i]%(k+1);
		aux=perm[poz];
		perm[poz]=perm[k];
		perm[k]=aux;
		i++;
	}
}

void inversare_linie(RGB *imagine_intermediara, header date)
{
	RGB **mat;
	mat=(RGB**)malloc(date.inaltime_img*sizeof(RGB*));
    for(int i=0;i<date.inaltime_img;i++)
    {	
    	mat[i]=(RGB*)malloc(date.latime_img*sizeof(RGB));
    }
    if(mat==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
    unsigned int i, j, k=0;
    for(i=0;i<date.inaltime_img;i++)
		for(j=0;j<date.latime_img;j++)
		{
			mat[i][j].B=imagine_intermediara[k].B;
			mat[i][j].G=imagine_intermediara[k].G;
			mat[i][j].R=imagine_intermediara[k].R;
			k++;
		}
    inversare(mat, date);
   	k=0;
	for(i=0;i<date.inaltime_img;i++)
		for(j=0;j<date.latime_img;j++)
		{
			imagine_intermediara[k].B=mat[i][j].B;
			imagine_intermediara[k].G=mat[i][j].G;
			imagine_intermediara[k].R=mat[i][j].R;
			k++;
		}
	for(int i=0;i<date.inaltime_img;i++)
		free(mat[i]);
	free(mat);
}

void criptare(char* nume_fisier_destinatie, char* nume_fisier_sursa, char * nume_fisier_cheie_secreta)
{
	FILE *fcheie;
	fcheie=fopen(nume_fisier_cheie_secreta, "r");
	if(fcheie==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_cheie_secreta);
		exit(-1);
	}
	header date;
	citire_header(nume_fisier_sursa, &date);
	RGB *imagine_liniarizata;
	imagine_liniarizata=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	if(imagine_liniarizata==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	liniarizare(nume_fisier_sursa, imagine_liniarizata, date);
	unsigned int cheie, sv;
	fscanf(fcheie, "%u", &cheie);
	fscanf(fcheie, "%u", &sv);
	RGB *imagine_intermediara;
	imagine_intermediara=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	RGB *imagine_criptata;
	imagine_criptata=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	if(imagine_intermediara==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	if(imagine_criptata==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	unsigned int *r, *perm;
	r=(unsigned int*)malloc((2*date.latime_img*date.inaltime_img)*sizeof(unsigned int));
	if(r==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	perm=(unsigned int*)malloc((date.latime_img*date.inaltime_img)*sizeof(unsigned int));
	if(perm==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	XORSHIFT32(cheie, 2*date.latime_img*date.inaltime_img, r);
	permutare(date.latime_img*date.inaltime_img, r, perm);
	unsigned int i;
	for(i=0;i<date.latime_img*date.inaltime_img;i++)
	{
		imagine_intermediara[perm[i]]=imagine_liniarizata[i];
	}
	int dimens=date.inaltime_img*date.latime_img;
	imagine_criptata[0].B=(sv&0xFF)^(imagine_intermediara[0].B)^(r[dimens]&0xFF);
	imagine_criptata[0].G=((sv>>8)&0xFF)^(imagine_intermediara[0].G)^((r[dimens]>>8)&0xFF);
	imagine_criptata[0].R=((sv>>16)&0xFF)^(imagine_intermediara[0].R)^((r[dimens]>>16)&0xFF);
	for(i=1;i<date.inaltime_img*date.latime_img;i++)
	{
		imagine_criptata[i].B=(imagine_criptata[i-1].B)^(imagine_intermediara[i].B)^(r[dimens+i]&0xFF);
		imagine_criptata[i].G=(imagine_criptata[i-1].G)^(imagine_intermediara[i].G)^((r[dimens+i]>>8)&0xFF);
		imagine_criptata[i].R=(imagine_criptata[i-1].R)^(imagine_intermediara[i].R)^((r[dimens+i]>>16)&0xFF);
	}
	inversare_linie(imagine_criptata, date);
	scriere_cript(imagine_criptata, nume_fisier_destinatie, nume_fisier_sursa, date);
	free(imagine_liniarizata);
	free(imagine_intermediara);
	free(imagine_criptata);
	free(r);
	free(perm);
	fclose(fcheie);
}

void permutare_invers(unsigned int n, unsigned int *perm, unsigned int *perminv)
{
	int i;
	for(i=0;i<n;i++)
		perminv[perm[i]]=i;
}

void decriptare(char* nume_fisier_destinatie, char* nume_fisier_sursa, char * nume_fisier_cheie_secreta)
{
	FILE *fcheie;
	fcheie=fopen(nume_fisier_cheie_secreta, "r");
	if(fcheie==NULL)
	{
		printf("Eroare la deschiderea fisierului %s\n", nume_fisier_cheie_secreta);
		exit(-1);
	}
	header date;
	citire_header(nume_fisier_sursa, &date);
	RGB *imagine_criptata;
	imagine_criptata=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	if(imagine_criptata==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	liniarizare(nume_fisier_sursa, imagine_criptata, date);
	unsigned int cheie, sv;
	fscanf(fcheie, "%u", &cheie);
	fscanf(fcheie, "%u", &sv);
	RGB *imagine_intermediara;
	imagine_intermediara=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	if(imagine_intermediara==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	unsigned int *r, *perm;
	r=(unsigned int*)malloc((2*date.latime_img*date.inaltime_img)*sizeof(unsigned int));
	perm=(unsigned int*)malloc((date.latime_img*date.inaltime_img)*sizeof(unsigned int));
	if(r==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	if(perm==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	XORSHIFT32(cheie, 2*date.latime_img*date.inaltime_img, r);
	permutare(date.latime_img*date.inaltime_img, r, perm);
	unsigned int *perminv;
	perminv=(unsigned int*)malloc((date.latime_img*date.inaltime_img)*sizeof(unsigned int));
	if(perminv==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	permutare_invers(date.latime_img*date.inaltime_img, perm, perminv);
	unsigned int i;
	unsigned int dimens=date.latime_img*date.inaltime_img;
	imagine_intermediara[0].B=(sv&0xFF)^(imagine_criptata[0].B)^(r[dimens]&0xFF);
	imagine_intermediara[0].G=((sv>>8)&0xFF)^(imagine_criptata[0].G)^((r[dimens]>>8)&0xFF);
	imagine_intermediara[0].R=((sv>>16)&0xFF)^(imagine_criptata[0].R)^((r[dimens]>>16)&0xFF);
	for(i=1;i<date.inaltime_img*date.latime_img;i++)
	{
		imagine_intermediara[i].B=(imagine_criptata[i-1].B)^(imagine_criptata[i].B)^(r[dimens+i]&0xFF);
		imagine_intermediara[i].G=(imagine_criptata[i-1].G)^(imagine_criptata[i].G)^((r[dimens+i]>>8)&0xFF);
		imagine_intermediara[i].R=(imagine_criptata[i-1].R)^(imagine_criptata[i].R)^((r[dimens+i]>>16)&0xFF);
	}
	RGB *imagine_decriptata;
	imagine_decriptata=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	if(imagine_decriptata==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	for(i=0;i<date.inaltime_img*date.latime_img;i++)
	{
		imagine_decriptata[perminv[i]]=imagine_intermediara[i];
	}
	inversare_linie(imagine_decriptata, date);
	scriere_cript(imagine_decriptata, nume_fisier_destinatie, nume_fisier_sursa, date);
	free(imagine_intermediara);
	free(imagine_criptata);
	free(imagine_decriptata);
	free(perminv);
	free(r);
	free(perm);
	fclose(fcheie);
}

void calcul_chi_patrat(char* nume_fisier)
{
	FILE *fin;
	fin=fopen(nume_fisier, "rb");
	if(fin==NULL)
	{
		printf("Eroare la deschiderea imaginii %s\n", nume_fisier);
		exit(-1);
	}
	double chi_b=0, chi_g=0, chi_r=0;
	unsigned int *frecventa_B, *frecventa_G, *frecventa_R;
	frecventa_B=(unsigned int*)calloc(256, sizeof(unsigned int));
	frecventa_G=(unsigned int*)calloc(256, sizeof(unsigned int));
	frecventa_R=(unsigned int*)calloc(256, sizeof(unsigned int));
	header date;
	citire_header(nume_fisier, &date);
	RGB *imagine;
	imagine=(RGB*)malloc((date.inaltime_img*date.latime_img)*sizeof(RGB));
	if(imagine==NULL)
	{
		printf("Eroare la alocarea memoriei\n");
		exit(-1);
	}
	liniarizare(nume_fisier, imagine, date);
	unsigned int i;
	for(i=0;i<date.inaltime_img*date.latime_img;i++)
	{
		frecventa_B[imagine[i].B]++;
		frecventa_G[imagine[i].G]++;
		frecventa_R[imagine[i].R]++;
	}
	double frecventa_estimata;
	frecventa_estimata=(date.inaltime_img*date.latime_img)/256.0;
	for(i=0;i<=255;i++)
	{
		chi_b=chi_b+(((frecventa_B[i]-frecventa_estimata)*(frecventa_B[i]-frecventa_estimata))/frecventa_estimata);
		chi_g=chi_g+(((frecventa_G[i]-frecventa_estimata)*(frecventa_G[i]-frecventa_estimata))/frecventa_estimata);
		chi_r=chi_r+(((frecventa_R[i]-frecventa_estimata)*(frecventa_R[i]-frecventa_estimata))/frecventa_estimata);
	}
	printf("R: %.2f\n", chi_r);
	printf("G: %.2f\n", chi_g);
	printf("B: %.2f\n", chi_b);
	free(imagine);
	free(frecventa_B);
	free(frecventa_G);
	free(frecventa_R);
	fclose(fin);
}

void chi_test(char *nume_fisier_sursa, char *nume_fisier_destinatie_cript)
{
	printf("\nValorile testului chi patrat:\n");
	printf("\nImagine necriptata (%s):\n", nume_fisier_sursa);
	calcul_chi_patrat(nume_fisier_sursa);
	printf("\nImagine criptata (%s):\n", nume_fisier_destinatie_cript);
	calcul_chi_patrat(nume_fisier_destinatie_cript);
}

void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
	FILE *fin, *fout;
	unsigned int dim_img, latime_img, inaltime_img;
	unsigned char pRGB[3], header[54], aux;
	fin = fopen(nume_fisier_sursa, "rb");
	if(fin == NULL)
   	{ 
   		printf("Nu am gasit imaginea sursa din care citesc\n"); 
	   	exit(-1);
	}
	fout = fopen(nume_fisier_destinatie, "wb+");
	fseek(fin, 2, SEEK_SET);
   	fread(&dim_img, sizeof(unsigned int), 1, fin);
   	fseek(fin, 18, SEEK_SET);
   	fread(&latime_img, sizeof(unsigned int), 1, fin);
   	fread(&inaltime_img, sizeof(unsigned int), 1, fin);
	fseek(fin,0,SEEK_SET);
	unsigned char c;
	while(fread(&c,1,1,fin)==1)
	{
		fwrite(&c,1,1,fout);
		fflush(fout);
	}
	fclose(fin);
	int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;
	fseek(fout, 54, SEEK_SET);
	int i,j;
	for(i = 0; i < inaltime_img; i++)
	{
		for(j = 0; j < latime_img; j++)
		{
			fread(pRGB, 3, 1, fout);
			aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
			pRGB[0] = pRGB[1] = pRGB[2] = aux;
        	fseek(fout, -3, SEEK_CUR);
        	fwrite(pRGB, 3, 1, fout);
        	fflush(fout);
		}
		fseek(fout,padding,SEEK_CUR);
	}
	fclose(fout);
}

float corelare(char *nume_sablon, RGB **mat)
{
	header date_sablon;
	citire_header(nume_sablon, &date_sablon);
	RGB **mat_sablon;
	mat_sablon=(RGB**)malloc(date_sablon.inaltime_img*sizeof(RGB*));
    for(int i=0;i<date_sablon.inaltime_img;i++)
    {	
    	mat_sablon[i]=(RGB*)malloc(date_sablon.latime_img*sizeof(RGB));
    }
    citire(nume_sablon, date_sablon, mat_sablon);
    unsigned int i, j, k=0;
    unsigned char *Sp;
    Sp=(unsigned char*)malloc((date_sablon.inaltime_img*date_sablon.latime_img)*sizeof(unsigned char));
    float S_mediu;
    unsigned char*fI;
    fI=(unsigned char *)malloc((date_sablon.inaltime_img*date_sablon.latime_img)*sizeof(unsigned char));
    float fI_mediu;
    float deviatie_s, suma=0, sumaSp=0;
    for(i=0;i<date_sablon.inaltime_img;i++)
    	for(j=0;j<date_sablon.latime_img;j++)
    	{
    		Sp[k]=mat_sablon[i][j].R;
    		fI[k]=mat[i][j].R;
    		k++;
    	}
    for(i=0;i<k;i++)
    	sumaSp=sumaSp+(float)Sp[i];
    unsigned int n=date_sablon.inaltime_img*date_sablon.latime_img;
        S_mediu=sumaSp/n;
    for(i=0;i<k;i++)
    {
    	suma=suma+((Sp[i]-S_mediu)*(Sp[i]-S_mediu));
    }
    deviatie_s=sqrt(suma/(n-1));
    float deviatie_f, sumaf=0;
    suma=0;
    k=0;
    for(i=0;i<date_sablon.inaltime_img;i++)
    	for(j=0;j<date_sablon.latime_img;j++)
    	{
    		fI[k]=mat[i][j].R;
    		k++;
    	}
    for(i=0;i<k;i++)
    	sumaf=sumaf+fI[i];
    fI_mediu=sumaf/n;
    for(i=0;i<k;i++)
    {
    	suma=suma+((fI[i]-fI_mediu)*(fI[i]-fI_mediu));
    }
    deviatie_f=sqrt(suma/(n-1));
    float sumaCor=0;
    for(i=0;i<k;i++)
    {
    	sumaCor=sumaCor+(((fI[i]-fI_mediu)*(Sp[i]-S_mediu))/(deviatie_f*deviatie_s));
    }
    float Corr;
    Corr=sumaCor/n;
    for(i=0;i<date_sablon.inaltime_img;i++)
		free(mat_sablon[i]);
	free(mat_sablon);
	free(Sp);
	free(fI);
	return Corr;
}

void template_matching(char *nume_fisier_sursa, char*nume_sablon, float prag, detectie *det, unsigned int *poz)
{
	header date_img, date_sablon;
	citire_header(nume_fisier_sursa, &date_img);
	citire_header(nume_sablon, &date_sablon);
	RGB **mat_imagine, **mat_sablon;
	unsigned int i, j, a, b, nrcorr=0;
	float cor;
	mat_imagine=(RGB **)malloc(date_img.inaltime_img*sizeof(RGB*));
	for(i=0;i<date_img.inaltime_img;i++)
		mat_imagine[i]=(RGB*)malloc(date_img.latime_img*sizeof(RGB));
	mat_sablon=(RGB **)malloc(date_sablon.inaltime_img*sizeof(RGB*));
	for(i=0;i<date_sablon.inaltime_img;i++)
		mat_sablon[i]=(RGB*)malloc(date_sablon.latime_img*sizeof(RGB));
	citire(nume_fisier_sursa, date_img, mat_imagine);
	citire(nume_sablon, date_sablon, mat_sablon);
	RGB ** matrice;
	matrice=(RGB**)malloc(date_sablon.inaltime_img*sizeof(RGB*));
    for(int i=0;i<date_sablon.inaltime_img;i++)
    {	
    	matrice[i]=(RGB*)malloc(date_sablon.latime_img*sizeof(RGB));
    }
	for(a=0;a<date_img.inaltime_img;a++)
	{
		for(b=0;b<date_img.latime_img;b++)
		{
			
			if(a+date_sablon.inaltime_img<=date_img.inaltime_img && b+date_sablon.latime_img<=date_img.latime_img)
			{
				for(i=0;i<date_sablon.inaltime_img;i++)
				{
			    	for(j=0;j<date_sablon.latime_img;j++)
			    		matrice[i][j]=mat_imagine[a+i][b+j];
				}
				cor=corelare(nume_sablon, matrice);
				if(cor>=prag)
				{
					nrcorr++;
					det[(*poz)].inaltime=date_sablon.inaltime_img;
					det[(*poz)].latime=date_sablon.latime_img;
					det[(*poz)].coloana=b;
					det[(*poz)].linie=a;
					det[(*poz)].corelatie=cor;
					(*poz)++;
				}
			}
		}
	}
	for(i=0;i<date_img.inaltime_img;i++)
		free(mat_imagine[i]);
	free(mat_imagine);
	for(i=0;i<date_sablon.inaltime_img;i++)
		free(mat_sablon[i]);
	free(mat_sablon);
	for(i=0;i<date_sablon.inaltime_img;i++)
		free(matrice[i]);
	free(matrice);
}

int cmp(const void *a, const void *b)
{
	if((((detectie*)a)->corelatie)>=(((detectie*)b)->corelatie))
		return -1;
	return 1;
}

void colorare(RGB **mat_imagine, header date_imagine, detectie *det, unsigned int poz)
{
	unsigned int i, j, k, l;
	unsigned int cnt;
	for(cnt=0;cnt<poz;cnt++)
	{
		for(i=0;i<date_imagine.inaltime_img;i++)
			for(j=0;j<date_imagine.latime_img;j++)
			{
				if(i==det[cnt].linie && j==det[cnt].coloana)
				{
					for(k=0;k<det[cnt].inaltime;k++)
					{
						mat_imagine[k+i][j]=det[cnt].culoare;
						mat_imagine[k+i][j+(det[cnt].latime-1)]=det[cnt].culoare;
					}
					for(l=1;l<det[cnt].latime-1;l++)
					{	
						mat_imagine[i][j+l]=det[cnt].culoare;
						mat_imagine[i+(det[cnt].inaltime-1)][j+l]=det[cnt].culoare;
					}
					i=date_imagine.inaltime_img;
					j=date_imagine.latime_img;
				}
			}
	}
}

void scriere_pattern(char *nume_fisier_destinatie, char *nume_fisier_sursa, detectie *det, unsigned int poz)
{
	FILE *fout;
	fout=fopen(nume_fisier_destinatie, "wb+");
	header date_img;
	citire_header(nume_fisier_sursa, &date_img);
	copiere_header(nume_fisier_destinatie, nume_fisier_sursa);
	int i, j;
	RGB ** mat_imagine;
	mat_imagine=(RGB **)malloc(date_img.inaltime_img*sizeof(RGB*));
	for(i=0;i<date_img.inaltime_img;i++)
		mat_imagine[i]=(RGB*)malloc(date_img.latime_img*sizeof(RGB));
	citire(nume_fisier_sursa, date_img, mat_imagine);
	colorare(mat_imagine, date_img, det, poz);
	fseek(fout, 54, SEEK_SET);
	for(i=0;i<date_img.inaltime_img;i++)
	{
		for(j=0;j<date_img.latime_img;j++)
		{
			fwrite(&mat_imagine[i][j].B, 1, 1, fout);
			fwrite(&mat_imagine[i][j].G, 1, 1, fout);
			fwrite(&mat_imagine[i][j].R, 1, 1, fout);
		}
		for(int n=0;n<date_img.padding;n++)
			fputc(0, fout);
		fflush(fout);
	}
	fclose(fout);
}

unsigned int min(unsigned int a, unsigned int b)
{
	if (a>b)
		return b;
	else
		return a;
}

unsigned int max(unsigned int a, unsigned int b)
{
	if (a>b)
		return a;
	else
		return b;
}

unsigned int arie_intersectie(detectie di, detectie dj)
{
	detectie aux_i1, aux_i2, aux_j1, aux_j2;
	aux_i1.linie=di.linie+di.inaltime-1;
	aux_i1.coloana=di.coloana;
	aux_i2.linie=di.linie;
	aux_i2.coloana=di.coloana+di.latime-1;
	aux_j1.linie=dj.linie+dj.inaltime-1;
	aux_j1.coloana=dj.coloana;
	aux_j2.linie=dj.linie;
	aux_j2.coloana=dj.coloana+dj.latime-1;
	return (min(aux_i2.linie, aux_j2.linie)-max(aux_i1.linie, aux_j1.linie))*(min(aux_i2.coloana, aux_j2.coloana)-max(aux_i1.coloana, aux_j1.coloana));
}

int verif_suprapunere(detectie di, detectie dj)
{
	detectie aux_i, aux_j;
	aux_i.linie=di.linie+di.inaltime-1;
	aux_i.coloana=di.coloana+di.latime-1;
	aux_j.linie=dj.linie+dj.inaltime-1;
	aux_j.coloana=dj.coloana+dj.latime-1;
	if(di.linie>aux_j.linie || dj.linie>aux_i.linie)
		return 0;
	if(di.coloana>aux_j.coloana || dj.coloana>aux_i.coloana)
		return 0;
	return 1;
}

float calcul_suprapunere(detectie *det, unsigned int *poz)
{
	unsigned int i, j, arieInters, arie1, arie2;
	float suprapunere;
	for(i=0;i<(*poz)-1;i++)
		for(j=i+1;j<(*poz);j++)
			if(det[i].corelatie>det[j].corelatie)
			{
				if(verif_suprapunere(det[i], det[j])!=0)
				{	
					arieInters=arie_intersectie(det[i], det[j]);
					arie1=det[i].latime*det[i].inaltime;
					arie2=det[j].latime*det[j].inaltime;
					suprapunere=arieInters/(arie1+arie2-arieInters);
					if(suprapunere>0.2)
					{
						for(int k=j;k<(*poz)-1;k++)
						{
							det[k]=det[k+1];
						}
						(*poz)--;									
					}
				}
			}
}

int main()
{
	char *nume_fisier_sursa, *nume_fisier_destinatie_cript, *nume_fisier_cheie_secreta, *nume_fisier_destinatie_decript, *nume_fisier_sursa_decript, *nume_fisier_cheie_secreta_decript;
	nume_fisier_sursa=(char *)malloc(30*sizeof(char));
	nume_fisier_destinatie_cript=(char *)malloc(30*sizeof(char));
	nume_fisier_destinatie_decript=(char *)malloc(30*sizeof(char));
	nume_fisier_cheie_secreta=(char *)malloc(30*sizeof(char));
	nume_fisier_cheie_secreta_decript=(char *)malloc(30*sizeof(char));
	nume_fisier_sursa_decript=(char *)malloc(30*sizeof(char));
	printf("\nCRIPTARE/DECRIPTARE\n");
	printf("\nNumele fisierului sursa la criptare: \n");
	scanf("%s", nume_fisier_sursa);
	printf("Numele fisierului destinatie la criptare: \n");
	scanf("%s", nume_fisier_destinatie_cript);
	printf("Numele fisierului care contine cheia secreta: \n");
	scanf("%s", nume_fisier_cheie_secreta);
	criptare(nume_fisier_destinatie_cript, nume_fisier_sursa, nume_fisier_cheie_secreta);
	printf("\nCriptare terminata\n\n");
	printf("Numele fisierului sursa la decriptare: \n");
	scanf("%s", nume_fisier_sursa_decript);
	printf("Numele fisierului destinatie la decriptare: \n");
	scanf("%s", nume_fisier_destinatie_decript);
	printf("Numele fisierului care contine cheia secreta: \n");
	scanf("%s", nume_fisier_cheie_secreta_decript);
	decriptare(nume_fisier_destinatie_decript, nume_fisier_sursa_decript, nume_fisier_cheie_secreta_decript);
	printf("\nDecriptare terminata\n");
	chi_test(nume_fisier_sursa, nume_fisier_destinatie_cript);
	free(nume_fisier_sursa);
	free(nume_fisier_destinatie_cript);
	free(nume_fisier_destinatie_decript);
	free(nume_fisier_cheie_secreta);
	free(nume_fisier_cheie_secreta_decript);
	free(nume_fisier_sursa_decript);

	printf("\nRECUNOASTERE PATTERN-URI\n");
	char *nume_sursa, *nume_destinatie, *nume_sablon, *nume_sablon_grayscale, *nume_fisier_sablon;
	nume_sursa=(char *)malloc(30*sizeof(char));
	nume_destinatie=(char *)malloc(30*sizeof(char));
	nume_sablon=(char *)malloc(30*sizeof(char));
	nume_sablon_grayscale=(char *)malloc(30*sizeof(char));
	nume_fisier_sablon=(char *)malloc(30*sizeof(char));
	printf("\nNumele fisierului sursa pentru recunoasterea sabloanelor:\n");
	scanf("%s", nume_sursa);
	char nume_sursa_grayscale[]="grayscale.bmp";
	grayscale_image(nume_sursa, nume_sursa_grayscale);
	header date_img;
	citire_header(nume_sursa, &date_img);
	detectie *det;
	det=(detectie *)malloc((date_img.inaltime_img*date_img.latime_img)*sizeof(detectie));
	unsigned int n, i, j, poz=0, pozinit;
	unsigned char *culoare_sablon;
	culoare_sablon=(unsigned char*)malloc(3*sizeof(unsigned char));
	float prag=0.5;
	printf("Numele fisierului care contine informatii despre sabloane:\n");
	scanf("%s", nume_fisier_sablon);
	FILE *f_sablon=fopen(nume_fisier_sablon, "r");
	if(f_sablon==NULL)
	{
		printf("Nu s-a gasit fisierul %s\n", nume_fisier_sablon);
		exit(-1);
	}
	printf("Citire si prelucrare sabloane din fisier\n");
	fscanf(f_sablon, "%d", &n);
	for(i=1;i<=n;i++)
	{
		fscanf(f_sablon,"%s", nume_sablon);
		fscanf(f_sablon,"%s", nume_sablon_grayscale);
		grayscale_image(nume_sablon, nume_sablon_grayscale);
		pozinit=poz;
		template_matching(nume_sursa_grayscale, nume_sablon_grayscale, prag, det, &poz);
		fscanf(f_sablon,"%u %u %u", &culoare_sablon[0], &culoare_sablon[1], &culoare_sablon[2]);
		for(j=pozinit;j<poz;j++)
		{	
			det[j].culoare.R=culoare_sablon[0];
			det[j].culoare.G=culoare_sablon[1];
			det[j].culoare.B=culoare_sablon[2];
		}
	}
	det=realloc(det, poz*sizeof(detectie));
	qsort(det, poz, sizeof(detectie), cmp);
	calcul_suprapunere(det, &poz);
	det=realloc(det, poz*sizeof(detectie));
	printf("Numele fisierului destinatie pentru recunoasterea sabloanelor:\n");
	scanf("%s", nume_destinatie);
	scriere_pattern(nume_destinatie, nume_sursa, det, poz);
	free(culoare_sablon);
	free(nume_sursa);
	free(nume_destinatie);
	free(nume_sablon);
	free(nume_sablon_grayscale);
	free(det);
	fclose(f_sablon);
	printf("\n");
	return 0;
}