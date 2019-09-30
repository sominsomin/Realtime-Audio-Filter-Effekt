/**
 * @file
 * @author Ben, Simon
 * @brief Polyphonic filter applied to input using midi.
 * Fofi takes a list of midi notes (provided by the polyphonizer plugin) applies
 * a bandpass filter/peaking equalizer for the corresponding frequencies.
 * Gain and bandwidth can be set to adjust the output.
 * !! Handle with care. This  will heavily distort when playing notes
 *    close to each other. A compressor for safty is provided.
 */

#include "fofi~.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "m_pd.h"

/** Maps midi note to frequency  */
const float MIDI_FREQ_MAP[128] = {8.175798915643707,  8.661957218027252,
				  9.177023997418988,  9.722718241315029,
				  10.300861153527183, 10.913382232281373,
				  11.562325709738575, 12.249857374429663,
				  12.978271799373287, 13.75,
				  14.567617547440307, 15.433853164253883,
				  16.351597831287414, 17.323914436054505,
				  18.354047994837977, 19.445436482630058,
				  20.601722307054366, 21.826764464562746,
				  23.12465141947715,  24.499714748859326,
				  25.956543598746574, 27.5,
				  29.13523509488062,  30.86770632850775,
				  32.70319566257483,  34.64782887210901,
				  36.70809598967594,  38.890872965260115,
				  41.20344461410875,  43.653528929125486,
				  46.2493028389543,   48.999429497718666,
				  51.91308719749314,  55.0,
				  58.27047018976124,  61.7354126570155,
				  65.40639132514966,  69.29565774421802,
				  73.41619197935188,  77.78174593052023,
				  82.4068892282175,   87.30705785825097,
				  92.4986056779086,   97.99885899543733,
				  103.82617439498628, 110.0,
				  116.54094037952248, 123.47082531403103,
				  130.8127826502993,  138.59131548843604,
				  146.8323839587038,  155.56349186104046,
				  164.81377845643496, 174.61411571650194,
				  184.9972113558172,  195.99771799087463,
				  207.65234878997256, 220.0,
				  233.08188075904496, 246.94165062806206,
				  261.6255653005986,  277.1826309768721,
				  293.6647679174076,  311.1269837220809,
				  329.6275569128699,  349.2282314330039,
				  369.9944227116344,  391.99543598174927,
				  415.3046975799451,  440.0,
				  466.1637615180899,  493.8833012561241,
				  523.2511306011972,  554.3652619537442,
				  587.3295358348151,  622.2539674441618,
				  659.2551138257398,  698.4564628660078,
				  739.9888454232688,  783.9908719634985,
				  830.6093951598903,  880.0,
				  932.3275230361799,  987.7666025122483,
				  1046.5022612023945, 1108.7305239074883,
				  1174.6590716696303, 1244.5079348883237,
				  1318.5102276514797, 1396.9129257320155,
				  1479.9776908465376, 1567.981743926997,
				  1661.2187903197805, 1760.0,
				  1864.6550460723597, 1975.533205024496,
				  2093.004522404789,  2217.4610478149766,
				  2349.31814333926,   2489.0158697766474,
				  2637.02045530296,   2793.825851464031,
				  2959.955381693075,  3135.9634878539946,
				  3322.437580639561,  3520.0,
				  3729.3100921447194, 3951.066410048992,
				  4186.009044809578,  4434.922095629953,
				  4698.63628667852,   4978.031739553295,
				  5274.04091060592,   5587.651702928062,
				  5919.91076338615,   6271.926975707989,
				  6644.875161279122,  7040.0,
				  7458.620184289437,  7902.132820097988,
				  8372.018089619156,  8869.844191259906,
				  9397.272573357044,  9956.06347910659,
				  10548.081821211836, 11175.303405856126,
				  11839.8215267723,   12543.853951415975};

/** Fofi Type parameter for perform function*/
enum FOFI_TYPE { PEQ = 0, BP = 1 };

float peaking_equalizer(float in, t_prev_samples *prev, float f_sampleRate,
			float f_gain, float f_centerFrequency,
			float f_peakWidth) {
  // NOTE: This is implemented according to the provided link for readability.
  // Compiler will take care of optimization
  float wc = 2 * M_PI * (f_centerFrequency / f_sampleRate);
  float mu = pow(10, (float)(f_gain));
  float kq = 4 / (1 + mu) * tan(wc / (2 * f_peakWidth));
  float Cpk = (1 + kq * mu) / (1 + kq);
  float b1 = (-2 * cos(wc)) / (1 + kq * mu);
  float b2 = (1 - kq * mu) / (1 + kq * mu);
  float a1 = (-2 * cos(wc)) / (1 + kq);
  float a2 = (1 - kq) / (1 + kq);

  float out = (Cpk * in) + (b1 * prev->in[0]) + (b2 * prev->in[1]) -
	      (a1 * prev->out[0]) - (a2 * prev->out[1]);

  prev->out[1] = prev->out[0];
  prev->out[0] = out;
  prev->in[1] = prev->in[0];
  prev->in[0] = in;
  return out;
}

float bandpass(float in, t_prev_samples *prev, float f_sampleRate, float f_gain,
	       float f_centerFrequency) {
  // NOTE: This is implemented according to the provided link for readability.
  // Compiler will take care of optimization

  float w0 = 2.0 * M_PI * f_centerFrequency / f_sampleRate;
  float s = sin(w0);
  float alpha = s / (2.0 * f_gain);
  float b0 = f_gain * alpha;
  float b1 = 0.0;
  float b2 = -s / 2.0;
  float a0 = 1.0 + alpha;
  float a1 = -2.0 * cos(w0);
  float a2 = 1.0 - alpha;
  float out = b0 / a0 * in + b1 / a0 * prev->in[0] + b2 / a0 * prev->in[1] -
	      a1 / a0 * prev->out[0] - a2 / a0 * prev->out[1];

  prev->out[1] = prev->out[0];
  prev->out[0] = out;
  prev->in[1] = prev->in[0];
  prev->in[0] = in;
  return out;
}

/******************************************************************************
 * PD Class implementation                                                    *
 ******************************************************************************/

t_int *fofi_tilde_perform(t_int *w) {
  /* the first element is a pointer to the dataspace of this object */
  t_fofi_tilde *x = (t_fofi_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int num_samples = (int)(w[4]);

  /* Make sure we don't break everything with 0 values*/
  x->f_gain = x->f_gain == 0 ? 0.001 : x->f_gain;
  x->f_peakWidth = x->f_peakWidth < 0.1 ? 0.1 : x->f_peakWidth;

  /* Note that in/out point to the same memory,
     for every loop(note) we use the previous output as input! */
  switch ((int)x->f_type) {
    /* Note that we loop in the switch cases here to avoid switching with
     * every loop. "type" is not a constant and we assume that the compiler
     * can't optimize this automatically.
     */
  case PEQ:
    for (int n = 0; n < NUM_MIDINOTES; n++) {
      if (atom_getfloat(&x->notes[n]) <= 0)
	continue;
      for (int i = 0; i < num_samples; i++)
	out[i] = peaking_equalizer(in[i], &x->prev_samples[n], x->f_sampleRate,
				   x->f_gain, MIDI_FREQ_MAP[n], x->f_peakWidth);
    }
    break;
  case BP:
    for (int n = 0; n < NUM_MIDINOTES; n++) {
      if (atom_getfloat(&x->notes[n]) <= 0)
	continue;
      for (int i = 0; i < num_samples; i++)
	out[i] = bandpass(in[i], &x->prev_samples[n], x->f_sampleRate,
			  x->f_gain, MIDI_FREQ_MAP[n]);
    }
    break;
  }

  /* return a pointer to the dataspace for the next dsp-object */
  return (w + 5);
}

void fofi_tilde_list(t_fofi_tilde *x, t_symbol *s, int argc, t_atom *argv) {
  if (argc != NUM_MIDINOTES)
    pd_error(x, "Received Invalid note list");

  for (int i = 0; i < NUM_MIDINOTES; i++)
    x->notes[i] = argv[i];
}

void fofi_tilde_dsp(t_fofi_tilde *x, t_signal **sp) {
  x->f_sampleRate = sp[0]->s_sr;
  dsp_add(fofi_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void fofi_tilde_free(t_fofi_tilde *x) {
  inlet_free(x->x_in2);
  inlet_free(x->x_in3);
  outlet_free(x->x_out);
}

void *fofi_tilde_new(t_floatarg f) {
  t_fofi_tilde *x = (t_fofi_tilde *)pd_new(fofi_tilde_class);

  x->f_type = f;
  x->x_in2 = floatinlet_new(&x->x_obj, &x->f_gain);
  x->x_in3 = floatinlet_new(&x->x_obj, &x->f_peakWidth);

  x->x_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void fofi_tilde_setup(void) {
  fofi_tilde_class = class_new(gensym("fofi~"), (t_newmethod)fofi_tilde_new,
			       (t_method)fofi_tilde_free, sizeof(t_fofi_tilde),
			       CLASS_DEFAULT, A_DEFFLOAT, 0);

  class_addmethod(fofi_tilde_class, (t_method)fofi_tilde_dsp, gensym("dsp"),
		  A_CANT, 0);

  class_addlist(fofi_tilde_class, (t_method)fofi_tilde_list);

  CLASS_MAINSIGNALIN(fofi_tilde_class, t_fofi_tilde, f);
}
