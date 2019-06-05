/*
 * HOWTO write an External for Pure data
 * (c) 2001-2006 IOhannes m zmölnig zmoelnig[AT]iem.at
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


/**
 * define a new "class"
 */
static t_class *fofi_tilde_class;


/**
 * this is the dataspace of our new object
 * the first element is the mandatory "t_object"
 * "f" is a dummy and is used to be able to send floats AS signals.
 */
typedef struct _fofi_tilde {
	t_object  x_obj;

	t_sample f_centerFrequency;
	t_sample f_peakWidth;
	t_sample f_gain;

	t_sample f;

	t_inlet *x_in2;
	t_inlet *x_in3;
	t_inlet *x_in4;
	t_inlet *x_in5;

	t_outlet*x_out1;
	t_outlet*x_out2;
}
	t_fofi_tilde;


/**
 * this is the core of the object
 * this perform-routine is called for each signal block
 * the name of this function is arbitrary and is registered to Pd in the
 * fofi_tilde_dsp() function, each time the DSP is turned on
 *
 * the argument to this function is just a pointer within an array
 * we have to know for ourselves how many elements inthis array are
 * reserved for us (hint: we declare the number of used elements in the
 * fofi_tilde_dsp() at registration
 *
 * since all elements are of type "t_int" we have to cast them to whatever
 * we think is apropriate; "apropriate" is how we registered this function
 * in fofi_tilde_dsp()
 */
t_int *fofi_tilde_perform(t_int *w)
{

	static t_sample last_out1[2]; // last two samples for output 1
	static t_sample last_out2[2]; // last two samples for output 2
	static t_sample last_in1[2];  // last two samples for input 1
	static t_sample last_in2[2];  // last two samples for input 2

	/* the first element is a pointer to the dataspace of this object */
	t_fofi_tilde *x = (t_fofi_tilde *)(w[1]);

	/* here is a pointer to the t_sample arrays that hold the 2 input signals */
	t_sample  *in1 =    (t_sample *)(w[2]);
	t_sample  *in2 =    (t_sample *)(w[3]);

	/* here comes the signalblock that will hold the output signal */
	t_sample  *out1 =    (t_sample *)(w[4]);
	t_sample  *out2 =    (t_sample *)(w[5]);

	float fs = 48000; //Sampling Rate TODO: get actual sample rate
	float wc = 2 * M_PI * (x->f_centerFrequency / fs);
	float mu = pow(10, (float)  (x->f_gain));
	float kq = 4 / (1 + mu) * tan( wc / (2 * x->f_peakWidth));

	float Cpk = (1 + kq*mu)/(1+kq);
	float b1 = (-2*cos(wc))/(1+kq*mu);
	float b2 = (1-kq*mu)/(1+kq*mu);
	float a1 = (-2*cos(wc))/(1+kq);
	float a2 = (1-kq)/(1+kq);


	for (	int i = 0; i < (int)(w[6]) ; i++ ) {
		out1[i] = (Cpk * in1[i]) + (b1 * last_in1[0]) + (b2 * last_in1[1]) - a1*last_out1[0] - a2*last_out1[1];
		out2[i] = (Cpk * in2[i]) + (b1 * last_in2[0]) + (b2 * last_in2[1]) - a1*last_out2[0] - a2*last_out2[1];
	}

	// TODO: figure out what the signal vectors are
	last_out1[1] = last_out1[0];
	last_out1[0] = out1[0];

	last_out2[1] = last_out2[0];
	last_out2[0] = out2[0];

	last_in1[1] = last_in1[0];
	last_in1[0] = in1[0];

	last_in2[1] = last_in2[0];
	last_in2[0] = in2[0];


	/* return a pointer to the dataspace for the next dsp-object */
	return (w+7);
}


/**
 * register a special perform-routine at the dsp-engine
 * this function gets called whenever the DSP is turned ON
 * the name of this function is registered in fofi_tilde_setup()
 */
void fofi_tilde_dsp(t_fofi_tilde *x, t_signal **sp)
{
	dsp_add(fofi_tilde_perform, 6, x,
					sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

/**
 * this is the "destructor" of the class;
 * it allows us to free dynamically allocated ressources
 */
void fofi_tilde_free(t_fofi_tilde *x)
{
	/* free any ressources associated with the given inlet */
	inlet_free(x->x_in2);
	inlet_free(x->x_in3);
	inlet_free(x->x_in4);
	inlet_free(x->x_in5);

	/* free any ressources associated with the given outlet */
	outlet_free(x->x_out1);
	outlet_free(x->x_out2);
}

/**
 * this is the "constructor" of the class
 * the argument is the initial mixing-factor
 */
void *fofi_tilde_new(t_floatarg f)
{
	t_fofi_tilde *x = (t_fofi_tilde *)pd_new(fofi_tilde_class);

	/* create a new signal-inlet */
	x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

	/* create a new passive inlet for the gain */
	x->x_in3 = floatinlet_new (&x->x_obj, &x->f_gain);
	x->x_in4 = floatinlet_new (&x->x_obj, &x->f_centerFrequency);
	x->x_in5 = floatinlet_new (&x->x_obj, &x->f_peakWidth);

	/* create a new signal-outlet */
	x->x_out1 = outlet_new(&x->x_obj, &s_signal);
	x->x_out2 = outlet_new(&x->x_obj, &s_signal);

	return (void *)x;
}


/**
 * define the function-space of the class
 * within a single-object external the name of this function is very special
 */
void fofi_tilde_setup(void) {
	fofi_tilde_class = class_new(gensym("fofi~"),
				(t_newmethod)fofi_tilde_new,
				(t_method)fofi_tilde_free,
	sizeof(t_fofi_tilde),
				CLASS_DEFAULT,
				A_DEFFLOAT, 0);

	/* whenever the audio-engine is turned on, the "fofi_tilde_dsp()"
	 * function will get called
	 */
	class_addmethod(fofi_tilde_class,
				(t_method)fofi_tilde_dsp, gensym("dsp"), 0);

	/* if no signal is connected to the first inlet, we can as well
	 * connect a number box to it and use it as "signal"
	 */
	CLASS_MAINSIGNALIN(fofi_tilde_class, t_fofi_tilde, f);
}
