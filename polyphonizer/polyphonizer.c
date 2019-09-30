/**
 * A very simple PD plugin which takes note messages (provided by a midi input
 * gen) and sets their velocity value (first inlet) in a list of fixed size (128
 * - one for each midi note) This list is then passed on as a list message -
 * whenever a midi message is received
 * We use a list of fixed size to avoid dynamic memory allocation and
 * avoid addiotional bound checks when receiving the list
 */

#include "m_pd.h"

static t_class *polyphonizer_class;
const int NUM_MIDINOTES = 128;

typedef struct _polyphonizer {
  t_object x_obj;
  t_atom notes[NUM_MIDINOTES];
  float current_vel;

  t_inlet *x_in2;
  t_outlet *x_out;

} t_polyphonizer;

void polyphonizer_note(t_polyphonizer *x, t_floatarg f) {
  if ((int)f >= NUM_MIDINOTES || (int)f < 0) {
    pd_error(x, "Invalid note number: %d", (int)f);
    return;
  }
  SETFLOAT(&x->notes[(int)f], x->current_vel);
  outlet_list(x->x_out, &s_list, NUM_MIDINOTES, (t_atom *)&x->notes);
}

void *polyphonizer_new() {
  t_polyphonizer *x = (t_polyphonizer *)pd_new(polyphonizer_class);
  x->x_out = outlet_new(&x->x_obj, &s_list);

  x->x_in2 = floatinlet_new(&x->x_obj, &x->current_vel);

  for (int i = 0; i < NUM_MIDINOTES; i++)
    SETFLOAT(&x->notes[i], 0);

  return (void *)x;
}

void polyphonizer_free(t_polyphonizer *x) {
  inlet_free(x->x_in2);
  outlet_free(x->x_out);
}

void polyphonizer_setup(void) {
  polyphonizer_class = class_new(
      gensym("polyphonizer"), (t_newmethod)polyphonizer_new,
      (t_method)polyphonizer_free, sizeof(t_polyphonizer), CLASS_DEFAULT, 0);
  class_addmethod(polyphonizer_class, (t_method)polyphonizer_note,
		  gensym("note"), A_DEFFLOAT, 0);
}
