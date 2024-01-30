#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <vector>
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;
// srand((unsigned int)time(0));

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

void FillTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
	//三角形面积为0
	if (t0.y == t1.y && t0.y == t2.y)return;
	//把该三角形做成从下到上t0,t1,t2的三角形
	if (t0.y > t1.y)std::swap(t0, t1);
	if (t0.y > t2.y)std::swap(t0, t2);
	if (t1.y > t2.y)std::swap(t1, t2);
	int total_height = t2.y - t0.y; //总高度差
	//以i在y方向移动，横着画线，交点用相似三角形求
	for (int i = 0; i < total_height; i++) {
		//根据t1将三角形分割成上下两部分
		bool second_half = i > t1.y - t0.y || t1.y == t0.y; //要么是过了t1,要么是平底三角形
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;   
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0))/segment_height;  
		//计算AB的坐标
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		//还要保证此算法A在B的左边
		if (A.x > B.x) std::swap(A, B);
		//根据当前i和它与三角形交出的边界点AB一条一条划横线着色
		for (int j = A.x; j <= B.x; j++)
			image.set(j, t0.y + i, color);
	}
}


int main(int argc, char** argv) 
{
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) 
    {
        std::vector<int> face = model->face(i);

        Vec2i dots2i[3];
        for(int j=0;j<3;j++)
        {
            Vec3f dots3f1[3];
            Vec3f dots3f2[3];
            // Vec3f v0 = model->vert(face[0]);
            // Vec3f v1 = model->vert(face[1]);
            // Vec3f v2 = model->vert(face[2]);
            dots3f1[j]=model->vert(face[j]);
            dots3f2[j]=model->vert(face[(j+1)%3]);
            dots2i[j].x=(int)((dots3f1[j].x+1.)*width/2);
            dots2i[j].y=(int)((dots3f1[j].y+1.)*height/2);
            dots2i[(j+1)%3].x=(int)((dots3f2[j].x+1.)*width/2);
            dots2i[(j+1)%3].y=(int)((dots3f2[j].y+1.)*height/2);
            // DrawLine(dots2i[j].x, dots2i[j].y, dots2i[(j+1)%3].x, dots2i[(j+1)%3].y, image, white);
        // int x0 = (v0.x+1.)*width/2.;//让图像完整呈现在屏幕中间，视口变换（viewport transform）
        // int y0 = (v0.y+1.)*height/2.;
        // int x1 = (v1.x+1.)*width/2.;
        // int y1 = (v1.y+1.)*height/2.;
        }
        
        FillTriangle(dots2i[0],dots2i[1],dots2i[2],image,TGAColor(rand()%255,rand()%255,rand()%255,rand()%255));
    }
    
    image.flip_vertically(); //左下角是原点
    image.write_tga_file("output.tga");
    delete model;
    model=nullptr;
    return 0;
}