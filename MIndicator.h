#pragma once
#include "MyStruct.h"

class MIndicator
{
public:
	MIndicator(void);
	~MIndicator(void);

	int iMAOnArrayX(double* TempBuffer, int CaculatedBars, int Bars, double MA_Period, double* ExtMapBuffer);
	int iEMAOnArrayX(double* TempBuffer, int CaculatedBars, int Bars, double MA_Period, double* ExtMapBuffer);
	int iATRX(BarRateInfo* sourceDataBuffer, int CaculatedBars, int Bars, double AtrPeriod, double* AtrMinusBuffer, double* AtrBuffer);

	int iKDJ(BarRateInfo* sourceDataBuffer, int CaculatedBars, int Bars, int NPeriod, int KPeriod, int DPeriod, int JPeriod, double* RSVBuffer, double* KBuffer, double* DBuffer, double* JBuffer);
	int iStoch(BarRateInfo* sourceDataBuffer, int HLMode, int CaculatedBars, int Bars, int KPeriod, int DPeriod, int Slowing, double* HighesBuffer, double* LowesBuffer, double* SignalBuffer, double* MainBuffer);

	int iBolingerBand(double* iClose, int CaculatedBars, int Bars, int BandsPeriod, double BandsDeviations, double* MovingBuffer, double* UpperBuffer, double* LowerBuffer);
};