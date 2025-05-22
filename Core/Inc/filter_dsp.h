/*
 * filter_dsp.h
 *
 *  Created on: May 16, 2025
 *      Author: sasha
 */

#ifndef INC_FILTER_DSP_H_
#define INC_FILTER_DSP_H_
#include <arm_math.h>


#endif /* INC_FILTER_DSP_H_ */
#define NUM_STAGES 1
enum filterType{
	LP,
	HP,
	BP,
	NUM_FILTER_TYPES,
};
typedef struct {
	arm_biquad_cascade_df2T_instance_f32 arm_inst_left;
	arm_biquad_cascade_df2T_instance_f32 arm_inst_right;
	float32_t pCoeffs[NUM_STAGES*5];
	float32_t pStateL[NUM_STAGES*2];
	float32_t pStateR[NUM_STAGES*2];
	float32_t audioFreq;
}filterUnit;

void initFilter(filterUnit* filterObjectLR, int audioFreq);
void doFilter(filterUnit *, float32_t *srcL,
			  float32_t *srcR, float32_t *dstL,
			  float32_t *dstR, int num_samples);
void compute_lowpass_coeffs(filterUnit*, float cutoff, float res);
void compute_coeffs(filterUnit*, float cutoff, float res, enum filterType type);
