/*
 * FaMe IT ECG Filter for Cardiac Booster
 * 
 * Copyright (c) 2019 FaMe IT
 * 
 * Created : Monday May 13th 2019 08:01:57 by Arjan Mels <admin@fame-it.net>
 * Modified: Monday May 13th 2019 08:01:57 by Arjan Mels <admin@fame-it.net>
 * 
 * Revisions:
 * 28-05-2019	AM	Initial Version
 */

#ifndef _FILTER_H_
#define _FILTER_H_

#include "Arduino.h"
#include "buffer.h"

// Filters generated with: http://www.schwietering.com/jayduino/filtuino/

//Band pass bessel filter order=2 alpha1=3.125E-5 alpha2=0.0625
// =3.2kHz sample rate, with Lower Corner = 0.05Hz, Upper Corner=100Hz
// using double as otherwise the filter is not stable due to the small coefficients
class FilterBandPass_0p05Hz_100Hz
{
private:
	double v[5] = {0.0};

public:
	double step(double x) //class II
	{
		v[0] = v[1];
		v[1] = v[2];
		v[2] = v[3];
		v[3] = v[4];
		//		v[4] = (1.380146926774043596e-2 * x) + (-0.64807225828556236369 * v[0]) + (2.89330302088747792766 * v[1]) + (-4.84238246141484562912 * v[2]) + (3.59715169850951177466 * v[3]); // 3.2kHz
		v[4] = (3.806945221707519911e-3 * x) + (-0.80544227575555593646 * v[0]) + (3.40228683406218523899 * v[1]) + (-5.38824590252450175143 * v[2]) + (3.79140134419695495893 * v[3]); // 6.4kHz
		return (v[0] + v[4]) - 2 * v[2];
	}
};

//Low pass bessel filter order=2 alpha1=0.0015625
// = 3.2kHz, with Corner = 5Hz
// using double as otherwise the filter is not stable due to the small coefficients
class FilterLowPass_5Hz
{
private:
	double v[3] = {0.0};

public:
	double step(double x) //class II
	{
		v[0] = v[1];
		v[1] = v[2];
		//3.2kHz		v[2] = (3.856967427123824554e-5 * x) + (-0.97860218703325974055 * v[0]) + (1.97844790833617478754 * v[1]);
		v[2] = (9.694443114521835187e-6 * x) + (-0.98924330401454885209 * v[0]) + (1.98920452624209076475 * v[1]); // 6.4kHz
		return (v[0] + v[2]) + 2 * v[1];
	}
};

// end of generated filters

class FilterAvgAndVariance
{
protected:
	double prevavg = 0.0;
	double prevvar = 0.0;
	double alpha;

public:
	FilterAvgAndVariance(double freq, double fc, double init = 0.0)
	{
		alpha = 2 * M_PI * fc / (2 * M_PI * fc + freq);
		prevavg = init;
	}

	double step(double x)
	{
		prevvar = (1 - alpha) * (prevvar + alpha * (x - prevavg) * (x - prevavg));
		prevavg = prevavg + alpha * (x - prevavg);
		return prevavg;
	}

	double getAverage()
	{
		return prevavg;
	}

	double getVariance()
	{
		return prevvar;
	}
};

class FilterMedian
{
protected:
	double prev[2];

public:
	FilterMedian(double x)
	{
		prev[1] = x;
		prev[0] = x;
	}
	double step(double x)
	{
		double res;
		if ((x >= prev[0] && x <= prev[1]) || (x <= prev[0] && x >= prev[1]))
			res = x;
		else if ((prev[0] >= prev[1] && prev[0] <= x) || (prev[0] <= prev[1] && prev[0] >= x))
			res = prev[0];
		else
			res = prev[1];
		prev[1] = prev[0];
		prev[0] = x;
		return res;
	}
};

class FilterAvgAndLocalVariance
{
protected:
	double prev;
	double prevvar;
	double prevavg;
	double fc;

public:
	FilterAvgAndLocalVariance(double fc, double init = 0.0)
	{
		this->prev = init;
		this->fc = fc;
	}
	double step(double x, double dt)
	{
		double alpha = 2 * M_PI * fc / (2 * M_PI * fc + 1.0 / dt);
		prevvar = (1 - alpha) * (prevvar + alpha * (x - prev) * (x - prev));
		prevavg = prevavg + alpha * (x - prevavg);
		prev = x;
		return prevavg;
	}

	double getAverage()
	{
		return prevavg;
	}

	double getVariance()
	{
		return prevvar;
	}
};

template <class T, size_t SHIFT>
class SigmaDelta
{
private:
	T state = 0;

public:
	T step(T in)
	{
		state += in;
		T retval = state >> SHIFT;
		state -= retval << SHIFT;
		return retval;
	}
};

double softclip(double x, double lower, double upper)
{
	double sv = (x - (upper + lower) / 2) / (upper - lower);
	if (sv > 2.0 / 3.0)
		sv = 2.0 / 3.0;
	else if (sv < -2.0 / 3.0)
		sv = -2.0 / 3.0;
	else
		sv = sv - sv * sv * sv / 3;

	return sv * (upper - lower) + (upper + lower) / 2;
}

// NOT cuurently used an dnot fully debugged
// Incoming sample rate is 200Hz
class FilterPanTompkins
{
private:
	static const int32_t FS = 200;

	CircularBuffer<int32_t, 13> in;
	CircularBuffer<int32_t, 33> lp;
	CircularBuffer<int32_t, 30> hp; // 30 is window size for moving average
	CircularBuffer<int32_t, 11> sq;

	int32_t thresholdIntegral1 = 0;
	int32_t thresholdIntegral2 = 0;
	int32_t thresholdFiltered1 = 0;
	int32_t thresholdFiltered2 = 0;
	int32_t peakIntegral = 0;
	int32_t peakFiltered = 0;

	int32_t peakSignalIntegral = 0;
	int32_t peakSignalFiltered = 0;
	int32_t peakNoiseIntegral = 0;
	int32_t peakNoiseFiltered = 0;

	int32_t sample = 0;
	int32_t lastQRS = 0;
	int32_t currentSlope = 0;
	int32_t lastSlope = 0;
	bool qrs = false;

	CircularBuffer<int32_t, 8> RR1;
	CircularBuffer<int32_t, 8> RR2;

	void updateThresholds()
	{
		thresholdIntegral1 = peakNoiseIntegral + (peakSignalIntegral - peakNoiseIntegral) / 4;
		thresholdIntegral2 = thresholdIntegral1 / 2;
		thresholdFiltered1 = peakNoiseFiltered + (peakSignalFiltered - peakNoiseFiltered) / 4;
		thresholdFiltered2 = thresholdFiltered1 / 2;
	}

public:
	int32_t step(int32_t x)
	{
		in.add(x);
		lp.next();
		hp.next();

		lp[0] = 2 * lp[-1] - lp[-2] + in[0] - 2 * in[-6] + in[-12];
		hp[0] = 32 * lp[-16] - (hp[-1] + lp[0] - lp[-32]);

		int32_t integral = hp.average();

		int32_t dt = (-hp[-4] - 2 * hp[-3] + 2 * hp[-1] + hp[0]) / 32 / 36; // divide by gains to keep number of bits to 12 before squaring
		sq.add(dt * dt);

		if (integral > thresholdIntegral1 || hp[0] > thresholdFiltered1)
		{
			peakIntegral = integral;
			peakFiltered = hp[0];
		}

		if (integral > thresholdIntegral1 && hp[0] > thresholdFiltered1)
		{
			// There's a 200ms latency. If the new peak respects this condition, we can keep testing.
			if (sample > lastQRS + FS * 200 / 1000)
			{
				currentSlope = sq.max();
				// If it respects the 200ms latency, but it doesn't respect the 360ms latency, we check the slope.
				// The squared slope is "M" shaped. So we have to check nearby samples to make sure we're really looking
				// at its peak value, rather than a low one.
				if (sample > lastQRS + FS * 360 / 1000 || currentSlope > lastSlope / 2)
				{
					qrs = true;
					lastSlope = currentSlope;
				}
				else
				{
					// T-WAVE: don't update noise peak?
					qrs = false;
				}
			}
			else // If the new peak doesn't respect the 200ms latency, it's noise. Update thresholds and move on to the next sample.
			{
				qrs = false;
			}

			if (qrs)
			{
				peakSignalIntegral = peakIntegral / 8 + peakSignalIntegral * 7 / 8;
				peakSignalFiltered = peakFiltered / 8 + peakSignalFiltered * 7 / 8;
				updateThresholds();
			}
			else
			{
				peakNoiseIntegral = peakIntegral / 8 + peakNoiseIntegral * 7 / 8;
				peakNoiseFiltered = peakFiltered / 8 + peakNoiseFiltered * 7 / 8;
				updateThresholds();

				return integral;
			}
		}
		sample++;
		return integral;
		//return hp[0] / 32.0 / 36.0;
	}
};

#endif