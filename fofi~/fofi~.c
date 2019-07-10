#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "m_pd.h"

static t_class *fofi_tilde_class;

const int NUM_MIDINOTES = 128;

typedef struct pe_last_values {
	t_sample last_out[2];
	t_sample last_in[2];
} t_pe_last_values;

float peaking_equalizer(t_sample in, t_sample *last_in , t_sample *last_out, float f_centerFrequency,
											 float f_gain, float f_peakWidth){
	// TODO: User proper variable names
	float fs = 48000; //Sampling Rate TODO: get actual sample rate
	float wc = 2 * M_PI * (f_centerFrequency / fs);
	float mu = pow(10, (float)  (f_gain));
	float kq =4 / (1 + mu) * tan( wc / (2 * f_peakWidth));
	float Cpk = (1 + kq*mu)/(1+kq);
	float b1 = (-2*cos(wc))/(1+kq*mu);
	float b2 = (1-kq*mu)/(1+kq*mu);
	float a1 = (-2*cos(wc))/(1+kq);
	float a2 = (1-kq)/(1+kq);

	float out = (Cpk * in) + (b1 * last_in[0]) + (b2 * last_in[1]) - ( a1 * last_out[0] ) - ( a2 * last_out[1] );

	last_out[1] = last_out[0];
	last_out[0] = out;
	last_in[1] = last_in[0];
	last_in[0] = in;
	return out;

}

float bandpass(t_sample in, t_sample *last_in , t_sample *last_out, float f_centerFrequency,
							 float f_gain){

	// TODO: User proper variable names
	float fs = 48000.0; //Sampling Rate TODO: get actual sample rate
	float x = in;

	float w0 = 2.0*M_PI*f_centerFrequency/fs;
	float s = sin(w0);
	float alpha = s/(2.0*f_gain);

	float b0 = f_gain*alpha;
	float b1 = 0.0;
	float b2 = -s/2.0;
	float a0 = 1.0 + alpha;
	float a1 = -2.0*cos(w0);
	float a2 = 1.0 - alpha;

	float out = b0/a0 * x + b1/a0 * last_in[0] + b2/a0 * last_in[1] - a1/a0 * last_out[0] - a2/a0 * last_out[1];

	last_out[1] = last_out[0];
	last_out[0] = out;
	last_in[1] = last_in[0];
	last_in[0] = in;
	return out;
}

typedef struct _fofi_tilde {
	t_object  x_obj;

	t_atom notes[NUM_MIDINOTES];
	t_pe_last_values last_values[NUM_MIDINOTES];

	t_sample f_peakWidth;
	t_sample f_gain;

	t_sample f;

	t_inlet *x_in2;
	t_inlet *x_in3;

	t_outlet*x_out;

}	t_fofi_tilde;


t_int *fofi_tilde_perform(t_int *w) {
	/* the first element is a pointer to the dataspace of this object */
	t_fofi_tilde *x = (t_fofi_tilde *)(w[1]);

	t_sample  *in =    (t_sample *)(w[2]);
	t_sample  *out =    (t_sample *)(w[3]);

	int num_samples = (int)(w[4]);

	/* Make sure we don't break everything with 0 values*/
	x->f_gain = x->f_gain == 0 ? 0.001 : x->f_gain;
	x->f_peakWidth = x->f_peakWidth == 0 ? 0.001 : x->f_peakWidth ;

	for (int n = 0; n < NUM_MIDINOTES ; n++){
		if (atom_getfloat(&x->notes[n]) <= 0)
			continue;

		// TODO: Use a lookup table for freq/midi mapping
		float freq = 440.0*pow(2,(((float) n)-69)/12);

		for (	int i = 0; i < num_samples ; i++ ){
			out[i] = peaking_equalizer(in[i], x->last_values[n].last_in, x->last_values[n].last_out, freq, x->f_gain, x->f_peakWidth );
			/* out[i] = bandpass(in[i], x->last_values[n].last_in, x->last_values[n].last_out, freq, x->f_gain ); */
		}
	}

	/* return a pointer to the dataspace for the next dsp-object */
	return (w+5);
}


void fofi_tilde_list(t_fofi_tilde *x, t_symbol *s, int argc, t_atom *argv) {
	if (argc != NUM_MIDINOTES)
		pd_error(x, "Received Invalid note list");

	for (int i = 0 ; i < NUM_MIDINOTES; i++)
		x->notes[i]=argv[i];
}

void fofi_tilde_dsp(t_fofi_tilde *x, t_signal **sp) {
	dsp_add(fofi_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void fofi_tilde_free(t_fofi_tilde *x) {
	inlet_free(x->x_in2);
	inlet_free(x->x_in3);
	outlet_free(x->x_out);
}

void *fofi_tilde_new(t_floatarg f) {
	t_fofi_tilde *x = (t_fofi_tilde *)pd_new(fofi_tilde_class);

	x->x_in2 = floatinlet_new (&x->x_obj, &x->f_gain);
	x->x_in3 = floatinlet_new (&x->x_obj, &x->f_peakWidth);

	x->x_out = outlet_new(&x->x_obj, &s_signal);


	/* for (int i = 0; i < NUM_MIDINOTES ; i++){ */
	/*	x->last_values[i] = {{0,0} {0,0}}; */
	/* } */

	return (void *)x;
}

void fofi_tilde_setup(void) {
	fofi_tilde_class = class_new(gensym("fofi~"),
															 (t_newmethod)fofi_tilde_new,
															 (t_method)fofi_tilde_free,
															 sizeof(t_fofi_tilde),
															 CLASS_DEFAULT,
															 A_DEFFLOAT, 0);

	class_addmethod(fofi_tilde_class,
									(t_method)fofi_tilde_dsp, gensym("dsp"), A_CANT, 0);

	class_addlist(fofi_tilde_class, (t_method)fofi_tilde_list);

	CLASS_MAINSIGNALIN(fofi_tilde_class, t_fofi_tilde, f);
}
