/*----------------------------------------------------*/
/* VolumeLIC.cpp - 3D Flow LIC�㷨                    */
/*                                                    */
/* sunch_nal, 2009                                    */
/* Institute of Meteorology                           */
/* PLA University of Science and Technology           */
/*----------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include "VolumeLIC.h"

//**���� | ��������----------------------------------------------------------------------------/
VolumeLIC::VolumeLIC(int _row, int _col, int _lyr)
{
	cout<<"��ʼ��ʼ��..."<<endl;
	row = _row;
	col = _col;
	lyr = _lyr;
    
	//����洢�Ŀռ�
	DISCRETE_FILTER_SIZE = 100;
	pBox = new float[DISCRETE_FILTER_SIZE];
	
	particles = new Particle**[lyr];
	pVectData = new Vec3DData**[lyr];
	pEnhnTexr = new float**[lyr];
	pTexrCopy = new float**[lyr];
	pOutptTex = new unsigned char**[lyr];
	pNoiseTex = new unsigned char**[lyr];
	for (int i=0; i < lyr; i++)
	{
		particles[i] = new Particle*[row];
		pVectData[i] = new Vec3DData*[row];
		pEnhnTexr[i] = new float*[row];
		pTexrCopy[i] = new float*[row];
		pOutptTex[i] = new unsigned char*[row];
		pNoiseTex[i] = new unsigned char*[row];
		for (int j=0; j < row; j++)
		{
			particles[i][j] = new Particle[col];
			pVectData[i][j] = new Vec3DData[col];
			pEnhnTexr[i][j] = new float[col];
			pTexrCopy[i][j] = new float[col];
			pOutptTex[i][j] = new unsigned char[col];
			pNoiseTex[i][j] = new unsigned char[col];
		}
	}//for

	InitPartsGnNoise();
	GenBoxFilterFunc();

	cout<<"��ʼ�����..."<<endl;
}

VolumeLIC::~VolumeLIC()
{
	cout<<"��ʼ�ͷ��ڴ�..."<<endl;
	int i = 0, j = 0, k = 0;
	//1 �ջ���ָ��
	for (i = 0; i < lyr; i++)
	{
		for (j = 0; j < row; j++)
		{
			delete[] particles[i][j];
			delete[] pVectData[i][j];
			delete[] pEnhnTexr[i][j];
			delete[] pTexrCopy[i][j];
			delete[] pOutptTex[i][j];
			delete[] pNoiseTex[i][j];
		}
	}
	//2 �ջ���ָ��
	for (k = 0; k < lyr; k++)
	{
		delete[] particles[k];
		delete[] pVectData[k];
		delete[] pEnhnTexr[k];
		delete[] pTexrCopy[k];
		delete[] pOutptTex[k];
		delete[] pNoiseTex[k];
	}
	//3 �ջز�ָ��
	delete[] particles;
	particles = NULL;
	delete[] pVectData;
	pVectData = NULL;
	delete[] pEnhnTexr;
	pEnhnTexr = NULL;
	delete[] pTexrCopy;
	pTexrCopy = NULL;
	delete[] pOutptTex;
	pOutptTex = NULL;
	delete[] pNoiseTex;
	pNoiseTex = NULL;
	cout<<"�ͷ��ڴ����..."<<endl;
}
//**-------------------------------------------------------------------------------------------/

//**��3Dʸ��������-----------------------------------------------------------------------------/
void VolumeLIC::Read3DVectorData(char* pName)
{
	cout<<"��ʼ��ȡ����..."<<endl;
    
	//������
	float* pData = new float[row * col * lyr * 3];
    FILE* fp;
	fp = fopen(pName, "rb");
	fread(pData, sizeof(float), row*col*lyr*3, fp);

	//��������
    float x = 0, y = 0, z = 0;
	float vcMag = 0, scale = 0;
	for (int k = 0; k < lyr; k++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				x = *pData++;
				y = *pData++;
				z = *pData++;
				vcMag = sqrt(x*x + y*y + z*z);//
				scale = (vcMag==0) ? 0 : 1.0f/vcMag;
				pVectData[k][i][j].x = x * scale;
				pVectData[k][i][j].y = y * scale;
				pVectData[k][i][j].z = z * scale;/**/
			}
		}//for
	}//for

    pData = NULL;
	delete[] pData;

	fclose(fp);
	fp = NULL;

	cout<<"���ݶ�ȡ����..."<<endl;
}
//**-------------------------------------------------------------------------------------------/

//**��ʼ���ʵ㼯�����ɰ�����-------------------------------------------------------------------/
void VolumeLIC::InitPartsGnNoise()
{
	int r = 0;
	
	for (int k = 0; k < lyr; k++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				r = rand();
			    r = ( (r & 0xff) + ( (r & 0xff00) >> 8 ) ) & 0xff;
                //r = r > 127 ? 255 : 0;
				
				pNoiseTex[k][i][j] = (unsigned char)r;
				pEnhnTexr[k][i][j] = (unsigned char)r;
				particles[k][i][j].pReceiveX.empty();
				particles[k][i][j].pReceiveY.empty();
				particles[k][i][j].pReceiveZ.empty();
				particles[k][i][j].pRecevWgt.empty();
			}
		}
	}//for
}
//**-------------------------------------------------------------------------------------------/

//**�����LUTsiz = DISCRETE_FILTER_SIZE--------------------------------------------------------/
void VolumeLIC::GenBoxFilterFunc()
{
	for(int i = 0; i < DISCRETE_FILTER_SIZE; i++)
	{
		pBox[i] = (float)i;
	}
}
//**-------------------------------------------------------------------------------------------/

//*˫�Բ�ֵ�Ҷ�ֵ------------------------------------------------------------------------------/
float VolumeLIC::BilnrInterpltDen(float** pTexVal, float x, float y)
{
	float den = 0;
	float verPt0 = 0, verPt1 = 0, verPt2 = 0, verPt3 = 0;
	int pt0_x = (int)x,    pt0_y = (int)y;
	int pt1_x = pt0_x + 1, pt1_y = pt0_y;
	int pt2_x = pt0_x + 1, pt2_y = pt0_y + 1;
	int pt3_x = pt0_x,     pt3_y = pt0_y + 1;

	if (pt0_x < 0 || pt0_x > col - 2 || pt0_y < 0 || pt0_y > row - 2)
	{
// 		pt0_x = pt0_x < 0 ? 0 : pt0_x;
// 		pt0_y = pt0_y < 0 ? 0 : pt0_y;
// 		pt0_x = pt0_x > col - 1 ? col - 1 : pt0_x;
// 		pt0_y = pt0_y > row - 1 ? row - 1 : pt0_y;
// 		den = pTexVal[pt0_y][pt0_x];
		den = 128;
	}
	else
	{
		verPt0 = pTexVal[pt0_y][pt0_x];
		verPt1 = pTexVal[pt1_y][pt1_x];
		verPt2 = pTexVal[pt2_y][pt2_x];
		verPt3 = pTexVal[pt3_y][pt3_x];

		float differX = x - pt0_x;
		float differY = y - pt0_y;

		den = (verPt1 - verPt0) * differX + (verPt3 - verPt0) * differY
			+ (verPt2 + verPt0 - verPt1 - verPt3) * differX * differY + verPt0;
	}

	return den;
}
//**-------------------------------------------------------------------------------------------/

//**˫���Բ�ֵʸ��ֵ---------------------------------------------------------------------------/
void VolumeLIC::BilnrInterpltVec(Vec3DData** pVect, float x, float y, Vec3DData& vec)
{
	int pt0_x = (int)x,    pt0_y = (int)y;
	int pt1_x = pt0_x + 1, pt1_y = pt0_y;
	int pt2_x = pt0_x + 1, pt2_y = pt0_y + 1;
	int pt3_x = pt0_x,     pt3_y = pt0_y + 1;
	float differX = 0, differY = 0;
	Vec3DData verPt0, verPt1, verPt2, verPt3;

	if (pt0_x < 0 || pt0_x > col - 2 || pt0_y < 0 || pt0_y > row - 2)
	{
		pt0_x = pt0_x < 0 ? 0 : pt0_x;
		pt0_y = pt0_y < 0 ? 0 : pt0_y;
		pt0_x = pt0_x > col - 1 ? col - 1 : pt0_x;
		pt0_y = pt0_y > row - 1 ? row - 1 : pt0_y;
		vec.x = pVect[pt0_y][pt0_x].x;
		vec.y = pVect[pt0_y][pt0_x].y;
		vec.z = pVect[pt0_y][pt0_x].z;
	}
	else
	{
		differX = x - pt0_x;
		differY = y - pt0_y;

		verPt0.x = pVect[pt0_y][pt0_x].x;
		verPt1.x = pVect[pt1_y][pt1_x].x;
		verPt2.x = pVect[pt2_y][pt2_x].x;
		verPt3.x = pVect[pt3_y][pt3_x].x;
		vec.x = (verPt1.x - verPt0.x) * differX + (verPt3.x - verPt0.x) * differY
			  + (verPt2.x + verPt0.x - verPt1.x - verPt3.x) * differX * differY + verPt0.x;

		verPt0.y = pVect[pt0_y][pt0_x].y;
		verPt1.y = pVect[pt1_y][pt1_x].y;
		verPt2.y = pVect[pt2_y][pt2_x].y;
		verPt3.y = pVect[pt3_y][pt3_x].y;
		vec.y = (verPt1.y - verPt0.y) * differX + (verPt3.y - verPt0.y) * differY
			  + (verPt2.y + verPt0.y - verPt1.y - verPt3.y) * differX * differY + verPt0.y;
        
		pt0_x = (int)(x + 0.5f);
		pt0_y = (int)(y + 0.5f);
		vec.z = pVect[pt0_y][pt0_x].z;
	}
}
//**-------------------------------------------------------------------------------------------/

//**�����Բ�ֵ�Ҷ�ֵ---------------------------------------------------------------------------/
float VolumeLIC::TrilnInterpltDen(float*** pTexVal, Point pos)
{
	int i = (int)(pos.y);  //��
	int j = (int)(pos.x);  //��
	int k = (int)(pos.z);  //��
	float den = 0;

	if (i < 0 || i > row - 2 || j < 0 || j > col - 2 || k < 0 || k > lyr - 2)
	{
		i = i < 0 ? 0 : i;
		j = j < 0 ? 0 : j;
		k = k < 0 ? 0 : k;
		i = i > row - 1 ? row - 1 : i;
		j = j > col - 1 ? col - 1 : j;
		k = k > lyr - 1 ? lyr - 1 : k;
		den = pTexVal[k][i][j];
	}
	else
	{
		float x = pos.x - j;
		float y = pos.y - i;
		float z = pos.z - k;

		den = pTexVal[k][i+1][j] * x * y * (1-z)
			+ pTexVal[k][i+1][j+1] * y * (1-x) * (1-z)
			+ pTexVal[k][i][j+1] * z * y * (1-x)
			+ pTexVal[k][i][j] * x * y * z
			+ pTexVal[k+1][i+1][j] * (1-z) * (1-y) * x
			+ pTexVal[k+1][i+1][j+1] * (1-x) * (1-y) * (1-z)
			+ pTexVal[k+1][i][j+1] * (1-x) * (1-y) * z
			+ pTexVal[k+1][i][j] * (1-y) * x * z;
	}//else

	return den;
}
//**-------------------------------------------------------------------------------------------/

//**�����Բ�ֵʸ��ֵ---------------------------------------------------------------------------/
void VolumeLIC::TrilnInterpltVec(Point pos, Vec3DData& vec)
{
	int i = (int)(pos.y);  //��
	int j = (int)(pos.x);  //��
	int k = (int)(pos.z);  //��

 	if (i < 0 || i > row - 2 || j < 0 || j > col - 2 || k < 0 || k > lyr - 2)
 	{
		i = i < 0 ? 0 : i;
		j = j < 0 ? 0 : j;
		k = k < 0 ? 0 : k;
		i = i > row - 1 ? row - 1 : i;
		j = j > col - 1 ? col - 1 : j;
		k = k > lyr - 1 ? lyr - 1 : k;
		vec.x = pVectData[k][i][j].x;
		vec.y = pVectData[k][i][j].y;
		vec.z = pVectData[k][i][j].z;
	}
	else
	{
		float x = pos.x - j;
		float y = pos.y - i;
		float z = pos.z - k;

		vec.x = pVectData[k][i+1][j].x * x * y * (1-z)
			  + pVectData[k][i+1][j+1].x * y * (1-x) * (1-z)
			  + pVectData[k][i][j+1].x * z * y * (1-x)
			  + pVectData[k][i][j].x * x * y * z
			  + pVectData[k+1][i+1][j].x * (1-z) * (1-y) * x
			  + pVectData[k+1][i+1][j+1].x * (1-x) * (1-y) * (1-z)
			  + pVectData[k+1][i][j+1].x * (1-x) * (1-y) * z
			  + pVectData[k+1][i][j].x * (1-y) * x * z;

		vec.y = pVectData[k][i+1][j].y * x * y * (1-z)
			  + pVectData[k][i+1][j+1].y * y * (1-x) * (1-z)
			  + pVectData[k][i][j+1].y * z * y * (1-x)
			  + pVectData[k][i][j].y * x * y * z
			  + pVectData[k+1][i+1][j].y * (1-z) * (1-y) * x
			  + pVectData[k+1][i+1][j+1].y * (1-x) * (1-y) * (1-z)
			  + pVectData[k+1][i][j+1].y * (1-x) * (1-y) * z
			  + pVectData[k+1][i][j].y * (1-y) * x * z;

		vec.z = pVectData[k][i+1][j].z * x * y * (1-z)
			  + pVectData[k][i+1][j+1].z * y * (1-x) * (1-z)
			  + pVectData[k][i][j+1].z * z * y * (1-x)
			  + pVectData[k][i][j].z * x * y * z
			  + pVectData[k+1][i+1][j].z * (1-z) * (1-y) * x
			  + pVectData[k+1][i+1][j+1].z * (1-x) * (1-y) * (1-z)
			  + pVectData[k+1][i][j+1].z * (1-x) * (1-y) * z
			  + pVectData[k+1][i][j].z * (1-y) * x * z;
	}
}
//**-------------------------------------------------------------------------------------------/

//**��ʼ����3D LIC-----------------------------------------------------------------------------/
void VolumeLIC::StartComptVolLIC(int ADVSum)
{
	int       flag   = 0;
	int       advDir = 0;
	int       advNum = 0;       //�ʵ���ƽ������
	int       layrID = 0;
	float     adpStp = 0;       //����Ӧ׷�ٲ���
	float     segLen = 0;       //�ʵ�ÿ��ƽ������
	float     tmpLen = 0;
	float     prvLen = 0;
	float     curLen = 0;       //�ʵ�ƽ���ܾ���
	float     smpWgt = 0;       //��ǰ�������Ȩ��ֵ
	float     W_ACUM = 0;       //�˺�������ֵ
	float     w_acum[2];        //�ۼ�Ȩ��֮��
	float     pos_x  = 0, pos_y = 0, pos_z = 0;
	float     krnLen = ADVSum;  //����˳�
	float     len2ID = (DISCRETE_FILTER_SIZE - 1) / krnLen;

	Point     curtPt;        //�ʵ㵱ǰλ������
	Vec3DData vec;           //�ʵ㵱ǰλ��ʸ��ֵ

	cout<<"��ʼ�ʵ�ƽ��..."<<endl;
    ADVSum = 3 * ADVSum;
	for (int k = 0; k < lyr; k++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				w_acum[0] = w_acum[1] = 0.0f;
				//�������ڷ���׷
				for (advDir = 0; advDir < 2; advDir++)
				{
					advNum = 0;
					curLen = 0;
					curtPt.x = j + 0.5f; //��
					curtPt.y = i + 0.5f; //��
					curtPt.z = k; //�� + 0.5f

					while (curLen < krnLen && advNum < ADVSum)
					{
						//��ֵ����ǰ���ٶ�ֵ
						//TrilnInterpltVec(curtPt, vec);
						layrID = (int)(curtPt.z + 0.5f);//k/**/;
						BilnrInterpltVec(pVectData[layrID], curtPt.x, curtPt.y, vec);
						
						if (fabs(vec.x) < 0.0000001 && fabs(vec.y) < 0.0000001 && fabs(vec.z) < 0.0000001)
						{
							//��Ϊ�ؼ��㼴һ�������Ϊ����������ʱ,��ֹ׷��
							break;
						}

						vec.x = (advDir == 0) ? vec.x : -vec.x;
						vec.y = (advDir == 0) ? vec.y : -vec.y;
						vec.z = (advDir == 0) ? vec.z : -vec.z;
						
						segLen = 999.0f;
						segLen = (vec.x < 0) ? ((int)curtPt.x - curtPt.x) / vec.x : segLen;
						segLen = (vec.x > 0) ? ((int)(int(curtPt.x) + 1.5f) - curtPt.x) / vec.x : segLen;
						tmpLen = 999.0f;
						tmpLen = (vec.y < 0) ? ((int)curtPt.y - curtPt.y) / vec.y : tmpLen;
						tmpLen = (vec.y > 0) ? ((int)(int(curtPt.y) + 1.5f) - curtPt.y) / vec.y : tmpLen;
						segLen = (segLen < tmpLen) ? segLen : tmpLen;
						
						prvLen  = curLen;
						curLen += segLen;//�������߳���
						segLen += 0.005f;//����׷�پ���,��ֹ�������
						segLen  = (curLen > krnLen) ? ( (curLen = krnLen) - prvLen ) : segLen;

						//����������һ��λ��
						Point nextPt;
						nextPt.x = curtPt.x + vec.x * segLen;
						nextPt.y = curtPt.y + vec.y * segLen;
						nextPt.z = curtPt.z + vec.z * segLen;//k;// * 2

						//��¼׷�ٵ�λ��
						pos_x = (nextPt.x + curtPt.x) * 0.5f;
						pos_y = (nextPt.y + curtPt.y) * 0.5f;
						pos_z = (nextPt.z + curtPt.z) * 0.5f;
						particles[k][i][j].pReceiveX.push_back(pos_x);
						particles[k][i][j].pReceiveY.push_back(pos_y);
						particles[k][i][j].pReceiveZ.push_back(pos_z);

						//���㹱��ֵ
						W_ACUM = pBox[ int(curLen * len2ID) ];
						smpWgt = W_ACUM - w_acum[advDir];
						particles[k][i][j].pRecevWgt.push_back(smpWgt);
						w_acum[advDir] = W_ACUM;

						curtPt = nextPt;
						advNum++;

						if (  nextPt.x < 0.0 || nextPt.x > col-1
						   || nextPt.y < 0.0 || nextPt.y > row-1
						   || nextPt.z < 0.0 || nextPt.z > lyr-1
						   )
						{
							//׷�ٳ��������ݱ߽�,��ֹ׷��
							break;
						}
					}//while
				}//for ׷�ٷ���
			}//for ��
		}//for ��

		int per = (int)(10 * k / lyr);
		if (flag != per && per%2 == 0)
		{
			flag = per;
			cout<<"�����:"<<per * 10<<"%"<<endl;
		}
		else
		{
			int a = 0;
		}
	}//for ��

    cout<<"�����"<<"100%"<<endl;
	cout<<"�ʵ�ƽ������..."<<endl;
}
//**-------------------------------------------------------------------------------------------/

//**�ʵ���-----------------------------------------------------------------------------------/
void VolumeLIC::ConvoltnParticle()
{
	int    recID  = 0; //�ʵ��ʶ
	int    recNum = 0; //�����ʵ���Ŀ
	float  smp_x  = 0, smp_y = 0, smp_z = 0;
	int    smp_k  = 0;
	float  texVal = 0; //����ĻҶ�ֵ
	float  smpWgt = 0; //��ǰ�������Ȩ��ֵ
	float  texSum = 0; //������Ҷ�ֵ֮��
	float  wgtSum = 0; //������Ȩ��֮��

	for (int k = 0; k < lyr; k++)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				texSum = 0;
				wgtSum = 0;
				recID  = 0;
				recNum = particles[k][i][j].pReceiveX.size();
				while (recID < recNum)
				{
					smp_x  = particles[k][i][j].pReceiveX[recID];
					smp_y  = particles[k][i][j].pReceiveY[recID];
					smp_k  = (int)(particles[k][i][j].pReceiveZ[recID] + 0.5f);//particles[k][i][j].pReceiveZ[recID];//
					smpWgt = particles[k][i][j].pRecevWgt[recID];

					smp_k = smp_k < 0 ? 0 : smp_k;
					smp_k = smp_k > lyr - 1 ? lyr - 1 : smp_k;
					texVal = BilnrInterpltDen(pEnhnTexr[smp_k], smp_x, smp_y);
					//texVal = TrilnInterpltDen(pEnhnTexr, pt);

					texSum += texVal * smpWgt;
					wgtSum += smpWgt;

					recID++;
				}//while

				wgtSum = (wgtSum < 1.0f) ? 1.0f : wgtSum; 
				texVal = texSum / wgtSum + 0.5f;
				texVal = (texVal <   0.0f) ?   0.0f : texVal;
				texVal = (texVal > 255.0f) ? 255.0f : texVal; 
				pOutptTex[k][i][j] = (unsigned char)texVal;
			}//for
		}//for
	}//for
}
//**-------------------------------------------------------------------------------------------/

//**3Dʸ����ǿ��-------------------------------------------------------------------------------/
void VolumeLIC::Enhanced3DVcLine()
{
	int i  = 0, j = 0, k = 0;

	//1 ��������������ע��
	float compVal = 0;

	for (k = 0; k < lyr; k++)
	{
		for (i = 0; i < row; i++)
		{
			for (j  = 0; j < col; j++)
			{
				compVal = pOutptTex[k][i][j] * 0.9f + pNoiseTex[k][i][j] * 0.1f;
				pEnhnTexr[k][i][j] = compVal;
				pTexrCopy[k][i][j] = compVal;
			}
		}//for
	}//for

	//2 һά��ͨ�˲�
	float texVal = 0;
	float vec_x = 0, vec_y = 0;
	float newVx = 0, newVy = 0;
	float dx = 0, dy = 0;
	float len = 1.5f;
	float smp1_x = 0, smp1_y = 0;
	float smp2_x = 0, smp2_y = 0;
	float smpVal_1 = 0, smpVal_2 = 0;

	for (k = 0; k < lyr; k++)
	{
		for (i = 0; i < row; i++)
		{
			for (j = 0; j < col; j++)
			{
				vec_x = pVectData[k][i][j].x;
				vec_y = pVectData[k][i][j].y;
				newVx = (-1) * vec_y;
				newVy = vec_x;
				dx = len * newVx;
				dy = len * newVy;
				smp1_x = j + dx;
				smp1_y = i + dy;
				smp2_x = j - dx;
				smp2_y = i - dy;

				smpVal_1 = BilnrInterpltDen(pTexrCopy[k], smp1_x, smp1_y);
				smpVal_2 = BilnrInterpltDen(pTexrCopy[k], smp2_x, smp2_y);

				texVal = 2 * pEnhnTexr[k][i][j] - (smpVal_1 + smpVal_2) * 0.5f;;
				texVal = texVal > 255 ? 255 : texVal;
				texVal = texVal <   0 ?   0 : texVal;
				pEnhnTexr[k][i][j] = texVal;
			}
		}//for
	}//for
}
//**-------------------------------------------------------------------------------------------/

//**�����������-------------------------------------------------------------------------------/
unsigned char*** VolumeLIC::GetOutputTexture()
{
	return pOutptTex;
}
//**-------------------------------------------------------------------------------------------/

