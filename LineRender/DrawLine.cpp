#include <iostream>
#include "tgaimage.h"
// using namespace std;

const TGAColor white(255,255,255,255);
const TGAColor red(255,0,0,255);
const TGAColor green(0,255,0,255);
const TGAColor blue(0,0,255,255);

void DrawLine(int x0,int y0,int x1,int y1,TGAImage& image,TGAColor color)
{
    if(abs(y1-y0) <= abs(x1-x0))
    {
        for(int x=x0;abs(x-x1) != 0;x+= (x1-x0)/abs(x1-x0))
        {   
            float ratio=(float)(y1-y0)/(x1-x0);
            int y=(int)(y0+ratio*(x-x0));
            image.set(x,y,color);
        }
    }
    else
    {
        for(int y=y0;abs(y-y1) != 0;y+= (y1-y0)/abs(y1-y0))
        {   
            float ratio=(float)(x1-x0)/(y1-y0);
            int x=(int)(x0+ratio*(y-y0));
            image.set(x,y,color);
        }
    }
}

int main()
{
    TGAImage image(200,200,TGAImage::Format::RGB);
    DrawLine(200,800,0,0,image,red);
    image.flip_vertically();
    image.write_tga_file("Line.tga");
}