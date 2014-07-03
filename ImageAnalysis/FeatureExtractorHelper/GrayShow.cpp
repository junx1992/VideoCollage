//////////////////////////////////////////////////////////////////////
// GrayShow.cpp: implementation of the CGrayShow class.
// �Ҷȹ��־����㷨ʵ��
//
//////////////////////////////////////////////////////////////////////
#include "windows.h"
#include "GrayShow.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrayShow::CGrayShow()
{
	PMatrixRD = NULL;	//45�ȷ����ϵĻҶȹ��־���
	PMatrixLD = NULL;	//135�ȷ����ϵĻҶȹ��־���
	PMatrixV = NULL;		//90�ȷ����ϵĻҶȹ��־���
	PMatrixH = NULL;		//0�ȷ����ϵĻҶȹ��־���
	ImageArray = NULL;	//ԭʼ��ͼ����������

	m_tOriPixelArray = NULL;//ԭʼλͼ����
	m_tResPixelArray = NULL;//�ο�λͼ����

	distance = 5;
	FilterWindowWidth = 16;
	GrayLayerNum = 8; //��ʼ����Ϊ8���ҶȲ㣬�����޸�
	//�������־����ΪGrayLayerNum��GrayLayerNum
	int i;
	PMatrixH = new int* [GrayLayerNum];
	PMatrixLD= new int* [GrayLayerNum];
	PMatrixRD= new int* [GrayLayerNum];
	PMatrixV = new int*[GrayLayerNum];
	for(i=0; i<GrayLayerNum; i++)
	{
		PMatrixH[i] = new int[GrayLayerNum];
		PMatrixLD[i]= new int[GrayLayerNum];
		PMatrixRD[i]= new int[GrayLayerNum];
		PMatrixV[i] = new int[GrayLayerNum];
	}

}

CGrayShow::~CGrayShow()
{
	int i;
	
	for(i=0; i<GrayLayerNum; i++)
	{
		delete [] PMatrixH[i];
		delete [] PMatrixLD[i];
		delete [] PMatrixRD[i];
		delete [] PMatrixV[i];
	}
	delete [] PMatrixH;
	delete [] PMatrixLD;
	delete [] PMatrixRD;
	delete [] PMatrixV;

	if(m_tOriPixelArray !=NULL)
	{
		for(i=0; i<ImageHeight; i++)
		{
			delete [] m_tOriPixelArray[i];
			delete [] m_tResPixelArray[i];
			delete [] ImageArray[i];
		}
		delete [] m_tOriPixelArray;
		delete [] m_tResPixelArray;
		delete [] ImageArray;
	}
}


//added by f-xyuan
void CGrayShow::LoadImage(const BYTE* pbImage, long width, long height, long stride)
{
	int i,j;
	//����Ƿ�Ϊ�գ���Ϊ����Ҫ�ͷ��ڴ�
	if(m_tOriPixelArray !=NULL)
	{
		for(i=0; i<ImageHeight; i++)
		{
			delete m_tOriPixelArray[i];
			delete m_tResPixelArray[i];
			delete ImageArray[i];
		}
	}

	ImageHeight = height;
	ImageWidth  = width;
	const BYTE	*colorTable = pbImage;

	m_tOriPixelArray  = new RGBQUAD*[ImageHeight];
	m_tResPixelArray  = new RGBQUAD*[ImageHeight];
	ImageArray = new BYTE*[ImageHeight];
	for(int l=0 ; l<ImageHeight; l++)
	{
		m_tOriPixelArray[l] = new RGBQUAD[ImageWidth];
		m_tResPixelArray[l] = new RGBQUAD[ImageWidth];
		ImageArray[l] = new BYTE[ImageWidth];
	}

	for(i=ImageHeight-1; i>=0; i--)
	{
		int count = 0;
		for(j=0; j<ImageWidth; j++)
		{
			m_tOriPixelArray[i][j].rgbBlue =colorTable[count++];
			m_tOriPixelArray[i][j].rgbGreen=colorTable[count++];
			m_tOriPixelArray[i][j].rgbRed  =colorTable[count++];
			m_tOriPixelArray[i][j].rgbReserved = 0;
			m_tResPixelArray[i][j]=m_tOriPixelArray[i][j];
			ImageArray[i][j] = (BYTE)(((unsigned int)m_tOriPixelArray[i][j].rgbBlue
								+(unsigned int)m_tOriPixelArray[i][j].rgbGreen
								+(unsigned int)m_tOriPixelArray[i][j].rgbRed)/3);
		}
		colorTable += stride;
		//the buffer returned by DirectShow may not contain the filled bytes
		//so remove the following sentence.
//		count += (4-(ImageWidth*byteBitCount)%4)%4;
	}
}

////////////////////////////////////////////////////////////////////////////////////
//�������ܣ�������������
//������FeatureEnergy������
//      FeatureEntropy����
//      FeatureInertiaQuadrature�����Ծ�
//      FeatureCorrelation�����
//      FeatureLocalCalm���ֲ�ƽ��
//      pMatrix�����־���
//      dim�����־����ά��
///////////////////////////////////////////////////////////////////////////////////
void CGrayShow::ComputeFeature(double &FeatureEnergy, double &FeatureEntropy, 
							   double &FeatureInertiaQuadrature, double &FeatureCorrelation, 
							   double &FeatureLocalCalm, int** pMatrix, int dim)
{
	int i,j;
	double **pdMatrix;
	pdMatrix = new double*[dim];
	for(i=0; i<dim; i++)
		pdMatrix[i] = new double[dim];

	int total = 0;
	for(i=0; i<dim; i++)
	{
		for(j=0; j<dim; j++)
		{
			total += pMatrix[i][j];
		}
	}

	for(i=0; i<dim; i++)
	{
		for(j=0; j<dim; j++)
		{
			pdMatrix[i][j] = (double)pMatrix[i][j]/(double)total;
		}
	}
	
	FeatureEnergy = 0.0;
	FeatureEntropy = 0.0;
	FeatureInertiaQuadrature = 0.0;
	FeatureLocalCalm = 0.0;


	//�����������ء����Ծء��ֲ�ƽ��
	for(i=0; i<dim; i++)
	{
		for(j=0; j<dim; j++)
		{
			//����
			FeatureEnergy += pdMatrix[i][j]*pdMatrix[i][j];

			//��
			if(pdMatrix[i][j]>1e-12)
			{
				FeatureEntropy -= pdMatrix[i][j]*log(pdMatrix[i][j]);
			}

			//���Ծ�
			FeatureInertiaQuadrature += (double)(i-j)*(double)(i-j)*pdMatrix[i][j];

			//�ֲ�ƽ��
			FeatureLocalCalm += pdMatrix[i][j]/(1+(double)(i-j)*(double)(i-j));
		}
	}

	//����ux
	double ux = 0.0;
	double localtotal = 0.0;
	for(i=0; i<dim; i++)
	{
		localtotal = 0.0;
		for(j=0; j<dim; j++)
		{
			localtotal += pdMatrix[i][j];
		}
		ux += (double)i * localtotal;
	}

	//����uy
	double uy = 0.0;
	for(j=0; j<dim; j++)
	{
		localtotal = 0.0;
		for(i=0; i<dim; i++)
		{
			localtotal += pdMatrix[i][j];
		}
		uy += (double)j * localtotal;
	}

	//����sigmax
	double sigmax = 0.0;
	for(i=0; i<dim; i++)
	{
		localtotal = 0.0;
		for(j=0; j<dim; j++)
		{
			localtotal += pdMatrix[i][j];
		}
		sigmax += (double)(i-ux) * (double)(i-ux) * localtotal;
	}

	//����sigmay
	double sigmay = 0.0;
	for(j=0; j<dim; j++)
	{
		localtotal = 0.0;
		for(i=0; i<dim; i++)
		{
			localtotal += pdMatrix[i][j];
		}
		sigmay += (double)(j-uy) * (double)(j-uy) * localtotal;
	}

	//�������
	FeatureCorrelation = 0.0;
	for(i=0; i<dim; i++)
	{
		for(j=0; j<dim; j++)
		{
			FeatureCorrelation += (double)(i-ux) * (double)(j-uy) * pdMatrix[i][j];
		}
	}
	FeatureCorrelation /= sigmax;
	FeatureCorrelation /= sigmay;

	for(i=0; i<dim; i++)
		delete [] pdMatrix[i];
	delete [] pdMatrix;
}

/////////////////////////////////////////////////////////////////////////////////////
//���ܣ����㹲�־���
//������LocalImage����������ľֲ���������ͼ��
//      LocalImageWidth���ֲ�����������
////////////////////////////////////////////////////////////////////////////////////
void CGrayShow::ComputeMatrix(BYTE **LocalImage, int LocalImageWidth)
{
	BYTE **NewImage;
	NewImage = new BYTE*[LocalImageWidth];

	int i,j;
	for(i=0; i<LocalImageWidth; i++)
		NewImage[i] = new BYTE[LocalImageWidth];

	for(i=0; i<LocalImageWidth; i++)
	{
		for(j=0; j<LocalImageWidth; j++)
		{
			//�ֳ�GrayLayerNum���Ҷȼ�
			NewImage[i][j] = LocalImage[i][j] / (256/GrayLayerNum);
		}
	}

	for(i=0; i<GrayLayerNum; i++)
	{
		//
		for(j=0; j<GrayLayerNum; j++)
		{
			PMatrixH[i][j]  = 0;
			PMatrixLD[i][j] = 0;
			PMatrixRD[i][j] = 0;
			PMatrixV[i][j]  = 0;
		}
	}

	//����0�ȵĻҶȹ�����
	for(i=0; i<LocalImageWidth; i++)
	{
		for(j=0; j<LocalImageWidth-distance; j++)
		{
			PMatrixH[(unsigned int)NewImage[i][j]][(unsigned int)NewImage[i][j+distance]] += 1;
			PMatrixH[(unsigned int)NewImage[i][j+distance]][(unsigned int)NewImage[i][j]] += 1;
		}
	}

	//����90�ȵĻҶȹ�����
	for(i=0; i<LocalImageWidth-distance; i++)
	{
		for(j=0; j<LocalImageWidth; j++)
		{
			PMatrixV[(unsigned int)NewImage[i][j]][(unsigned int)NewImage[i+distance][j]] += 1;
			PMatrixV[(unsigned int)NewImage[i+distance][j]][(unsigned int)NewImage[i][j]] += 1;
		}
	}

	//����135�ȵĻҶȹ�����
	for(i=0; i<LocalImageWidth-distance; i++)
	{
		for(j=0; j<LocalImageWidth-distance; j++)
		{
			int newi, newj;
			newi = i+distance;
			newj = j+distance;
			PMatrixLD[(unsigned int)NewImage[i][j]][(unsigned int)NewImage[newi][newj]] += 1;
			PMatrixLD[(unsigned int)NewImage[newi][newj]][(unsigned int)NewImage[i][j]] += 1;
		}
	}

	//����45�ȵĻҶȹ�����
	for(i=distance; i<LocalImageWidth; i++)
	{
		for(j=0; j<LocalImageWidth-distance; j++)
		{
			int newi, newj;
			newi = i-distance;
			newj = j+distance;
			PMatrixRD[(unsigned int)NewImage[i][j]][(unsigned int)NewImage[newi][newj]] += 1;
			PMatrixRD[(unsigned int)NewImage[newi][newj]][(unsigned int)NewImage[i][j]] += 1;
		}
	}

	for(i=0; i<LocalImageWidth; i++)
		delete [] NewImage[i];
	delete [] NewImage;
}
