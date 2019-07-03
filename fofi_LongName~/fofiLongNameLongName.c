/*
 * HOWTO write an External for Pure data
 * (c) 2001-2006 IOhannes m zm�lnig zmoelnig[AT]iem.at
 *
 * this is the source-code for the fourth example in the HOWTO
 * it creates a simple dsp-object:
 * 2 input signals are mixed into 1 output signal
 * the mixing-factor can be set via the 3rd inlet
 *
 * for legal issues please see the file LICENSE.txt
 */


/**
 * include the interface to Pd
 */
#include "m_pd.h"
#include <math.h>
#include <string.h>

/**
 * define a new "class"
 */
static t_class *fofiLongNameLongName_tilde_class;


/**
 * this is the dataspace of our new object
 * the first element is the mandatory "t_object"
 * "f" is a dummy and is used to be able to send floats AS signals.
 */
typedef struct _fofiLongNameLongName_tilde {
	t_object  x_obj;

	t_sample f_centerFrequency;
	t_sample f_centerFrequency_2;
	t_sample f_centerFrequency_3;
	t_sample f_centerFrequency_4;
	t_sample f_peakWidth;
	t_sample f_gain;

	t_sample f;


	t_inlet *x_in2;
	t_inlet *x_in3;
	t_inlet *x_in4;
	t_inlet *x_in5;
	t_inlet *x_in6;
	t_inlet *x_in7;
	t_inlet *x_in8;

	t_outlet *x_out1;
	t_outlet *x_out2;

}	t_fofiLongNameLongName_tilde;


// check pointers and stuff, how that shit works

float peakingEqualizer( t_sample (*in), t_sample (*last_in)[2] , t_sample (*last_out)[2], float f_centerFrequency,
	 										float f_gain, float f_peakWidth )
{

	float fs = 41000; //Sampling Rate TODO: get actual sample rate
	float wc = 2 * M_PI * (f_centerFrequency / fs);
	float mu = 2.0;
	if ( f_gain < 0.6 ) {
		mu = pow(10, (float)  (f_gain));
	}
	float kq = 4 / (1 + mu) * tan( wc / (2));;
	if ( f_peakWidth != 0) {
		kq = 4 / (1 + mu) * tan( wc / (2 * f_peakWidth));
	}

	float Cpk = (1 + kq*mu)/(1+kq);
	float b1 = (-2*cos(wc))/(1+kq*mu);
	float b2 = (1-kq*mu)/(1+kq*mu);
	float a1 = (-2*cos(wc))/(1+kq);
	float a2 = (1-kq)/(1+kq);

	float out = 0;
	out = (Cpk * *in) + (b1 * *last_in[0]) + (b2 * *last_in[1]) - ( a1 * *last_out[0] ) - ( a2 * *last_out[1] );
			//
			// // update last_out/in
	*last_out[1] = *last_out[0];
	*last_out[0] = out;
			//
			// last_out2[1] = last_out2[0];
			// last_out2[0] = out2[i];
			//
	*last_in[1] = *last_in[0];
	*last_in[0] = *in;
			//
			// last_in2[1] = last_in2[0];
			// last_in2[0] = In2[i];
	return out;

}

/**
 * this is the core of the object
 * this perform-routine is called for each signal block
 * the name of this function is arbitrary and is registered to Pd in the
 * fofiLongNameLongName_tilde_dsp() function, each time the DSP is turned on
 *
 * the argument to this function is just a pointer within an array
 * we have to know for ourselves how many elements inthis array are
 * reserved for us (hint: we declare the number of used elements in the
 * fofiLongNameLongName_tilde_dsp() at registration
 *
 * since all elements are of type "t_int" we have to cast them to whatever
 * we think is apropriate; "apropriate" is how we registered this function
 * in fofiLongNameLongName_tilde_dsp()
 */
t_int *fofiLongNameLongName_tilde_perform(t_int *w)
{
	static t_sample last_out1[2] = {0, 0}; // last two samples for output 1
	static t_sample last_out2[2] = {0, 0}; // last two samples for output 2
	static t_sample last_in1[2] = {0, 0};  // last two samples for input 1
	static t_sample last_in2[2] = {0, 0};  // last two samples for input 2


	/* NOTE: Apparently inlets and outlets can share the same addresses which leads to issues if
		 the input is not stored in an extra variable https://forum.pdpatchrepo.info/topic/10293/multiple-signal-outlets-in-external/8
	*/
	/* static int NUM_SAMPLES = 64; // use fixed size for now */
	/* t_sample  in1[64], in2[64]; */

	/* memcpy(&in1, (t_sample *)(w[2]), NUM_SAMPLES*sizeof(t_sample)); */
	/* memcpy(&in2, (t_sample *)(w[3]), NUM_SAMPLES*sizeof(t_sample)); */
	t_sample  *in1 =    (t_sample *)(w[2]);
	t_sample  *in2 =    (t_sample *)(w[3]);

	t_sample	*In1 = (t_sample *)(w[2]);
	t_sample  *In2 = (t_sample *)(w[3]);

	/* the first element is a pointer to the dataspace of this object */
	t_fofiLongNameLongName_tilde *x = (t_fofiLongNameLongName_tilde *)(w[1]);


	/* here comes the signalblock that will hold the output signal */
	t_sample  *out1 =    (t_sample *)(w[4]);
	t_sample  *out2 =    (t_sample *)(w[5]);

	int num_samples = (int)(w[6]);

	// TODO: Give these variables proper names OR add comments to clarify what these are!

	//sometimes the values explode, need to do some errorhandling

	float fs = 41000; //Sampling Rate TODO: get actual sample rate
	float wc = 2 * M_PI * (x->f_centerFrequency / fs);
	float mu = 2.0;
	if ( x->f_gain < 0.6 ) {
		mu = pow(10, (float)  (x->f_gain));
	}
	float kq = 4 / (1 + mu) * tan( wc / (2));;
	if ( x->f_peakWidth != 0) {
		kq = 4 / (1 + mu) * tan( wc / (2 * x->f_peakWidth));
	}

	float Cpk = (1 + kq*mu)/(1+kq);
	float b1 = (-2*cos(wc))/(1+kq*mu);
	float b2 = (1-kq*mu)/(1+kq*mu);
	float a1 = (-2*cos(wc))/(1+kq);
	float a2 = (1-kq)/(1+kq);

	// Set  samples




	for (	int i = 0; i < num_samples ; i++ ) {
		In1[i] = in1[i];
		In2[i] = in2[i];

		out1[i] = peakingEqualizer(&In1[i], &last_in1, &last_out1, x->f_centerFrequency, x->f_gain, x->f_peakWidth );
		out2[i] = peakingEqualizer(&In2[i], &last_in2, &last_out2, x->f_centerFrequency, x->f_gain, x->f_peakWidth );

		// // save input value here
		// // sometimes they seem to be overwritten with the output
		// In1[i] = in1[i];
		// In2[i] = in2[i];
		//
		// out1[i] = (Cpk * In1[i]) + (b1 * last_in1[0]) + (b2 * last_in1[1]) - a1*last_out1[0] - a2*last_out1[1];
		// out2[i] = (Cpk * In2[i]) + (b1 * last_in2[0]) + (b2 * last_in2[1]) - a1*last_out2[0] - a2*last_out2[1];
		//
		// // update last_out/in
		// last_out1[1] = last_out1[0];
		// last_out1[0] = out1[i];
		//
		// last_out2[1] = last_out2[0];
		// last_out2[0] = out2[i];
		//
		// last_in1[1] = last_in1[0];
		// last_in1[0] = In1[i];
		//
		// last_in2[1] = last_in2[0];
		// last_in2[0] = In2[i];
	}



	/* return a pointer to the dataspace for the next dsp-object */
	return (w+7);
}


/**
 * register a special perform-routine at the dsp-engine
 * this function gets called whenever the DSP is turned ON
 * the name of this function is registered in fofiLongNameLongName_tilde_setup()
 */
void fofiLongNameLongName_tilde_dsp(t_fofiLongNameLongName_tilde *x, t_signal **sp)
{
	dsp_add(fofiLongNameLongName_tilde_perform, 9, x,
					sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[0]->s_n);
}

/**
 * this is the "destructor" of the class;
 * it allows us to free dynamically allocated ressources
 */
void fofiLongNameLongName_tilde_free(t_fofiLongNameLongName_tilde *x)
{
	/* free any ressources associated with the given inlet */
	inlet_free(x->x_in2);
	inlet_free(x->x_in3);
	inlet_free(x->x_in4);
	inlet_free(x->x_in5);
	inlet_free(x->x_in6);
	inlet_free(x->x_in7);
	inlet_free(x->x_in8);

	/* free any ressources associated with the given outlet */
	outlet_free(x->x_out1);
	outlet_free(x->x_out2);
}

/**
 * this is the "constructor" of the class
 * the argument is the initial mixing-factor
 */
void *fofiLongNameLongName_tilde_new(t_floatarg f)
{
	t_fofiLongNameLongName_tilde *x = (t_fofiLongNameLongName_tilde *)pd_new(fofiLongNameLongName_tilde_class);

	/* create a new signal-inlet */
	x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

	/* create a new passive inlet for the gain */
	x->x_in3 = floatinlet_new (&x->x_obj, &x->f_centerFrequency);
	x->x_in4 = floatinlet_new (&x->x_obj, &x->f_centerFrequency_2);
	x->x_in5 = floatinlet_new (&x->x_obj, &x->f_centerFrequency_3);
	x->x_in6 = floatinlet_new (&x->x_obj, &x->f_centerFrequency_4);
	x->x_in7 = floatinlet_new (&x->x_obj, &x->f_gain);
	x->x_in8 = floatinlet_new (&x->x_obj, &x->f_peakWidth);

	/* create a new signal-outlet */
	x->x_out1 = outlet_new(&x->x_obj, &s_signal);
	x->x_out2 = outlet_new(&x->x_obj, &s_signal);

	return (void *)x;
}


/**
 * define the function-space of the class
 * within a single-object external the name of this function is very special
 */
void fofiLongNameLongName_tilde_setup(void) {
	fofiLongNameLongName_tilde_class = class_new(gensym("fofiLongNameLongName~"),
															 (t_newmethod)fofiLongNameLongName_tilde_new,
															 (t_method)fofiLongNameLongName_tilde_free,
															 sizeof(t_fofiLongNameLongName_tilde),
															 CLASS_DEFAULT,
															 A_DEFFLOAT, 0);

	/* whenever the audio-engine is turned on, the "fofiLongNameLongName_tilde_dsp()"
	 * function will get called
	 */
	class_addmethod(fofiLongNameLongName_tilde_class,
									(t_method)fofiLongNameLongName_tilde_dsp, gensym("dsp"), A_CANT, 0);

	/* if no signal is connected to the first inlet, we can as well
	 * connect a number box to it and use it as "signal"
	 */
	CLASS_MAINSIGNALIN(fofiLongNameLongName_tilde_class, t_fofiLongNameLongName_tilde, f);
}
