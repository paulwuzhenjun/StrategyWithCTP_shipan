#include "StdAfx.h"
#include "MIndicator.h"
#include "math.h"

double round(double v, int p)
{
	v *= pow(double(10), int(p));
	v = int(v + 0.5);
	v /= pow(double(10), int(p));
	return v;
}

MIndicator::MIndicator(void)
{
}

MIndicator::~MIndicator(void)
{
}

int MIndicator::iMAOnArrayX(double* TempBuffer, int CaculatedBars, int Bars, double MA_Period, double* ExtMapBuffer) {
	double sum = 0;
	int    j, pos = 0;
	if (CaculatedBars == 0) {
		//---- zero initial bars
		for (j = 0; j < MA_Period; j++) ExtMapBuffer[j] = 0;
		//---- initial accumulation
		for (j = 0; j < (MA_Period - 1); j++, pos++)
			sum += TempBuffer[pos];
		//---- main calculation loop
		while (pos < Bars)
		{
			sum += TempBuffer[pos];
			ExtMapBuffer[pos] = (double)sum / MA_Period;
			sum -= TempBuffer[(int)(pos - MA_Period + 1)];
			pos++;
		}
	}
	else {
		pos = CaculatedBars;
		while (pos < Bars) {
			sum = ExtMapBuffer[pos - 1] * (double)MA_Period + TempBuffer[pos] - TempBuffer[(int)(pos - MA_Period)];
			ExtMapBuffer[pos] = (double)sum / MA_Period;
			pos++;
		}
	}
	return 0;
}
//价格下标从0到Bars-1, 下标为Bars-1的柱体最新柱体
int MIndicator::iEMAOnArrayX(double* TempBuffer, int CaculatedBars, int Bars, double MA_Period, double* ExtMapBuffer) {
	double pr = (double)2.0 / (MA_Period + 1.0);

	int    pos = CaculatedBars;
	if (CaculatedBars == 0)pos = 1;
	int    end = Bars - 1;
	//---- main calculation loop
	while (pos <= end)
	{
		if (pos == 1) ExtMapBuffer[pos - 1] = TempBuffer[pos - 1];
		ExtMapBuffer[pos] = TempBuffer[pos] * pr + ExtMapBuffer[pos - 1] * (1.0 - pr);
		pos++;
	}

	return 0;
}
int MIndicator::iATRX(BarRateInfo* sourceDataBuffer, int CaculatedBars, int Bars, double AtrPeriod, double* AtrMinusBuffer, double* AtrBuffer) {
	int i;

	if (CaculatedBars == 0) {
		//----
		if (Bars <= AtrPeriod) {
			for (i = 0; i < Bars; i++) AtrBuffer[i] = 0.0;
			return(0);
		}
		//---- initial zero
		for (i = 0; i < AtrPeriod; i++) AtrBuffer[i] = 0.0;
		//----
		i = 0;
		AtrMinusBuffer[0] = 0;
		while (i < Bars)
		{
			double high = sourceDataBuffer[i].high;
			double low = sourceDataBuffer[i].low;
			if (i == 0) AtrMinusBuffer[i] = high - low;
			else
			{
				double prevclose = sourceDataBuffer[i - 1].close;
				AtrMinusBuffer[i] = max(high, prevclose) - min(low, prevclose);
			}
			i++;
		}
		//----
		iMAOnArrayX(AtrMinusBuffer, CaculatedBars, Bars, AtrPeriod, AtrBuffer);
	}
	else {
		i = CaculatedBars;
		while (i < Bars)
		{
			double high = sourceDataBuffer[i].high;
			double low = sourceDataBuffer[i].low;
			double prevclose = sourceDataBuffer[i - 1].close;
			AtrMinusBuffer[i] = max(high, prevclose) - min(low, prevclose);
			i++;
		}
		//----
		iMAOnArrayX(AtrMinusBuffer, CaculatedBars, Bars, AtrPeriod, AtrBuffer);
	}

	return 0;
}

int MIndicator::iStoch(BarRateInfo* sourceDataBuffer, int HLMode, int CaculatedBars, int Bars, int KPeriod, int DPeriod, int Slowing, double* HighesBuffer, double* LowesBuffer, double* SignalBuffer, double* MainBuffer) {
	int i = 0, k = 0;
	double price = 0.0;

	if (CaculatedBars == 0) {
		for (i = 0; i < (KPeriod + DPeriod + Slowing); i++) {
			HighesBuffer[i] = 0.0;
			LowesBuffer[i] = 0.0;
			SignalBuffer[i] = 0.0;
			MainBuffer[i] = 0.0;
		}
	}
	//---- minimums counting
	if (CaculatedBars > KPeriod)i = CaculatedBars;
	else i = KPeriod - 1;
	while (i < Bars)
	{
		double min = 1000000;
		k = i - KPeriod + 1;
		while (k <= i)
		{
			if (HLMode == 0)price = sourceDataBuffer[k].close;
			else price = sourceDataBuffer[k].low;
			if (min > price) min = price;
			k++;
		}
		LowesBuffer[i] = min;
		i++;
	}
	//---- maximums counting
	if (CaculatedBars > KPeriod)i = CaculatedBars;
	else i = KPeriod - 1;
	while (i < Bars)
	{
		double max = -1000000;
		k = i - KPeriod + 1;
		while (k <= i)
		{
			if (HLMode == 0)price = sourceDataBuffer[k].close;
			else price = sourceDataBuffer[k].high;
			if (max < price) max = price;
			k++;
		}
		HighesBuffer[i] = max;
		i++;
	}
	//---- %K line
	if (CaculatedBars > (KPeriod + Slowing))i = CaculatedBars;
	else i = KPeriod + Slowing - 1;
	while (i < Bars)
	{
		double sumlow = 0.0;
		double sumhigh = 0.0;
		for (k = (i - Slowing + 1); k <= i; k++)
		{
			sumlow += sourceDataBuffer[k].close - LowesBuffer[k];
			sumhigh += HighesBuffer[k] - LowesBuffer[k];
		}
		if (sumhigh == 0.0) MainBuffer[i] = 100.0;
		else MainBuffer[i] = sumlow / sumhigh * 100.0;
		i++;
	}

	//---- signal line is simple movimg average
	iMAOnArrayX(MainBuffer, CaculatedBars, Bars, DPeriod, SignalBuffer);
	//----
	return 0;
}

int MIndicator::iKDJ(BarRateInfo* sourceDataBuffer, int CaculatedBars, int Bars, int NPeriod, int KPeriod, int DPeriod, int JPeriod, double* RSVBuffer, double* KBuffer, double* DBuffer, double* JBuffer)
{
	int i = 0, k = 0;
	double price = 0.0;

	if (Bars <= NPeriod) return -1;

	if (CaculatedBars == 0) {
		for (i = 0; i < NPeriod; i++) {
			RSVBuffer[i] = 50;
			KBuffer[i] = 50;
			DBuffer[i] = 50;
			JBuffer[i] = 50;
		}
		int startpos = NPeriod - 1;
		for (i = startpos; i < Bars; i++) {
			double minlow = 999999;
			double maxhigh = -999999;
			for (int j = i; j >= (i - NPeriod + 1); j--) {
				if (sourceDataBuffer[j].high > maxhigh)maxhigh = sourceDataBuffer[j].high;
				if (sourceDataBuffer[j].low < minlow)minlow = sourceDataBuffer[j].low;
			}
			RSVBuffer[i] = (sourceDataBuffer[i].close - minlow) / (maxhigh - minlow) * 100;
			KBuffer[i] = (double)(KPeriod - 1) / KPeriod * KBuffer[i - 1] + 1.0 / KPeriod * RSVBuffer[i];
			DBuffer[i] = (double)(DPeriod - 1) / DPeriod * DBuffer[i - 1] + 1.0 / DPeriod * KBuffer[i];
			JBuffer[i] = JPeriod * DBuffer[i] - (JPeriod - 1) * KBuffer[i];
		}
	}
	else {
		int startpos = CaculatedBars;
		for (i = startpos; i < Bars; i++) {
			double minlow = 999999;
			double maxhigh = -999999;
			for (int j = i; j >= (i - NPeriod + 1); j--) {
				if (sourceDataBuffer[j].high > maxhigh)maxhigh = sourceDataBuffer[j].high;
				if (sourceDataBuffer[j].low < minlow)minlow = sourceDataBuffer[j].low;
			}
			RSVBuffer[i] = (sourceDataBuffer[i].close - minlow) / (maxhigh - minlow) * 100;
			KBuffer[i] = (double)(KPeriod - 1) / KPeriod * KBuffer[i - 1] + 1.0 / KPeriod * RSVBuffer[i];
			DBuffer[i] = (double)(DPeriod - 1) / DPeriod * DBuffer[i - 1] + 1.0 / DPeriod * KBuffer[i];
			JBuffer[i] = JPeriod * DBuffer[i] - (JPeriod - 1) * KBuffer[i];
		}
	}

	return 0;
}

int MIndicator::iBolingerBand(double* iClose, int CaculatedBars, int Bars, int BandsPeriod, double BandsDeviations, double* MovingBuffer, double* UpperBuffer, double* LowerBuffer) {
	int    i, k;
	double deviation;
	double sum, oldval, newres;
	//----
	if (CaculatedBars == 0) {
		if (Bars <= BandsPeriod) return(0);
		//---- initial zero

		for (i = 0; i < BandsPeriod; i++)
		{
			MovingBuffer[i] = 0;
			UpperBuffer[i] = 0;
			LowerBuffer[i] = 0;
		}
		//----

		iMAOnArrayX(iClose, CaculatedBars, Bars, BandsPeriod, MovingBuffer);

		//----
		i = BandsPeriod - 1;

		while (i < Bars)
		{
			sum = 0.0;
			k = i - (BandsPeriod - 1);
			oldval = MovingBuffer[i];
			while (k <= i)
			{
				newres = iClose[k] - oldval;
				sum += newres * newres;
				k++;
			}
			deviation = BandsDeviations * sqrt((double)sum / BandsPeriod);
			UpperBuffer[i] = oldval + deviation;
			LowerBuffer[i] = oldval - deviation;
			i++;
		}
	}
	else {
		int startpos = CaculatedBars;
		iMAOnArrayX(iClose, startpos, Bars, BandsPeriod, MovingBuffer);

		i = startpos;
		while (i < Bars)
		{
			sum = 0.0;
			k = i - (BandsPeriod - 1);
			oldval = MovingBuffer[i];
			while (k <= i)
			{
				newres = iClose[k] - oldval;
				sum += newres * newres;
				k++;
			}
			deviation = BandsDeviations * sqrt((double)sum / BandsPeriod);
			UpperBuffer[i] = oldval + deviation;
			LowerBuffer[i] = oldval - deviation;
			i++;
		}
	}
	//----

	return(0);
}