#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <vector>
#include <iostream>
#include <limits>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = nullptr;
const int width  = 800;
const int height = 800;
const Vec3f LightDir(0,0,-1);//定义光照方向
// srand((unsigned int)time(0));

void DrawLine(int x0,int y0,int x1,int y1,TGAImage& image,TGAColor color)
{
    if(abs(y1-y0) <= abs(x1-x0))//斜率小于1
    {
        for(int x=x0;abs(x-x1) != 0;x+= (x1-x0)/abs(x1-x0))
        {   
            float ratio=(float)(y1-y0)/(x1-x0);
            int y=(int)(y0+ratio*(x-x0));
            image.set(x,y,color);
        }
    }
    else//斜率大于1
    {
        for(int y=y0;abs(y-y1) != 0;y+= (y1-y0)/abs(y1-y0))
        {   
            float ratio=(float)(x1-x0)/(y1-y0);
            int x=(int)(x0+ratio*(y-y0));
            image.set(x,y,color);
        }
    }
}

Vec3f barycentric(Vec3f v1, Vec3f v2, Vec3f v3, Vec3f p)
{
	//分母等于0的情况
	if ((-(v1.x - v2.x) * (v3.y - v2.y) + (v1.y - v2.y) * (v3.x - v2.x)) == 0)
		return Vec3f(1, 0, 0);
	if (-(v2.x - v3.x) * (v1.y - v3.y) + (v2.y - v3.y) * (v1.x - v3.x) == 0)
		return Vec3f(1, 0, 0);//这里应该是（0，1，0）？
	float alpha = (-(p.x - v2.x) * (v3.y - v2.y) + (p.y - v2.y) * (v3.x - v2.x)) / (-(v1.x - v2.x) * (v3.y - v2.y) + (v1.y - v2.y) * (v3.x - v2.x));
	float beta = (-(p.x - v3.x) * (v1.y - v3.y) + (p.y - v3.y) * (v1.x - v3.x)) / (-(v2.x - v3.x) * (v1.y - v3.y) + (v2.y - v3.y) * (v1.x - v3.x));
	float gamma = 1 - alpha - beta;
 
	return Vec3f(alpha, beta, gamma);
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

//经过视口变换后，对每个三角形所覆盖的包围盒内的像素进行深度插值
void TriAngleShader(Vec3f* coors,float* zbuffer,TGAImage& image,TGAColor color)
{
    //定义包围盒和夹值
    Vec2f boxmin(std::numeric_limits<float>::max(),std::numeric_limits<float>::max());//左下角
    Vec2f boxmax(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max());//右上角
    Vec2f clamp(image.get_width()-1,image.get_height()-1);

    //确定包围盒
    for(int i=0;i<3;i++)
        for(int j=0;j<2;j++)
        {
            boxmin[j]=std::max(0.0f,std::min(coors[i][j],boxmin[j]));//找到包围盒中最小的x,y
            boxmax[j]=std::min(clamp[j],std::max(coors[i][j],boxmax[j]));//找到包围盒中最大的x,y
        }
    // boxmin.x=std::min(std::min(vertex1.x,vertex2.x),vertex3.x);
    // boxmin.y=std::min(std::min(vertex1.y,vertex2.y),vertex3.y);
    // boxmax.x=std::max(std::max(vertex1.x,vertex2.x),vertex3.x);
    // boxmax.y=std::max(std::max(vertex1.y,vertex2.y),vertex3.y);

    //开始遍历包围盒中的点
    Vec3f VertexInBox;
    for(VertexInBox.x=boxmin.x;VertexInBox.x <= boxmax.x;VertexInBox.x++)
        for(VertexInBox.y=boxmin.y;VertexInBox.y <= boxmax.y;VertexInBox.y++)
            {
                Vec3f bary_coor=barycentric(coors[0],coors[1],coors[2],VertexInBox);
                if(bary_coor.x < 0 || bary_coor.y <0 || bary_coor.z <0 ) continue;
                //对在包围盒里面的点进行深度插值
                VertexInBox.z = bary_coor.x*coors[0].z+bary_coor.y*coors[1].z+bary_coor.z*coors[2].z;
                //更新z-buffer
                if(zbuffer[int(VertexInBox.x+VertexInBox.y*width)]<VertexInBox.z) 
                {
                    zbuffer[int(VertexInBox.x+VertexInBox.y*width)]=VertexInBox.z;
                    image.set(VertexInBox.x,VertexInBox.y,color);
                }
            }
}

//将三维空间的坐标转化到视口中
Vec3f world2viewport(Vec3f p)
{
    return Vec3f(int((p.x+1.)/2*width+.5),int((p.y+1.)/2*width+.5),p.z);//这里考虑了像素中心点的位置
}


int main(int argc, char** argv) 
{
    if (2==argc) 
    {
        model = new Model(argv[1]);
    } 
    else 
    {
        model = new Model("obj/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    //定义zbuffer
    float* zbuffer=new float[width*height];
    for(int i=0;i<width*height;i++) zbuffer[i]= -std::numeric_limits<float>::max();
    for(int i=0;i<model->nfaces();i++)
    {
        std::vector<int> face=model->face(i);
        Vec3f screen_coor[3]={world2viewport(model->vert(face[0])),world2viewport(model->vert(face[1])),world2viewport(model->vert(face[2]))};
        Vec3f world_coor[3]={model->vert(face[0]),model->vert(face[1]),model->vert(face[2])};
        //求这个面的法向量
        Vec3f n=cross(world_coor[2]-world_coor[0],world_coor[1]-world_coor[0]).normalize();
        //求这个面的光照强度
        float intensity = n*LightDir;
        if(intensity>0) //去掉背面
        
        TriAngleShader(screen_coor,zbuffer,image,TGAColor(intensity*255,intensity*255,intensity*255,255));
    }
    image.flip_vertically(); //左下角是原点
    image.write_tga_file("output.tga");
    delete model;
    model=nullptr;
    return 0;
}
