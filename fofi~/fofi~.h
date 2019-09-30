/**
 * @file
 * @author Simon Zoller, Benedikt Wieder
 * @brief Polyphonic filter applied to input using midi.
 *
 */

#pragma once
#include "m_pd.h"

/** For readability  */
const int NUM_MIDINOTES = 128;

/**
 * @brief Struct to store last two previous input/output samples
 *
 * In order to apply a filter we need to store the two previous input and output
 * samples. This struct encapsulates both since we need to store these
 * for each note we play individually
 */
typedef struct prev_samples {
  float out[2]; /**< Last two output samples */
  float in[2];  /**< Last two input samples */
} t_prev_samples;

/**
  @brief Implementation of a peaking equalizer based on a second-order infinite
  impulse response (IIR) filter

  @see
  https://www.dummies.com/education/science/science-engineering/how-to-characterize-the-peaking-filter-for-an-audio-graphic-equalizer/

  @param in The current sample to be processed
  @param prev Previous two input/output samples (see @ref prev_samples)
  @param f_centerfrequency Center frequency for peak
  @param f_gain Gain for center frequency (ensure > 0)
  @param f_peakWidth Filter bandwidth (inversly proportional - larger values
  imply a smaller bandwidth) (ensure > 0)
  @param f_sampleRate
  @return processed output sample
 */
float peaking_equalizer(float in, t_prev_samples *prev, float f_sampleRate,
			float f_gain, float f_centerFrequency,
			float f_peakWidth);
/**
  @brief Implementation of a bandpass equalizer

  Based on a second-order infinite impulse response (IIR) filter.
  Implemented with constant skirt gain and peak gain = Q
  @see https://www.w3.org/2011/audio/audio-eq-cookbook.html

  @param in The current sample to be processed
  @param prev Previous two input/output samples (see @ref prev_samples)
  @param f_centerfrequency Center frequency for peak
  @param f_gain Gain for center frequency (ensure > 0)
  @param f_sampleRate
  @return processed output sample
 */
float bandpass(float in, t_prev_samples *prev, float f_sampleRate, float f_gain,
	       float f_centerFrequency);


static t_class *fofi_tilde_class;
/**
 * This is the dataspace of our new object
 * the first element is the mandatory "t_object"
 * "f" is a dummy and is used to be able to send floats AS signals.
 */
typedef struct _fofi_tilde {
  t_object x_obj;

  t_atom notes[NUM_MIDINOTES];                /**< Stores incoming midi notes */
  t_prev_samples prev_samples[NUM_MIDINOTES]; /**< Stores previous samples for
						 each midi note */
  t_sample f;
  t_sample f_sampleRate; /**< PD sample rate */
  t_sample f_gain;       /**< Gain parameter for peaking EQ/bandpass  */
  t_sample f_peakWidth;  /**< Peak width parameter for peaking EQ  */
  t_sample f_type;       /**< Switch between peaking EQ/Bandpass  */

  t_inlet *x_in2; /**< Gain inlet  */
  t_inlet *x_in3; /**< Peak width inlet */

  t_outlet *x_out;

} t_fofi_tilde;

/**
 * Tthis perform-routine is called for each signal block
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
t_int *fofi_tilde_perform(t_int *w);

/**
 *@brief method for list messages (Must be list with 128 integer "atom" values =
 *Midi notes)
 */
void fofi_tilde_list(t_fofi_tilde *x, t_symbol *s, int argc, t_atom *argv);

/**
 * Registers perform-routine at the dsp-engine
 * this function gets called whenever the DSP is turned ON
 * the name of this function is registered in fofi_tilde_setup()
 */
void fofi_tilde_dsp(t_fofi_tilde *x, t_signal **sp);

/**
 * @brief PD class  "destructor"
 *
 * frees dynamically allocated ressources
 */
void fofi_tilde_free(t_fofi_tilde *x);

/**
 * @brief PD class "constructor"
 * @param f selects FOFI_TYPE (0=PEQ, 1=BP)
 */
void *fofi_tilde_new(t_floatarg f);

/**
 * Define the function-space of the class
 * within a single-object external the name of this function is very special
 */
void fofi_tilde_setup(void);
