#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "utils.h"
#include "image.h"
#include "readpic.h"

/*
  SoftImage pic to ppm converter
  Also supports the raw format
*/

int main(int argc,char **argv)
{
  int i,j;
  int raw = FALSE, flip = FALSE, alpha = FALSE, p7;
  unsigned char r,g,b,a;
  Image *img;
  uint32 pixel;

  /* Check the arguments */
  if (argc < 2) {
    fprintf(stderr,"Usage: %s [-a -h -f] picfilename\n",argv[0]);
    fprintf(stderr,"       -r create a raw file instead\n");
    fprintf(stderr,"       -f flip the image about horizontal axis\n");
    fprintf(stderr,"       -a include the alpha channel\n");
    fprintf(stderr,"       -p7 PAM P7 format(alpha included)\n");
    exit(-1);
  }

  /* Parse the command line options */
  for (i=1;i<argc-1;i++) {
    if (strcmp(argv[i],"-r") == 0)
      raw = TRUE;
      if (strcmp(argv[i],"-f") == 0)
         flip = TRUE;
      if (strcmp(argv[i],"-a") == 0)
         alpha = TRUE;
      if (strcmp(argv[i],"-p7") == 0) {
        alpha = TRUE;
        p7 = TRUE;
      }
  }

  /* Read the image */
  if ((img = FF_PIC_load(argv[argc-1])) == NULL)
    exit(-1);

  /* Write the PPM header */
  if (!raw) {
    if(p7) {
      printf("P7\n");
      printf("WIDTH %d\n", img->width);
      printf("HEIGHT %d\n", img->height);
      printf("DEPTH 4\n");
      printf("MAXVAL 255\n");
      printf("TUPLTYPE RGB_ALPHA\n");
      printf("ENDHDR\n");
    } else {
      printf("P6\n");
      printf("%d %d\n",img->width,img->height);
      printf("255\n");
    }
  }

  /* Write the binary data */
  for (j=0;j<img->height;j++) {
    for (i=0;i<img->width;i++) {

      if (flip)
        pixel = img->bitmap[(img->height-j-1)*img->width+i];
      else
        pixel = img->bitmap[j*img->width+i];

      a = (pixel & 0xff000000) >> 24;
      b = (pixel & 0x00ff0000) >> 16;
      g = (pixel & 0x0000ff00) >> 8;
      r = (pixel & 0x000000ff);

      if (alpha) {
        fwrite(&pixel,sizeof(uint32),1,stdout);
      } else {
        fwrite(&r,sizeof(char),1,stdout);
        fwrite(&g,sizeof(char),1,stdout);
        fwrite(&b,sizeof(char),1,stdout);
      }
    }
  }

}

