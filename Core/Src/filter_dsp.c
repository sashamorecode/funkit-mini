/*
 * filter_dsp.c
 *
 *  Created on: May 16, 2025
 *      Author: sasha
 */


#include "main.h"
#include "filter_dsp.h"








void initFilter(filterUnit* filterObject, int audioFreq){
	filterObject->audioFreq = (float) audioFreq;
	arm_biquad_cascade_df2T_init_f32(&filterObject->arm_inst_left,NUM_STAGES, filterObject->pCoeffs, &filterObject->pStateL[0]);
	arm_biquad_cascade_df2T_init_f32(&filterObject->arm_inst_right, NUM_STAGES, filterObject->pCoeffs, &filterObject->pStateR[0]);
	compute_lowpass_coeffs(filterObject, 10000.f, 1.f);
}


/**
  * @brief TIM1 Initialization Function
  * @param [in] *srcL: input bufffer of size SAMPLES/2
  * @param [in] *srcR: input bufffer of size SAMPLES/2
  * @param [out] *dstL: pointer that will be set to the internal output buffer
  * @param [out] *dstR: pointer that will be set to the internal output buffer
  * @retval None
  */

void doFilter(filterUnit* filterObjectLR, float32_t *srcL, float32_t *srcR, float32_t *dstL, float32_t *dstR, int num_samples){

	arm_biquad_cascade_df2T_f32(&(filterObjectLR->arm_inst_left), srcL, dstR, num_samples);
	arm_biquad_cascade_df2T_f32(&(filterObjectLR->arm_inst_right), srcR, dstR, num_samples);
}

/*
void doFilter(filterUnit* filterObjectLR, float32_t *srcL, float32_t *srcR, float32_t *dstL, float32_t *dstR, int num_samples){

}
*/
void compute_lowpass_coeffs(filterUnit* filterObjectLR, float cutoff, float res){
	const float omega = 2.f * PI * (cutoff/filterObjectLR->audioFreq);
	const float cs = cos(omega);
	const float alpha = sin(omega)/(cs*res);
	const float a0 = 1 + alpha;
	const float b0 = (1 - cs) / 2;
	const float b1 = (1 - cs);
	const float b2 = b0;
	const float a1 = -2 * cs;
	const float a2 = 1 - alpha;

	filterObjectLR->pCoeffs[0] = b0/a0; //b0
	filterObjectLR->pCoeffs[1] = b1/a0; //b1
 	filterObjectLR->pCoeffs[2] = b2/a0;//b2
 	filterObjectLR->pCoeffs[3] = -a1/a0;//a1
 	filterObjectLR->pCoeffs[4] = -a2/a0;//a2
}

void compute_highpass_coeffs(filterUnit* filterObjectLR, float cutoff, float res){
	const float omega = 2.f * PI * (cutoff/filterObjectLR->audioFreq);
	const float cs = cos(omega);
	const float alpha = sin(omega)/(cs*res);
	const float a0 = 1 + alpha;
	const float b0 = (1 - cs) / 2;
	const float b1 = (1 - cs);
	const float b2 = b0;
	const float a1 = -2 * cs;
	const float a2 = 1 - alpha;

	filterObjectLR->pCoeffs[0] = b0/a0; //b0
	filterObjectLR->pCoeffs[1] = b1/a0; //b1
 	filterObjectLR->pCoeffs[2] = b2/a0;//b2
 	filterObjectLR->pCoeffs[3] = -a1/a0;//a1
 	filterObjectLR->pCoeffs[4] = -a2/a0;//a2
}


void compute_coeffs(filterUnit* filterObject, float cutoff, float res, enum filterType type){
	switch(type){
	case LP:
		compute_lowpass_coeffs(filterObject, cutoff, res);
		break;
	case HP:
		compute_highpass_coeffs(filterObject, cutoff, res);
		break;
	case BP:
		break;
	case NUM_EFFECTS:
		break;
	}
}


