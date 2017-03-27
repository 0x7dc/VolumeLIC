/*----------------------------------------------------*/
/* VolumeLIC.h - Enhanced 3D Flow LIC算法             */
/*                                                    */
/* sunch_nal, 2009                                    */
/* Institute of Meteorology                           */
/* PLA University of Science and Technology           */
/*----------------------------------------------------*/

#include <vector>
using namespace std;

//**定义矢量数据-----------------------------------------------/
struct Vec3DData 
{
	float x;
	float y;
	float z;
};
//**-----------------------------------------------------------/

//**定义质点结构体---------------------------------------------/
struct Particle
{
	vector<float>  pReceiveX;
	vector<float>  pReceiveY;
	vector<float>  pReceiveZ;
	vector<float>  pRecevWgt;
};
//**-----------------------------------------------------------/

//**定义点-----------------------------------------------------/
class Point
{
public:
	float x, y, z;

	Point(float xx = 0, float yy = 0, float zz = 0)
	{
		x = xx;
		y = yy;
		z = zz;
	}

	Point operator+(Point &p)
	{
		Point pp;
		pp.x = x + p.x;
		pp.y = y + p.y;
		pp.z = z + p.z;

		return pp;
	}

	Point operator*(float &d)
	{
		Point temp;
		temp.x = x * d;
		temp.y = y * d;
		temp.z = z * d;

		return temp;
	}
};
//**-----------------------------------------------------------/

//**3D Flow LIC算法--------------------------------------------/
class VolumeLIC
{
private:
	int row;
	int col;
	int lyr;
	int DISCRETE_FILTER_SIZE;     //卷积核长度
	float* pBox;                  //盒型卷积核
	Particle***  particles;       //质点集
	Vec3DData*** pVectData;       //体数据
	float***     pEnhnTexr;       //强化纹理
	float***     pTexrCopy;       //纹理副本
	unsigned char***  pOutptTex;  //输出纹理
	unsigned char***  pNoiseTex;  //噪声纹理
public:
	VolumeLIC(int _row, int _col, int _lyr);
	~VolumeLIC();

	void  InitPartsGnNoise();
	void  GenBoxFilterFunc();
	void  ConvoltnParticle();
	void  Enhanced3DVcLine();
	void  Read3DVectorData(char* pName);
	void  StartComptVolLIC(int ADVSum);
	float TrilnInterpltDen(float*** pTexVal, Point pos);
	void  TrilnInterpltVec(Point pos, Vec3DData& vec);
	void  BilnrInterpltVec(Vec3DData** pVect, float x, float y, Vec3DData& vec);
	float BilnrInterpltDen(float** pTexVal, float x, float y);
	unsigned char*** GetOutputTexture();
};
//**-----------------------------------------------------------/