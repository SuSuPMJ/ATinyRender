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
const Vec3f LightDir(0,0,1);//定义光照方向
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
        Vec2i screen_coor[3];
        Vec3f world_coor[3];
        for(int j=0; j<3 ;j++)
        {
            world_coor[j]=model->vert(face[j]);
            screen_coor[j]=Vec2i((world_coor[j].x+1.0)/2*width,(world_coor[j].y+1.0)/2*height);
        }
        //求面的法向量
        Vec3f n=(world_coor[1]-world_coor[0])^(world_coor[2]-world_coor[1]);
        n.normalize();//正则化
        float LightIntensity= n * LightDir;//这里简单定义每个面接收到的光照强度为1
        if(LightIntensity > 0) FillTriangle(screen_coor[0],screen_coor[1],screen_coor[2],image,TGAColor(LightIntensity*255,LightIntensity*255,LightIntensity*255,255));
    }
    
    image.flip_vertically(); //左下角是原点
    image.write_tga_file("output.tga");
    delete model;
    model=nullptr;
    return 0;
}