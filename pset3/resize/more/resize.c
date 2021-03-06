// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include<math.h>

#include "bmp.h"

int findNearestOnePercentN(float f,float width);
int resize(double* f,char *infile,char *outfile);
int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize f infile outfile\n");
        return 1;
    }
    // f should be num
    double f;
    if( (f=atof(argv[1]))==0 )
    {
        fprintf(stderr, "Usage: f should be num\n");
        return 1;
    }


    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    resize(&f, infile, outfile);

    // success
    return 0;
}





/*
void check()
{


}
*/
int resize(double* f,char *infile,char *outfile)
{

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }
    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    //integer times larger
    if(*f>=1 && *f<=100)
    {
       //new BMP file width,new height
        BITMAPFILEHEADER nbf=bf;
        BITMAPINFOHEADER nbi=bi;
        nbi.biWidth*=*f;
        nbi.biHeight*=*f;
        int nbiHeight = abs(nbi.biHeight);
        int nbiWidth = nbi.biWidth;
       // determine padding for original BMP file and newpadding for new BMP file scanlines
        int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
        int npadding = (4 - (nbiWidth * sizeof(RGBTRIPLE)) % 4) % 4;
        // write outfile's BITMAPFILEHEADER:bfSize
        nbf.bfSize=54+nbiHeight*(nbiWidth*3+npadding);
        fwrite(&nbf, sizeof(BITMAPFILEHEADER), 1, outptr);

        // write outfile's BITMAPINFOHEADER:biWidth,biHeight
        nbi.biSizeImage=nbf.bfSize-54;
        fwrite(&nbi, sizeof(BITMAPINFOHEADER), 1, outptr);

        // iterate over infile's scanlines to  magnify the picture
        for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
        {
                //declare virable to store the new BMP RGBTRIPLEs
                RGBTRIPLE *ntriple=malloc(nbiWidth*sizeof(RGBTRIPLE));
                // iterate over pixels in scanline
                for (int j = 0; j < bi.biWidth; j++)
                {
                    // temporary storage
                    RGBTRIPLE triple;
                    // read RGB triple from infile
                    fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                    // write RGB triple to outfile,repeat f times
                    int re;
                    for (re=0; re < *f; re++)
                    {
                        fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                        int index=((int)*f)*j+re;
                        ntriple[index]=triple;  //store the new BMP RGBTRIPLEs
                    }
                }

                // skip over padding, if any
                fseek(inptr, padding, SEEK_CUR);

                // then add it back (to demonstrate how)
                for (int k = 0; k < npadding; k++)
                {
                    fputc(0x00, outptr);
                }
                //write the same line to outfile  f times repeatedly, including the padding
                for (int l=0;l<*f-1;l++)
                {
                    fwrite(ntriple, sizeof(RGBTRIPLE), nbiWidth, outptr);
                    for (int k = 0; k < npadding; k++)
                     {
                        fputc(0x00, outptr);
                     }
                }
                //free memory
                free(ntriple);
        }
    }
    else if(*f>0 && *f<1)
    {
    //set new BMP file header
        //new BMP file width,new height initialization
        BITMAPFILEHEADER nbf=bf;
        BITMAPINFOHEADER nbi=bi;
         //support least 0.01 precision
        if(*f-0.01<0)
            *f=0.01;
        //caculate sample step
        int wSampleStep=findNearestOnePercentN(*f,nbi.biWidth);
        int hSampleStep=findNearestOnePercentN(*f,abs(nbi.biHeight));
        //when sample step equals one ,which means f=1
         //waitting
        //caculate new BMP file width,new height
        if(nbi.biWidth%wSampleStep==0)
            nbi.biWidth=nbi.biWidth/wSampleStep;
        else
            nbi.biWidth=nbi.biWidth/wSampleStep+1;
        if(bi.biHeight%hSampleStep==0)
            nbi.biHeight=bi.biHeight/hSampleStep;
        else
            nbi.biHeight=bi.biHeight/hSampleStep+1;

        int nbiHeight = abs(nbi.biHeight);
        int nbiWidth = nbi.biWidth;

       // determine padding for original BMP file and newpadding for new BMP file scanlines
        int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
        int npadding = (4 - (nbiWidth * sizeof(RGBTRIPLE)) % 4) % 4;
        // write outfile's BITMAPFILEHEADER:bfSize
        nbf.bfSize=54+nbiHeight*(nbiWidth*3+npadding);
        fwrite(&nbf, sizeof(BITMAPFILEHEADER), 1, outptr);

        // write outfile's BITMAPINFOHEADER:biWidth,biHeight
        nbi.biSizeImage=nbf.bfSize-54;
        fwrite(&nbi, sizeof(BITMAPINFOHEADER), 1, outptr);

       //debug
       // printf("position:%ld\n",ftell(inptr));

    // iterate over infile's scanlines to sample the picture
        for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i+=hSampleStep)
        {
                //declare virable to store the new BMP RGBTRIPLEs
               // RGBTRIPLE *ntriple=malloc(nbiWidth*sizeof(RGBTRIPLE));
                // iterate over pixels in scanline
                for (int j = 0; j < bi.biWidth; j+=wSampleStep)
                {
                    // temporary storage
                    RGBTRIPLE triple;
                    // read RGB triple from infile
                    fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                    // write RGB triple to outfile,repeat f times
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                    int left=bi.biWidth%wSampleStep;
                    //debug
                    //printf("bi.biWidth:%d\n",bi.biWidth);
                    //printf("left:%d\n",left);
                    if( left!=0 && bi.biWidth-j<wSampleStep)
                    {
                        fseek(inptr, (left-1)*3, SEEK_CUR);
                       //debug
                       //printf("branch1:%d\n",left*3);
                    }
                    else
                    {
                        fseek(inptr, (wSampleStep-1)*3, SEEK_CUR);
                       //debug
                       //printf("branch2:%d\n",(wSampleStep-1)*3);
                    }
                   //debug
                   //printf("position1:%ld\n",ftell(inptr));
                }

                // skip over padding, if any
                fseek(inptr, padding, SEEK_CUR);

                // then add it back (to demonstrate how)
                for (int k = 0; k < npadding; k++)
                {
                    fputc(0x00, outptr);
                }
                fseek(inptr, (hSampleStep-1)*(3*bi.biWidth+padding), SEEK_CUR);
                //debug
                //printf("position2:%ld\n",ftell(inptr));
        }
    }


    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);
    return 0;
}


int findNearestOnePercentN(float f,float width)
{
  // float f=atof(argv[1]);
//    float width=atof(argv[2]);
    float appValue=1.0/1.0;
    float minViration=fabs(f-1.0/1.0);
    float mo=1.0;
    for(float mom=1.0;width/mom>=3.0;mom++)//define the lower edge,3*3
    {
           // printf("1/%f: %f\n",mom,1.0/mom);
            if(fabs(f-1.0/mom)<minViration)
            {
                minViration=fabs(f-1.0/mom);
                appValue=1.0/mom;
                mo=mom;
            }
    }
    //debug
   // printf("mo:%f\n",mo);
    return (int)mo;
}