/*----------------------------------------------------*/
/* mainApp.cpp - VolumeLIC 3D Flow Visualization      */
/*                                                    */
/* sunch_nal, 2009                                    */
/* Institute of Meteorology                           */
/* PLA University of Science and Technology           */
/*----------------------------------------------------*/

#include <iostream.h>
#include <fstream>
#include "VolumeLIC.h"

//**���峣��-------------------------------------------/
#define ROW     128   //��
#define COL     128   //��
#define LYR     128   //��
//**---------------------------------------------------/

//**������--------------------------------------------------------------------------------------------------------/
int main(int argc, char** argv) 
{	
	char* pName   = "data\\tornado0.dat";
	int   advSum  = 12;      //ƽ������
	VolumeLIC volLIC(ROW, COL, LYR);
	volLIC.Read3DVectorData(pName);
	volLIC.StartComptVolLIC(advSum);
	
	cout<<"��ʼʸ����ǿ��..."<<endl;
	for (int i = 0; i < 8; i++)
	{
		volLIC.ConvoltnParticle();
		volLIC.Enhanced3DVcLine();
		cout<<"��"<<i<<"��ʸ����ǿ��"<<endl;
	}

    volLIC.ConvoltnParticle();
	cout<<"ʸ����ǿ������..."<<endl;
 
	cout<<"��ʼ�������..."<<endl;
	unsigned char*** pOutputTex = volLIC.GetOutputTexture();
	int size = ROW * COL * LYR;
	unsigned char* pData = new unsigned char[size];
	unsigned char* pKeep = pData;
	FILE* fp;
	fp = fopen("data\\output.dat", "wb");
	for (int k = 0; k < LYR; k++)
	{
		for (int i = 0; i < ROW; i++)
		{
			for (int j =0; j < COL; j++)
			{
				*pData = pOutputTex[k][i][j];
				pData++;
			}
		}//for
	}//for

	pData = pKeep;
	fwrite(pData, sizeof(unsigned char), size, fp);
	fclose(fp);
	delete[] pData;
	pData = NULL;
	pKeep = NULL;
	cout<<"�����������..."<<endl;

    return 0;
}
//**--------------------------------------------------------------------------------------------------------------/